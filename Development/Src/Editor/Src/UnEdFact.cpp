/*=============================================================================
	UnEdFact.cpp: Editor class factories.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "Factories.h"
#include "UnScrPrecom.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineParticleClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineInterpolationClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"
#include "LensFlare.h"
#include "EditorImageClasses.h"
#include "UnColladaImporter.h"
#include "UnTerrain.h"
#include "ScopedTransaction.h"
#include "BusyCursor.h"
#include "..\..\UnrealEd\Inc\DlgColladaResource.h"
#include "SpeedTree.h"
#include "EngineMaterialClasses.h"
#include "BSPOps.h"

// Needed for DDS support.
#pragma pack(push,8)
#include <ddraw.h>
#pragma pack(pop)

/*------------------------------------------------------------------------------
	UTextureCubeFactoryNew implementation.
------------------------------------------------------------------------------*/

void UTextureCubeFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureCubeFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UTextureCube::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Cubemap");
}
UObject* UTextureCubeFactoryNew::FactoryCreateNew( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	return StaticConstructObject(InClass,InParent,InName,Flags);
}
IMPLEMENT_CLASS(UTextureCubeFactoryNew);

/*------------------------------------------------------------------------------
	UMaterialInstanceConstantFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UMaterialInstanceConstantFactoryNew::StaticConstructor
//

void UMaterialInstanceConstantFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UMaterialInstanceConstantFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UMaterialInstanceConstant::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("MaterialInstanceConstant");	
}
//
//	UMaterialInstanceConstantFactoryNew::FactoryCreateNew
//

UObject* UMaterialInstanceConstantFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UMaterialInstanceConstantFactoryNew);


/*------------------------------------------------------------------------------
    UMaterialInstanceTimeVaryingFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UMaterialInstanceConstantFactoryNew::StaticConstructor
//


void UMaterialInstanceTimeVaryingFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UMaterialInstanceTimeVaryingFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UMaterialInstanceTimeVarying::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("MaterialInstanceTimeVarying(WIP)");	
}
//
//	UMaterialInstanceTimeVaryingFactoryNew::FactoryCreateNew
//

UObject* UMaterialInstanceTimeVaryingFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UMaterialInstanceTimeVaryingFactoryNew);

/*------------------------------------------------------------------------------
	UMaterialFactory implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UMaterialFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UMaterial::StaticClass();
	new(Formats)FString(TEXT("t3d;Material"));
	bCreateNew = 0;
	bText = 1;
	bEditorImport = TRUE;
}

UObject* UMaterialFactory::FactoryCreateText( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn )
{
	return NULL;
}

/**
 *	Initializes the given Material from the MaterialData text block supplied.
 *	The MaterialData text block is assumed to have been generated by the UMaterialExporterT3D.
 *
 */
UBOOL UMaterialFactory::InitializeFromT3DMaterialDataText(UMaterial* InMaterial, const FString& Text, FFeedbackContext* Warn )
{
	const TCHAR* Buffer = *Text;

	UScriptStruct* ColorMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("ColorMaterialInput"));
	check(ColorMaterialInputStruct);
	UScriptStruct* ScalarMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("ScalarMaterialInput"));
	check(ScalarMaterialInputStruct);
	UScriptStruct* VectorMaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("VectorMaterialInput"));
	check(VectorMaterialInputStruct);
	UScriptStruct* Vector2MaterialInputStruct = FindField<UScriptStruct>(UMaterial::StaticClass(), TEXT("Vector2MaterialInput"));
	check(Vector2MaterialInputStruct);

	TMap<UMaterialExpression*,FString> MaterialExpressionPropData;

	UBOOL bIsMissingTextures = FALSE;
	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		FString ParsedText;
		
		if (Parse(Str, TEXT("VERSION="), ParsedText))
		{
			///** Versioning system... */
			//Ar.Logf(TEXT("%sVersion=%d.%d") LINE_TERMINATOR, appSpc(TextIndent), VersionMax, VersionMin);
		}
		else
		if (GetBEGIN(&Str, TEXT("EXPRESSIONOBJECTLIST")))
		{
			while(ParseLine(&Buffer,StrLine))
			{
				Str = *StrLine;

				if (GetBEGIN(&Str, TEXT("OBJECT")))
				{
					// If is assumed that this will be a material expression!
					UClass* ObjectClass;
					if (ParseObject<UClass>(Str, TEXT("CLASS="), ObjectClass, ANY_PACKAGE))
					{
						// Get the name of the object.
						FName NewObjectName(NAME_None);
						Parse(Str, TEXT("NAME="), NewObjectName);

						// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
						FString ArchetypeName;
						UPackage* Archetype = NULL;
						if (Parse(Str, TEXT("Archetype="), ArchetypeName))
						{
							// if given a name, break it up along the ' so separate the class from the name
							TArray<FString> Refs;
							ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
							// find the class
							UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
							if (ArchetypeClass)
							{
								if (ArchetypeClass->IsChildOf(ObjectClass))
								{
									// if we had the class, find the archetype
									// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
									Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
								}
								else
								{
									Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
										Str, *Refs(0));
								}
							}
						}

						// Get property text.
						FString PropText, StrLine;
						while
						(	GetEND( &Buffer, TEXT("OBJECT") )==0
						&&	ParseLine( &Buffer, StrLine ) )
						{
							PropText += *StrLine;
							PropText += TEXT("\r\n");
						}

						UMaterialExpression* NewMatExp = Cast<UMaterialExpression>(StaticConstructObject(ObjectClass, InMaterial, NewObjectName, 0, Archetype));
						check(NewMatExp);
						MaterialExpressionPropData.Set(NewMatExp, PropText);
					}
				}
				else
				if (GetEND(&Str, TEXT("EXPRESSIONOBJECTLIST")))
				{
					break;
				}
			}
		}
		else
		if (Parse(Str, TEXT("PHYSMATERIAL="), ParsedText))
		{
			FString OutClass;
			FString OutName;
			ParseObjectPropertyName(ParsedText, OutClass, OutName);

			if (OutName.Len() > 0)
			{
				// Find the object...
				InMaterial->PhysMaterial = Cast<UPhysicalMaterial>(UObject::StaticFindObject(UPhysicalMaterial::StaticClass(), ANY_PACKAGE, *OutName));
				if (InMaterial->PhysMaterial == NULL)
				{
					warnf(TEXT("Failed to find PhysicalMaterial: %s"), *OutName);
				}
			}
		}
		else
		if (Parse(Str, TEXT("DIFFUSECOLOR="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ColorMaterialInputStruct, Str, (BYTE*)&(InMaterial->DiffuseColor), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("DIFFUSEPOWER="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ScalarMaterialInputStruct, Str, (BYTE*)&(InMaterial->DiffusePower), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("SPECULARCOLOR="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ColorMaterialInputStruct, Str, (BYTE*)&(InMaterial->SpecularColor), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("SPECULARPOWER="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ScalarMaterialInputStruct, Str, (BYTE*)&(InMaterial->SpecularPower), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("NORMAL="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(VectorMaterialInputStruct, Str, (BYTE*)&(InMaterial->Normal), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("EMISSIVECOLOR="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ColorMaterialInputStruct, Str, (BYTE*)&(InMaterial->EmissiveColor), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("OPACITY="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ScalarMaterialInputStruct, Str, (BYTE*)&(InMaterial->Opacity), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("OPACITYMASK="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ScalarMaterialInputStruct, Str, (BYTE*)&(InMaterial->OpacityMask), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("OPACITYMASKCLIPVALUE="), ParsedText))
		{
			InMaterial->OpacityMaskClipValue = appAtof(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("DISTORTION="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(Vector2MaterialInputStruct, Str, (BYTE*)&(InMaterial->Distortion), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("BLENDMODE="), ParsedText))
		{
			InMaterial->BlendMode = UMaterial::GetBlendModeFromString(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("LIGHTINGMODEL="), ParsedText))
		{
			InMaterial->LightingModel = UMaterial::GetMaterialLightingModelFromString(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("CUSTOMLIGHTING="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ColorMaterialInputStruct, Str, (BYTE*)&(InMaterial->CustomLighting), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("TWOSIDEDLIGHTINGMASK="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ScalarMaterialInputStruct, Str, (BYTE*)&(InMaterial->TwoSidedLightingMask), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("TWOSIDEDLIGHTINGCOLOR="), ParsedText))
		{
			if (ParsedText.Len() > 0)
			{
				INT Index = StrLine.InStr(TEXT("="));
				Str += Index + 1;
				UStructProperty_ImportText(ColorMaterialInputStruct, Str, (BYTE*)&(InMaterial->TwoSidedLightingColor), 0, InMaterial, NULL);
			}
		}
		else
		if (Parse(Str, TEXT("TwoSided="), ParsedText))
		{
			InMaterial->TwoSided = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("DisableDepthTest="), ParsedText))
		{
			InMaterial->bDisableDepthTest = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedAsLightFunction="), ParsedText))
		{
			InMaterial->bUsedAsLightFunction = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithFogVolumes="), ParsedText))
		{
			InMaterial->bUsedWithFogVolumes = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedAsSpecialEngineMaterial="), ParsedText))
		{
			InMaterial->bUsedAsSpecialEngineMaterial = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithSkeletalMesh="), ParsedText))
		{
			InMaterial->bUsedWithSkeletalMesh = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithFracturedMeshes="), ParsedText))
		{
			InMaterial->bUsedWithFracturedMeshes = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithParticleSprites="), ParsedText))
		{
			InMaterial->bUsedWithParticleSprites = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithBeamTrails="), ParsedText))
		{
			InMaterial->bUsedWithBeamTrails = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithParticleSubUV="), ParsedText))
		{
			InMaterial->bUsedWithParticleSubUV = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithFoliage="), ParsedText))
		{
			InMaterial->bUsedWithFoliage = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithSpeedTree="), ParsedText))
		{
			InMaterial->bUsedWithSpeedTree = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithStaticLighting="), ParsedText))
		{
			InMaterial->bUsedWithStaticLighting = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithLensFlare="), ParsedText))
		{
			InMaterial->bUsedWithLensFlare = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithGammaCorrection="), ParsedText))
		{
			InMaterial->bUsedWithGammaCorrection = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithInstancedMeshParticles="), ParsedText))
		{
			InMaterial->bUsedWithInstancedMeshParticles = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithFluidSurfaces="), ParsedText))
		{
			InMaterial->bUsedWithFluidSurfaces = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithDecals="), ParsedText))
		{
			InMaterial->bUsedWithDecals = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bUsedWithMaterialEffect="), ParsedText))
		{
			InMaterial->bUsedWithMaterialEffect = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("Wireframe="), ParsedText))
		{
			InMaterial->Wireframe = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bIsFallbackMaterial="), ParsedText))
		{
			InMaterial->bIsFallbackMaterial = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("FallbackMaterial="), ParsedText))
		{
			FString OutClass;
			FString OutName;
			ParseObjectPropertyName(ParsedText, OutClass, OutName);

			if (OutName.Len() > 0)
			{
				// Find the object...
				InMaterial->FallbackMaterial = Cast<UMaterial>(UObject::StaticFindObject(UMaterial::StaticClass(), ANY_PACKAGE, *OutName));
				if (InMaterial->FallbackMaterial == NULL)
				{
					warnf(TEXT("Failed to find FallbackMaterial: %s"), *OutName);
				}
			}
		}
		else
		if (Parse(Str, TEXT("EditorX="), ParsedText))
		{
			InMaterial->EditorX = appAtoi(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("EditorY="), ParsedText))
		{
			InMaterial->EditorY = appAtoi(*ParsedText);
		}
		else
		if (GetBEGIN(&Str, TEXT("EXPRESSIONLIST")))
		{
			while( ParseLine(&Buffer,StrLine) )
			{
				const TCHAR* Str = *StrLine;

				if (Parse(Str, TEXT("EXPRESSION="), ParsedText))
				{
					if (ParsedText.Len() > 0)
					{
						// Find the object...
						UMaterialExpression* MatExp = FindObject<UMaterialExpression>(InMaterial, *ParsedText);
						if (MatExp)
						{
							InMaterial->Expressions.AddItem(MatExp);
						}
						else
						{
							warnf(TEXT("Failed to find expression %s (for material %s)"), *ParsedText, *(InMaterial->GetPathName()));
						}
					}
				}
				else
				if (GetEND(&Str, TEXT("EXPRESSIONLIST")))
				{
					break;
				}
			}
		}
		else
		if (GetBEGIN(&Str, TEXT("EXPRESSIONCOMMENTLIST")))
		{
			while( ParseLine(&Buffer,StrLine) )
			{
				const TCHAR* Str = *StrLine;

				if (Parse(Str, TEXT("COMMENT="), ParsedText))
				{
					if (ParsedText.Len() > 0)
					{
						// Find the object...
						UMaterialExpressionComment* MatExp = FindObject<UMaterialExpressionComment>(InMaterial, *ParsedText);
						if (MatExp)
						{
							InMaterial->EditorComments.AddItem(MatExp);
						}
						else
						{
							warnf(TEXT("Failed to find comment expression %s (for material %s)"), *ParsedText, *(InMaterial->GetPathName()));
						}
					}
				}
				else
				if (GetEND(&Str, TEXT("EXPRESSIONCOMMENTLIST")))
				{
					break;
				}
			}
		}
		else
		if (Parse(Str, TEXT("bUsesDistortion="), ParsedText))
		{
			InMaterial->bUsesDistortion = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bIsMasked="), ParsedText))
		{
			InMaterial->bIsMasked = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bSupportsSinglePassSHLight="), ParsedText))
		{
			InMaterial->bSupportsSinglePassSHLight = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("bIsPreviewMaterial="), ParsedText))
		{
			InMaterial->bIsPreviewMaterial = BOOL_STRING_IS_TRUE(ParsedText);
		}
		// Ignoring Compounds as they aren't supported (yet)...
		// var editoronly array<MaterialExpressionCompound> EditorCompounds;
		else
		if (GetBEGIN(&Str, TEXT("REFERENCEDTEXTURELIST")))
		{
			while( ParseLine(&Buffer,StrLine) )
			{
				const TCHAR* Str = *StrLine;

				if (Parse(Str, TEXT("REFERENCEDTEXTURE="), ParsedText))
				{
					if (ParsedText.Len() > 0)
					{
						if (ParsedText.InStr(TEXT("LT_Floors.BSP.T_LT_Floors")) != -1)
						{
							int dummy = 0;
						}
						// Find the object...
//						UTexture* Texture = FindObject<UTexture>(InMaterial, *ParsedText);
						UTexture* Texture = FindObject<UTexture>(ANY_PACKAGE, *ParsedText);
						if (Texture == NULL)
						{
							FString ClassName, ObjectName;
							if (ParseObjectPropertyName(ParsedText, ClassName, ObjectName))
							{
								Texture = FindObject<UTexture>(ANY_PACKAGE, *ObjectName);
							}
						}
						if (Texture)
						{
							InMaterial->ReferencedTextures.AddItem(Texture);
						}
						else
						{
							warnf(TEXT("Failed to find texture %s (for material %s)"), *ParsedText, *(InMaterial->GetPathName()));
						}
					}
				}
			}
		}
		else
		if (Parse(Str, TEXT("CONTENTTAGS="), ParsedText))
		{
			INT Index = StrLine.InStr(TEXT("="));
			Str += Index + 1;
			UArrayProperty* TheTagsProp = FindField<UArrayProperty>(UMaterial::StaticClass(), TEXT("ContentTags"));
			if (TheTagsProp)
			{
				TheTagsProp->ImportText(Str, (BYTE*)&(InMaterial->ContentTags), 0, InMaterial);
			}
		}
		else
		if (GetEND(&Str, TEXT("MATERIALDATA")))
		{
			break;
		}
	}

	// For each material expression, we need to fix it up...
	for (TObjectIterator<UMaterialExpression> MatExpIt; MatExpIt; ++MatExpIt)
	{
		UMaterialExpression* LoadMatExp = *MatExpIt;
		FString* PropText = MaterialExpressionPropData.Find(LoadMatExp);
		if (PropText)
		{
			LoadMatExp->PreEditChange(NULL);
			ImportObjectProperties((BYTE*)LoadMatExp, **PropText, LoadMatExp->GetClass(), LoadMatExp, LoadMatExp, Warn, 0 );
			UMaterialExpressionTextureSample* TextureSampleExp = Cast<UMaterialExpressionTextureSample>(LoadMatExp);
			if (TextureSampleExp)
			{
				if (TextureSampleExp->Texture == NULL)
				{
					int dummy = 0;
				}
				UMaterialExpressionTextureSampleParameterCube* CubeSampleExp = Cast<UMaterialExpressionTextureSampleParameterCube>(LoadMatExp);
				if (CubeSampleExp)
				{
					if (CubeSampleExp->Texture == NULL)
					{
						CubeSampleExp->Texture = FindObject<UTexture>(ANY_PACKAGE, TEXT("EngineResources.DefaultTextureCube"));
					}
				}
				else
				if (TextureSampleExp->Texture == NULL)
				{
					// All the others take Texture2D (or some derivative)...
					TextureSampleExp->Texture = FindObject<UTexture>(ANY_PACKAGE, TEXT("EngineResources.DefaultTexture"));
				}
			}
			// Let the actor deal with having been imported, if desired.
			LoadMatExp->PostEditImport();
			// Notify actor its properties have changed.
			LoadMatExp->PostEditChange(NULL);
		}
	}

	return TRUE;
}

IMPLEMENT_CLASS(UMaterialFactory);

/*------------------------------------------------------------------------------
	UMaterialFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UMaterialFactoryNew::StaticConstructor
//

void UMaterialFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UMaterialFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UMaterial::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Material");
}
//
//	UMaterialFactoryNew::FactoryCreateNew
//

UObject* UMaterialFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UMaterialFactoryNew);

/*------------------------------------------------------------------------------
	UDecalMaterialFactoryNew implementation.
------------------------------------------------------------------------------*/

void UDecalMaterialFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UDecalMaterialFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UDecalMaterial::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Decal Material");
}
UObject* UDecalMaterialFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UDecalMaterialFactoryNew);

/*------------------------------------------------------------------------------
	UClassFactoryUC implementation.
------------------------------------------------------------------------------*/

/**
 * Returns the number of braces in the string specified; when an opening brace is encountered, the count is incremented; when a closing
 * brace is encountered, the count is decremented.
 */
static INT GetLineBraceCount( const TCHAR* Str )
{
	check(Str);

	INT Result = 0;
	while ( *Str )
	{
		if ( *Str == TEXT('{') )
		{
			Result++;
		}
		else if ( *Str == TEXT('}') )
		{
			Result--;
		}

		Str++;
	}

	return Result;
}

// Directory will be stored relative to the appGameDir()
const TCHAR* const UClassFactoryUC::ProcessedFileDirectory = TEXT("PreProcessedFiles/");
const TCHAR* const UClassFactoryUC::ProcessedFileExtension = TEXT(".UC");
const TCHAR* const UClassFactoryUC::ExportPostProcessedParameter = TEXT("intermediate");
const TCHAR* const UClassFactoryUC::SuppressPreprocessorParameter = TEXT("nopreprocess");

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UClassFactoryUC::InitializeIntrinsicPropertyValues()
{
	new(Formats)FString(TEXT("uc;Unreal class definitions"));
	SupportedClass = UClass::StaticClass();
	bCreateNew = FALSE;
	bText = TRUE;
}
UClassFactoryUC::UClassFactoryUC()
{
}
UObject* UClassFactoryUC::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
	// preprocessor not quite ready yet - define syntax will be changing
	FString ProcessedBuffer;
	FString sourceFileName = Name.ToString();
	sourceFileName = sourceFileName + TEXT(".") + Type;

	// this must be declared outside the try block, since it's the ContextObject for the output device.
	// If it's declared inside the try block, when an exception is thrown it will go out of scope when we
	// jump to the catch block
	FMacroProcessingFilter Filter(*InParent->GetName(), sourceFileName, Warn);
	try
	{
		// if there are no macro invite characters in the file, no need to run it through the preprocessor
		if ( appStrchr(Buffer, FMacroProcessingFilter::CALL_MACRO_CHAR) != NULL &&
			!ParseParam(appCmdLine(), SuppressPreprocessorParameter) )
		{
			// uncomment these two lines to have the input buffer stripped of comments during the preprocessing phase
	//		FCommentStrippingFilter CommentStripper(sourceFileName);
	//		FSequencedTextFilter Filter(CommentStripper, Macro, sourceFileName);
			Filter.Process(Buffer, BufferEnd, ProcessedBuffer);

			// check if we want to export the post-processed text
			if ( ParseParam(appCmdLine(), ExportPostProcessedParameter) )
			{
				FString iFileName = Name.ToString();
				FString iDirName = appGameDir() * ProcessedFileDirectory;
				GFileManager->MakeDirectory(*iDirName); // create the base directory if it doesn't already exist
				iDirName += InParent->GetName(); // append the name of the package
				GFileManager->MakeDirectory(*iDirName);
				iFileName += ProcessedFileExtension;
				iFileName = iDirName + TEXT("/") + iFileName;
				appSaveStringToFile(ProcessedBuffer, *iFileName);
			}

			const TCHAR* Ptr = *ProcessedBuffer; // have to make a copy of the pointer at the beginning of the FString
			Buffer = Ptr;
			BufferEnd = Buffer + ProcessedBuffer.Len();
		}

		const TCHAR* InBuffer=Buffer;
		FString StrLine, ClassName, BaseClassName;

		// Validate format.
		if( Class != UClass::StaticClass() )
		{
			Warn->Logf( TEXT("Can only import classes"), Type );
			return NULL;
		}
		if( appStricmp(Type,TEXT("UC"))!=0 )
		{
			Warn->Logf( TEXT("Can't import classes from files of type '%s'"), Type );
			return NULL;
		}

		// Import the script text.
		TArray<FName> DependentOn;
// never ship with class-redirectors enabled
#if !SHIPPING_PC_GAME
		TArray<FName> RedirectorNames;
#endif
		FStringOutputDevice ScriptText, DefaultPropText, CppText;
		INT CurrentLine = 0;

		// The layer of multi-line comment we are in.
		INT CommentDim = 0;

		// is the parsed class name an interface?
		UBOOL bIsInterface = FALSE;

		while( ParseLine(&Buffer,StrLine,1) )
		{
			CurrentLine++;
			const TCHAR* Str = *StrLine, *Temp;
			UBOOL bProcess = CommentDim <= 0;	// for skipping nested multi-line comments
			INT BraceCount=0;

			if( bProcess && ParseCommand(&Str,TEXT("cpptext")) )
			{
				ScriptText.Logf( TEXT("// (cpptext)\r\n// (cpptext)\r\n")  );
				ParseLine(&Buffer,StrLine,1);
				Str = *StrLine;

				CurrentLine++;

				//@fixme - line count messed up by this call (what if there are two empty lines before the opening brace?)
				ParseNext( &Str );
				if( *Str!='{' )
				{
					appThrowf( LocalizeSecure(LocalizeUnrealEd("Error_MissingLBraceAfterCpptext"), *ClassName) );
				}
				else
				{
					BraceCount += GetLineBraceCount(Str);
				}

				// Get cpptext.
				while( ParseLine(&Buffer,StrLine,1) )
				{
					CurrentLine++;
					ScriptText.Logf( TEXT("// (cpptext)\r\n")  );
					Str = *StrLine;
					BraceCount += GetLineBraceCount(Str);
					if ( BraceCount == 0 )
					{
						break;
					}

					CppText.Logf( TEXT("%s\r\n"), *StrLine );
				}
			}
			else if( bProcess && ParseCommand(&Str,TEXT("defaultproperties")) )
			{
				// Get default properties text.
				while( ParseLine(&Buffer,StrLine,1) )
				{
					CurrentLine++;
					BraceCount += GetLineBraceCount(*StrLine);

					if ( StrLine.InStr(TEXT("{")) != INDEX_NONE )
					{
						DefaultPropText.Logf(TEXT("linenumber=%i\r\n"), CurrentLine);
						break;
					}
				}

				while( ParseLine(&Buffer,StrLine,1) )
				{
					CurrentLine++;
					bProcess = CommentDim <= 0;
					INT Pos, EndPos, StrBegin, StrEnd;
					Pos = EndPos = StrBegin = StrEnd = INDEX_NONE;

					UBOOL bEscaped = FALSE;
					for ( INT CharPos = 0; CharPos < StrLine.Len(); CharPos++ )
					{
						if ( bEscaped )
						{
							bEscaped = FALSE;
						}
						else if ( StrLine[CharPos] == TEXT('\\') )
						{
							bEscaped = TRUE;
						}
						else if ( StrLine[CharPos] == TEXT('\"') )
						{
							if ( StrBegin == INDEX_NONE )
							{
								StrBegin = CharPos;
							}
							else
							{
								StrEnd = CharPos;
								break;
							}
						}
					}

					// Stub out the comments, ignoring anything inside literal strings.
					Pos = StrLine.InStr(TEXT("//"));
					if ( Pos>=0 )
					{
						if (StrBegin == INDEX_NONE || Pos < StrBegin || Pos > StrEnd)
							StrLine = StrLine.Left( Pos );

						if (StrLine == TEXT(""))
						{
							DefaultPropText.Log(TEXT("\r\n"));
							continue;
						}
					}

					// look for a /* ... */ block, ignoring anything inside literal strings
					Pos = StrLine.InStr(TEXT("/*"));
					EndPos = StrLine.InStr(TEXT("*/"));
					if ( Pos >=0 )
					{
						if (StrBegin == INDEX_NONE || Pos < StrBegin || Pos > StrEnd)
						{
							if (EndPos != INDEX_NONE && (EndPos < StrBegin || EndPos > StrEnd))
							{
								StrLine = StrLine.Left(Pos) + StrLine.Mid(EndPos + 2);
								EndPos = INDEX_NONE;
							}
							else 
							{
								// either no closing token or the closing token is inside a literal string (which means it doesn't matter)
								StrLine = StrLine.Left( Pos );
								CommentDim++;
							}
						}

						bProcess = CommentDim <= 1;
					}

					if( EndPos>=0 )
					{
						// if the closing token is not inside a string
						if (StrBegin == INDEX_NONE || EndPos < StrBegin || EndPos > StrEnd)
						{
							StrLine = StrLine.Mid( EndPos+2 );
							CommentDim--;
						}

						bProcess = CommentDim <= 0;
					}

					Str = *StrLine;
					ParseNext( &Str );

					BraceCount += GetLineBraceCount(Str);
					if( *Str=='}' && bProcess && BraceCount == 0 )
					{
						break;
					}

					if ( !bProcess || StrLine == TEXT(""))
					{
						DefaultPropText.Log(TEXT("\r\n"));
						continue;
					}
					DefaultPropText.Logf( TEXT("%s\r\n"), *StrLine );
				}
			}
			else
			{
				// the script preprocessor will emit #linenumber tokens when necessary, so check for those
				if ( ParseCommand(&Str,TEXT("#linenumber")) )
				{
					FString LineNumberText;

					// found one, parse the number and reset our CurrentLine (used for logging defaultproperties warnings)
					ParseToken(Str, LineNumberText, FALSE);
					CurrentLine = appAtoi(*LineNumberText);
				}

				// Get script text.
				ScriptText.Logf( TEXT("%s\r\n"), *StrLine );

				INT Pos = INDEX_NONE, EndPos = INDEX_NONE, StrBegin = INDEX_NONE, StrEnd = INDEX_NONE;
				
				UBOOL bEscaped = FALSE;
				for ( INT CharPos = 0; CharPos < StrLine.Len(); CharPos++ )
				{
					if ( bEscaped )
					{
						bEscaped = FALSE;
					}
					else if ( StrLine[CharPos] == TEXT('\\') )
					{
						bEscaped = TRUE;
					}
					else if ( StrLine[CharPos] == TEXT('\"') )
					{
						if ( StrBegin == INDEX_NONE )
						{
							StrBegin = CharPos;
						}
						else
						{
							StrEnd = CharPos;
							break;
						}
					}
				}

				// Stub out the comments, ignoring anything inside literal strings.
				Pos = StrLine.InStr(TEXT("//"));
				if( Pos>=0 )
				{
					if (StrBegin == INDEX_NONE || Pos < StrBegin || Pos > StrEnd)
						StrLine = StrLine.Left( Pos );

					if (StrLine == TEXT(""))
						continue;
				}

				// look for a / * ... * / block, ignoring anything inside literal strings
				Pos = StrLine.InStr(TEXT("/*"));
				EndPos = StrLine.InStr(TEXT("*/"));
				if ( Pos >=0 )
				{
					if (StrBegin == INDEX_NONE || Pos < StrBegin || Pos > StrEnd)
					{
						if (EndPos != INDEX_NONE && (EndPos < StrBegin || EndPos > StrEnd))
						{
							StrLine = StrLine.Left(Pos) + StrLine.Mid(EndPos + 2);
							EndPos = INDEX_NONE;
						}
						else 
						{
							StrLine = StrLine.Left( Pos );
							CommentDim++;
						}
					}
					bProcess = CommentDim <= 1;
				}
				if( EndPos>=0 )
				{
					if (StrBegin == INDEX_NONE || EndPos < StrBegin || EndPos > StrEnd)
					{
						StrLine = StrLine.Mid( EndPos+2 );
						CommentDim--;
					}

					bProcess = CommentDim <= 0;
				}

				if (!bProcess || StrLine == TEXT(""))
					continue;

				Str = *StrLine;

				// Get class or interface name
				if( ClassName == TEXT("") )
				{
					if( (Temp=appStrfind(Str, TEXT("class"))) != 0 )
					{
						Temp += appStrlen(TEXT("class")) + 1; // plus at least one delimitor
						ParseToken(Temp, ClassName, 0);
					}
					else if( (Temp=appStrfind(Str, TEXT("interface"))) != 0 )
					{
						bIsInterface = TRUE;

						Temp += appStrlen(TEXT("interface")) + 1; // plus at least one delimiter
						ParseToken(Temp, ClassName, 0); // space delimited

						// strip off the trailing ';' characters
						while( ClassName.Right(1) == TEXT(";") )
						{
							ClassName = ClassName.LeftChop(1);
						}
					}
				}

				if
				(	BaseClassName==TEXT("")
				&&	ClassName != TEXT("Object")
				&&	ClassName != TEXT("Interface")
				&&	(Temp=appStrfind(Str, TEXT("extends")))!=0 )
				{
					Temp+=7;
					ParseToken( Temp, BaseClassName, 0 );
					while( BaseClassName.Right(1)==TEXT(";") )
						BaseClassName = BaseClassName.LeftChop( 1 );
				}

				// check for classes which this class depends on and add them to the DependentOn List
				if ( appStrfind(Str, TEXT("DependsOn")) != NULL )
				{
					const TCHAR* PreviousBuffer = Buffer - StrLine.Len() - 2;	// add 2 for the CRLF
					// PreviousBuffer will only be greater than Buffer if the class names were spanned across more than one line
					if ( ParseDependentClassGroup( PreviousBuffer, TEXT("DependsOn"), DependentOn ) && PreviousBuffer > Buffer )
					{
						// now copy the text that was parsed into an string so that we can add it to the ScriptText buffer
						INT CharactersProcessed = PreviousBuffer - Buffer;
						FString ProcessedCharacters(CharactersProcessed, Buffer);
						ScriptText.Log(*ProcessedCharacters);

						Buffer = PreviousBuffer;
					}
				}
				// only non-interface classes can have 'implements' keyword
				else if ( !bIsInterface && appStrfind(Str, TEXT("Implements")) != NULL )
				{
					const TCHAR* PreviousBuffer = Buffer - StrLine.Len() - 2;	// add 2 for the CRLF
					// PreviousBuffer will only be greater than Buffer if the class names were spanned across more than one line
					if ( ParseDependentClassGroup(PreviousBuffer, TEXT("Implements"), DependentOn) && PreviousBuffer > Buffer )
					{
						// now move the text that was parsed into a temporary buffer so that we can feed it to the script text output archive
						// now copy the text that was parsed into an string so that we can add it to the ScriptText buffer
						INT CharactersProcessed = PreviousBuffer - Buffer;
						FString ProcessedCharacters(CharactersProcessed, Buffer);
						ScriptText.Log(*ProcessedCharacters);

						Buffer = PreviousBuffer;
					}
				}
// never ship with class-redirectors enabled
#if !SHIPPING_PC_GAME
				else if ( appStrfind(Str,TEXT("ClassRedirect")) != NULL )
				{
					const TCHAR* PreviousBuffer = Buffer - StrLine.Len() - 2;	// add 2 for the CRLF

					// PreviousBuffer will only be greater than Buffer if the class names were spanned across more than one line
					if ( ParseDependentClassGroup( PreviousBuffer, TEXT("ClassRedirect"), RedirectorNames ) && PreviousBuffer > Buffer )
					{
						// now copy the text that was parsed into an string so that we can add it to the ScriptText buffer
						INT CharactersProcessed = PreviousBuffer - Buffer;
						FString ProcessedCharacters(CharactersProcessed, Buffer);
						ScriptText.Log(*ProcessedCharacters);

						Buffer = PreviousBuffer;
					}
				}
#endif
			}
		}

		// a base interface implicitly inherits from class 'Interface', unless it is the 'Interface' class itself
		if( bIsInterface == TRUE && (BaseClassName.Len() == 0 || BaseClassName == TEXT("Object")) )
		{
			if ( ClassName != TEXT("Interface") )
			{
				BaseClassName = TEXT("Interface");
			}
			else
			{
				BaseClassName = TEXT("Object");
			}
		}

#if BATMAN
		// Disable non-useful debug message.
#else
		debugf(TEXT("Class: %s extends %s"),*ClassName,*BaseClassName);
#endif

		// Handle failure.
		if( ClassName==TEXT("") || (BaseClassName==TEXT("") && ClassName!=TEXT("Object")) )
		{
			Warn->Logf ( NAME_Error, 
					TEXT("Bad class definition '%s'/'%s'/%i/%i"), *ClassName, *BaseClassName, BufferEnd-InBuffer, appStrlen(InBuffer) );
			return NULL;
		}
		else if ( ClassName==BaseClassName )
		{
			Warn->Logf ( NAME_Error, TEXT("Class is extending itself '%s'"), *ClassName );
			return NULL;
		}
		else if( ClassName!=Name.ToString() )
		{
			Warn->Logf ( NAME_Error, TEXT("Script vs. class name mismatch (%s/%s)"), *Name.ToString(), *ClassName );
		}

// never ship with class-redirectors enabled
#if !SHIPPING_PC_GAME
		TLookupMap<UObjectRedirector*> ClassRedirectors, CDORedirectors;
		for ( INT RedirectIdx = 0; RedirectIdx < RedirectorNames.Num(); RedirectIdx++ )
		{
			FName PreviousClassName = RedirectorNames(RedirectIdx);
			if ( PreviousClassName != NAME_None )
			{
				if ( PreviousClassName == *ClassName )
				{
					Warn->Logf(TEXT("Failed to create auto-redirect for renamed class '%s': specified name is the same as the current name"), *ClassName);
				}

				FName PreviousClassDefaultObjectName = *FString::Printf(TEXT("Default__%s"), *PreviousClassName.ToString());

				UObject* ExistingClass = FindObject<UObject>( ANY_PACKAGE, *PreviousClassName.ToString() );
				UObject* ExistingCDO = FindObject<UObject>( ANY_PACKAGE, *PreviousClassDefaultObjectName.ToString() );

				UObjectRedirector* ClassRedirector = Cast<UObjectRedirector>(ExistingClass);
				UObjectRedirector* CDORedirector = Cast<UObjectRedirector>(ExistingCDO);
				if ( ClassRedirector == NULL )
				{
					if ( ExistingClass != NULL )
					{
						Warn->Logf(TEXT("Failed to create auto-redirect for renamed class '%s': existing object found using old name: %s"),
							*ClassName, *ExistingClass->GetFullName());
						break;
					}

					ClassRedirector = ConstructObject<UObjectRedirector>(UObjectRedirector::StaticClass(), InParent, PreviousClassName, RF_Standalone|RF_Public);
				}

				if ( CDORedirector == NULL )
				{
					if ( ExistingCDO != NULL )
					{
						Warn->Logf(TEXT("Failed to create auto-redirect for class default object of renamed class '%s': existing object found using old name: %s"),
							*ClassName, *ExistingCDO->GetFullName());
						break;
					}

					CDORedirector = ConstructObject<UObjectRedirector>(UObjectRedirector::StaticClass(), InParent, PreviousClassDefaultObjectName, RF_Standalone|RF_Public);
				}

				if ( ClassRedirector != NULL )
				{
					ClassRedirectors.AddItem(ClassRedirector);
				}
				else
				{
					Warn->Logf(TEXT("Failed to create auto-redirect for renamed class '%s': unknown error  :( "), *ClassName);
				}

				if ( CDORedirector != NULL )
				{
					CDORedirectors.AddItem(CDORedirector);
				}
				else
				{
					Warn->Logf(TEXT("Failed to create auto-redirect for class default object of renamed class '%s': unknown error  :( "), *ClassName);
				}
			}
			else
			{
				Warn->Logf(TEXT("Failed to create auto-redirect for renamed class '%s': Invalid value specified for previous class name!"), *ClassName);
			}
		}
#endif

		// In case the file system and the class disagree on the case of the
		// class name replace the fname with the one from the scrip class file
		// This is needed because not all source control systems respect the
		// original filename's case
		FName ClassNameReplace(*ClassName,FNAME_Replace);
		if (ClassNameReplace != Name)
		{
			Warn->Logf ( NAME_Error, TEXT("Script vs. class name mismatch (%s/%s)"), *Name.ToString(), *ClassNameReplace.ToString() );
		}

		UClass* ResultClass = FindObject<UClass>( InParent, *ClassName );

		// if we aren't generating headers, then we shouldn't set misaligned object, since it won't get cleared
#if SHIPPING_PC_GAME
		const UBOOL bSkipNativeHeaderGeneration = TRUE;
#else
		UBOOL bSkipNativeHeaderGeneration = FALSE;
		GConfig->GetBool(TEXT("Editor.EditorEngine"), TEXT("SkipNativeHeaderGeneration"), bSkipNativeHeaderGeneration, GEngineIni);
#endif

		if( ResultClass && ResultClass->HasAnyFlags(RF_Native) )
		{
			if (!bSkipNativeHeaderGeneration)
			{
				// Gracefully update an existing hardcoded class.
				debugf( NAME_Log, TEXT("Updated native class '%s'"), *ResultClass->GetFullName() );

				// assume that the property layout for this native class is going to be modified, and
				// set the RF_MisalignedObject flag to prevent classes of this type from being created
				// - when the header is generated for this class, we'll unset the flag once we verify
				// that the property layout hasn't been changed

				if ( !ResultClass->HasAnyClassFlags(CLASS_NoExport) )
				{
					if ( !bIsInterface && ResultClass != UObject::StaticClass() && !ResultClass->HasAnyFlags(RF_MisalignedObject) )
					{
						ResultClass->SetFlags(RF_MisalignedObject);

						// propagate to all children currently in memory, ignoring the object class
						for ( TObjectIterator<UClass> It; It; ++It )
						{
							if ( It->GetSuperClass() == ResultClass && !It->HasAnyClassFlags(CLASS_NoExport))
							{
								It->SetFlags(RF_MisalignedObject);
							}
						}
					}
				}
			}

			UClass* SuperClass = ResultClass->GetSuperClass();
			if ( SuperClass && SuperClass->GetName() != BaseClassName )
			{
				// the code that handles the DependsOn list in the script compiler doesn't work correctly if we manually add the Object class to a class's DependsOn list
				// if Object is also the class's parent.  The only way this can happen (since specifying a parent class in a DependsOn statement is a compiler error) is
				// in this block of code, so just handle that here rather than trying to make the script compiler handle this gracefully
				if ( BaseClassName != TEXT("Object") )
				{
					// we're changing the parent of a native class, which may result in the
					// child class being parsed before the new parent class, so add the new
					// parent class to this class's DependsOn() array to guarantee that it
					// will be parsed before this class
					DependentOn.AddUniqueItem(*BaseClassName);
				}

				// if the new parent class is an existing native class, attempt to change the parent for this class to the new class 
				UClass* NewSuperClass = FindObject<UClass>(ANY_PACKAGE, *BaseClassName);
				if ( NewSuperClass != NULL )
				{
					ResultClass->ChangeParentClass(NewSuperClass);
				}
			}
		}
		else
		{
			// Create new class.
			ResultClass = new( InParent, *ClassName, Flags )UClass( NULL );

			// add CLASS_Interface flag if the class is an interface
			// NOTE: at this pre-parsing/importing stage, we cannot know if our super class is an interface or not,
			// we leave the validation to the script compiler
			if( bIsInterface == TRUE )
			{
				ResultClass->ClassFlags |= CLASS_Interface;
			}

			// Find or forward-declare base class.
			ResultClass->SuperField = FindObject<UClass>( InParent, *BaseClassName );
			if( !ResultClass->SuperField )
			{
				//@todo ronp - do we really want to do this?  seems like it would allow you to extend from a base in a dependent package.
				ResultClass->SuperField = FindObject<UClass>( ANY_PACKAGE, *BaseClassName );
			}

			if( !ResultClass->SuperField )
			{
				// don't know its parent class yet
				ResultClass->SuperField = new( InParent, *BaseClassName )UClass( NULL );
			}
			else if ( bIsInterface == FALSE )
			{
				// if the parent is misaligned, then so are we
				ResultClass->SetFlags( ResultClass->SuperField->GetFlags() & RF_MisalignedObject );
			}

			if ( ResultClass->SuperField )
			{
				ResultClass->ClassCastFlags |= ResultClass->GetSuperClass()->ClassCastFlags;
			}
			debugf( NAME_Log, TEXT("Imported: %s"), *ResultClass->GetFullName() );
		}

		// Set class info.
		ResultClass->ScriptText      = new( ResultClass, TEXT("ScriptText"),   RF_NotForClient|RF_NotForServer )UTextBuffer( *ScriptText );
		ResultClass->DefaultPropText = DefaultPropText;
		ResultClass->DependentOn	 = DependentOn;
		if( CppText.Len() )
		{
			ResultClass->CppText     = new( ResultClass, TEXT("CppText"),      RF_NotForClient|RF_NotForServer )UTextBuffer( *CppText );
		}

// never ship with class-redirectors enabled
#if !SHIPPING_PC_GAME
		for ( INT RedirectorIdx = 0; RedirectorIdx < ClassRedirectors.Num(); RedirectorIdx++ )
		{
			UObjectRedirector* Redirector = ClassRedirectors(RedirectorIdx);
			debugf(TEXT("Assigning target on class redirector %s for %s"), *Redirector->GetPathName(), *ResultClass->GetName());
			Redirector->DestinationObject = ResultClass;
		}

		for ( INT RedirectorIdx = 0; RedirectorIdx < CDORedirectors.Num(); RedirectorIdx++ )
		{
			UObjectRedirector* Redirector = CDORedirectors(RedirectorIdx);
			if ( ResultClass->ClassDefaultObject == NULL )
			{
				ResultClass->ClassDefaultObject = Redirector;
			}
			else
			{
				Redirector->DestinationObject = ResultClass->ClassDefaultObject;
			}
		}
#endif
		return ResultClass;
	}
	catch( TCHAR* ErrorMsg )
	{
		// Catch and log any warnings
		Warn->Log( NAME_Error, ErrorMsg );
		return NULL;
	}
}

/**
 * Parses the text specified for a collection of comma-delimited class names, surrounded by parenthesis, using the specified keyword.
 *
 * @param	InputText					pointer to the text buffer to parse; will be advanced past the text that was parsed.
 * @param	GroupName				the group name to parse (i.e. DependsOn, Implements, Inherits, etc.)
 * @param	out_ParsedClassNames	receives the list of class names that were parsed.
 *
 * @return	TRUE if the group name specified was found and entries were added to the list
 */
UBOOL UClassFactoryUC::ParseDependentClassGroup( const TCHAR*& InputText, const TCHAR* const GroupName, TArray<FName>& out_ParsedClassNames )
{
	UBOOL bSuccess = FALSE;
	const TCHAR* Temp = InputText;

	// EndOfClassDeclaration is used to prevent matches in areas not part of the class declaration (i.e. the rest of the .uc file after the class declaration)
	const TCHAR* EndOfClassDeclaration = appStrfind(Temp, TEXT(";"));
	while ( (Temp=appStrfind(Temp, GroupName))!=0 && (EndOfClassDeclaration == NULL || Temp < EndOfClassDeclaration) )
	{
		// advance past the DependsOn/Implements keyword
		Temp += appStrlen(GroupName);

		// advance to the opening parenthesis
		ParseNext(&Temp);
		if ( *Temp++ != TEXT('(') )
		{
			appThrowf(TEXT("Missing opening '(' in %s list"), GroupName);
		}

		// advance to the next word in the list
		ParseNext(&Temp);
		if ( *Temp == 0 )
		{
			appThrowf(TEXT("End of file encountered while attempting to parse opening '(' in %s list"), GroupName);
		}
		else if ( *Temp == TEXT(')') )
		{
			appThrowf(TEXT("Unexpected ')' - missing class name in %s list"), GroupName);
		}

		// this is used to detect when multiple class names have been specified using something other than a comma as the delimiter
		UBOOL bParsingWord = FALSE;
		FString NextWord;
		do 
		{
			// if the next character isn't a valid character for a DependsOn/Implements class name, advance to the next valid character
			if ( appIsWhitespace(*Temp) || appIsLinebreak(*Temp) || (*Temp == TEXT('\r') && appIsLinebreak(*(Temp+1))) )
			{
				ParseNext(&Temp);

				// if this character is the closing paren., stop here
				if ( *Temp == 0 || *Temp == TEXT(')') )
				{
					break;
				}

				// otherwise, the next character must be a comma
				if ( bParsingWord && *Temp != TEXT(',') )
				{
					appThrowf(TEXT("Missing ',' or closing ')' in %s list"), GroupName);
				}
			}
			
			// if we've hit a comma, add the current word to the list
			if ( *Temp == TEXT(',') )
			{
				if ( NextWord.Len() == 0 )
				{
					appThrowf(TEXT("Unexpected ',' - missing class name in %s list"), GroupName);
				}

				out_ParsedClassNames.AddUniqueItem(*NextWord);
				NextWord = TEXT("");

				bSuccess = TRUE;
				bParsingWord = FALSE;
			}
			else
			{
				bParsingWord = TRUE;
				NextWord += *Temp;
			}

			Temp++;
		} while( *Temp != 0 && *Temp != TEXT(')') );

		if ( *Temp == 0 )
		{
			appThrowf(TEXT("End of file encountered while attempting to parse closing ')' in %s list"), GroupName);
		}

		ParseNext(&Temp);
		if ( *Temp++ != TEXT(')') )
		{
			appThrowf(TEXT("Missing closing ')' in %s expression"), GroupName);
		}
		else if ( NextWord.Len() == 0 )
		{
			appThrowf(TEXT("Unexpected ')' - missing class name in %s list"), GroupName);
		}

		bSuccess = TRUE;
		out_ParsedClassNames.AddUniqueItem(*NextWord);
		InputText = Temp;
	}

	return bSuccess;
}

IMPLEMENT_CLASS(UClassFactoryUC);


/*------------------------------------------------------------------------------
	ULevelFactory.
------------------------------------------------------------------------------*/

/**
 * Iterates over an object's properties making sure that any UObjectProperty properties
 * that refer to non-NULL actors refer to valid actors.
 *
 * @return		FALSE if no object references were NULL'd out, TRUE otherwise.
 */
static UBOOL ForceValidActorRefs(UStruct* Struct, BYTE* Data)
{
	UBOOL bChangedObjectPointer = FALSE;

	//@todo DB: Optimize this!!
	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Struct); It; ++It )
	{
		for( INT i=0; i<It->ArrayDim; i++ )
		{
			BYTE* Value = Data + It->Offset + i*It->ElementSize;
			if( Cast<UObjectProperty>(*It) )
			{
				UObject*& Obj = *(UObject**)Value;
				if( Cast<AActor>(Obj) && !Obj->HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject) )
				{
					UBOOL bFound = FALSE;
					for( FActorIterator It; It; ++It )
					{
						AActor* Actor = *It;
						if( Actor == Obj )
						{
							bFound = TRUE;
							break;
						}
					}
					
					if( !bFound )
					{
						debugf( NAME_Log, TEXT("Usurped %s"), *Obj->GetClass()->GetName() );
						Obj = NULL;
						bChangedObjectPointer = TRUE;
					}
				}
			}
			else if( Cast<UStructProperty>(*It, CLASS_IsAUStructProperty) )
			{
				bChangedObjectPointer |= ForceValidActorRefs( ((UStructProperty*)*It)->Struct, Value );
			}
		}
	}

	return bChangedObjectPointer;
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void ULevelFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UWorld::StaticClass();
	new(Formats)FString(TEXT("t3d;Unreal World"));
	bCreateNew = FALSE;
	bText = TRUE;
	bEditorImport = TRUE;
}
ULevelFactory::ULevelFactory()
{
	bEditorImport = 1;
}
UObject* ULevelFactory::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
#ifdef MULTI_LEVEL_IMPORT
	// this level is the current level for pasting. If we get a named level, not for pasting, we will look up the level, and overwrite this
	ULevel*				OldCurrentLevel = GWorld->CurrentLevel;
	check(OldCurrentLevel);
#endif

	UPackage* RootMapPackage = Cast<UPackage>(InParent);
	TMap<FString, UPackage*> MapPackages;
	// Assumes data is being imported over top of a new, valid map.
	ParseNext( &Buffer );
	if (GetBEGIN(&Buffer, TEXT("MAP")))
	{
		if (RootMapPackage)
		{
			FString MapName;
			if (Parse(Buffer, TEXT("Name="), MapName))
			{
				// Advance the buffer
				Buffer += appStrlen(TEXT("Name="));
				Buffer += MapName.Len();
				// Rename it!
				RootMapPackage->Rename(*MapName, NULL, REN_ForceNoResetLoaders);
				// Stick it in the package map
				MapPackages.Set(MapName, RootMapPackage);
			}
		}
	}
	else
	{
		return GWorld;
	}

	UBOOL bIsExpectingNewMapTag = FALSE;

	// Mark all actors as already existing.
	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;	
		Actor->bTempEditor = 1;
	}

	// Unselect all actors.
	GEditor->SelectNone( FALSE, FALSE );

	// Mark us importing a T3D (only from a file, not from copy/paste).
	GEditor->IsImportingT3D = appStricmp(Type,TEXT("paste")) != 0;
	GIsImportingT3D = GEditor->IsImportingT3D;

	// We need to detect if the .t3d file is the entire level or just selected actors, because we
	// don't want to replace the WorldInfo and BuildBrush if they already exist. To know if we
	// can skip the WorldInfo and BuilderBrush (which will always be the first two actors if the entire
	// level was exported), we make sure the first actor is a WorldInfo, if it is, and we already had
	// a WorldInfo, then we skip the builder brush
	// In other words, if we are importing a full level into a full level, we don't want to import
	// the WorldInfo and BuildBrush
	UBOOL bShouldSkipImportSpecialActors = false;
	UBOOL bHitLevelToken = false;

	FString MapPackageText;

	INT ActorIndex = 0;

	// Maintain a list of a new actors and the text they were created from.
	TMap<AActor*,FString> NewActorMap;

	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		// If we're still waiting to see a 'MAP' tag, then check for that
		if( bIsExpectingNewMapTag )
		{
			if( GetBEGIN( &Str, TEXT("MAP")) )
			{
				bIsExpectingNewMapTag = FALSE;
			}
			else
			{
				// Not a new map tag, so continue on
			}
		}
		else if( GetEND(&Str,TEXT("MAP")) )
		{
			// End of brush polys.
			bIsExpectingNewMapTag = TRUE;
		}
		else if (!bHitLevelToken && GetBEGIN(&Str, TEXT("PACKAGEOBJECT")))
		{
			// Read in the object
			FString ObjectName;
			FString ClassName;
			FString PackageName;
			
			Parse(Str, TEXT("Name="), ObjectName);
			Parse(Str, TEXT("Class="), ClassName);
			Parse(Str, TEXT("PARENTPACKAGE="), PackageName);

			debugf(TEXT("Pre-level object: Package %s - %s - %s"), *PackageName, *ClassName, *ObjectName);
		}
		else if (!bHitLevelToken && GetEND(&Str, TEXT("PACKAGEOBJECT")))
		{
			// Finished with the object
		}
		else if( GetBEGIN(&Str,TEXT("LEVEL")) )
		{
			bHitLevelToken = TRUE;
#ifdef MULTI_LEVEL_IMPORT
			// try to look up the named level. if this fails, we will need to create a new level
			if (ParseObject<ULevel>(Str, TEXT("NAME="), GWorld->CurrentLevel, GWorld->GetOuter()) == false)
			{
				// get the name
				FString LevelName;
				// if there is no name, that means we are pasting, so just put this guy into the CurrentLevel - don't make a new one
				if (Parse(Str, TEXT("NAME="), LevelName))
				{
					// create a new named level
					GWorld->CurrentLevel = new(GWorld->GetOuter(), *LevelName)ULevel(FURL(NULL));
				}
			}
#endif
		}
		else if( GetEND(&Str,TEXT("LEVEL")) )
		{
#ifdef MULTI_LEVEL_IMPORT
			// any actors outside of a level block go into the current level
			GWorld->CurrentLevel = OldCurrentLevel;
#endif
		}
		else if( GetBEGIN(&Str,TEXT("ACTOR")) )
		{
			UClass* TempClass;
			if( ParseObject<UClass>( Str, TEXT("CLASS="), TempClass, ANY_PACKAGE ) )
			{
				// Get actor name.
				FName ActorName(NAME_None);
				Parse( Str, TEXT("NAME="), ActorName );

				// Make sure this name is unique.
				AActor* Found=NULL;
				if( ActorName!=NAME_None )
				{
					// look in the current level for the same named actor
					Found = FindObject<AActor>( GWorld->CurrentLevel, *ActorName.ToString() );
				}
				if( Found )
				{
					Found->Rename();
				}

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				FString ArchetypeName;
				AActor* Archetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					TArray<FString> Refs;
					ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
					// find the class
					UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
					if ( ArchetypeClass )
					{
						if ( ArchetypeClass->IsChildOf(AActor::StaticClass()) )
						{
							// if we had the class, find the archetype
							// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
							Archetype = Cast<AActor>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
						}
						else
						{
							Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Actor"),
								Str, *Refs(0));
						}
					}
				}

				if (TempClass->IsChildOf(AWorldInfo::StaticClass()))
				{
					// if we see a WorldInfo, then we are importing an entire level, so if we
					// are importing into an existing level, then we should not import the next actor
					// which will be the builder brush
					check(ActorIndex == 0);

					// if we have any actors, then we are importing into an existing level
					if (GWorld->CurrentLevel->Actors.Num())
					{
						check(GWorld->CurrentLevel->Actors(0)->IsA(AWorldInfo::StaticClass()));

						// full level into full level, skip the first two actors
						bShouldSkipImportSpecialActors = true;
					}
				}

				// Get property text.
				FString PropText, StrLine;
				while
				(	GetEND( &Buffer, TEXT("ACTOR") )==0
				&&	ParseLine( &Buffer, StrLine ) )
				{
					PropText += *StrLine;
					PropText += TEXT("\r\n");
				}

				// If we need to skip the WorldInfo and BuilderBrush, skip the first two actors.  Note that
				// at this point, we already know that we have a WorldInfo and BuilderBrush in the .t3d.
				if ( !(bShouldSkipImportSpecialActors && ActorIndex < 2) )
				{
					// Don't import the default physics volume, as it doesn't have a UModel associated with it
					// and thus will not import properly.
					if ( !TempClass->IsChildOf(ADefaultPhysicsVolume::StaticClass()) )
					{
						// Create a new actor.
						AActor* NewActor = GWorld->SpawnActor( TempClass, ActorName,
																FVector(0,0,0), TempClass->GetDefaultActor()->Rotation,
																Archetype, TRUE, FALSE );
						check( NewActor );

						// Store the new actor and the text it should be initialized with.
						NewActorMap.Set( NewActor, *PropText );
					}
				}

				// increment the number of actors we imported
				ActorIndex++;
			}
		}
		else if (GetBEGIN(&Str, TEXT("TERRAIN")))
		{
			UClass* TempClass;
			if (ParseObject<UClass>(Str, TEXT("CLASS="), TempClass, ANY_PACKAGE))
			{
				// Get actor name.
				FName TerrainName(NAME_None);
				Parse(Str, TEXT("NAME="), TerrainName);

				// Make sure this name is unique.
				ATerrain* Found = NULL;
				if (TerrainName != NAME_None)
				{
					// look in the current level for the same named actor
					Found = FindObject<ATerrain>(GWorld->CurrentLevel, *TerrainName.ToString());
				}

				// If it isn't, then rename the found object...
				if (Found)
				{
					Found->Rename();
				}

				UTerrainFactory* TerrainFactory = new UTerrainFactory();
				check(TerrainFactory);

				TerrainFactory->ActorMap		= &NewActorMap;

				TerrainFactory->FactoryCreateText(ATerrain::StaticClass(), UPackage::StaticClass(), TerrainName, 0, 0,
					Type, Buffer, BufferEnd, Warn);
			}
		}
		else if( GetBEGIN(&Str,TEXT("SURFACE")) )
		{
			FString PropText, StrLine;

			UMaterialInterface* SrcMaterial = NULL;
			FVector SrcBase, SrcTextureU, SrcTextureV, SrcNormal;
			DWORD SrcPolyFlags = PF_DefaultFlags;
			INT Check = 0;

			SrcBase = FVector(0,0,0);
			SrcTextureU = FVector(0,0,0);
			SrcTextureV = FVector(0,0,0);
			SrcNormal = FVector(0,0,0);

			do
			{
				if( ParseCommand(&Buffer,TEXT("TEXTURE")) )
				{
					Buffer++;	// Move past the '=' sign

					FString TextureName;
					ParseLine(&Buffer, TextureName, TRUE);
					if ( TextureName != TEXT("None") )
					{
						SrcMaterial = Cast<UMaterialInterface>(StaticLoadObject( UMaterialInterface::StaticClass(), NULL, *TextureName, NULL, LOAD_NoWarn, NULL ));
					}
					Check++;
				}
				else if( ParseCommand(&Buffer,TEXT("BASE")) )
				{
					GetFVECTOR( Buffer, SrcBase );
					Check++;
				}
				else if( ParseCommand(&Buffer,TEXT("TEXTUREU")) )
				{
					GetFVECTOR( Buffer, SrcTextureU );
					Check++;
				}
				else if( ParseCommand(&Buffer,TEXT("TEXTUREV")) )
				{
					GetFVECTOR( Buffer, SrcTextureV );
					Check++;
				}
				else if( ParseCommand(&Buffer,TEXT("NORMAL")) )
				{
					GetFVECTOR( Buffer, SrcNormal );
					Check++;
				}
				else if( ParseCommand(&Buffer,TEXT("POLYFLAGS")) )
				{
					Parse( Buffer, TEXT("="), SrcPolyFlags );
					Check++;
				}
			}
			while
			(	GetEND( &Buffer, TEXT("SURFACE") )==0
				&&	ParseLine( &Buffer, StrLine ) );

			if( Check == 5 )
			{
				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("PasteTextureToSurface")) );

				for( INT i = 0 ; i < GWorld->PersistentLevel->Model->Surfs.Num() ; i++ )
				{
					FBspSurf* DstSurf = &GWorld->PersistentLevel->Model->Surfs(i);

					if( DstSurf->PolyFlags & PF_Selected )
					{
						GWorld->PersistentLevel->Model->ModifySurf( i, 1 );

						const FVector DstNormal = GWorld->PersistentLevel->Model->Vectors( DstSurf->vNormal );

						// Need to compensate for changes in the polygon normal.
						const FRotator SrcRot = SrcNormal.Rotation();
						const FRotator DstRot = DstNormal.Rotation();
						const FRotationMatrix RotMatrix( DstRot - SrcRot );

						FVector NewBase	= RotMatrix.TransformFVector( SrcBase );
						FVector NewTextureU = RotMatrix.TransformNormal( SrcTextureU );
						FVector NewTextureV = RotMatrix.TransformNormal( SrcTextureV );

						DstSurf->Material = SrcMaterial;
						DstSurf->pBase = FBSPOps::bspAddPoint( GWorld->PersistentLevel->Model, &NewBase, 1 );
						DstSurf->vTextureU = FBSPOps::bspAddVector( GWorld->PersistentLevel->Model, &NewTextureU, 0 );
						DstSurf->vTextureV = FBSPOps::bspAddVector( GWorld->PersistentLevel->Model, &NewTextureV, 0 );
						DstSurf->PolyFlags = SrcPolyFlags;

						DstSurf->PolyFlags &= ~PF_Selected;

						GEditor->polyUpdateMaster( GWorld->PersistentLevel->Model, i, 1 );
					}
				}
			}
		}
		else if (GetBEGIN(&Str,TEXT("MAPPACKAGE")))
		{
			// Get all the text.
			while ((GetEND(&Buffer, TEXT("MAPPACKAGE") )==0) && ParseLine(&Buffer, StrLine))
			{
				MapPackageText += *StrLine;
				MapPackageText += TEXT("\r\n");
			}
		}
	}

	// Import actor properties.
	// We do this after creating all actors so that actor references can be matched up.
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();

	// if we're pasting, then propagate useful actor properties to the propagator target
	UBOOL bPropagateActors = appStricmp(Type, TEXT("paste")) == 0;

	if (GIsImportingT3D && (MapPackageText.Len() > 0))
	{
		UPackageFactory* PackageFactory = new UPackageFactory();
		check(PackageFactory);

		FName NewPackageName(*(RootMapPackage->GetName()));

		const TCHAR* MapPkg_BufferStart = *MapPackageText;
		const TCHAR* MapPkg_BufferEnd = MapPkg_BufferStart + MapPackageText.Len();
		PackageFactory->FactoryCreateText(UPackage::StaticClass(), NULL, NewPackageName, 0, 0, TEXT("T3D"), MapPkg_BufferStart, MapPkg_BufferEnd, Warn);
	}

	for( FActorIterator It; It; ++It )
	{
		AActor* Actor = *It;

		// Import properties if the new actor is 
		UBOOL		bActorChanged = FALSE;
		FString*	PropText = NewActorMap.Find(Actor);
		if( PropText )
		{
			Actor->PreEditChange(NULL);
			ImportObjectProperties( (BYTE*)Actor, **PropText, Actor->GetClass(), Actor, Actor, Warn, 0 );
			bActorChanged = TRUE;

			// propagate the new actor if needed
			if (bPropagateActors)
			{
				GObjectPropagator->PropagateActor(Actor);
			}

			// This actor is new, so it should not have been marked as having existed before the import.
			check( !Actor->bTempEditor );
			GEditor->SelectActor( Actor, TRUE, NULL, FALSE );
		}
		else
		{
			// This actor is old, so it should have be marked as having existed before the import.
			check( Actor->bTempEditor == 1 );
		}

		// If this is a newly imported static brush, validate it.  If it's a newly imported dynamic brush, rebuild it.
		// Previously, this just called bspValidateBrush.  However, that caused the dynamic brushes which require a valid BSP tree
		// to be built to break after being duplicated.  Calling RebuildBrush will rebuild the BSP tree from the imported polygons.
		ABrush* Brush = Cast<ABrush>(Actor);
		if( bActorChanged && Brush && Brush->Brush )
		{
			const UBOOL bIsStaticBrush = Brush->IsStaticBrush();
			if( bIsStaticBrush )
			{
				FBSPOps::bspValidateBrush( Brush->Brush, TRUE, FALSE );
			}
			else
			{
				FBSPOps::RebuildBrush( Brush->Brush );
			}
		}

		// Make sure all references to actors are valid.
		if( Actor->WorldInfo != WorldInfo )
		{
			Actor->WorldInfo = WorldInfo;
			const UBOOL bFixedUpObjectRefs = ForceValidActorRefs( Actor->GetClass(), (BYTE*)Actor );

			// Aactor references were fixed up, so treat the actor as having been changed.
			if ( bFixedUpObjectRefs )
			{
				bActorChanged = TRUE;
			}
		}

		// Copy brushes' model pointers over to their BrushComponent, to keep compatibility with old T3Ds.
		if( Brush && bActorChanged )
		{
			if( Brush->BrushComponent ) // Should always be the case, but not asserting so that old broken content won't crash.
			{
				Brush->BrushComponent->Brush = Brush->Brush;

				// We need to avoid duplicating default/ builder brushes. This is done by destroying all brushes that are CSG_Active and are not
				// the default brush in their respective levels.
				if( Brush->IsStaticBrush() && Brush->CsgOper==CSG_Active )
				{
					UBOOL bIsDefaultBrush = FALSE;
					
					// Iterate over all levels and compare current actor to the level's default brush.
					for( INT LevelIndex=0; LevelIndex<GWorld->Levels.Num(); LevelIndex++ )
					{
						ULevel* Level = GWorld->Levels(LevelIndex);
						if( Level->GetBrush() == Brush )
						{
							bIsDefaultBrush = TRUE;
							break;
						}
					}

					// Destroy actor if it's a builder brush but not the default brush in any of the currently loaded levels.
					if( !bIsDefaultBrush )
					{
						GWorld->DestroyActor( Brush );

						// Since the actor has been destroyed, skip the rest of this iteration of the loop.
						continue;
					}
				}
			}
		}
		
		// If the actor was imported . . .
		if( bActorChanged )
		{
			// Let the actor deal with having been imported, if desired.
			Actor->PostEditImport();

			// Notify actor its properties have changed.
			Actor->PostEditChange(NULL);
		}
	}

	// Mark us as no longer importing a T3D.
	GEditor->IsImportingT3D = 0;
	GIsImportingT3D = FALSE;

	return GWorld;
}
IMPLEMENT_CLASS(ULevelFactory);

/*-----------------------------------------------------------------------------
	UPackageFactory.
-----------------------------------------------------------------------------*/
UPackageFactory::UPackageFactory()
{
	bEditorImport = 1;
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPackageFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UPackage::StaticClass();
	new(Formats)FString(TEXT("T3DPKG;Unreal Package"));
	bCreateNew = FALSE;
	bText = TRUE;
	bEditorImport = TRUE;
}

UObject* UPackageFactory::FactoryCreateText( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd, FFeedbackContext* Warn )
{
	UBOOL bSavedImportingT3D = GIsImportingT3D;
	// Mark us as no longer importing a T3D.
	GEditor->IsImportingT3D = TRUE;
	GIsImportingT3D = TRUE;

	if (InParent != NULL)
	{
		return NULL;
	}

	TMap<FString, UPackage*> MapPackages;
	UBOOL bImportingMapPackage = FALSE;

	UPackage* TopLevelPackage = NULL;
	UPackage* RootMapPackage = NULL;
	if (GWorld)
	{
		RootMapPackage = GWorld->GetOutermost();
	}

	if (RootMapPackage)
	{
		if (RootMapPackage->GetName() == Name.ToString())
		{
			// Loading into the Map package!
			MapPackages.Set(RootMapPackage->GetName(), RootMapPackage);
			TopLevelPackage = RootMapPackage;
			bImportingMapPackage = TRUE;
		}
	}

	// Unselect all actors.
	GEditor->SelectNone( FALSE, FALSE );

	// Mark us importing a T3D (only from a file, not from copy/paste).
	GEditor->IsImportingT3D = appStricmp(Type,TEXT("paste")) != 0;
	GIsImportingT3D = GEditor->IsImportingT3D;

	// Maintain a list of a new package objects and the text they were created from.
	TMap<UObject*,FString> NewPackageObjectMap;
	// The SMData text for static meshes
	TMap<UStaticMesh*,FString> NewStaticMeshMap;
	// The TextureData text for textures
	TMap<UTexture*,FString> NewTextureMap;
	// The MaterialData text for materials
	TMap<UMaterial*,FString> NewMaterialMap;

	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		if (GetBEGIN(&Str, TEXT("TOPLEVELPACKAGE")) && !bImportingMapPackage)
		{
			//Begin TopLevelPackage Class=Package Name=ExportTest_ORIG Archetype=Package'Core.Default__Package'
			UClass* TempClass;
			if( ParseObject<UClass>( Str, TEXT("CLASS="), TempClass, ANY_PACKAGE ) )
			{
				// Get actor name.
				FName PackageName(NAME_None);
				Parse( Str, TEXT("NAME="), PackageName );

				if (FindObject<UPackage>(ANY_PACKAGE, *(PackageName.ToString())))
				{
					warnf(TEXT("Package factory can only handle the map package or new packages!"));
					return NULL;
				}
				TopLevelPackage = CreatePackage(NULL, *(PackageName.ToString()));
				TopLevelPackage->SetFlags(RF_Standalone|RF_Public);
				MapPackages.Set(TopLevelPackage->GetName(), TopLevelPackage);

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				FString ArchetypeName;
				AActor* Archetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
				}
			}
		}
		else if (GetBEGIN(&Str,TEXT("PACKAGE")))
		{
			FString ParentPackageName;
			Parse(Str, TEXT("PARENTPACKAGE="), ParentPackageName);
			UClass* PkgClass;
			if (ParseObject<UClass>(Str, TEXT("CLASS="), PkgClass, ANY_PACKAGE))
			{
				// Get the name of the object.
				FName NewPackageName(NAME_None);
				Parse(Str, TEXT("NAME="), NewPackageName);

				// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
				FString ArchetypeName;
				UPackage* Archetype = NULL;
				if (Parse(Str, TEXT("Archetype="), ArchetypeName))
				{
					// if given a name, break it up along the ' so separate the class from the name
					TArray<FString> Refs;
					ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
					// find the class
					UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
					if (ArchetypeClass)
					{
						if (ArchetypeClass->IsChildOf(UPackage::StaticClass()))
						{
							// if we had the class, find the archetype
							// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
							Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
						}
						else
						{
							Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
								Str, *Refs(0));
						}
					}

					UPackage* ParentPkg = NULL;
					UPackage** ppParentPkg = MapPackages.Find(ParentPackageName);
					if (ppParentPkg)
					{
						ParentPkg = *ppParentPkg;
					}
					check(ParentPkg);

					UPackage* NewPackage = Cast<UPackage>(StaticConstructObject(UPackage::StaticClass(), ParentPkg, NewPackageName, 0, Archetype));
					check(NewPackage);
					NewPackage->SetFlags(RF_Standalone|RF_Public);
					MapPackages.Set(NewPackageName.ToString(), NewPackage);
				}
			}
		}
		else if (GetBEGIN(&Str,TEXT("PACKAGEOBJECT")))
		{
			FString ParentPackageName;
			Parse(Str, TEXT("PARENTPACKAGE="), ParentPackageName);

			UPackage** ppParentPkg = MapPackages.Find(ParentPackageName);
			if (ppParentPkg != NULL)
			{
				UPackage* ParentPkg = *ppParentPkg;

				UClass* Class;
				if (ParseObject<UClass>(Str, TEXT("CLASS="), Class, ANY_PACKAGE))
				{
					// Get the name of the object.
					FName NewObjectName(NAME_None);
					Parse(Str, TEXT("NAME="), NewObjectName);
					// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
					FString ArchetypeName;
					UPackage* Archetype = NULL;
					if (Parse(Str, TEXT("Archetype="), ArchetypeName))
					{
						// if given a name, break it up along the ' so separate the class from the name
						TArray<FString> Refs;
						ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
						// find the class
						UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
						if (ArchetypeClass)
						{
							if (ArchetypeClass->IsChildOf(Class))
							{
								// if we had the class, find the archetype
								// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
								Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
							}
							else
							{
								Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
									Str, *Refs(0));
							}
						}

						// Get property text.
						FString PropText, StrLine;
						while ((GetEND( &Buffer, TEXT("PACKAGEOBJECT") )==0) && ParseLine(&Buffer, StrLine))
						{
							PropText += *StrLine;
							PropText += TEXT("\r\n");
						}

						debugf(TEXT("Constructing PackageObject %s (in %s)"), *(NewObjectName.ToString()), ParentPkg ? *(ParentPkg->GetName()) : TEXT(""));

						UObject* NewObject = StaticConstructObject(Class, ParentPkg, NewObjectName, 0, Archetype);
						check(NewObject);

						debugf(TEXT("Constructed PackageObject %s"), *(NewObject->GetPathName()));

						// Read the rest of the properties in and store off for later...
						NewPackageObjectMap.Set(NewObject, PropText);
					}
				}
			}
		}
		else if (GetBEGIN(&Str,TEXT("PACKAGESTATICMESH")))
		{
			FString ParentPackageName;
			Parse(Str, TEXT("PARENTPACKAGE="), ParentPackageName);

			UPackage** ppParentPkg = MapPackages.Find(ParentPackageName);
			if (ppParentPkg != NULL)
			{
				UPackage* ParentPkg = *ppParentPkg;
				// Grab the next line...
				ParseLine(&Buffer,StrLine);
				Str = *StrLine;
				if (GetBEGIN(&Str, TEXT("STATICMESH")))
				{
					UClass* Class;
					if (ParseObject<UClass>(Str, TEXT("CLASS="), Class, ANY_PACKAGE))
					{
						// Get the name of the object.
						FName NewObjectName(NAME_None);
						Parse(Str, TEXT("NAME="), NewObjectName);
						// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
						FString ArchetypeName;
						UPackage* Archetype = NULL;
						if (Parse(Str, TEXT("Archetype="), ArchetypeName))
						{
							// if given a name, break it up along the ' so separate the class from the name
							TArray<FString> Refs;
							ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
							// find the class
							UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
							if (ArchetypeClass)
							{
								if (ArchetypeClass->IsChildOf(Class))
								{
									// if we had the class, find the archetype
									// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
									Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
								}
								else
								{
									Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
										Str, *Refs(0));
								}
							}

							FString PropText, SMDataText, StrLine;

							// Get property text.
							while ((GetBEGIN(&Buffer, TEXT("SMDATA"))==0) && (GetEND( &Buffer, TEXT("STATICMESH") )==0) && ParseLine(&Buffer, StrLine))
							{
								PropText += *StrLine;
								PropText += TEXT("\r\n");
							}

							while ((GetEND(&Buffer, TEXT("SMDATA"))==0) && (GetEND( &Buffer, TEXT("STATICMESH") )==0) && ParseLine(&Buffer, StrLine))
							{
								SMDataText += *StrLine;
								SMDataText += TEXT("\r\n");
							}

							UObject* NewObject = StaticConstructObject(Class, ParentPkg, NewObjectName, 0, Archetype);
							check(NewObject);

							// Read the rest of the properties in and store off for later...
							NewPackageObjectMap.Set(NewObject, PropText);
							UStaticMesh* NewStaticMesh = CastChecked<UStaticMesh>(NewObject);
							NewStaticMeshMap.Set(NewStaticMesh, SMDataText);
						}
					}
				}
			}
		}
		else if (GetBEGIN(&Str, TEXT("PACKAGETEXTURE")))
		{
			FString ParentPackageName;
			Parse(Str, TEXT("PARENTPACKAGE="), ParentPackageName);

			UPackage** ppParentPkg = MapPackages.Find(ParentPackageName);
			if (ppParentPkg != NULL)
			{
				UPackage* ParentPkg = *ppParentPkg;
				// Grab the next line...
				// Grab the next line...
				ParseLine(&Buffer,StrLine);
				Str = *StrLine;
				if (GetBEGIN(&Str, TEXT("TEXTURE")))
				{
					UClass* Class;
					if (ParseObject<UClass>(Str, TEXT("CLASS="), Class, ANY_PACKAGE))
					{
						// Get the name of the object.
						FName NewObjectName(NAME_None);
						Parse(Str, TEXT("NAME="), NewObjectName);
						// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
						FString ArchetypeName;
						UPackage* Archetype = NULL;
						if (Parse(Str, TEXT("Archetype="), ArchetypeName))
						{
							// if given a name, break it up along the ' so separate the class from the name
							TArray<FString> Refs;
							ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
							// find the class
							UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
							if (ArchetypeClass)
							{
								if (ArchetypeClass->IsChildOf(Class))
								{
									// if we had the class, find the archetype
									// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
									Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
								}
								else
								{
									Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
										Str, *Refs(0));
								}
							}

							FString PropText, TextureDataText, StrLine;

							// Get property text.
							while ((GetBEGIN(&Buffer, TEXT("TEXTUREDATA"))==0) && (GetEND( &Buffer, TEXT("TEXTURE") )==0) && ParseLine(&Buffer, StrLine))
							{
								PropText += *StrLine;
								PropText += TEXT("\r\n");
							}

							while ((GetEND(&Buffer, TEXT("TEXTUREDATA"))==0) && (GetEND( &Buffer, TEXT("TEXTURE") )==0) && ParseLine(&Buffer, StrLine))
							{
								TextureDataText += *StrLine;
								TextureDataText += TEXT("\r\n");
							}

							UObject* NewObject = StaticConstructObject(Class, ParentPkg, NewObjectName, 0, Archetype);
							check(NewObject);

							// Read the rest of the properties in and store off for later...
							NewPackageObjectMap.Set(NewObject, PropText);
							UTexture* NewTexture = CastChecked<UTexture>(NewObject);
							if (NewTexture)
							{
								NewTextureMap.Set(NewTexture, TextureDataText);
							}
						}
					}
				}
			}
			//NewTextureMap
		}
		else if (GetBEGIN(&Str, TEXT("PACKAGEMATERIAL")))
		{
			FString ParentPackageName;
			Parse(Str, TEXT("PARENTPACKAGE="), ParentPackageName);

			UPackage** ppParentPkg = MapPackages.Find(ParentPackageName);
			if (ppParentPkg != NULL)
			{
				UPackage* ParentPkg = *ppParentPkg;
				// Grab the next line...
				ParseLine(&Buffer,StrLine);
				Str = *StrLine;
				if (GetBEGIN(&Str, TEXT("MATERIAL")))
				{
					UClass* Class;
					if (ParseObject<UClass>(Str, TEXT("CLASS="), Class, ANY_PACKAGE))
					{
						// Get the name of the object.
						FName NewObjectName(NAME_None);
						Parse(Str, TEXT("NAME="), NewObjectName);
						// if an archetype was specified in the Begin Object block, use that as the template for the ConstructObject call.
						FString ArchetypeName;
						UPackage* Archetype = NULL;
						if (Parse(Str, TEXT("Archetype="), ArchetypeName))
						{
							// if given a name, break it up along the ' so separate the class from the name
							TArray<FString> Refs;
							ArchetypeName.ParseIntoArray(&Refs, TEXT("'"), TRUE);
							// find the class
							UClass* ArchetypeClass = (UClass*)UObject::StaticFindObject(UClass::StaticClass(), ANY_PACKAGE, *Refs(0));
							if (ArchetypeClass)
							{
								if (ArchetypeClass->IsChildOf(Class))
								{
									// if we had the class, find the archetype
									// @fixme ronp subobjects: this _may_ need StaticLoadObject, but there is currently a bug in StaticLoadObject that it can't take a non-package pathname properly
									Archetype = Cast<UPackage>(UObject::StaticFindObject(ArchetypeClass, ANY_PACKAGE, *Refs(1)));
								}
								else
								{
									Warn->Logf(NAME_Warning, TEXT("Invalid archetype specified in subobject definition '%s': %s is not a child of Package"),
										Str, *Refs(0));
								}
							}

							FString PropText, MaterialDataText, StrLine;

							// Get property text.
							while ((GetBEGIN(&Buffer, TEXT("MATERIALDATA"))==0) && (GetEND( &Buffer, TEXT("MATERIAL") )==0) && ParseLine(&Buffer, StrLine))
							{
								PropText += *StrLine;
								PropText += TEXT("\r\n");
							}

							while ((GetEND(&Buffer, TEXT("MATERIALDATA"))==0) && (GetEND( &Buffer, TEXT("MATERIAL") )==0) && ParseLine(&Buffer, StrLine))
							{
								MaterialDataText += *StrLine;
								MaterialDataText += TEXT("\r\n");
							}

							UObject* NewObject = StaticConstructObject(Class, ParentPkg, NewObjectName, 0, Archetype);
							check(NewObject);

							// Read the rest of the properties in and store off for later...
							NewPackageObjectMap.Set(NewObject, PropText);
							UMaterial* NewMaterial = CastChecked<UMaterial>(NewObject);
							NewMaterialMap.Set(NewMaterial, MaterialDataText);
						}
					}
				}
			}
		}
	}

	UStaticMeshFactory*	StaticMeshFactory = new UStaticMeshFactory;
	check(StaticMeshFactory);
	UTextureFactory* TextureFactory = new UTextureFactory;
	check(TextureFactory);
	UMaterialFactory* MaterialFactory = new UMaterialFactory;
	check(MaterialFactory);

	for (FObjectIterator ObjIt; ObjIt; ++ObjIt)
	{
		UObject* LoadObject = *ObjIt;

		if (LoadObject)
		{
			UBOOL bModifiedObject = FALSE;

			UStaticMesh* LoadStaticMesh = Cast<UStaticMesh>(LoadObject);
			UTexture2D* LoadTexture2D = Cast<UTexture2D>(LoadObject);
			UTextureCube* LoadTextureCube = Cast<UTextureCube>(LoadObject);
			UMaterial* LoadMaterial = Cast<UMaterial>(LoadObject);
			UMaterialInstance* LoadMaterialInstance = Cast<UMaterialInstance>(LoadObject);

			FString* PropText = NewPackageObjectMap.Find(LoadObject);
			if (PropText)
			{
				LoadObject->PreEditChange(NULL);
				ImportObjectProperties((BYTE*)LoadObject, **PropText, LoadObject->GetClass(), LoadObject, LoadObject, Warn, 0 );
				bModifiedObject = TRUE;
			}

			if (LoadStaticMesh)
			{
				FString* SMDataText = NewStaticMeshMap.Find(LoadStaticMesh);
				if (SMDataText)
				{
					// Handle the SMData
					StaticMeshFactory->InitializeFromT3DSMDataText(LoadStaticMesh, *SMDataText, Warn);
					bModifiedObject = TRUE;
				}
			}
			else
			if (LoadTexture2D)
			{
				FString* TextureDataText = NewTextureMap.Find(LoadTexture2D);
				if (TextureDataText)
				{
					// Handle the texture data
					TextureFactory->InitializeFromT3DTextureDataText(LoadTexture2D, *TextureDataText, Warn);
					bModifiedObject = TRUE;
				}
			}
			else
			if (LoadTextureCube)
			{
				FString* TextureDataText = NewTextureMap.Find(LoadTextureCube);
				if (TextureDataText)
				{
					// Handle the texture data
					TextureFactory->InitializeFromT3DTextureDataText(LoadTextureCube, *TextureDataText, Warn);
					bModifiedObject = TRUE;
				}
			}
			else
			if (LoadMaterial)
			{
				FString* MaterialDataText = NewMaterialMap.Find(LoadMaterial);
				if (MaterialDataText)
				{
					// Handle the material data
					MaterialFactory->InitializeFromT3DMaterialDataText(LoadMaterial, *MaterialDataText, Warn);
					bModifiedObject = TRUE;
				}
			}
			else
			if (LoadMaterialInstance)
			{
				if (bModifiedObject == TRUE)
				{
					LoadMaterialInstance->bStaticPermutationDirty = TRUE;
				}
			}

			if (bModifiedObject == TRUE)
			{
				// Let the actor deal with having been imported, if desired.
				LoadObject->PostEditImport();
				// Notify actor its properties have changed.
				LoadObject->PostEditChange(NULL);
				LoadObject->SetFlags(RF_Standalone | RF_Public | RF_InitializedProps);
				LoadObject->MarkPackageDirty();
			}
		}
	}

	// Mark us as no longer importing a T3D.
	GEditor->IsImportingT3D = bSavedImportingT3D;
	GIsImportingT3D = bSavedImportingT3D;

	return TopLevelPackage;
}

IMPLEMENT_CLASS(UPackageFactory);

/*-----------------------------------------------------------------------------
	UPolysFactory.
-----------------------------------------------------------------------------*/

struct FASEMaterial
{
	FASEMaterial()
	{
		Width = Height = 256;
		UTiling = VTiling = 1;
		Material = NULL;
		bTwoSided = bUnlit = bAlphaTexture = bMasked = bTranslucent = 0;
	}
	FASEMaterial( const TCHAR* InName, INT InWidth, INT InHeight, FLOAT InUTiling, FLOAT InVTiling, UMaterialInterface* InMaterial, UBOOL InTwoSided, UBOOL InUnlit, UBOOL InAlphaTexture, UBOOL InMasked, UBOOL InTranslucent )
	{
		appStrcpy( Name, InName );
		Width = InWidth;
		Height = InHeight;
		UTiling = InUTiling;
		VTiling = InVTiling;
		Material = InMaterial;
		bTwoSided = InTwoSided;
		bUnlit = InUnlit;
		bAlphaTexture = InAlphaTexture;
		bMasked = InMasked;
		bTranslucent = InTranslucent;
	}

	TCHAR Name[128];
	INT Width, Height;
	FLOAT UTiling, VTiling;
	UMaterialInterface* Material;
	UBOOL bTwoSided, bUnlit, bAlphaTexture, bMasked, bTranslucent;
};

struct FASEMaterialHeader
{
	FASEMaterialHeader()
	{
	}

	TArray<FASEMaterial> Materials;
};

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPolysFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UPolys::StaticClass();
	new(Formats)FString(TEXT("t3d;Unreal brush text"));
	bCreateNew = FALSE;
	bText = TRUE;
}
UPolysFactory::UPolysFactory()
{
}
UObject* UPolysFactory::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
	// Create polys.
	UPolys* Polys = Context ? CastChecked<UPolys>(Context) : new(InParent,Name,Flags)UPolys;

	// Eat up if present.
	GetBEGIN( &Buffer, TEXT("POLYLIST") );

	// Parse all stuff.
	INT First=1, GotBase=0;
	FString StrLine, ExtraLine;
	FPoly Poly;
	while( ParseLine( &Buffer, StrLine ) )
	{
		const TCHAR* Str = *StrLine;
		if( GetEND(&Str,TEXT("POLYLIST")) )
		{
			// End of brush polys.
			break;
		}
		//
		//
		// AutoCad - DXF File
		//
		//
		else if( appStrstr(Str,TEXT("ENTITIES")) && First )
		{
			debugf(NAME_Log,TEXT("Reading Autocad DXF file"));
			INT Started=0, NumPts=0, IsFace=0;
			FVector PointPool[4096];
			FPoly NewPoly; NewPoly.Init();

			while
			(	ParseLine( &Buffer, StrLine, 1 )
			&&	ParseLine( &Buffer, ExtraLine, 1 ) )
			{
				// Handle the line.
				Str = *ExtraLine;
				INT Code = appAtoi(*StrLine);
				if( Code==0 )
				{
					// Finish up current poly.
					if( Started )
					{
						if( NewPoly.Vertices.Num() == 0 )
						{
							// Got a vertex definition.
							NumPts++;
						}
						else if( NewPoly.Vertices.Num()>=3 )
						{
							// Got a poly definition.
							if( IsFace ) NewPoly.Reverse();
							NewPoly.Base = NewPoly.Vertices(0);
							NewPoly.Finalize(NULL,0);
							new(Polys->Element)FPoly( NewPoly );
						}
						else
						{
							// Bad.
							Warn->Logf( TEXT("DXF: Bad vertex count %i"), NewPoly.Vertices.Num() );
						}
						
						// Prepare for next.
						NewPoly.Init();
					}
					Started=0;

					if( ParseCommand(&Str,TEXT("VERTEX")) )
					{
						// Start of new vertex.
						PointPool[NumPts] = FVector(0,0,0);
						Started = 1;
						IsFace  = 0;
					}
					else if( ParseCommand(&Str,TEXT("3DFACE")) )
					{
						// Start of 3d face definition.
						Started = 1;
						IsFace  = 1;
					}
					else if( ParseCommand(&Str,TEXT("SEQEND")) )
					{
						// End of sequence.
						NumPts=0;
					}
					else if( ParseCommand(&Str,TEXT("EOF")) )
					{
						// End of file.
						break;
					}
				}
				else if( Started )
				{
					// Replace commas with periods to handle european dxf's.
					//for( TCHAR* Stupid = appStrchr(*ExtraLine,','); Stupid; Stupid=appStrchr(Stupid,',') )
					//	*Stupid = '.';

					// Handle codes.
					if( Code>=10 && Code<=19 )
					{
						// X coordinate.
						INT VertexIndex = Code-10;
						if( IsFace && VertexIndex >= NewPoly.Vertices.Num() )
						{
							NewPoly.Vertices.AddZeroed(VertexIndex - NewPoly.Vertices.Num() + 1);
						}
						NewPoly.Vertices(VertexIndex).X = PointPool[NumPts].X = appAtof(*ExtraLine);
					}
					else if( Code>=20 && Code<=29 )
					{
						// Y coordinate.
						INT VertexIndex = Code-20;
						NewPoly.Vertices(VertexIndex).Y = PointPool[NumPts].Y = appAtof(*ExtraLine);
					}
					else if( Code>=30 && Code<=39 )
					{
						// Z coordinate.
						INT VertexIndex = Code-30;
						NewPoly.Vertices(VertexIndex).Z = PointPool[NumPts].Z = appAtof(*ExtraLine);
					}
					else if( Code>=71 && Code<=79 && (Code-71)==NewPoly.Vertices.Num() )
					{
						INT iPoint = Abs(appAtoi(*ExtraLine));
						if( iPoint>0 && iPoint<=NumPts )
							new(NewPoly.Vertices) FVector(PointPool[iPoint-1]);
						else debugf( NAME_Warning, TEXT("DXF: Invalid point index %i/%i"), iPoint, NumPts );
					}
				}
			}
		}
		//
		//
		// 3D Studio MAX - ASC File
		//
		//
		else if( appStrstr(Str,TEXT("Tri-mesh,")) && First )
		{
			debugf( NAME_Log, TEXT("Reading 3D Studio ASC file") );
			FVector PointPool[4096];

			AscReloop:
			INT NumVerts = 0, TempNumPolys=0, TempVerts=0;
			while( ParseLine( &Buffer, StrLine ) )
			{
				Str = *StrLine;

				FString VertText = FString::Printf( TEXT("Vertex %i:"), NumVerts );
				FString FaceText = FString::Printf( TEXT("Face %i:"), TempNumPolys );
				if( appStrstr(Str,*VertText) )
				{
					PointPool[NumVerts].X = appAtof(appStrstr(Str,TEXT("X:"))+2);
					PointPool[NumVerts].Y = appAtof(appStrstr(Str,TEXT("Y:"))+2);
					PointPool[NumVerts].Z = appAtof(appStrstr(Str,TEXT("Z:"))+2);
					NumVerts++;
					TempVerts++;
				}
				else if( appStrstr(Str,*FaceText) )
				{
					Poly.Init();
					new(Poly.Vertices)FVector(PointPool[appAtoi(appStrstr(Str,TEXT("A:"))+2)]);
					new(Poly.Vertices)FVector(PointPool[appAtoi(appStrstr(Str,TEXT("B:"))+2)]);
					new(Poly.Vertices)FVector(PointPool[appAtoi(appStrstr(Str,TEXT("C:"))+2)]);
					Poly.Base = Poly.Vertices(0);
					Poly.Finalize(NULL,0);
					new(Polys->Element)FPoly(Poly);
					TempNumPolys++;
				}
				else if( appStrstr(Str,TEXT("Tri-mesh,")) )
					goto AscReloop;
			}
			debugf( NAME_Log, TEXT("Imported %i vertices, %i faces"), TempVerts, Polys->Element.Num() );
		}
		//
		//
		// 3D Studio MAX - ASE File
		//
		//
		else if( appStrstr(Str,TEXT("*3DSMAX_ASCIIEXPORT")) && First )
		{
			debugf( NAME_Log, TEXT("Reading 3D Studio ASE file") );

			TArray<FVector> Vertex;						// 1 FVector per entry
			TArray<INT> FaceIdx;						// 3 INT's for vertex indices per entry
			TArray<INT> FaceMaterialsIdx;				// 1 INT for material ID per face
			TArray<FVector> TexCoord;					// 1 FVector per entry
			TArray<INT> FaceTexCoordIdx;				// 3 INT's per entry
			TArray<FASEMaterialHeader> ASEMaterialHeaders;	// 1 per material (multiple sub-materials inside each one)
			TArray<DWORD>	SmoothingGroups;			// 1 DWORD per face.
			
			INT NumVertex = 0, NumFaces = 0, NumTVertex = 0, NumTFaces = 0, ASEMaterialRef = -1;

			UBOOL IgnoreMcdGeometry = 0;

			enum {
				GROUP_NONE			= 0,
				GROUP_MATERIAL		= 1,
				GROUP_GEOM			= 2,
			} Group;

			enum {
				SECTION_NONE		= 0,
				SECTION_MATERIAL	= 1,
				SECTION_MAP_DIFFUSE	= 2,
				SECTION_VERTS		= 3,
				SECTION_FACES		= 4,
				SECTION_TVERTS		= 5,
				SECTION_TFACES		= 6,
			} Section;

			Group = GROUP_NONE;
			Section = SECTION_NONE;
			while( ParseLine( &Buffer, StrLine ) )
			{
				Str = *StrLine;

				if( Group == GROUP_NONE )
				{
					if( StrLine.InStr(TEXT("*MATERIAL_LIST")) != -1 )
						Group = GROUP_MATERIAL;
					else if( StrLine.InStr(TEXT("*GEOMOBJECT")) != -1 )
						Group = GROUP_GEOM;
				}
				else if ( Group == GROUP_MATERIAL )
				{
					static FLOAT UTiling = 1, VTiling = 1;
					static UMaterialInterface* Material = NULL;
					static INT Height = 256, Width = 256;
					static UBOOL bTwoSided = 0, bUnlit = 0, bAlphaTexture = 0, bMasked = 0, bTranslucent = 0;

					// Determine the section and/or extract individual values
					if( StrLine == TEXT("}") )
						Group = GROUP_NONE;
					else if( StrLine.InStr(TEXT("*MATERIAL ")) != -1 )
						Section = SECTION_MATERIAL;
					else if( StrLine.InStr(TEXT("*MATERIAL_WIRE")) != -1 )
					{
						if( StrLine.InStr(TEXT("*MATERIAL_WIRESIZE")) == -1 )
							bTranslucent = 1;
					}
					else if( StrLine.InStr(TEXT("*MATERIAL_TWOSIDED")) != -1 )
					{
						bTwoSided = 1;
					}
					else if( StrLine.InStr(TEXT("*MATERIAL_SELFILLUM")) != -1 )
					{
						INT Pos = StrLine.InStr( TEXT("*") );
						FString NewStr = StrLine.Right( StrLine.Len() - Pos );
						FLOAT temp;
						appSSCANF( *NewStr, TEXT("*MATERIAL_SELFILLUM %f"), &temp );
						if( temp == 100.f || temp == 1.f )
							bUnlit = 1;
					}
					else if( StrLine.InStr(TEXT("*MATERIAL_TRANSPARENCY")) != -1 )
					{
						INT Pos = StrLine.InStr( TEXT("*") );
						FString NewStr = StrLine.Right( StrLine.Len() - Pos );
						FLOAT temp;
						appSSCANF( *NewStr, TEXT("*MATERIAL_TRANSPARENCY %f"), &temp );
						if( temp > 0.f )
							bAlphaTexture = 1;
					}
					else if( StrLine.InStr(TEXT("*MATERIAL_SHADING")) != -1 )
					{
						INT Pos = StrLine.InStr( TEXT("*") );
						FString NewStr = StrLine.Right( StrLine.Len() - Pos );
						TCHAR Buffer[20];
						appSSCANF( *NewStr, TEXT("*MATERIAL_SHADING %s"), Buffer );
						if( !appStrcmp( Buffer, TEXT("Constant") ) )
							bMasked = 1;
					}
					else if( StrLine.InStr(TEXT("*MAP_DIFFUSE")) != -1 )
					{
						Section = SECTION_MAP_DIFFUSE;
						Material = NULL;
						UTiling = VTiling = 1;
						Width = Height = 256;
					}
					else
					{
						if ( Section == SECTION_MATERIAL )
						{
							// We are entering a new material definition.  Allocate a new material header.
							new( ASEMaterialHeaders )FASEMaterialHeader();
							Section = SECTION_NONE;
						}
						else if ( Section == SECTION_MAP_DIFFUSE )
						{
							if( StrLine.InStr(TEXT("*BITMAP")) != -1 )
							{
								// Remove tabs from the front of this string.  The number of tabs differs
								// depending on how many materials are in the file.
								INT Pos = StrLine.InStr( TEXT("*") );
								FString NewStr = StrLine.Right( StrLine.Len() - Pos );

								NewStr = NewStr.Right( NewStr.Len() - NewStr.InStr(TEXT("\\"), -1 ) - 1 );	// Strip off path info
								NewStr = NewStr.Left( NewStr.Len() - 5 );									// Strip off '.bmp"' at the end

								// Find the texture
								Material = NULL;
							}
							else if( StrLine.InStr(TEXT("*UVW_U_TILING")) != -1 )
							{
								INT Pos = StrLine.InStr( TEXT("*") );
								FString NewStr = StrLine.Right( StrLine.Len() - Pos );
								appSSCANF( *NewStr, TEXT("*UVW_U_TILING %f"), &UTiling );
							}
							else if( StrLine.InStr(TEXT("*UVW_V_TILING")) != -1 )
							{
								INT Pos = StrLine.InStr( TEXT("*") );
								FString NewStr = StrLine.Right( StrLine.Len() - Pos );
								appSSCANF( *NewStr, TEXT("*UVW_V_TILING %f"), &VTiling );

								check(ASEMaterialHeaders.Num());
								new( ASEMaterialHeaders(ASEMaterialHeaders.Num()-1).Materials )FASEMaterial(*Name.ToString(), Width, Height, UTiling, VTiling, Material, bTwoSided, bUnlit, bAlphaTexture, bMasked, bTranslucent );

								Section = SECTION_NONE;
								bTwoSided = bUnlit = bAlphaTexture = bMasked = bTranslucent = 0;
							}
						}
					}
				}
				else if ( Group == GROUP_GEOM )
				{
					// Determine the section and/or extract individual values
					if( StrLine == TEXT("}") )
					{
						IgnoreMcdGeometry = 0;
						Group = GROUP_NONE;
					}
					// See if this is an MCD thing
					else if( StrLine.InStr(TEXT("*NODE_NAME")) != -1 )
					{
						TCHAR NodeName[512];                     
						appSSCANF( Str, TEXT("\t*NODE_NAME \"%s\""), NodeName );
						if( appStrstr(NodeName, TEXT("MCD")) == NodeName )
							IgnoreMcdGeometry = 1;
						else 
							IgnoreMcdGeometry = 0;
					}

					// Now do nothing if it's an MCD Geom
					if( !IgnoreMcdGeometry )
					{              
						if( StrLine.InStr(TEXT("*MESH_NUMVERTEX")) != -1 )
							appSSCANF( Str, TEXT("\t\t*MESH_NUMVERTEX %d"), &NumVertex );
						else if( StrLine.InStr(TEXT("*MESH_NUMFACES")) != -1 )
							appSSCANF( Str, TEXT("\t\t*MESH_NUMFACES %d"), &NumFaces );
						else if( StrLine.InStr(TEXT("*MESH_VERTEX_LIST")) != -1 )
							Section = SECTION_VERTS;
						else if( StrLine.InStr(TEXT("*MESH_FACE_LIST")) != -1 )
							Section = SECTION_FACES;
						else if( StrLine.InStr(TEXT("*MESH_NUMTVERTEX")) != -1 )
							appSSCANF( Str, TEXT("\t\t*MESH_NUMTVERTEX %d"), &NumTVertex );
						else if( StrLine == TEXT("\t\t*MESH_TVERTLIST {") )
							Section = SECTION_TVERTS;
						else if( StrLine.InStr(TEXT("*MESH_NUMTVFACES")) != -1 )
							appSSCANF( Str, TEXT("\t\t*MESH_NUMTVFACES %d"), &NumTFaces );
						else if( StrLine.InStr(TEXT("*MATERIAL_REF")) != -1 )
							appSSCANF( Str, TEXT("\t*MATERIAL_REF %d"), &ASEMaterialRef );
						else if( StrLine == TEXT("\t\t*MESH_TFACELIST {") )
							Section = SECTION_TFACES;
						else
						{
							// Extract data specific to sections
							if( Section == SECTION_VERTS )
							{
								if( StrLine.InStr(TEXT("\t\t}")) != -1 )
									Section = SECTION_NONE;
								else
								{
									INT temp;
									FVector vtx;
									appSSCANF( Str, TEXT("\t\t\t*MESH_VERTEX    %d\t%f\t%f\t%f"),
										&temp, &vtx.X, &vtx.Y, &vtx.Z );
									new(Vertex)FVector(vtx);
								}
							}
							else if( Section == SECTION_FACES )
							{
								if( StrLine.InStr(TEXT("\t\t}")) != -1 )
									Section = SECTION_NONE;
								else
								{
									INT temp, idx1, idx2, idx3;
									appSSCANF( Str, TEXT("\t\t\t*MESH_FACE %d:    A: %d B: %d C: %d"),
										&temp, &idx1, &idx2, &idx3 );
									new(FaceIdx)INT(idx1);
									new(FaceIdx)INT(idx2);
									new(FaceIdx)INT(idx3);

									// Determine the right  part of StrLine which contains the smoothing group(s).
									FString SmoothTag(TEXT("*MESH_SMOOTHING"));
									INT SmGroupsLocation = StrLine.InStr( SmoothTag );
									if( SmGroupsLocation != -1 )
									{
										FString	SmoothingString;
										DWORD	SmoothingMask = 0;

										SmoothingString = StrLine.Right( StrLine.Len() - SmGroupsLocation - SmoothTag.Len() );

										while(SmoothingString.Len())
										{
											INT	Length = SmoothingString.InStr(TEXT(",")),
												SmoothingGroup = (Length != -1) ? appAtoi(*SmoothingString.Left(Length)) : appAtoi(*SmoothingString);

											if(SmoothingGroup <= 32)
												SmoothingMask |= (1 << (SmoothingGroup - 1));

											SmoothingString = (Length != -1) ? SmoothingString.Right(SmoothingString.Len() - Length - 1) : TEXT("");
										};

										SmoothingGroups.AddItem(SmoothingMask);
									}
									else
										SmoothingGroups.AddItem(0);

									// Sometimes "MESH_SMOOTHING" is a blank instead of a number, so we just grab the 
									// part of the string we need and parse out the material id.
									INT MaterialID;
									StrLine = StrLine.Right( StrLine.Len() - StrLine.InStr( TEXT("*MESH_MTLID"), -1 ) - 1 );
									appSSCANF( *StrLine , TEXT("MESH_MTLID %d"), &MaterialID );
									new(FaceMaterialsIdx)INT(MaterialID);
								}
							}
							else if( Section == SECTION_TVERTS )
							{
								if( StrLine.InStr(TEXT("\t\t}")) != -1 )
									Section = SECTION_NONE;
								else
								{
									INT temp;
									FVector vtx;
									appSSCANF( Str, TEXT("\t\t\t*MESH_TVERT %d\t%f\t%f"),
										&temp, &vtx.X, &vtx.Y );
									vtx.Z = 0;
									new(TexCoord)FVector(vtx);
								}
							}
							else if( Section == SECTION_TFACES )
							{
								if( StrLine == TEXT("\t\t}") )
									Section = SECTION_NONE;
								else
								{
									INT temp, idx1, idx2, idx3;
									appSSCANF( Str, TEXT("\t\t\t*MESH_TFACE %d\t%d\t%d\t%d"),
										&temp, &idx1, &idx2, &idx3 );
									new(FaceTexCoordIdx)INT(idx1);
									new(FaceTexCoordIdx)INT(idx2);
									new(FaceTexCoordIdx)INT(idx3);
								}
							}
						}
					}
				}
			}

			// Create the polys from the gathered info.
			if( FaceIdx.Num() != FaceTexCoordIdx.Num() )
			{
				appMsgf( AMT_OK, LocalizeSecure(LocalizeUnrealEd("Error_48"), FaceIdx.Num(), FaceTexCoordIdx.Num()));
				continue;
			}
			if( ASEMaterialRef == -1 )
			{
				appMsgf( AMT_OK, *LocalizeUnrealEd("Error_49") );
			}
			for( INT x = 0 ; x < FaceIdx.Num() ; x += 3 )
			{
				Poly.Init();
				new(Poly.Vertices) FVector(Vertex( FaceIdx(x) ));
				new(Poly.Vertices) FVector(Vertex( FaceIdx(x+1) ));
				new(Poly.Vertices) FVector(Vertex( FaceIdx(x+2) ));

				FASEMaterial ASEMaterial;
				if( ASEMaterialRef != -1 )
					if( ASEMaterialHeaders(ASEMaterialRef).Materials.Num() )
						if( ASEMaterialHeaders(ASEMaterialRef).Materials.Num() == 1 )
							ASEMaterial = ASEMaterialHeaders(ASEMaterialRef).Materials(0);
						else
						{
							// Sometimes invalid material references appear in the ASE file.  We can't do anything about
							// it, so when that happens just use the first material.
							if( FaceMaterialsIdx(x/3) >= ASEMaterialHeaders(ASEMaterialRef).Materials.Num() )
								ASEMaterial = ASEMaterialHeaders(ASEMaterialRef).Materials(0);
							else
								ASEMaterial = ASEMaterialHeaders(ASEMaterialRef).Materials( FaceMaterialsIdx(x/3) );
						}

				if( ASEMaterial.Material )
					Poly.Material = ASEMaterial.Material;

				Poly.SmoothingMask = SmoothingGroups(x / 3);

				Poly.Finalize(NULL,1);

				// The brushes come in flipped across the X axis, so adjust for that.
				FVector Flip(1,-1,1);
				Poly.Vertices(0) *= Flip;
				Poly.Vertices(1) *= Flip;
				Poly.Vertices(2) *= Flip;

				FVector	ST1 = TexCoord(FaceTexCoordIdx(x + 0)),
						ST2 = TexCoord(FaceTexCoordIdx(x + 1)),
						ST3 = TexCoord(FaceTexCoordIdx(x + 2));

				FTexCoordsToVectors(
					Poly.Vertices(0),
					FVector(ST1.X * ASEMaterial.Width * ASEMaterial.UTiling,(1.0f - ST1.Y) * ASEMaterial.Height * ASEMaterial.VTiling,ST1.Z),
					Poly.Vertices(1),
					FVector(ST2.X * ASEMaterial.Width * ASEMaterial.UTiling,(1.0f - ST2.Y) * ASEMaterial.Height * ASEMaterial.VTiling,ST2.Z),
					Poly.Vertices(2),
					FVector(ST3.X * ASEMaterial.Width * ASEMaterial.UTiling,(1.0f - ST3.Y) * ASEMaterial.Height * ASEMaterial.VTiling,ST3.Z),
					&Poly.Base,
					&Poly.TextureU,
					&Poly.TextureV
					);

				Poly.Reverse();
				Poly.CalcNormal();

				new(Polys->Element)FPoly(Poly);
			}

			debugf( NAME_Log, TEXT("Imported %i vertices, %i faces"), NumVertex, NumFaces );
		}
		//
		//
		// T3D FORMAT
		//
		//
		else if( GetBEGIN(&Str,TEXT("POLYGON")) )
		{
			// Init to defaults and get group/item and texture.
			Poly.Init();
			Parse( Str, TEXT("LINK="), Poly.iLink );
			Parse( Str, TEXT("ITEM="), Poly.ItemName );
			Parse( Str, TEXT("FLAGS="), Poly.PolyFlags );
			Parse( Str, TEXT("SHADOWMAPSCALE="), Poly.ShadowMapScale );
			Parse( Str, TEXT("LIGHTINGCHANNELS="), Poly.LightingChannels );
			Poly.PolyFlags &= ~PF_NoImport;

			FString TextureName;
			// only load the texture if it was present
			if (Parse( Str, TEXT("TEXTURE="), TextureName ))
			{
				Poly.Material = Cast<UMaterialInterface>(StaticFindObject( UMaterialInterface::StaticClass(), ANY_PACKAGE, *TextureName ) );
/***
				if (Poly.Material == NULL)
				{
					Poly.Material = Cast<UMaterialInterface>(StaticLoadObject( UMaterialInterface::StaticClass(), NULL, *TextureName, NULL,  LOAD_NoWarn, NULL ) );
				}
***/
			}
		}
		else if( ParseCommand(&Str,TEXT("PAN")) )
		{
			INT	PanU = 0,
				PanV = 0;

			Parse( Str, TEXT("U="), PanU );
			Parse( Str, TEXT("V="), PanV );

			Poly.Base += Poly.TextureU * PanU;
			Poly.Base += Poly.TextureV * PanV;
		}
		else if( ParseCommand(&Str,TEXT("ORIGIN")) )
		{
			GotBase=1;
			GetFVECTOR( Str, Poly.Base );
		}
		else if( ParseCommand(&Str,TEXT("VERTEX")) )
		{
			FVector TempVertex;
			GetFVECTOR( Str, TempVertex );
			new(Poly.Vertices) FVector(TempVertex);
		}
		else if( ParseCommand(&Str,TEXT("TEXTUREU")) )
		{
			GetFVECTOR( Str, Poly.TextureU );
		}
		else if( ParseCommand(&Str,TEXT("TEXTUREV")) )
		{
			GetFVECTOR( Str, Poly.TextureV );
		}
		else if( GetEND(&Str,TEXT("POLYGON")) )
		{
			if( !GotBase )
				Poly.Base = Poly.Vertices(0);
			if( Poly.Finalize(NULL,1)==0 )
				new(Polys->Element)FPoly(Poly);
			GotBase=0;
		}
	}

	// Success.
	return Polys;
}
IMPLEMENT_CLASS(UPolysFactory);

/*-----------------------------------------------------------------------------
	UModelFactory.
-----------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UModelFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UModel::StaticClass();
	new(Formats)FString(TEXT("t3d;Unreal model text"));
	bCreateNew = FALSE;
	bText = TRUE;
}
UModelFactory::UModelFactory()
{
}
UObject* UModelFactory::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
	ABrush* TempOwner = (ABrush*)Context;
	UModel* Model = new( InParent, Name, Flags )UModel( TempOwner, 1 );

	const TCHAR* StrPtr;
	FString StrLine;
	if( TempOwner )
	{
		TempOwner->InitPosRotScale();
		GEditor->GetSelectedActors()->Deselect( TempOwner );
		TempOwner->bTempEditor = 0;
	}
	while( ParseLine( &Buffer, StrLine ) )
	{
		StrPtr = *StrLine;
		if( GetEND(&StrPtr,TEXT("BRUSH")) )
		{
			break;
		}
		else if( GetBEGIN (&StrPtr,TEXT("POLYLIST")) )
		{
			UPolysFactory* PolysFactory = new UPolysFactory;
			Model->Polys = (UPolys*)PolysFactory->FactoryCreateText(UPolys::StaticClass(),Model,NAME_None,0,NULL,Type,Buffer,BufferEnd,Warn);
			check(Model->Polys);
		}
		if( TempOwner )
		{
			if      (ParseCommand(&StrPtr,TEXT("PREPIVOT"	))) GetFVECTOR 	(StrPtr,TempOwner->PrePivot);
			else if (ParseCommand(&StrPtr,TEXT("LOCATION"	))) GetFVECTOR	(StrPtr,TempOwner->Location);
			else if (ParseCommand(&StrPtr,TEXT("ROTATION"	))) GetFROTATOR  (StrPtr,TempOwner->Rotation,1);
			if( ParseCommand(&StrPtr,TEXT("SETTINGS")) )
			{
				Parse( StrPtr, TEXT("CSG="), TempOwner->CsgOper );
				Parse( StrPtr, TEXT("POLYFLAGS="), TempOwner->PolyFlags );
			}
		}
	}

	return Model;
}
IMPLEMENT_CLASS(UModelFactory);

/*-----------------------------------------------------------------------------
	USoundTTSFactory.
-----------------------------------------------------------------------------*/

void CreateSoundCue( USoundNodeWave* Sound, UObject* InParent, EObjectFlags Flags, UBOOL bIncludeAttenuationNode, FLOAT CueVolume )
{
	// then first create the actual sound cue
	FString SoundCueName = FString::Printf( TEXT( "%sCue" ), *Sound->GetName() );

	// Create sound cue.
	USoundCue* SoundCue = ConstructObject<USoundCue>( USoundCue::StaticClass(), InParent, *SoundCueName, Flags );

	// Create new editor data struct and add to map in SoundCue.
	FSoundNodeEditorData WaveEdData;
	appMemset( &WaveEdData, 0, sizeof( FSoundNodeEditorData ) );
	WaveEdData.NodePosX = 200;
	SoundCue->EditorData.Set( Sound, WaveEdData );

	// Apply the initial volume.
	SoundCue->VolumeMultiplier = CueVolume;

	if( bIncludeAttenuationNode )
	{
		USoundNode* AttenuationNode = ConstructObject<USoundNode>( USoundNodeAttenuation::StaticClass(), SoundCue, NAME_None );

		// If this node allows >0 children but by default has zero - create a connector for starters
		if( ( AttenuationNode->GetMaxChildNodes() > 0 || AttenuationNode->GetMaxChildNodes() == -1 ) && AttenuationNode->ChildNodes.Num() == 0 )
		{
			AttenuationNode->CreateStartingConnectors();
		}

		// Create new editor data struct and add to map in SoundCue.
		FSoundNodeEditorData AttenuationEdData;
		appMemset( &AttenuationEdData, 0, sizeof( FSoundNodeEditorData ) );
		AttenuationEdData.NodePosX = 50;
		SoundCue->EditorData.Set( AttenuationNode, AttenuationEdData );

		// Link the attenuation node to the wave.
		AttenuationNode->ChildNodes( 0 ) = Sound;

		// Link the attenuation node to root.
		SoundCue->FirstNode = AttenuationNode;
	}
	else
	{
		// Link the wave to root.
		SoundCue->FirstNode = Sound;
	}
}

void USoundTTSFactory::StaticConstructor( void )
{
	new( GetClass(), TEXT( "bUseTTS" ), RF_Public ) UBoolProperty( CPP_PROPERTY( bUseTTS ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "SpokenText" ), RF_Public ) UStrProperty( CPP_PROPERTY( SpokenText ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "bAutoCreateCue" ), RF_Public ) UBoolProperty( CPP_PROPERTY( bAutoCreateCue ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "bIncludeAttenuationNode" ), RF_Public ) UBoolProperty( CPP_PROPERTY( bIncludeAttenuationNode ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "CueVolume" ), RF_Public ) UFloatProperty( CPP_PROPERTY( CueVolume ), TEXT( "" ), CPF_Edit );
	new( GetClass()->HideCategories ) FName( TEXT( "Object" ) );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundTTSFactory::InitializeIntrinsicPropertyValues( void )
{
	SupportedClass = USoundNodeWave::StaticClass();
	bCreateNew = TRUE;
	bEditAfterNew = FALSE;
	bAutoCreateCue = TRUE;
	bIncludeAttenuationNode = TRUE;
	bUseTTS = TRUE;
	CueVolume = 0.75f;
	Description = TEXT( "SoundNodeWaveTTS" );
}

UObject* USoundTTSFactory::FactoryCreateNew( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	USoundNodeWave* SoundNodeWave = CastChecked<USoundNodeWave>( StaticConstructObject( InClass, InParent, InName, Flags ) );

	SoundNodeWave->bUseTTS = bUseTTS;
	SoundNodeWave->SpokenText = SpokenText;

	if( bAutoCreateCue )
	{
		CreateSoundCue( SoundNodeWave, InParent, Flags, bIncludeAttenuationNode, CueVolume );
	}

	return( SoundNodeWave );
}

IMPLEMENT_CLASS( USoundTTSFactory );

/*-----------------------------------------------------------------------------
	USoundFactory.
-----------------------------------------------------------------------------*/

void USoundFactory::StaticConstructor( void )
{
	new( GetClass(), TEXT( "bAutoCreateCue" ), RF_Public ) UBoolProperty( CPP_PROPERTY( bAutoCreateCue ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "bIncludeAttenuationNode" ), RF_Public ) UBoolProperty( CPP_PROPERTY( bIncludeAttenuationNode ), TEXT( "" ), CPF_Edit );
	new( GetClass(), TEXT( "CueVolume" ), RF_Public ) UFloatProperty( CPP_PROPERTY( CueVolume ), TEXT( "" ), CPF_Edit );
	new( GetClass()->HideCategories ) FName( TEXT( "Object" ) );
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundFactory::InitializeIntrinsicPropertyValues( void )
{
	SupportedClass = USoundNodeWave::StaticClass();
	new( Formats ) FString( TEXT( "wav;Sound" ) );
	bCreateNew = FALSE;
	bAutoCreateCue = FALSE;
	bIncludeAttenuationNode = TRUE;
	CueVolume = 0.75f;
}

USoundFactory::USoundFactory( void )
{
	bEditorImport = TRUE;
}

UObject* USoundFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		FileType,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	if(	appStricmp( FileType, TEXT( "WAV" ) ) == 0 )
	{
		// if the sound already exists, remember the user settings
		USoundNodeWave* ExistingSound = FindObject<USoundNodeWave>( InParent, *Name.ToString() );

		UBOOL bUseExistingSettings = FALSE;
		BITFIELD bExistingAlwaysLocalise = 0;
		BITFIELD bExistingManualWordWrap = 0;
		BITFIELD bExistingMature = 0;
		BITFIELD bExistingForceRealtimeDecompression = 0;
		BITFIELD bExistingUseTTS = 0;
		BYTE ExistingTTSSpeaker = 0;
		INT ExistingCompressionQuality = 0;
		FString ExistingComment;
		FString ExistingSpokenText;
		TArray<FSubtitleCue> ExistingSubtitles;

		if( ExistingSound )
		{
			// Prompt the user for what to do if a 'To All' response wasn't already given.
			if( OverwriteYesOrNoToAllState != ART_YesAll && OverwriteYesOrNoToAllState != ART_NoAll )
			{
				OverwriteYesOrNoToAllState = appMsgf( AMT_YesNoYesAllNoAllCancel, LocalizeSecure( LocalizeUnrealEd( "ImportedSoundAlreadyExists_F" ), *Name.ToString() ) );
			}

			switch( OverwriteYesOrNoToAllState )
			{

			case ART_Yes:
			case ART_YesAll:
				{
					// Overwrite existing settings
					bUseExistingSettings = FALSE;
					break;
				}
			case ART_No:
			case ART_NoAll:
				{
					// Preserve existing settings
					bUseExistingSettings = TRUE;
					bExistingManualWordWrap = ExistingSound->bManualWordWrap;
					bExistingMature = ExistingSound->bMature;
					bExistingForceRealtimeDecompression = ExistingSound->bForceRealtimeDecompression;
					bExistingUseTTS = ExistingSound->bUseTTS;
					ExistingTTSSpeaker = ExistingSound->TTSSpeaker;
					ExistingCompressionQuality = ExistingSound->CompressionQuality;
					ExistingComment = ExistingSound->Comment;
					ExistingSpokenText = ExistingSound->SpokenText;

					for( INT SubtitleIndex = 0; SubtitleIndex < ExistingSound->Subtitles.Num(); ++SubtitleIndex )
					{
						ExistingSubtitles.AddItem( ExistingSound->Subtitles( SubtitleIndex ) );
					}

					break;
				}
			case wxCANCEL:
			default:
				{
					return NULL;
				}
			}
		}

		// Create new sound and import raw data.
		USoundNodeWave* Sound = new( InParent, Name, Flags ) USoundNodeWave;

		TArray<BYTE> RawWaveData;
		RawWaveData.Empty( BufferEnd - Buffer );
		RawWaveData.Add( BufferEnd - Buffer );
		appMemcpy( RawWaveData.GetData(), Buffer, RawWaveData.Num() );

		// Compressed data is now out of date.
		Sound->CompressedPCData.RemoveBulkData();
		Sound->CompressedXbox360Data.RemoveBulkData();
		Sound->CompressedPS3Data.RemoveBulkData();
		Sound->NumChannels = 0;

		Sound->RawData.Lock( LOCK_READ_WRITE );
		void* LockedData = Sound->RawData.Realloc( BufferEnd-Buffer );		
		appMemcpy( LockedData, Buffer, BufferEnd-Buffer ); 
		Sound->RawData.Unlock();
		
		// Calculate duration.
		FWaveModInfo WaveInfo;
		if( WaveInfo.ReadWaveInfo( RawWaveData.GetTypedData(), RawWaveData.Num() ) )
		{
			INT DurationDiv = *WaveInfo.pChannels * *WaveInfo.pBitsPerSample * *WaveInfo.pSamplesPerSec;  
			if( DurationDiv ) 
			{
				Sound->Duration = *WaveInfo.pWaveDataSize * 8.0f / DurationDiv;
			}
			else
			{
				Sound->Duration = 0.0f;
			}

			if( *WaveInfo.pBitsPerSample != 16 )
			{
				Warn->Logf( NAME_Error, TEXT( "Currently, only 16 bit WAV files are supported." ) );
				return NULL;
			}

			if( *WaveInfo.pChannels != 1 && *WaveInfo.pChannels != 2 )
			{
				Warn->Logf( NAME_Error, TEXT( "Currently, only mono or stereo WAV files are supported." ) );
				return NULL;
			}
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT( "Bad wave file '%s'" ), *Name.ToString() );
			return NULL;
		}

		// Copy over the settings of the pre-existing sound if it exists
		if( ExistingSound && bUseExistingSettings )
		{
			for( INT SubtitleIndex = 0; SubtitleIndex < ExistingSubtitles.Num(); ++SubtitleIndex )
			{
				Sound->Subtitles.AddItem( ExistingSubtitles( SubtitleIndex ) );
			}

			Sound->bManualWordWrap = bExistingManualWordWrap;
			Sound->bMature = bExistingMature;
			Sound->Comment = ExistingComment;
			Sound->CompressionQuality = ExistingCompressionQuality;
			Sound->bForceRealtimeDecompression = bExistingForceRealtimeDecompression;
			Sound->bUseTTS = bExistingUseTTS;
			Sound->SpokenText = ExistingSpokenText;
			Sound->TTSSpeaker = ExistingTTSSpeaker;

			// Call PostEditChange() to update text to speech
			Sound->PostEditChange( NULL );
		}

		// if we're auto creating a default cue
		if( bAutoCreateCue )
		{
			CreateSoundCue( Sound, InParent, Flags, bIncludeAttenuationNode, CueVolume );
		}
		
		return Sound;
	}
	else
	{
		// Unrecognized.
		Warn->Logf( NAME_Error, TEXT("Unrecognized sound format '%s' in %s"), FileType, *Name.ToString() );
		return NULL;
	}
}

IMPLEMENT_CLASS( USoundFactory );

/*-----------------------------------------------------------------------------
	USoundSurroundFactory.
-----------------------------------------------------------------------------*/

void USoundSurroundFactory::StaticConstructor()
{
	new( GetClass(), TEXT( "CueVolume" ), RF_Public ) UFloatProperty( CPP_PROPERTY( CueVolume ), TEXT( "" ), CPF_Edit );
	new( GetClass()->HideCategories ) FName( TEXT( "Object" ) );
}

const FString USoundSurroundFactory::SpeakerLocations[SPEAKER_Count] =
{
	TEXT( "_fl" ),			// SPEAKER_FrontLeft
	TEXT( "_fr" ),			// SPEAKER_FrontRight
	TEXT( "_fc" ),			// SPEAKER_FrontCenter
	TEXT( "_lf" ),			// SPEAKER_LowFrequency
	TEXT( "_sl" ),			// SPEAKER_SideLeft
	TEXT( "_sr" ),			// SPEAKER_SideRight
	TEXT( "_bl" ),			// SPEAKER_BackLeft
	TEXT( "_br" )			// SPEAKER_BackRight
};

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundSurroundFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USoundNodeWave::StaticClass();
	new( Formats ) FString( TEXT( "WAV;Multichannel Sound" ) );
	bCreateNew = FALSE;
	CueVolume = 0.75f;
}

USoundSurroundFactory::USoundSurroundFactory( void )
{
	bEditorImport = TRUE;
}

UBOOL USoundSurroundFactory::FactoryCanImport( const FFilename& Filename )
{
	// Find the root name
	FString RootName = Filename.GetBaseFilename();
	FString SpeakerLocation = RootName.Right( 3 ).ToLower();

	// Find which channel this refers to		
	for( INT SpeakerIndex = 0; SpeakerIndex < SPEAKER_Count; SpeakerIndex++ )
	{
		if( SpeakerLocation == SpeakerLocations[SpeakerIndex] )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

UObject* USoundSurroundFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		FileType,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
 )
{
	INT		SpeakerIndex, i;

	// Only import wavs
	if(	appStricmp( FileType, TEXT( "WAV" ) ) == 0 )
	{
		// Find the root name
		FString RootName = Name.GetNameString();
		FString SpeakerLocation = RootName.Right( 3 ).ToLower();
		FName BaseName = FName( *RootName.LeftChop( 3 ) );

		// Find which channel this refers to		
		for( SpeakerIndex = 0; SpeakerIndex < SPEAKER_Count; SpeakerIndex++ )
		{
			if( SpeakerLocation == SpeakerLocations[SpeakerIndex] )
			{
				break;
			}
		}

		if( SpeakerIndex == SPEAKER_Count )
		{
			Warn->Logf( NAME_Error, TEXT( "Failed to find speaker location; valid extensions are _fl, _fr, _fc, _lf, _sl, _sr, _bl, _br." ) );
			return( NULL );
		}

		// Find existing soundnodewave
		USoundNodeWave* Sound = NULL;
		for( TObjectIterator<USoundNodeWave> It; It; ++It )
		{
			USoundNodeWave* CurrentSound = *It;
			if( CurrentSound->GetFName() == BaseName )
			{
				Sound = CurrentSound;
				break;
			}
		}

		// Create new sound if necessary
		if( Sound == NULL )
		{
			Sound = new( InParent, BaseName, Flags ) USoundNodeWave;
		}

		// Presize the offsets array, in case the sound was new or the original sound data was stripped by cooking.
		if ( Sound->ChannelOffsets.Num() != SPEAKER_Count )
		{
			Sound->ChannelOffsets.Empty( SPEAKER_Count );
			Sound->ChannelOffsets.AddZeroed( SPEAKER_Count );
		}
		// Presize the sizes array, in case the sound was new or the original sound data was stripped by cooking.
		if ( Sound->ChannelSizes.Num() != SPEAKER_Count )
		{
			Sound->ChannelSizes.Empty( SPEAKER_Count );
			Sound->ChannelSizes.AddZeroed( SPEAKER_Count );
		}

		// Compressed data is now out of date.
		Sound->CompressedPCData.RemoveBulkData();
		Sound->CompressedXbox360Data.RemoveBulkData();
		Sound->CompressedPS3Data.RemoveBulkData();

		// Delete the old version of the wave from the bulk data
		BYTE * RawWaveData[SPEAKER_Count] = { NULL };
		BYTE * RawData = ( BYTE * )Sound->RawData.Lock( LOCK_READ_WRITE );
		INT RawDataOffset = 0;
		INT TotalSize = 0;

		// Copy off the still used waves
		for( i = 0; i < SPEAKER_Count; i++ )
		{
			if( i != SpeakerIndex && Sound->ChannelSizes( i ) )
			{
				RawWaveData[i] = new BYTE [Sound->ChannelSizes( i )];
				appMemcpy( RawWaveData[i], RawData + Sound->ChannelOffsets( i ), Sound->ChannelSizes( i ) );
				TotalSize += Sound->ChannelSizes( i );
			}
		}

		// Copy them back without the one that will be updated
		RawData = ( BYTE * )Sound->RawData.Realloc( TotalSize );

		for( i = 0; i < SPEAKER_Count; i++ )
		{
			if( RawWaveData[i] )
			{
				appMemcpy( RawData + RawDataOffset, RawWaveData[i], Sound->ChannelSizes( i ) );
				Sound->ChannelOffsets( i ) = RawDataOffset;
				RawDataOffset += Sound->ChannelSizes( i );

				delete [] RawWaveData[i];
			}
		}

		UINT RawDataSize = BufferEnd - Buffer;
		BYTE* LockedData = ( BYTE * )Sound->RawData.Realloc( RawDataOffset + RawDataSize );		
		LockedData += RawDataOffset;
		appMemcpy( LockedData, Buffer, RawDataSize ); 

		Sound->ChannelOffsets( SpeakerIndex ) = RawDataOffset;
		Sound->ChannelSizes( SpeakerIndex ) = RawDataSize;

		Sound->RawData.Unlock();

		// Calculate duration.
		FWaveModInfo WaveInfo;
		if( WaveInfo.ReadWaveInfo( LockedData, RawDataSize ) )
		{
			// Calculate duration in seconds
			INT DurationDiv = *WaveInfo.pChannels * *WaveInfo.pBitsPerSample * *WaveInfo.pSamplesPerSec;  
			if( DurationDiv ) 
			{
				Sound->Duration = *WaveInfo.pWaveDataSize * 8.0f / DurationDiv;
			}
			else
			{
				Sound->Duration = 0.0f;
			}

			if( *WaveInfo.pBitsPerSample != 16 )
			{
				Warn->Logf( NAME_Error, TEXT( "Currently, only 16 bit WAV files are supported." ) );
				Sound = NULL;
			}

			if( *WaveInfo.pChannels != 1 )
			{
				Warn->Logf( NAME_Error, TEXT( "Currently, only mono WAV files can be imported as channels of surround audio." ) );
				Sound = NULL;
			}
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT( "Bad wave file header '%s'" ), *Name.ToString() );
			Sound = NULL;
		}

		return( Sound );
	}
	else
	{
		// Unrecognized.
		Warn->Logf( NAME_Error, TEXT("Unrecognized sound extension '%s' in %s"), FileType, *Name.ToString() );
	}

	return( NULL );
}

IMPLEMENT_CLASS( USoundSurroundFactory );

/*------------------------------------------------------------------------------
	USoundCueFactoryNew.
------------------------------------------------------------------------------*/

void USoundCueFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundCueFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USoundCue::StaticClass();
	Description	= TEXT("SoundCue");
	bCreateNew = TRUE;
	bEditAfterNew = TRUE;
}

UObject* USoundCueFactoryNew::FactoryCreateNew( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	USoundCue* SoundCue = ConstructObject<USoundCue>( USoundCue::StaticClass(), InParent, Name, Flags );
	return SoundCue;
}

IMPLEMENT_CLASS( USoundCueFactoryNew );

/*-----------------------------------------------------------------------------
	USoundModeFactory.
-----------------------------------------------------------------------------*/

void USoundModeFactory::StaticConstructor( void )
{
	new( GetClass()->HideCategories ) FName( TEXT( "Object" ) );
}

USoundModeFactory::USoundModeFactory( void )
{
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USoundModeFactory::InitializeIntrinsicPropertyValues( void )
{
	SupportedClass = USoundMode::StaticClass();
	Description = TEXT( "SoundMode" );
	bCreateNew = TRUE;
	bEditAfterNew = TRUE;
}

UObject* USoundModeFactory::FactoryCreateNew( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	USoundMode* Mode = ConstructObject<USoundMode>( USoundMode::StaticClass(), InParent, InName, Flags );
	if( GEngine && GEngine->GetAudioDevice() )
	{
		GEngine->GetAudioDevice()->AddMode( Mode );
	}
	return( Mode );
}

IMPLEMENT_CLASS( USoundModeFactory );

/*-----------------------------------------------------------------------------
	UFonixFactory.
-----------------------------------------------------------------------------*/

void UFonixFactory::StaticConstructor( void )
{
	new( GetClass()->HideCategories ) FName( TEXT( "Object" ) );
}

UFonixFactory::UFonixFactory( void )
{
}

/**
* Initializes property values for intrinsic classes.  It is called immediately after the class default object
* is initialized against its archetype, but before any objects of this class are created.
*/
void UFonixFactory::InitializeIntrinsicPropertyValues( void )
{
	SupportedClass = USpeechRecognition::StaticClass();
	Description = TEXT( "SpeechRecogniser" );
	bCreateNew = TRUE;
	bEditAfterNew = TRUE;
}

UObject* UFonixFactory::FactoryCreateNew( UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn )
{
	USpeechRecognition* Recognizer = ConstructObject<USpeechRecognition>( USpeechRecognition::StaticClass(), InParent, InName, Flags );
	return( Recognizer );
}

IMPLEMENT_CLASS( UFonixFactory );

/*------------------------------------------------------------------------------
	ULensFlareFactoryNew.
------------------------------------------------------------------------------*/
void ULensFlareFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void ULensFlareFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= ULensFlare::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("LensFlare");
}

UObject* ULensFlareFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	ULensFlare* NewFlare = ConstructObject<ULensFlare>( ULensFlare::StaticClass(), InParent, Name, Flags );
	return NewFlare;
}

IMPLEMENT_CLASS(ULensFlareFactoryNew);

/*------------------------------------------------------------------------------
	UParticleSystemFactoryNew.
------------------------------------------------------------------------------*/

void UParticleSystemFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UParticleSystemFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UParticleSystem::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("ParticleSystem");
}
UObject* UParticleSystemFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UParticleSystemFactoryNew);

/*------------------------------------------------------------------------------
	UPhysXParticleSystemFactoryNew.
------------------------------------------------------------------------------*/

void UPhysXParticleSystemFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPhysXParticleSystemFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UPhysXParticleSystem::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("PhysXParticleSystem");
}
UObject* UPhysXParticleSystemFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UPhysXParticleSystemFactoryNew);

/*------------------------------------------------------------------------------
	UAnimSetFactoryNew.
------------------------------------------------------------------------------*/

void UAnimSetFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UAnimSetFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UAnimSet::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("AnimSet");
}
UObject* UAnimSetFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UAnimSetFactoryNew);

/*------------------------------------------------------------------------------
	UAnimTreeFactoryNew.
------------------------------------------------------------------------------*/

void UAnimTreeFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UAnimTreeFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UAnimTree::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("AnimTree");
}
UObject* UAnimTreeFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UAnimTreeFactoryNew);

/*------------------------------------------------------------------------------
UPostProcessFactoryNew.
------------------------------------------------------------------------------*/

void UPostProcessFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPostProcessFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UPostProcessChain::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Post Process Effect");
}
UObject* UPostProcessFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UPostProcessFactoryNew);

/*------------------------------------------------------------------------------
	UPhysicalMaterialFactoryNew.
------------------------------------------------------------------------------*/

void UPhysicalMaterialFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UPhysicalMaterialFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass		= UPhysicalMaterial::StaticClass();
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	Description			= TEXT("Physical Material");
}
UObject* UPhysicalMaterialFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UPhysicalMaterialFactoryNew);

/*-----------------------------------------------------------------------------
	UTextureMovieFactory.
-----------------------------------------------------------------------------*/

void UTextureMovieFactory::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);

	UEnum* Enum = new(GetClass(),TEXT("MovieStreamSource"),RF_Public) UEnum(NULL);
	TArray<FName> EnumNames;
	EnumNames.AddItem( FName(TEXT("MovieStream_File")) );
	EnumNames.AddItem( FName(TEXT("MovieStream_Memory")) );
	Enum->SetEnums( EnumNames );

	new(GetClass(),TEXT("MovieStreamSource"),	RF_Public)UByteProperty	(CPP_PROPERTY(MovieStreamSource),	TEXT("Movie"), CPF_Edit, Enum	);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureMovieFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTextureMovie::StaticClass();

	new(Formats)FString(TEXT("bik;Bink Movie"));

	bCreateNew = FALSE;
}
UTextureMovieFactory::UTextureMovieFactory()
{
	bEditorImport = 1;
}
UObject* UTextureMovieFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	UTextureMovie* Movie = NULL;
	UClass* DecoderClass = NULL;

    // Bink codec
	if( appStricmp( Type, TEXT("BIK") ) == 0 )
	{
		DecoderClass = UObject::StaticLoadClass( UCodecMovie::StaticClass(), NULL, TEXT("Engine.CodecMovieBink"), NULL, LOAD_None, NULL );
	}
	else
	{
		// Unknown format.
		Warn->Logf( NAME_Error, TEXT("Bad movie format for movie import") );
 	}

	if( DecoderClass &&
		DecoderClass->GetDefaultObject<UCodecMovie>()->IsSupported() )
	{
		// create the movie texture object
		Movie = CastChecked<UTextureMovie>(StaticConstructObject(Class,InParent,Name,Flags));
		Movie->DecoderClass = DecoderClass;
		// load the lazy array with movie data		
		Movie->Data.Lock(LOCK_READ_WRITE);
		INT Length = BufferEnd - Buffer;
		appMemcpy( Movie->Data.Realloc( Length ), Buffer, Length );
		Movie->Data.Unlock();
		// Invalidate any materials using the newly imported movie texture. (occurs if you import over an existing movie)
		Movie->PostEditChange(NULL);
	}
	return Movie;
}
IMPLEMENT_CLASS(UTextureMovieFactory);

/*-----------------------------------------------------------------------------
	UTextureRenderTargetFactoryNew
-----------------------------------------------------------------------------*/

/** 
 * Constructor (default)
 */
UTextureRenderTargetFactoryNew::UTextureRenderTargetFactoryNew()
{
}

/** 
 * Init class
 */
void UTextureRenderTargetFactoryNew::StaticConstructor()
{
	// hide UObject props
	new(GetClass()->HideCategories) FName(NAME_Object);
	// hide UTexture props	
	new(GetClass()->HideCategories) FName(TEXT("Texture"));

	// enumerate all of the pixel formats 
	UEnum* FormatEnum = new(GetClass(),TEXT("Format"),RF_Public) UEnum(NULL);
	TArray<FName> EnumNames;
	for( BYTE Idx=0; Idx < PF_MAX; Idx++ )
	{
		if( FTextureRenderTargetResource::IsSupportedFormat((EPixelFormat)Idx) )
		{
			EnumNames.AddItem( FName(GPixelFormats[Idx].Name) );
		}		
	}	
	FormatEnum->SetEnums( EnumNames );

	// initialize the properties 
	new(GetClass(),TEXT("Width"),	RF_Public)UIntProperty  (CPP_PROPERTY(Width),	TEXT("Width"), CPF_Edit );
	new(GetClass(),TEXT("Height"),	RF_Public)UIntProperty  (CPP_PROPERTY(Height),	TEXT("Height"), CPF_Edit );
	new(GetClass(),TEXT("Format"),	RF_Public)UByteProperty	(CPP_PROPERTY(Format),	TEXT("Format"), CPF_Edit, FormatEnum	);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureRenderTargetFactoryNew::InitializeIntrinsicPropertyValues()
{
	// class type that will be created by this factory 
	SupportedClass		= UTextureRenderTarget2D::StaticClass();
	// only allow creation since a render target can't be imported
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	// don't allow importing
	bEditorImport		= 0;
	// textual description of the supported class type
	Description			= TEXT("RenderToTexture");

	// set default width/height/format
	Width = 256;
	Height = 256;
	Format = 0;
}
/** 
 * Create a new object of the supported type and return it
 */
UObject* UTextureRenderTargetFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	// create the new object
	UTextureRenderTarget2D* Result = CastChecked<UTextureRenderTarget2D>( StaticConstructObject(Class,InParent,Name,Flags) );
	// array of allowed formats
	TArray<BYTE> AllowedFormats;
	for( BYTE Idx=0; Idx < PF_MAX; Idx++ ) 
	{
		if( FTextureRenderTargetResource::IsSupportedFormat((EPixelFormat)Idx) ) 
		{
			AllowedFormats.AddItem(Idx);			
		}		
	}
	// initialize the resource
	Result->Init( Width, Height, (EPixelFormat)AllowedFormats(Format) );
	return( Result );
}

IMPLEMENT_CLASS(UTextureRenderTargetFactoryNew);

/*-----------------------------------------------------------------------------
	UTextureRenderTargetCubeFactoryNew
-----------------------------------------------------------------------------*/

/** 
 * Constructor (default)
 */
UTextureRenderTargetCubeFactoryNew::UTextureRenderTargetCubeFactoryNew()
{
}

/** 
 * Init class and set defaults
 */
void UTextureRenderTargetCubeFactoryNew::StaticConstructor()
{
	// hide UObject props
	new(GetClass()->HideCategories) FName(NAME_Object);
	// hide UTexture props	
	new(GetClass()->HideCategories) FName(TEXT("Texture"));

	// enumerate all of the pixel formats 
	UEnum* FormatEnum = new(GetClass(),TEXT("Format"),RF_Public) UEnum(NULL);
	TArray<FName> EnumNames;
	for( BYTE Idx=0; Idx < PF_MAX; Idx++ )
	{
		if( FTextureRenderTargetResource::IsSupportedFormat((EPixelFormat)Idx) )
		{
			EnumNames.AddItem( FName(GPixelFormats[Idx].Name) );
		}		
	}	
	FormatEnum->SetEnums( EnumNames );

	// initialize the properties 
	new(GetClass(),TEXT("Width"),	RF_Public)UIntProperty  (CPP_PROPERTY(Width),	TEXT("Width"), CPF_Edit );
	new(GetClass(),TEXT("Format"),	RF_Public)UByteProperty	(CPP_PROPERTY(Format),	TEXT("Format"), CPF_Edit, FormatEnum	);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureRenderTargetCubeFactoryNew::InitializeIntrinsicPropertyValues()
{
	// class type that will be created by this factory 
	SupportedClass		= UTextureRenderTargetCube::StaticClass();
	// only allow creation since a render target can't be imported
	bCreateNew			= TRUE;
	bEditAfterNew		= TRUE;
	// don't allow importing
	bEditorImport		= 0;
	// textual description of the supported class type
	Description			= TEXT("RenderToTextureCube");

	// set default width/format
	Width = 256;
	Format = 0;
}
/** 
 * Create a new object of the supported type and return it
 */
UObject* UTextureRenderTargetCubeFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	// create the new object
	UTextureRenderTargetCube* Result = CastChecked<UTextureRenderTargetCube>( StaticConstructObject(Class,InParent,Name,Flags) );
	// array of allowed formats
	TArray<BYTE> AllowedFormats;
	for( BYTE Idx=0; Idx < PF_MAX; Idx++ ) 
	{
		if( FTextureRenderTargetResource::IsSupportedFormat((EPixelFormat)Idx) ) 
		{
			AllowedFormats.AddItem(Idx);			
		}		
	}
	// initialize the resource
	Result->Init( Width, (EPixelFormat)AllowedFormats(Format) );
	return( Result );
}

IMPLEMENT_CLASS(UTextureRenderTargetCubeFactoryNew);


/*-----------------------------------------------------------------------------
	UTextureFactory.
-----------------------------------------------------------------------------*/

// .PCX file header.
#pragma pack(push,1)
class FPCXFileHeader
{
public:
	BYTE	Manufacturer;		// Always 10.
	BYTE	Version;			// PCX file version.
	BYTE	Encoding;			// 1=run-length, 0=none.
	BYTE	BitsPerPixel;		// 1,2,4, or 8.
	WORD	XMin;				// Dimensions of the image.
	WORD	YMin;				// Dimensions of the image.
	WORD	XMax;				// Dimensions of the image.
	WORD	YMax;				// Dimensions of the image.
	WORD	XDotsPerInch;		// Horizontal printer resolution.
	WORD	YDotsPerInch;		// Vertical printer resolution.
	BYTE	OldColorMap[48];	// Old colormap info data.
	BYTE	Reserved1;			// Must be 0.
	BYTE	NumPlanes;			// Number of color planes (1, 3, 4, etc).
	WORD	BytesPerLine;		// Number of bytes per scanline.
	WORD	PaletteType;		// How to interpret palette: 1=color, 2=gray.
	WORD	HScreenSize;		// Horizontal monitor size.
	WORD	VScreenSize;		// Vertical monitor size.
	BYTE	Reserved2[54];		// Must be 0.
	friend FArchive& operator<<( FArchive& Ar, FPCXFileHeader& H )
	{
		Ar << H.Manufacturer << H.Version << H.Encoding << H.BitsPerPixel;
		Ar << H.XMin << H.YMin << H.XMax << H.YMax << H.XDotsPerInch << H.YDotsPerInch;
		for( INT i=0; i<ARRAY_COUNT(H.OldColorMap); i++ )
			Ar << H.OldColorMap[i];
		Ar << H.Reserved1 << H.NumPlanes;
		Ar << H.BytesPerLine << H.PaletteType << H.HScreenSize << H.VScreenSize;
		for( INT i=0; i<ARRAY_COUNT(H.Reserved2); i++ )
			Ar << H.Reserved2[i];
		return Ar;
	}
};

struct FTGAFileHeader
{
	BYTE IdFieldLength;
	BYTE ColorMapType;
	BYTE ImageTypeCode;		// 2 for uncompressed RGB format
	WORD ColorMapOrigin;
	WORD ColorMapLength;
	BYTE ColorMapEntrySize;
	WORD XOrigin;
	WORD YOrigin;
	WORD Width;
	WORD Height;
	BYTE BitsPerPixel;
	BYTE ImageDescriptor;
	friend FArchive& operator<<( FArchive& Ar, FTGAFileHeader& H )
	{
		Ar << H.IdFieldLength << H.ColorMapType << H.ImageTypeCode;
		Ar << H.ColorMapOrigin << H.ColorMapLength << H.ColorMapEntrySize;
		Ar << H.XOrigin << H.YOrigin << H.Width << H.Height << H.BitsPerPixel;
		Ar << H.ImageDescriptor;
		return Ar;
	}
};

struct FTGAFileFooter
{
	DWORD ExtensionAreaOffset;
	DWORD DeveloperDirectoryOffset;
	BYTE Signature[16];
	BYTE TrailingPeriod;
	BYTE NullTerminator;
};

struct FDDSFileHeader
{
	DWORD				Magic;
	DDSURFACEDESC2		desc;
};

struct FPSDFileHeader
{                                                           
	INT     Signature;      // 8BPS
	SWORD   Version;        // Version
	SWORD   nChannels;      // Number of Channels (3=RGB) (4=RGBA)
	INT     Height;         // Number of Image Rows
	INT     Width;          // Number of Image Columns
	SWORD   Depth;          // Number of Bits per Channel
	SWORD   Mode;           // Image Mode (0=Bitmap)(1=Grayscale)(2=Indexed)(3=RGB)(4=CYMK)(7=Multichannel)
	BYTE    Pad[6];         // Padding

	/**
	 * @return Whether file has a valid signature
	 */
	UBOOL IsValid( void )
	{
		// Fail on bad signature
		if (Signature != 0x38425053)
			return FALSE;

		return TRUE;
	}

	/**
	 * @return Whether file has a supported version
	 */
	UBOOL IsSupported( void )
	{
		// Fail on bad version
		if( Version != 1 )
			return FALSE;   
		// Fail on anything other than 3 or 4 channels
		if ((nChannels!=3) && (nChannels!=4))
			return FALSE;
		// Fail on anything other than RGB
		// We can add support for indexed later if needed.
		if (Mode!=3)
			return FALSE;

		return TRUE;
	}
};

#pragma pack(pop)


static UBOOL psd_ReadData( FColor* pOut, const BYTE*& pBuffer, FPSDFileHeader& Info )
{
	const BYTE* pPlane = NULL;
	const BYTE* pRowTable = NULL;
	INT         iPlane;
	SWORD       CompressionType;
	INT         iPixel;
	INT         iRow;
	INT         CompressedBytes;
	INT         iByte;
	INT         Count;
	BYTE        Value;

	// Double check to make sure this is a valid request
	if (!Info.IsValid() || !Info.IsSupported())
	{
		return FALSE;
	}

	CONST BYTE* pCur = pBuffer + sizeof(FPSDFileHeader);
	INT         NPixels = Info.Width * Info.Height;

	INT  ClutSize =  ((INT)pCur[ 0] << 24) +
		((INT)pCur[ 1] << 16) +
		((INT)pCur[ 2] <<  8) +
		((INT)pCur[ 3] <<  0);
	pCur+=4;
	pCur += ClutSize;    

	// Skip Image Resource Section
	INT ImageResourceSize = ((INT)pCur[ 0] << 24) +
		((INT)pCur[ 1] << 16) +
		((INT)pCur[ 2] <<  8) +
		((INT)pCur[ 3] <<  0);
	pCur += 4+ImageResourceSize;

	// Skip Layer and Mask Section
	INT LayerAndMaskSize =  ((INT)pCur[ 0] << 24) +
		((INT)pCur[ 1] << 16) +
		((INT)pCur[ 2] <<  8) +
		((INT)pCur[ 3] <<  0);
	pCur += 4+LayerAndMaskSize;

	// Determine number of bytes per pixel
	INT BytesPerPixel = 3;
	switch( Info.Mode )
	{
	case 2:        
		BytesPerPixel = 1;        
		return FALSE;  // until we support indexed...
		break;
	case 3:
		if( Info.nChannels == 3 )                  
			BytesPerPixel = 3;        
		else                   
			BytesPerPixel = 4;       
		break;
	default:
		return FALSE;
		break;
	}

	// Get Compression Type
	CompressionType = ((INT)pCur[0] <<  8) + ((INT)pCur[1] <<  0);    
	pCur += 2;

	// Uncompressed?
	if( CompressionType == 0 )
	{
		// Loop through the planes
		for( iPlane=0 ; iPlane<Info.nChannels ; iPlane++ )
		{
			INT iWritePlane = iPlane;
			if( iWritePlane > BytesPerPixel-1 ) iWritePlane = BytesPerPixel-1;

			// Move Plane to Image Data
			INT NPixels = Info.Width * Info.Height;
			for( iPixel=0 ; iPixel<NPixels ; iPixel++ )
			{
				pOut[iPixel].R = pCur[ NPixels*0+iPixel ];
				pOut[iPixel].G = pCur[ NPixels*1+iPixel ];
				pOut[iPixel].B = pCur[ NPixels*2+iPixel ];

				if (BytesPerPixel==4)
					pOut[iPixel].A = pCur[ NPixels*3+iPixel ];
				else
					pOut[iPixel].A = 0;                
			}
		}
	}
	// RLE?
	else if( CompressionType == 1 )
	{
		// Setup RowTable
		pRowTable = pCur;
		pCur += Info.nChannels*Info.Height*2;

		// Loop through the planes
		for( iPlane=0 ; iPlane<Info.nChannels ; iPlane++ )
		{
			INT iWritePlane = iPlane;
			if( iWritePlane > BytesPerPixel-1 ) iWritePlane = BytesPerPixel-1;

			// Loop through the rows
			for( iRow=0 ; iRow<Info.Height ; iRow++ )
			{
				// Load a row
				CompressedBytes = (pRowTable[(iPlane*Info.Height+iRow)*2  ] << 8) +
					(pRowTable[(iPlane*Info.Height+iRow)*2+1] << 0);

				// Setup Plane
				pPlane = pCur;
				pCur += CompressedBytes;

				// Decompress Row
				iPixel = 0;
				iByte = 0;
				while( (iPixel < Info.Width) && (iByte < CompressedBytes) )
				{
					SBYTE code = (SBYTE)pPlane[iByte++];

					// Is it a repeat?
					if( code < 0 )
					{
						Count = -(INT)code + 1;
						Value = pPlane[iByte++];
						while( Count-- > 0 )
						{
							INT idx = (iPixel) + (iRow*Info.Width);
							switch(iWritePlane)
							{
							case 0: pOut[idx].R = Value; break;
							case 1: pOut[idx].G = Value; break;
							case 2: pOut[idx].B = Value; break;
							case 3: pOut[idx].A = Value; break;
							}                            
							iPixel++;
						}
					}
					// Must be a literal then
					else
					{
						Count = (INT)code + 1;
						while( Count-- > 0 )
						{
							Value = pPlane[iByte++];
							INT idx = (iPixel) + (iRow*Info.Width);

							switch(iWritePlane)
							{
							case 0: pOut[idx].R = Value; break;
							case 1: pOut[idx].G = Value; break;
							case 2: pOut[idx].B = Value; break;
							case 3: pOut[idx].A = Value; break;
							}  
							iPixel++;
						}
					}
				}

				// Confirm that we decoded the right number of bytes
				check( iByte  == CompressedBytes );
				check( iPixel == Info.Width );
			}
		}
	}
	else
		return FALSE;

	// Success!
	return( TRUE );
}

static void psd_GetPSDHeader( const BYTE* Buffer, FPSDFileHeader& Info )
{
	Info.Signature      =   ((INT)Buffer[ 0] << 24) +
		((INT)Buffer[ 1] << 16) +
		((INT)Buffer[ 2] <<  8) +
		((INT)Buffer[ 3] <<  0);
	Info.Version        =   ((INT)Buffer[ 4] <<  8) +
		((INT)Buffer[ 5] <<  0);
	Info.nChannels      =   ((INT)Buffer[12] <<  8) +
		((INT)Buffer[13] <<  0);
	Info.Height         =   ((INT)Buffer[14] << 24) +
		((INT)Buffer[15] << 16) +
		((INT)Buffer[16] <<  8) +
		((INT)Buffer[17] <<  0);
	Info.Width          =   ((INT)Buffer[18] << 24) +
		((INT)Buffer[19] << 16) +
		((INT)Buffer[20] <<  8) +
		((INT)Buffer[21] <<  0);
	Info.Depth          =   ((INT)Buffer[22] <<  8) +
		((INT)Buffer[23] <<  0);
	Info.Mode           =   ((INT)Buffer[24] <<  8) +
		((INT)Buffer[25] <<  0);
}

UBOOL UTextureFactory::bSuppressImportOverwriteDialog = FALSE;

void UTextureFactory::StaticConstructor()
{
	// This needs to be mirrored in UnTex.h, Texture.uc and UnEdFact.cpp.
	UEnum* Enum = new(GetClass(),TEXT("TextureCompressionSettings"),RF_Public) UEnum(NULL);
	TArray<FName> EnumNames;
	EnumNames.AddItem( FName(TEXT("TC_Default")) );
	EnumNames.AddItem( FName(TEXT("TC_Normalmap")) );
	EnumNames.AddItem( FName(TEXT("TC_Displacementmap")) );
	EnumNames.AddItem( FName(TEXT("TC_NormalmapAlpha")) );
	EnumNames.AddItem( FName(TEXT("TC_Grayscale")) );
	EnumNames.AddItem( FName(TEXT("TC_HighDynamicRange")) );
	Enum->SetEnums( EnumNames );

	new(GetClass()->HideCategories) FName(NAME_Object);
	new(GetClass(),TEXT("NoCompression")			,RF_Public) UBoolProperty(CPP_PROPERTY(NoCompression			),TEXT("Compression"),0							);
	new(GetClass(),TEXT("CompressionNoAlpha")		,RF_Public)	UBoolProperty(CPP_PROPERTY(NoAlpha					),TEXT("Compression"),CPF_Edit					);
	new(GetClass(),TEXT("CompressionSettings")		,RF_Public) UByteProperty(CPP_PROPERTY(CompressionSettings		),TEXT("Compression"),CPF_Edit, Enum			);	
	new(GetClass(),TEXT("DeferCompression")			,RF_Public) UBoolProperty(CPP_PROPERTY(bDeferCompression		),TEXT("Compression"),CPF_Edit					);
	new(GetClass(),TEXT("Create Material?")			,RF_Public) UBoolProperty(CPP_PROPERTY(bCreateMaterial			),TEXT("Compression"),CPF_Edit					);

	// This needs to be mirrored with Texture.uc::TextureGroup
	UEnum* TextureGroupEnum = new(GetClass(),TEXT("LODGroup"),RF_Public) UEnum(NULL);
	EnumNames.Empty();
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_World")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_WorldNormalMap")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_WorldSpecular")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Character")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_CharacterNormalMap")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_CharacterSpecular")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Weapon")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_WeaponNormalMap")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_WeaponSpecular")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Vehicle")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_VehicleNormalMap")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_VehicleSpecular")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Cinematic")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Effects")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_EffectsNotFiltered")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_Skybox")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_UI")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_LightAndShadowMap")) );
	EnumNames.AddItem( FName(TEXT("TEXTUREGROUP_RenderTarget")) );
	TextureGroupEnum->SetEnums( EnumNames );

	// This needs to be mirrored with Material.uc::EBlendMode
	UEnum* BlendEnum = new(GetClass(),TEXT("Blending"),RF_Public) UEnum(NULL);
	EnumNames.Empty();
	EnumNames.AddItem( FName(TEXT("BLEND_Opaque")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Masked")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Translucent")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Additive")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Modulate")) );
	BlendEnum->SetEnums( EnumNames );

	// This needs to be mirrored with Material.uc::EMaterialLightingModel
	UEnum* LightingModelEnum = new(GetClass(),TEXT("LightingModel"),RF_Public) UEnum(NULL);
	EnumNames.Empty();
	EnumNames.AddItem( FName(TEXT("MLM_Phong")) );
	EnumNames.AddItem( FName(TEXT("MLM_NonDirectional")) );
	EnumNames.AddItem( FName(TEXT("MLM_Unlit")) );
	EnumNames.AddItem( FName(TEXT("MLM_Custom")) );
	LightingModelEnum->SetEnums( EnumNames );

	new(GetClass(),TEXT("RGB To Diffuse")			,RF_Public) UBoolProperty(CPP_PROPERTY(bRGBToDiffuse			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("RGB To Emissive")			,RF_Public) UBoolProperty(CPP_PROPERTY(bRGBToEmissive			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Specular")		,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToSpecular			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Emissive")		,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToEmissive			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Opacity")			,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToOpacity			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Opacity Mask")	,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToOpacityMask		),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Two Sided?")				,RF_Public) UBoolProperty(CPP_PROPERTY(bTwoSided				),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Blending")					,RF_Public) UByteProperty(CPP_PROPERTY(Blending					),TEXT("Create Material"),CPF_Edit, BlendEnum	);
	new(GetClass(),TEXT("Lighting Model")			,RF_Public) UByteProperty(CPP_PROPERTY(LightingModel			),TEXT("Create Material"),CPF_Edit, LightingModelEnum	);	
	new(GetClass(),TEXT("LODGroup")					,RF_Public) UByteProperty(CPP_PROPERTY(LODGroup					),TEXT("Create Material"),CPF_Edit, TextureGroupEnum	);
	new(GetClass(),TEXT("FlipBook")					,RF_Public) UBoolProperty(CPP_PROPERTY(bFlipBook				),TEXT("FlipBook"),		  CPF_Edit				);	
	new(GetClass(),TEXT("Dither Mip-maps alpha?")	,RF_Public) UBoolProperty(CPP_PROPERTY(bDitherMipMapAlpha		),TEXT("DitherMipMaps"),	 CPF_Edit			);	

	new(GetClass(),TEXT("Preserve border R")	,RF_Public) UBoolProperty(CPP_PROPERTY(bPreserveBorderR		),TEXT("PreserverBorderR"),	 CPF_Edit			);	
	new(GetClass(),TEXT("Preserve border G")	,RF_Public) UBoolProperty(CPP_PROPERTY(bPreserveBorderG		),TEXT("PreserverBorderG"),	 CPF_Edit			);	
	new(GetClass(),TEXT("Preserve border B")	,RF_Public) UBoolProperty(CPP_PROPERTY(bPreserveBorderB		),TEXT("PreserverBorderB"),	 CPF_Edit			);	
	new(GetClass(),TEXT("Preserve border A")	,RF_Public) UBoolProperty(CPP_PROPERTY(bPreserveBorderA		),TEXT("PreserverBorderA"),	 CPF_Edit			);	
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureFactory::InitializeIntrinsicPropertyValues()
{

	SupportedClass = UTexture2D::StaticClass();
	new(Formats)FString(TEXT("bmp;Texture"));
	new(Formats)FString(TEXT("pcx;Texture"));
	new(Formats)FString(TEXT("tga;Texture"));
	new(Formats)FString(TEXT("float;Texture"));
	new(Formats)FString(TEXT("psd;Texture")); 
	bCreateNew = FALSE;
	bEditorImport = 1;
}
UTextureFactory::UTextureFactory()
{
	bEditorImport = 1;
	bFlipBook = FALSE;
}

UTexture2D* UTextureFactory::CreateTexture( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags )
{
	if (bFlipBook)
	{
		return CastChecked<UTexture2D>(StaticConstructObject(UTextureFlipBook::StaticClass(),InParent,Name,Flags));
	}
	else
	{
		return CastChecked<UTexture2D>(StaticConstructObject(Class,InParent,Name,Flags));
	}
}

/**
* Supresses the dialog box that, when importing over an existing texture, asks if the users wishes to overwrite its settings.
* This is primarily for reimporting textures.
*/
void UTextureFactory::SuppressImportOverwriteDialog()
{
	bSuppressImportOverwriteDialog = TRUE;
}

UObject* UTextureFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	// if the texture already exists, remember the user settings
	UTexture2D* ExistingTexture = FindObject<UTexture2D>( InParent, *Name.ToString() );

	BYTE			ExistingAddressX	= TA_Wrap;
	BYTE			ExistingAddressY	= TA_Wrap;
	UBOOL			ExistingCompressionFullDynamicRange = FALSE;
	BYTE			ExistingFilter		= TF_Linear;
	BYTE			ExistingLODGroup	= TEXTUREGROUP_World;
	BYTE			ExistingCompressionSettings = TC_Default;
	INT				ExistingLODBias		= 0;
	UBOOL			ExistingNeverStream = FALSE;
	UBOOL			ExistingSRGB		= FALSE;
	FLOAT			ExistingUnpackMin[4];
	FLOAT			ExistingUnpackMax[4];

	UBOOL			ExistingNoCompression = FALSE;
	UBOOL			ExistingNoAlpha = FALSE;
	UBOOL			ExistingDeferCompression = FALSE;

	UBOOL 			ExistingDitherMipMapAlpha = FALSE;
	UBOOL 			ExistingPreserveBorderR = FALSE;
	UBOOL 			ExistingPreserveBorderG = FALSE;
	UBOOL 			ExistingPreserveBorderB = FALSE;
	UBOOL 			ExistingPreserveBorderA = FALSE;

	UBOOL bUseExistingSettings = bSuppressImportOverwriteDialog;

	if(ExistingTexture && !bSuppressImportOverwriteDialog)
	{
		// Prompt the user for what to do if a 'To All' response wasn't already given.
		if( OverwriteYesOrNoToAllState != ART_YesAll && OverwriteYesOrNoToAllState != ART_NoAll )
		{
			OverwriteYesOrNoToAllState =
				appMsgf( AMT_YesNoYesAllNoAllCancel,
					*FString::Printf( LocalizeSecure(
						LocalizeUnrealEd( "ImportedTextureAlreadyExists_F" ), *Name.ToString() ) ) );
		}


		switch( OverwriteYesOrNoToAllState )
		{

		case ART_Yes:
		case ART_YesAll:
			{
				// Overwrite existing settings
				bUseExistingSettings = FALSE;
				break;
			}
		case ART_No:
		case ART_NoAll:
			{
				// Preserve existing settings
				bUseExistingSettings = TRUE;
				break;
			}
		case ART_Cancel:
		default:
			{
				return NULL;
			}
		}
	}

	// Don't suppress future textures from checking for overwrites unless the calling code explicitly asks for it
	bSuppressImportOverwriteDialog = FALSE;

	if (ExistingTexture && bUseExistingSettings)
	{
		// save settings
		ExistingAddressX	= ExistingTexture->AddressX;
		ExistingAddressY	= ExistingTexture->AddressY;
		ExistingCompressionFullDynamicRange = ExistingTexture->CompressionFullDynamicRange;
		ExistingCompressionSettings = ExistingTexture->CompressionSettings;
		ExistingFilter		= ExistingTexture->Filter;
		ExistingLODGroup	= ExistingTexture->LODGroup;
		ExistingLODBias		= ExistingTexture->LODBias;
		ExistingNeverStream = ExistingTexture->NeverStream;
		ExistingSRGB		= ExistingTexture->SRGB;
		appMemcpy(ExistingUnpackMin,ExistingTexture->UnpackMin,sizeof(ExistingUnpackMin));
		appMemcpy(ExistingUnpackMax,ExistingTexture->UnpackMax,sizeof(ExistingUnpackMax));

		ExistingNoCompression = ExistingTexture->CompressionNone;
		ExistingNoAlpha = ExistingTexture->CompressionNoAlpha;
		ExistingDeferCompression = ExistingTexture->DeferCompression;

		ExistingDitherMipMapAlpha = ExistingTexture->bDitherMipMapAlpha;
		ExistingPreserveBorderR = ExistingTexture->bPreserveBorderR;
		ExistingPreserveBorderG = ExistingTexture->bPreserveBorderG;
		ExistingPreserveBorderB = ExistingTexture->bPreserveBorderB;
		ExistingPreserveBorderA = ExistingTexture->bPreserveBorderA;
	}

	UTexture2D* Texture = CreateTexture( Class, InParent, Name, Flags );

	const FTGAFileHeader*    TGA   = (FTGAFileHeader *)Buffer;
	const FPCXFileHeader*    PCX   = (FPCXFileHeader *)Buffer;
	const FBitmapFileHeader* bmf   = (FBitmapFileHeader *)(Buffer + 0);
	const FBitmapInfoHeader* bmhdr = (FBitmapInfoHeader *)(Buffer + sizeof(FBitmapFileHeader));
	FPSDFileHeader			 psdhdr;

	// Validate it.
	INT Length = BufferEnd - Buffer;
	
	if (Length > sizeof(FPSDFileHeader))    
		psd_GetPSDHeader( Buffer, psdhdr );

	//
	// FLOAT (raw)
	//
	if( appStricmp( Type, TEXT("FLOAT") ) == 0 )
	{
		const INT	SrcComponents	= 3;
		INT			Dimension		= appCeil( appSqrt( Length / sizeof(FLOAT) / SrcComponents ) );
		INT			SizeX			= Dimension;
		INT			SizeY			= Dimension;

		if( SizeX * SizeY * SrcComponents * sizeof(FLOAT) != Length )
		{
			Warn->Logf( NAME_Error, TEXT("Couldn't figure out texture dimensions") );
			return NULL;
		}

		Texture->Init(SizeX,SizeY,PF_A8R8G8B8);
		Texture->RGBE					= 1;
		Texture->SRGB					= 0;
		Texture->CompressionNoMipmaps	= 1;
		CompressionSettings				= TC_HighDynamicRange;

		const INT	NumTexels			= SizeX * SizeY;
		FColor*		Dst					= (FColor*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
		FVector*	Src					= (FVector*) Buffer;
		
		for( INT y=0; y<SizeY; y++ )
		{
			for( INT x=0; x<SizeX; x++ )
			{
				FLinearColor SrcColor = FLinearColor( Src[(SizeY-y-1)*SizeX+x].X, Src[(SizeY-y-1)*SizeX+x].Y, Src[(SizeY-y-1)*SizeX+x].Z );
				Dst[y*SizeX+x] = SrcColor.ToRGBE();
			}
		}

		Texture->Mips(0).Data.Unlock();
	}
	//
	// BMP
	//
	else if( (Length>=sizeof(FBitmapFileHeader)+sizeof(FBitmapInfoHeader)) && Buffer[0]=='B' && Buffer[1]=='M' )
	{
		if ((bmhdr->biWidth&(bmhdr->biWidth-1)) || (bmhdr->biHeight&(bmhdr->biHeight-1)))
		{
			Warn->Logf( NAME_Error, TEXT("Texture dimensions are not powers of two") );
			return NULL;
		}
		if( bmhdr->biCompression != BCBI_RGB )
		{
			Warn->Logf( NAME_Error, TEXT("RLE compression of BMP images not supported") );
			return NULL;
		}
		if( bmhdr->biPlanes==1 && bmhdr->biBitCount==8 )
		{
			// Do palette.
			const BYTE* bmpal = (BYTE*)Buffer + sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);

			// Set texture properties.			
			Texture->Init( bmhdr->biWidth, bmhdr->biHeight, PF_A8R8G8B8 );

			TArray<FColor>	Palette;
			for( INT i=0; i<Min<INT>(256,bmhdr->biClrUsed?bmhdr->biClrUsed:126); i++ )
				Palette.AddItem(FColor( bmpal[i*4+2], bmpal[i*4+1], bmpal[i*4+0], bmpal[i*4+3] ));
			while( Palette.Num()<256 )
				Palette.AddItem(FColor(0,0,0,255));

			FColor* MipData = (FColor*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
			// Copy upside-down scanlines.
			for(UINT Y = 0;Y < bmhdr->biHeight;Y++)
			{
				for(UINT X = 0;X < bmhdr->biWidth;X++)
				{
					MipData[(Texture->SizeY - Y - 1) * Texture->SizeX + X] = Palette(*((BYTE*)Buffer + bmf->bfOffBits + Y * Align(bmhdr->biWidth,4) + X));
				}
			}
			Texture->Mips(0).Data.Unlock();
		}
		else if( bmhdr->biPlanes==1 && bmhdr->biBitCount==24 )
		{
			// Set texture properties.
			Texture->Init( bmhdr->biWidth, bmhdr->biHeight, PF_A8R8G8B8 );

			// Copy upside-down scanlines.
			const BYTE* Ptr = (BYTE*)Buffer + bmf->bfOffBits;
			BYTE* MipData = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
			for( INT y=0; y<(INT)bmhdr->biHeight; y++ ) 
			{
				BYTE* DestPtr = &MipData[(bmhdr->biHeight - 1 - y) * bmhdr->biWidth * 4];
				BYTE* SrcPtr = (BYTE*) &Ptr[y * Align(bmhdr->biWidth*3,4)];
				for( INT x=0; x<(INT)bmhdr->biWidth; x++ )
				{
					*DestPtr++ = *SrcPtr++;
					*DestPtr++ = *SrcPtr++;
					*DestPtr++ = *SrcPtr++;
					*DestPtr++ = 0xFF;
				}
			}
			Texture->Mips(0).Data.Unlock();
		}
		else if( bmhdr->biPlanes==1 && bmhdr->biBitCount==16 )
		{
			Warn->Logf( NAME_Error, TEXT("BMP 16 bit format no longer supported. Use terrain tools for importing/exporting heightmaps.") );
			return NULL;
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT("BMP uses an unsupported format (%i/%i)"), bmhdr->biPlanes, bmhdr->biBitCount );
			return NULL;
		}
	}
	//
	// PCX
	//
	else if( Length >= sizeof(FPCXFileHeader) && PCX->Manufacturer==10 )
	{
		INT NewU = PCX->XMax + 1 - PCX->XMin;
		INT NewV = PCX->YMax + 1 - PCX->YMin;
		if( (NewU&(NewU-1)) || (NewV&(NewV-1)) )
		{
			Warn->Logf( NAME_Error, TEXT("Texture dimensions are not powers of two") );
			return NULL;
		}
		else if( PCX->NumPlanes==1 && PCX->BitsPerPixel==8 )
		{
			// Set texture properties.
			Texture->Init( NewU, NewV, PF_A8R8G8B8 );

			// Import the palette.
			BYTE* PCXPalette = (BYTE *)(BufferEnd - 256 * 3);
			TArray<FColor>	Palette;
			for(UINT i=0; i<256; i++ )
				Palette.AddItem(FColor(PCXPalette[i*3+0],PCXPalette[i*3+1],PCXPalette[i*3+2],i == 0 ? 0 : 255));

			// Import it.
			FColor* DestPtr	= (FColor*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
			FColor* DestEnd	= DestPtr + NewU * NewV;
			Buffer += 128;
			while( DestPtr < DestEnd )
			{
				BYTE Color = *Buffer++;
				if( (Color & 0xc0) == 0xc0 )
				{
					UINT RunLength = Color & 0x3f;
					Color = *Buffer++;
					checkf( (DestPtr+RunLength)<DestEnd, TEXT("RLE going off the end of buffer") );
					for(UINT Index = 0;Index < RunLength;Index++)
						*DestPtr++ = Palette(Color);
				}
				else *DestPtr++ = Palette(Color);
			}
			Texture->Mips(0).Data.Unlock();
		}
		else if( PCX->NumPlanes==3 && PCX->BitsPerPixel==8 )
		{
			// Set texture properties.
			Texture->Init( NewU, NewV, PF_A8R8G8B8 );

			// Copy upside-down scanlines.
			Buffer += 128;
			INT CountU = Min<INT>(PCX->BytesPerLine,NewU);
			BYTE* Dest = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
			for( INT i=0; i<NewV; i++ )
			{

				// We need to decode image one line per time building RGB image color plane by color plane.
				INT RunLength, Overflow=0;
				BYTE Color=0;
				for( INT ColorPlane=2; ColorPlane>=0; ColorPlane-- )
				{
					for( INT j=0; j<CountU; j++ )
					{
						if(!Overflow)
						{
							Color = *Buffer++;
							if((Color & 0xc0) == 0xc0)
							{
								RunLength=Min ((Color&0x3f), CountU-j);
								Overflow=(Color&0x3f)-RunLength;
								Color=*Buffer++;
							}
							else
								RunLength = 1;
						}
						else
						{
							RunLength=Min (Overflow, CountU-j);
							Overflow=Overflow-RunLength;
						}
	
						checkf( ((i*NewU+RunLength)*4+ColorPlane) < (Texture->Mips(0).Data.GetElementCount()*Texture->Mips(0).Data.GetElementSize()), 
							TEXT("RLE going off the end of buffer") );
						for( INT k=j; k<j+RunLength; k++ )
						{
							Dest[ (i*NewU+k)*4 + ColorPlane ] = Color;
						}
						j+=RunLength-1;
					}
				}				
			}
			Texture->Mips(0).Data.Unlock();
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT("PCX uses an unsupported format (%i/%i)"), PCX->NumPlanes, PCX->BitsPerPixel );
			return NULL;
		}
	}
	//
	// TGA
	//
	else if( Length >= sizeof(FTGAFileHeader) && (TGA->ImageTypeCode == 2 || TGA->ImageTypeCode == 10) )
	{
		// Validate the width and height are powers of two
		if (TGA->Width & (TGA->Width - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture width (%i)"),TGA->Width);
			return NULL;
		}
		if (TGA->Height & (TGA->Height - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture height (%i)"),TGA->Height);
			return NULL;
		}
		if(TGA->ImageTypeCode == 10) // 10 = RLE compressed 
		{
			// RLE compression: CHUNKS: 1 -byte header, high bit 0 = raw, 1 = compressed
			// bits 0-6 are a 7-bit count; count+1 = number of raw pixels following, or rle pixels to be expanded. 

			if(TGA->BitsPerPixel == 32)
			{
				Texture->Init(TGA->Width,TGA->Height,PF_A8R8G8B8);

				BYTE*	IdData		= (BYTE*)TGA + sizeof(FTGAFileHeader); 
				BYTE*	ColorMap	= IdData + TGA->IdFieldLength;
				BYTE*	ImageData	= (BYTE*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);				
				DWORD*	TextureData = (DWORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
				DWORD	Pixel		= 0;
				INT     RLERun		= 0;
				INT     RAWRun		= 0;

				for(INT Y = TGA->Height-1; Y >=0; Y--) // Y-flipped.
				{					
					for(INT X = 0;X < TGA->Width;X++)
					{						
						if( RLERun > 0 )
						{
							RLERun--;  // reuse current Pixel data.
						}
						else if( RAWRun == 0 ) // new raw pixel or RLE-run.
						{
							BYTE RLEChunk = *(ImageData++);							
							if( RLEChunk & 0x80 )
							{
								RLERun = ( RLEChunk & 0x7F ) + 1;
								RAWRun = 1;
							}
							else
							{
								RAWRun = ( RLEChunk & 0x7F ) + 1;
							}
						}							
						// Retrieve new pixel data - raw run or single pixel for RLE stretch.
						if( RAWRun > 0 )
						{
							Pixel = *(DWORD*)ImageData; // RGBA 32-bit dword.
							ImageData += 4;
							RAWRun--;
							RLERun--;
						}
						// Store.
						*( (TextureData + Y*TGA->Width)+X ) = Pixel;
					}
				}
				Texture->Mips(0).Data.Unlock();
			}
			else if( TGA->BitsPerPixel == 24 )
			{	
				Texture->Init(TGA->Width,TGA->Height,PF_A8R8G8B8);

				BYTE*	IdData = (BYTE*)TGA + sizeof(FTGAFileHeader); 
				BYTE*	ColorMap = IdData + TGA->IdFieldLength;
				BYTE*	ImageData = (BYTE*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
				DWORD*	TextureData = (DWORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
				BYTE    Pixel[4];
				INT     RLERun = 0;
				INT     RAWRun = 0;
				
				for(INT Y = TGA->Height-1; Y >=0; Y--) // Y-flipped.
				{					
					for(INT X = 0;X < TGA->Width;X++)
					{						
						if( RLERun > 0 )
							RLERun--;  // reuse current Pixel data.
						else if( RAWRun == 0 ) // new raw pixel or RLE-run.
						{
							BYTE RLEChunk = *(ImageData++);
							if( RLEChunk & 0x80 )
							{
								RLERun = ( RLEChunk & 0x7F ) + 1;
								RAWRun = 1;
							}
							else
							{
								RAWRun = ( RLEChunk & 0x7F ) + 1;
							}
						}							
						// Retrieve new pixel data - raw run or single pixel for RLE stretch.
						if( RAWRun > 0 )
						{
							Pixel[0] = *(ImageData++);
							Pixel[1] = *(ImageData++);
							Pixel[2] = *(ImageData++);
							Pixel[3] = 255;
							RAWRun--;
							RLERun--;
						}
						// Store.
						*( (TextureData + Y*TGA->Width)+X ) = *(DWORD*)&Pixel;
					}
				}
				Texture->Mips(0).Data.Unlock();
			}
			else if( TGA->BitsPerPixel == 16 )
			{
				Texture->Init(TGA->Width,TGA->Height,PF_G16);
				
				BYTE*	IdData = (BYTE*)TGA + sizeof(FTGAFileHeader);
				BYTE*	ColorMap = IdData + TGA->IdFieldLength;				
				WORD*	ImageData = (WORD*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
				WORD*	TextureData = (WORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
				WORD    Pixel = 0;
				INT     RLERun = 0;
				INT     RAWRun = 0;

				for(INT Y = TGA->Height-1; Y >=0; Y--) // Y-flipped.
				{					
					for( INT X=0;X<TGA->Width;X++ )
					{						
						if( RLERun > 0 )
							RLERun--;  // reuse current Pixel data.
						else if( RAWRun == 0 ) // new raw pixel or RLE-run.
						{
							BYTE RLEChunk =  *((BYTE*)ImageData);
							ImageData = (WORD*)(((BYTE*)ImageData)+1);
							if( RLEChunk & 0x80 )
							{
								RLERun = ( RLEChunk & 0x7F ) + 1;
								RAWRun = 1;
							}
							else
							{
								RAWRun = ( RLEChunk & 0x7F ) + 1;
							}
						}							
						// Retrieve new pixel data - raw run or single pixel for RLE stretch.
						if( RAWRun > 0 )
						{ 
							Pixel = *(ImageData++);
							RAWRun--;
							RLERun--;
						}
						// Store.
						*( (TextureData + Y*TGA->Width)+X ) = Pixel;
					}
				}
				Texture->Mips(0).Data.Unlock();
			}
			else
			{
				Warn->Logf( NAME_Error, TEXT("TGA uses an unsupported rle-compressed bit-depth: %u"),TGA->BitsPerPixel);
				return NULL;
			}
		}
		else if(TGA->ImageTypeCode == 2) // 2 = Uncompressed 
		{
			if(TGA->BitsPerPixel == 32)
			{
				Texture->Init(TGA->Width,TGA->Height,PF_A8R8G8B8);

				BYTE*	IdData = (BYTE*)TGA + sizeof(FTGAFileHeader);
				BYTE*	ColorMap = IdData + TGA->IdFieldLength;
				DWORD*	ImageData = (DWORD*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
				DWORD*	TextureData = (DWORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);

				for(INT Y = 0;Y < TGA->Height;Y++)
				{
					appMemcpy(TextureData + Y * TGA->Width,ImageData + (TGA->Height - Y - 1) * TGA->Width,TGA->Width * 4);
				}
				Texture->Mips(0).Data.Unlock();
			}
			else if(TGA->BitsPerPixel == 16)
			{
				Texture->Init(TGA->Width,TGA->Height,PF_G16);

				BYTE*	IdData = (BYTE*)TGA + sizeof(FTGAFileHeader);
				BYTE*	ColorMap = IdData + TGA->IdFieldLength;
				WORD*	ImageData = (WORD*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
				WORD*	TextureData = (WORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);

				// Monochrome 16-bit format - terrain heightmaps.
				for(INT Y = 0;Y < TGA->Height;Y++)
				{
					appMemcpy(TextureData + Y * TGA->Width,ImageData + (TGA->Height - Y - 1) * TGA->Width,TGA->Width * 2);
			}            
				Texture->Mips(0).Data.Unlock();
			}            
			else if(TGA->BitsPerPixel == 24)
			{
				Texture->Init(TGA->Width,TGA->Height,PF_A8R8G8B8);

				BYTE*	IdData = (BYTE*)TGA + sizeof(FTGAFileHeader);
				BYTE*	ColorMap = IdData + TGA->IdFieldLength;
				BYTE*	ImageData = (BYTE*) (ColorMap + (TGA->ColorMapEntrySize + 4) / 8 * TGA->ColorMapLength);
				DWORD*	TextureData = (DWORD*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
				BYTE    Pixel[4];

				for(INT Y = 0;Y < TGA->Height;Y++)
				{
					for(INT X = 0;X < TGA->Width;X++)
					{
						Pixel[0] = *(( ImageData+( TGA->Height-Y-1 )*TGA->Width*3 )+X*3+0);
						Pixel[1] = *(( ImageData+( TGA->Height-Y-1 )*TGA->Width*3 )+X*3+1);
						Pixel[2] = *(( ImageData+( TGA->Height-Y-1 )*TGA->Width*3 )+X*3+2);
						Pixel[3] = 255;
						*((TextureData+Y*TGA->Width)+X) = *(DWORD*)&Pixel;
					}
				}
				Texture->Mips(0).Data.Unlock();
			}
			else
			{
				Warn->Logf( NAME_Error, TEXT("TGA uses an unsupported bit-depth: %u"),TGA->BitsPerPixel);
				return NULL;
			}
		}
		else
		{
			Warn->Logf( NAME_Error, TEXT("TGA is an unsupported type: %u"),TGA->ImageTypeCode);
			return NULL;
		}

		// Flip the image data if the flip bits are set in the TGA header.
		UBOOL FlipX = (TGA->ImageDescriptor & 0x10) ? 1 : 0;
		UBOOL FlipY = (TGA->ImageDescriptor & 0x20) ? 1 : 0;
		if(FlipY || FlipX)
		{
			TArray<BYTE> FlippedData;
			FlippedData.Add(Texture->Mips(0).Data.GetBulkDataSize());

			INT NumBlocksX = Texture->SizeX / GPixelFormats[Texture->Format].BlockSizeX;
			INT NumBlocksY = Texture->SizeY / GPixelFormats[Texture->Format].BlockSizeY;

			BYTE* MipData = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
			for(INT Y = 0;Y < NumBlocksY;Y++)
			{
				for(INT X  = 0;X < NumBlocksX;X++)
				{
					INT DestX = FlipX ? (NumBlocksX - X - 1) : X;
					INT DestY = FlipY ? (NumBlocksY - Y - 1) : Y;
					appMemcpy(
						&FlippedData((DestX + DestY * NumBlocksX) * GPixelFormats[Texture->Format].BlockBytes),
						&MipData[(X + Y * NumBlocksX) * GPixelFormats[Texture->Format].BlockBytes],
						GPixelFormats[Texture->Format].BlockBytes
						);
				}
			}
			appMemcpy(MipData,FlippedData.GetData(),FlippedData.Num());
			Texture->Mips(0).Data.Unlock();

		}
	}
	//
	// PSD File
	//
	else if (psdhdr.IsValid())
	{
		// Validate the width and height are powers of two
		if (psdhdr.Width & (psdhdr.Width - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture width (%i)"),psdhdr.Width);
			return NULL;
		}
		if (psdhdr.Height & (psdhdr.Height - 1))
		{
			Warn->Logf( NAME_Error, TEXT("Can't import non-power of two texture height (%i)"),psdhdr.Height);
			return NULL;
		}

		if (!psdhdr.IsSupported())
		{
			Warn->Logf( TEXT("Format of this PSD is not supported") );
			return NULL;
		}

		// The psd is supported. Load it up.        
		Texture->Init(psdhdr.Width,psdhdr.Height,PF_A8R8G8B8);

		FColor*	Dst	= (FColor*) Texture->Mips(0).Data.Lock(LOCK_READ_WRITE);
		if (!psd_ReadData( Dst, Buffer, psdhdr ))
		{
			Warn->Logf( TEXT("Failed to read this PSD") );
			Texture->Mips(0).Data.Unlock();
			return NULL;
		}
		Texture->Mips(0).Data.Unlock();
	}
	else
	{
		// Unknown format.
		Warn->Logf( NAME_Error, TEXT("Bad image format for texture import") );
		return NULL;
 	}

	// Figure out whether we're using a normal map LOD group.
	UBOOL bIsNormalMapLODGroup = FALSE;
	if( LODGroup == TEXTUREGROUP_WorldNormalMap 
	||	LODGroup == TEXTUREGROUP_CharacterNormalMap
	||	LODGroup == TEXTUREGROUP_VehicleNormalMap
	||	LODGroup == TEXTUREGROUP_WeaponNormalMap )
	{
		// Change from default to normal map.
		if( CompressionSettings == TC_Default )
		{
			CompressionSettings = TC_Normalmap;
		}
		bIsNormalMapLODGroup = TRUE;
	}

	// Packed normal map
	if( CompressionSettings == TC_Normalmap || CompressionSettings == TC_NormalmapAlpha )
	{
		Texture->SRGB = 0;
		FLOAT NormalMapUnpackMin[4] = { -1, -1, -1, +0 };
		FLOAT NormalMapUnpackMax[4] = { +1, +1, +1, +1 };
		appMemcpy(Texture->UnpackMin,NormalMapUnpackMin,sizeof(NormalMapUnpackMin));
		appMemcpy(Texture->UnpackMax,NormalMapUnpackMax,sizeof(NormalMapUnpackMax));
		if( !bIsNormalMapLODGroup )
		{
			Texture->LODGroup = TEXTUREGROUP_WorldNormalMap;
		}
	}
	else
	{
		Texture->LODGroup = LODGroup;
	}

	// Propagate options.
	Texture->CompressionSettings	= CompressionSettings;
	Texture->CompressionNone		= NoCompression;
	Texture->CompressionNoAlpha		= NoAlpha;
	Texture->DeferCompression		= bDeferCompression;
	Texture->bDitherMipMapAlpha		= bDitherMipMapAlpha;
	Texture->bPreserveBorderR		= bPreserveBorderR;
	Texture->bPreserveBorderG		= bPreserveBorderG;
	Texture->bPreserveBorderB		= bPreserveBorderB;
	Texture->bPreserveBorderA		= bPreserveBorderA;
	Texture->SourceFilePath         = CurrentFilename;
	Texture->SourceFileTimestamp.Empty();
	FFileManager::timestamp Timestamp;
	if (GFileManager->GetTimestamp( *CurrentFilename, Timestamp ))
	{
		Texture->SourceFileTimestamp = FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d:%02d"), Timestamp.Year, Timestamp.Month+1, Timestamp.Day, Timestamp.Hour, Timestamp.Minute, Timestamp.Second );        
	}

	// Restore user set options
	if (ExistingTexture && bUseExistingSettings)
	{
		Texture->AddressX		= ExistingAddressX;
		Texture->AddressY		= ExistingAddressY;
		Texture->CompressionFullDynamicRange = ExistingCompressionFullDynamicRange;
		Texture->CompressionSettings = ExistingCompressionSettings;
		Texture->Filter			= ExistingFilter;
		Texture->LODGroup		= ExistingLODGroup;
		Texture->LODBias		= ExistingLODBias;
		Texture->NeverStream	= ExistingNeverStream;
		Texture->SRGB			= ExistingSRGB;
		appMemcpy(Texture->UnpackMin,ExistingUnpackMin,sizeof(ExistingUnpackMin));
		appMemcpy(Texture->UnpackMax,ExistingUnpackMax,sizeof(ExistingUnpackMax));

		Texture->CompressionNone = ExistingNoCompression;
		Texture->CompressionNoAlpha = ExistingNoAlpha;
		Texture->DeferCompression = ExistingDeferCompression;

		Texture->bDitherMipMapAlpha = ExistingDitherMipMapAlpha;
		Texture->bPreserveBorderR = ExistingPreserveBorderR;
		Texture->bPreserveBorderG = ExistingPreserveBorderG;
		Texture->bPreserveBorderB = ExistingPreserveBorderB;
		Texture->bPreserveBorderA = ExistingPreserveBorderA;
	}

	// Compress RGBA textures and also store source art.
	if( Texture->Format == PF_A8R8G8B8 )
	{
		// PNG Compress.
		FPNGHelper PNG;
		PNG.InitRaw( Texture->Mips(0).Data.Lock(LOCK_READ_ONLY), Texture->Mips(0).Data.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
		TArray<BYTE> CompressedData = PNG.GetCompressedData();
		Texture->Mips(0).Data.Unlock();
		check( CompressedData.Num() );

		// Store source art.
		Texture->SourceArt.Lock(LOCK_READ_WRITE);
		void* SourceArtPointer = Texture->SourceArt.Realloc( CompressedData.Num() );
		appMemcpy( SourceArtPointer, CompressedData.GetData(), CompressedData.Num() );
		Texture->SourceArt.Unlock();

		// PostEditChange below will automatically recompress.
	}
	else
	{
		Texture->CompressionNone = 1;
	}

	// Invalidate any materials using the newly imported texture. (occurs if you import over an existing texture)
	Texture->PostEditChange(NULL);

	// If we are automatically creating a material for this texture...
	if( bCreateMaterial )
	{
		// Create the material
		UMaterialFactoryNew* Factory = new UMaterialFactoryNew;
		UMaterial* Material = (UMaterial*)Factory->FactoryCreateNew( UMaterial::StaticClass(), InParent, *FString::Printf( TEXT("%s_Mat"), *Name.ToString() ), Flags, Context, Warn );

		// Create a texture reference for the texture we just imported and hook it up to the diffuse channel
		UMaterialExpression* Expression = ConstructObject<UMaterialExpression>( UMaterialExpressionTextureSample::StaticClass(), Material );
		Material->Expressions.AddItem( Expression );
		TArray<FExpressionOutput> Outputs;

		// If the user hasn't turned on any of the link checkboxes, default "bRGBToDiffuse" to being on.
		if( !bRGBToDiffuse && !bRGBToEmissive && !bAlphaToSpecular && !bAlphaToEmissive && !bAlphaToOpacity && !bAlphaToOpacityMask )
		{
			bRGBToDiffuse = 1;
		}

		// Set up the links the user asked for
		if( bRGBToDiffuse )
		{
			Material->DiffuseColor.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->DiffuseColor.Expression)->Texture = Texture;

			Material->DiffuseColor.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->DiffuseColor.Mask = Output->Mask;
			Material->DiffuseColor.MaskR = Output->MaskR;
			Material->DiffuseColor.MaskG = Output->MaskG;
			Material->DiffuseColor.MaskB = Output->MaskB;
			Material->DiffuseColor.MaskA = Output->MaskA;
		}

		if( bRGBToEmissive )
		{
			Material->EmissiveColor.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->EmissiveColor.Expression)->Texture = Texture;

			Material->EmissiveColor.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->EmissiveColor.Mask = Output->Mask;
			Material->EmissiveColor.MaskR = Output->MaskR;
			Material->EmissiveColor.MaskG = Output->MaskG;
			Material->EmissiveColor.MaskB = Output->MaskB;
			Material->EmissiveColor.MaskA = Output->MaskA;
		}

		if( bAlphaToSpecular )
		{
			Material->SpecularColor.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->SpecularColor.Expression)->Texture = Texture;

			Material->SpecularColor.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->SpecularColor.Mask = Output->Mask;
			Material->SpecularColor.MaskR = 0;
			Material->SpecularColor.MaskG = 0;
			Material->SpecularColor.MaskB = 0;
			Material->SpecularColor.MaskA = 1;
		}

		if( bAlphaToEmissive )
		{
			Material->EmissiveColor.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->EmissiveColor.Expression)->Texture = Texture;

			Material->EmissiveColor.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->EmissiveColor.Mask = Output->Mask;
			Material->EmissiveColor.MaskR = 0;
			Material->EmissiveColor.MaskG = 0;
			Material->EmissiveColor.MaskB = 0;
			Material->EmissiveColor.MaskA = 1;
		}

		if( bAlphaToOpacity )
		{
			Material->Opacity.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->Opacity.Expression)->Texture = Texture;

			Material->Opacity.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->Opacity.Mask = Output->Mask;
			Material->Opacity.MaskR = 0;
			Material->Opacity.MaskG = 0;
			Material->Opacity.MaskB = 0;
			Material->Opacity.MaskA = 1;
		}

		if( bAlphaToOpacityMask )
		{
			Material->OpacityMask.Expression = Expression;
			((UMaterialExpressionTextureSample*)Material->OpacityMask.Expression)->Texture = Texture;

			Material->OpacityMask.Expression->GetOutputs(Outputs);
			FExpressionOutput* Output = &Outputs(0);
			Material->OpacityMask.Mask = Output->Mask;
			Material->OpacityMask.MaskR = 0;
			Material->OpacityMask.MaskG = 0;
			Material->OpacityMask.MaskB = 0;
			Material->OpacityMask.MaskA = 1;
		}

		Material->TwoSided	= bTwoSided;
		Material->BlendMode = Blending;
		Material->LightingModel = LightingModel;
	}

	return Texture;
}

/**
 *	Initializes the given texture from the TextureData text block supplied.
 *	The TextureData text block is assumed to have been generated by the UTextureExporterT3D.
 *
 *	@param	InTexture	The texture to initialize
 *	@param	Text		The texture data text generated by the TextureExporterT3D
 *	@param	Warn		Where to send warnings/errors
 *
 *	@param	UBOOL		TRUE if successful, FALSE if not
 */
UBOOL UTextureFactory::InitializeFromT3DTextureDataText(UTexture* InTexture, const FString& Text, FFeedbackContext* Warn)
{
	const TCHAR* Buffer = *Text;

	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		FString ParsedText;
		
		if (Parse(Str, TEXT("VERSION="), ParsedText))
		{
			///** Versioning system... */
			//Ar.Logf(TEXT("%sVersion=%d.%d") LINE_TERMINATOR, appSpc(TextIndent), VersionMax, VersionMin);
		}
		else
		if (Parse(Str, TEXT("SRGB="), ParsedText))
		{
			InTexture->SRGB = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("RGBE="), ParsedText))
		{
			InTexture->RGBE = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("UNPACKMIN="), ParsedText))
		{
			appSSCANF(*ParsedText, TEXT("%f,%f,%f,%f"), 
				&(InTexture->UnpackMin[0]), 
				&(InTexture->UnpackMin[1]), 
				&(InTexture->UnpackMin[2]), 
				&(InTexture->UnpackMin[3]));
		}
		else
		if (Parse(Str, TEXT("UNPACKMAX="), ParsedText))
		{
			appSSCANF(*ParsedText, TEXT("%f,%f,%f,%f"), 
				&(InTexture->UnpackMax[0]), 
				&(InTexture->UnpackMax[1]), 
				&(InTexture->UnpackMax[2]), 
				&(InTexture->UnpackMax[3]));
		}
		else
		if (Parse(Str, TEXT("COMPRESSIONNOALPHA="), ParsedText))
		{
			InTexture->CompressionNoAlpha = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("COMPRESSIONNONE="), ParsedText))
		{
			InTexture->CompressionNone = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("COMPRESSIONNOMIPMAPS="), ParsedText))
		{
			InTexture->CompressionNoMipmaps = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("COMPRESSIONFULLDYNAMICRANGE="), ParsedText))
		{
			InTexture->CompressionFullDynamicRange = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("DEFERCOMPRESSION="), ParsedText))
		{
			InTexture->DeferCompression = BOOL_STRING_IS_TRUE(ParsedText);
		}
		/** Allows artists to specify that a texture should never have its miplevels dropped which is useful for e.g. HUD and menu textures */
		else
		if (Parse(Str, TEXT("NEVERSTREAM="), ParsedText))
		{
			InTexture->NeverStream = BOOL_STRING_IS_TRUE(ParsedText);
		}
		/** When TRUE, mip-maps are dithered for smooth transitions. */
		else
		if (Parse(Str, TEXT("BDITHERMIPMAPALPHA="), ParsedText))
		{
			InTexture->bDitherMipMapAlpha = BOOL_STRING_IS_TRUE(ParsedText);
		}
		/** If TRUE, the color border pixels are preseved by mipmap generation.  One flag per color channel. */
		else
		if (Parse(Str, TEXT("BPRESERVEBORDERR="), ParsedText))
		{
			InTexture->bPreserveBorderR = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("BPRESERVEBORDERG="), ParsedText))
		{
			InTexture->bPreserveBorderG = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("BPRESERVEBORDERB="), ParsedText))
		{
			InTexture->bPreserveBorderB = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("BPRESERVEBORDERA="), ParsedText))
		{
			InTexture->bPreserveBorderA = BOOL_STRING_IS_TRUE(ParsedText);
		}
		/** If TRUE, the RHI texture will be created using TexCreate_NoTiling */
		else
		if (Parse(Str, TEXT("BNOTILING="), ParsedText))
		{
			InTexture->bNoTiling = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		if (Parse(Str, TEXT("COMPRESSIONSETTINGS="), ParsedText))
		{
			InTexture->CompressionSettings = UTexture::GetCompressionSettingsFromString(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("FILTER="), ParsedText))
		{
			InTexture->Filter = UTexture::GetTextureFilterFromString(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("LODGROUP="), ParsedText))
		{
			InTexture->LODGroup = UTexture::GetTextureGroupFromString(*ParsedText);
		}
		/** A bias to the index of the top mip level to use. */
		else
		if (Parse(Str, TEXT("LODBIAS="), ParsedText))
		{
			InTexture->LODBias = appAtoi(*ParsedText);
		}
		else
		if (Parse(Str, TEXT("SOURCEFILEPATH="), ParsedText))
		{
			InTexture->SourceFilePath = ParsedText;
		}
		else
		if (Parse(Str, TEXT("SOURCEFILETIMESTAMP="), ParsedText))
		{
			InTexture->SourceFileTimestamp = ParsedText;
		}
		else
		if (GetBEGIN(&Str, TEXT("SOURCEART")))
		{
			while(ParseLine(&Buffer,StrLine))
			{
				Str = *StrLine;
				if (GetBEGIN(&Str, TEXT("UNTYPEDBULKDATA")))
				{
					ImportUntypedBulkDataFromText(Buffer, InTexture->SourceArt);
				}
				else
				if (GetEND(&Str, TEXT("SOURCEART")))
				{
					break;
				}
			}
		}
		else
		if (GetBEGIN(&Str, TEXT("TEXTURE2DDATA")))
		{
			UTexture2D* InTexture2D = Cast<UTexture2D>(InTexture);
			if (InTexture2D)
			{
				InitializeFromT3DTexture2DDataText(InTexture2D, Buffer, Warn);
			}
			else
			{
				Warn->Errors.AddItem(FString::Printf(TEXT("TextureImport: Texture2DData found - InTexture is a %s"), *(InTexture->StaticClass()->GetName())));
				// Bug out??
			}
		}
		else
		if (GetBEGIN(&Str, TEXT("TEXTURECUBEDATA")))
		{
			UTextureCube* InTextureCube = Cast<UTextureCube>(InTexture);
			if (InTextureCube)
			{
				InitializeFromT3DTextureCubeDataText(InTextureCube, Buffer, Warn);
			}
			else
			{
				Warn->Errors.AddItem(FString::Printf(TEXT("TextureImport: InTextureCube found - InTexture is a %s"), *(InTexture->StaticClass()->GetName())));
				// Bug out??
			}
		}
		else
		if (GetEND(&Str, TEXT("TEXTUREDATA")))
		{
			break;
		}
	}

	InTexture->SetFlags(RF_Standalone | RF_Public | RF_InitializedProps);
	InTexture->MarkPackageDirty();

	return TRUE;
}

UBOOL UTextureFactory::InitializeFromT3DTexture2DDataText(UTexture2D* InTexture2D, const TCHAR*& Buffer, FFeedbackContext* Warn)
{
	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		FString ParsedText;
		
		/** The width of the texture.												*/
		if (Parse(Str, TEXT("SIZEX="), ParsedText))
		{
			InTexture2D->SizeX = appAtoi(*ParsedText);
		}
		else
		/** The height of the texture.												*/
		if (Parse(Str, TEXT("SIZEY="), ParsedText))
		{
			InTexture2D->SizeY = appAtoi(*ParsedText);
		}
		else
		/** The format of the texture data.											*/
		if (Parse(Str, TEXT("FORMAT="), ParsedText))
		{
		}
		else
		/** The addressing mode to use for the X axis.								*/
		if (Parse(Str, TEXT("ADDRESSX="), ParsedText))
		{
			InTexture2D->AddressX = UTexture::GetTextureAddressFromString(*ParsedText);
		}
		else
		/** The addressing mode to use for the Y axis.								*/
		if (Parse(Str, TEXT("ADDRESSY="), ParsedText))
		{
			InTexture2D->AddressY = UTexture::GetTextureAddressFromString(*ParsedText);
		}
		else
		/** Global/ serialized version of ForceMiplevelsToBeResident.				*/
		if (Parse(Str, TEXT("BGLOBALFORCEMIPLEVELSTOBERESIDENT="), ParsedText))
		{
			InTexture2D->bGlobalForceMipLevelsToBeResident = BOOL_STRING_IS_TRUE(ParsedText);
		}
		else
		/** 
		* Keep track of the first mip level stored in the packed miptail.
		* it's set to highest mip level if no there's no packed miptail 
		*/
		if (Parse(Str, TEXT("MIPTAILBASEIDX="), ParsedText))
		{
			InTexture2D->MipTailBaseIdx = appAtoi(*ParsedText);
		}
		else
		if (GetBEGIN(&Str, TEXT("Mip0")))
		{
			while(ParseLine(&Buffer,StrLine))
			{
				FTexture2DMipMap* TopLevelMip = NULL;
				if (InTexture2D->Mips.Num() == 0)
				{
					TopLevelMip = new(InTexture2D->Mips)FTexture2DMipMap;
				}
				else
				{
					TopLevelMip = &(InTexture2D->Mips(0));
				}

				Str = *StrLine;

				if (Parse(Str, TEXT("SIZEX="), ParsedText))
				{
					TopLevelMip->SizeX = appAtoi(*ParsedText);
				}
				else
				if (Parse(Str, TEXT("SIZEY="), ParsedText))
				{
					TopLevelMip->SizeY = appAtoi(*ParsedText);
				}
				else
				if (GetBEGIN(&Str, TEXT("TEXTUREMIPBULKDATA")))
				{
					while(ParseLine(&Buffer,StrLine))
					{
						Str = *StrLine;
						if (GetBEGIN(&Str, TEXT("UNTYPEDBULKDATA")))
						{
							ImportUntypedBulkDataFromText(Buffer, TopLevelMip->Data);
						}
						else
						if (GetEND(&Str, TEXT("TEXTUREMIPBULKDATA")))
						{
							break;
						}
					}
				}
				else
				if (GetEND(&Str, TEXT("Mip0")))
				{
					break;
				}
			}
		}
		else
		if (GetEND(&Str, TEXT("TEXTURE2DDATA")))
		{
			break;
		}
	}
	return TRUE;
}

void UTextureFactory::FindCubeMapFace(const FString& ParsedText, const FString& FaceString, UTextureCube& TextureCube, UTexture2D*& TextureFace)
{
	if (ParsedText.Len() > 0)
	{
		// Find the object...
		UTexture* Texture = FindObject<UTexture>(ANY_PACKAGE, *ParsedText);
		if (Texture == NULL)
		{
			FString ClassName, ObjectName;
			if (ParseObjectPropertyName(ParsedText, ClassName, ObjectName))
			{
				Texture = FindObject<UTexture>(ANY_PACKAGE, *ObjectName);
			}
		}

		UTexture2D* Texture2D = Cast<UTexture2D>(Texture);
		if (Texture2D)
		{
			TextureFace = Texture2D;
		}
		else
		{
			warnf(TEXT("Failed to find texture %s (for face %s in cubemap %s)"), *ParsedText, *FaceString, *(TextureCube.GetPathName()));
		}
	}
}

UBOOL UTextureFactory::InitializeFromT3DTextureCubeDataText(UTextureCube* InTextureCube, const TCHAR*& Buffer, FFeedbackContext* Warn)
{
	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;

		FString ParsedText;
		
		/** The width of the texture.												*/
		if (Parse(Str, TEXT("SIZEX="), ParsedText))
		{
			InTextureCube->SizeX = appAtoi(*ParsedText);
		}
		else
		/** The height of the texture.												*/
		if (Parse(Str, TEXT("SIZEY="), ParsedText))
		{
			InTextureCube->SizeY = appAtoi(*ParsedText);
		}
		else
		/** The format of the texture data.											*/
		if (Parse(Str, TEXT("FORMAT="), ParsedText))
		{
		}
		else
		if (Parse(Str, TEXT("FACEPOSX="), ParsedText))
		{
			FString FaceString(TEXT("FacePosX"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FacePosX);
		}
		else
		if (Parse(Str, TEXT("FACENEGX="), ParsedText))
		{
			FString FaceString(TEXT("FaceNegX"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FaceNegX);
		}
		else
		if (Parse(Str, TEXT("FACEPOSY="), ParsedText))
		{
			FString FaceString(TEXT("FacePosY"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FacePosY);
		}
		else
		if (Parse(Str, TEXT("FACENEGY="), ParsedText))
		{
			FString FaceString(TEXT("FaceNegY"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FaceNegY);
		}
		else
		if (Parse(Str, TEXT("FACEPOSZ="), ParsedText))
		{
			FString FaceString(TEXT("FacePosZ"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FacePosZ);
		}
		else
		if (Parse(Str, TEXT("FACENEGZ="), ParsedText))
		{
			FString FaceString(TEXT("FaceNegZ"));
			FindCubeMapFace(ParsedText, FaceString, *InTextureCube, InTextureCube->FaceNegZ);
		}
		else
		if (GetEND(&Str, TEXT("TEXTURECUBEDATA")))
		{
			break;
		}
	}
	return TRUE;
}

IMPLEMENT_CLASS(UTextureFactory);

/*------------------------------------------------------------------------------
	UTextureExporterPCX implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureExporterPCX::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTexture2D::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("PCX")) );
	new(FormatDescription)FString(TEXT("PCX File"));
}
UBOOL UTextureExporterPCX::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UTexture2D* Texture = CastChecked<UTexture2D>( Object );

	if( Texture->SourceArt.GetBulkDataSize() == 0 )
	{
		return FALSE;
	}

	FPNGHelper PNG;
	PNG.InitCompressed( Texture->SourceArt.Lock(LOCK_READ_ONLY), Texture->SourceArt.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
	TArray<BYTE> RawData = PNG.GetRawData();
	Texture->SourceArt.Unlock();

	// Set all PCX file header properties.
	FPCXFileHeader PCX;
	appMemzero( &PCX, sizeof(PCX) );
	PCX.Manufacturer	= 10;
	PCX.Version			= 05;
	PCX.Encoding		= 1;
	PCX.BitsPerPixel	= 8;
	PCX.XMin			= 0;
	PCX.YMin			= 0;
	PCX.XMax			= Texture->SizeX-1;
	PCX.YMax			= Texture->SizeY-1;
	PCX.XDotsPerInch	= Texture->SizeX;
	PCX.YDotsPerInch	= Texture->SizeY;
	PCX.BytesPerLine	= Texture->SizeX;
	PCX.PaletteType		= 0;
	PCX.HScreenSize		= 0;
	PCX.VScreenSize		= 0;

	// Copy all RLE bytes.
	BYTE RleCode=0xc1;

	PCX.NumPlanes = 3;
	Ar << PCX;
	for( INT Line=0; Line<Texture->SizeY; Line++ )
	{
		for( INT ColorPlane = 2; ColorPlane >= 0; ColorPlane-- )
		{
			BYTE* ScreenPtr = &RawData(0) + (Line * Texture->SizeX * 4) + ColorPlane;
			for( INT Row=0; Row<Texture->SizeX; Row++ )
			{
				if( (*ScreenPtr&0xc0)==0xc0 )
					Ar << RleCode;
				Ar << *ScreenPtr;
				ScreenPtr += 4;
			}
		}
	}

	return TRUE;

}
IMPLEMENT_CLASS(UTextureExporterPCX);

/*------------------------------------------------------------------------------
	UTextureExporterBMP implementation.
------------------------------------------------------------------------------*/

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureExporterBMP::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTexture2D::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("BMP")) );
	new(FormatDescription)FString(TEXT("Windows Bitmap"));
}

UBOOL UTextureExporterBMP::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UTexture2D* Texture = CastChecked<UTexture2D>( Object );

	// Figure out format.
	EPixelFormat Format = (EPixelFormat)Texture->Format;

	FBitmapFileHeader bmf;
	FBitmapInfoHeader bmhdr;

	// File header.
	bmf.bfType      = 'B' + (256*(INT)'M');
	bmf.bfReserved1 = 0;
	bmf.bfReserved2 = 0;
	INT biSizeImage;

	if( Format == PF_G16 )
	{
		biSizeImage		= Texture->SizeX * Texture->SizeY * 2;
		bmf.bfOffBits   = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);
		bmhdr.biBitCount= 16;
	}
	else if( Texture->SourceArt.GetBulkDataSize() )
	{
		biSizeImage		= Texture->SizeX * Texture->SizeY * 3;
		bmf.bfOffBits   = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);
		bmhdr.biBitCount= 24;
	}
	else
		return 0;

	bmf.bfSize		= bmf.bfOffBits + biSizeImage;
	Ar << bmf;

	// Info header.
	bmhdr.biSize          = sizeof(FBitmapInfoHeader);
	bmhdr.biWidth         = Texture->SizeX;
	bmhdr.biHeight        = Texture->SizeY;
	bmhdr.biPlanes        = 1;
	bmhdr.biCompression   = BCBI_RGB;
	bmhdr.biSizeImage     = biSizeImage;
	bmhdr.biXPelsPerMeter = 0;
	bmhdr.biYPelsPerMeter = 0;
	bmhdr.biClrUsed       = 0;
	bmhdr.biClrImportant  = 0;
	Ar << bmhdr;

	if( Format == PF_G16 )
	{
		BYTE* MipData = (BYTE*) Texture->Mips(0).Data.Lock(LOCK_READ_ONLY);
		for( INT i=Texture->SizeY-1; i>=0; i-- )
		{
			Ar.Serialize( &MipData[i*Texture->SizeX*2], Texture->SizeX*2 );
		}
		Texture->Mips(0).Data.Unlock();
	}
	else if( Texture->SourceArt.GetBulkDataSize() )
	{		
		FPNGHelper PNG;
		PNG.InitCompressed( Texture->SourceArt.Lock(LOCK_READ_ONLY), Texture->SourceArt.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
		TArray<BYTE> RawData = PNG.GetRawData();
		Texture->SourceArt.Unlock();

		// Upside-down scanlines.
		for( INT i=Texture->SizeY-1; i>=0; i-- )
		{
			BYTE* ScreenPtr = &RawData(i*Texture->SizeX*4);
			for( INT j=Texture->SizeX; j>0; j-- )
			{
				Ar << *ScreenPtr++;
				Ar << *ScreenPtr++;
				Ar << *ScreenPtr++;
				ScreenPtr++;
			}
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

UBOOL UTextureExporterBMP::ExportBinary(const BYTE* Data, EPixelFormat Format, INT SizeX, INT SizeY, 
	const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, DWORD PortFlags)
{
	// Figure out format.
	FBitmapFileHeader bmf;
	FBitmapInfoHeader bmhdr;

	// File header.
	bmf.bfType      = 'B' + (256*(INT)'M');
	bmf.bfReserved1 = 0;
	bmf.bfReserved2 = 0;
	INT biSizeImage;

	if (Format == PF_G16)
	{
		biSizeImage		= SizeX * SizeY * 2;
		bmf.bfOffBits   = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);
		bmhdr.biBitCount= 16;
	}
	else 
	if (Format == PF_A8R8G8B8)
	{
		biSizeImage		= SizeX * SizeY * 3;
		bmf.bfOffBits   = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);
		bmhdr.biBitCount= 24;
	}
	else
	{
		return 0;
	}

	bmf.bfSize		= bmf.bfOffBits + biSizeImage;
	Ar << bmf;

	// Info header.
	bmhdr.biSize          = sizeof(FBitmapInfoHeader);
	bmhdr.biWidth         = SizeX;
	bmhdr.biHeight        = SizeY;
	bmhdr.biPlanes        = 1;
	bmhdr.biCompression   = BCBI_RGB;
	bmhdr.biSizeImage     = biSizeImage;
	bmhdr.biXPelsPerMeter = 0;
	bmhdr.biYPelsPerMeter = 0;
	bmhdr.biClrUsed       = 0;
	bmhdr.biClrImportant  = 0;
	Ar << bmhdr;

	if (Format == PF_G16)
	{
		for (INT i = SizeY - 1; i >= 0; i--)
		{
			Ar.Serialize((void*)(&Data[(i * SizeX * 2)]), (SizeX * 2));
		}
	}
	else
	if (Format == PF_A8R8G8B8)
	{		
		// Upside-down scanlines.
		for (INT i = SizeY - 1; i >= 0; i--)
		{
			// Bad type-casting!!!!! (const to non-const)
			BYTE* ScreenPtr = (BYTE*)(&Data[i * SizeX * 4]);
			for (INT j = SizeX; j > 0; j--)
			{
				Ar << *ScreenPtr++;
				Ar << *ScreenPtr++;
				Ar << *ScreenPtr++;
				*ScreenPtr++;
			}
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

IMPLEMENT_CLASS(UTextureExporterBMP);

/*------------------------------------------------------------------------------
	UTextureExporterTGA implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UTextureExporterTGA::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTexture2D::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("TGA")) );
	new(FormatDescription)FString(TEXT("Targa"));
}
UBOOL UTextureExporterTGA::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UTexture2D* Texture = CastChecked<UTexture2D>( Object );

	if( Texture->SourceArt.GetBulkDataSize() == 0 )
	{
		return FALSE;
	}

	FPNGHelper PNG;
	PNG.InitCompressed( Texture->SourceArt.Lock(LOCK_READ_ONLY), Texture->SourceArt.GetBulkDataSize(), Texture->SizeX, Texture->SizeY );
	TArray<BYTE> RawData = PNG.GetRawData();
	Texture->SourceArt.Unlock();

	FTGAFileHeader TGA;
	appMemzero( &TGA, sizeof(TGA) );
	TGA.ImageTypeCode = 2;
	TGA.BitsPerPixel = 32;
	TGA.Height = Texture->SizeY;
	TGA.Width = Texture->SizeX;

	Ar.Serialize( &TGA, sizeof(TGA) );

	for( INT Y=0;Y < Texture->SizeY;Y++ )
		Ar.Serialize( &RawData( (Texture->SizeY - Y - 1) * Texture->SizeX * 4 ), Texture->SizeX * 4 );

	FTGAFileFooter Ftr;
	appMemzero( &Ftr, sizeof(Ftr) );
	appMemcpy( Ftr.Signature, "TRUEVISION-XFILE", 16 );
	Ftr.TrailingPeriod = '.';
	Ar.Serialize( &Ftr, sizeof(Ftr) );

	return TRUE;
}
IMPLEMENT_CLASS(UTextureExporterTGA);

/*------------------------------------------------------------------------------
	URenderTargetExporterTGA implementation .
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void URenderTargetExporterTGA::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTextureRenderTarget2D::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("TGA")) );
	new(FormatDescription)FString(TEXT("Targa"));
}
UBOOL URenderTargetExporterTGA::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UBOOL Result=FALSE;

#if GEMINI_TODO
	// kind of a weird exporter, but it needs a render device to access render targets
	if( !GRenderDevice )
	{
		return Result;
	}

	UTextureRenderTarget2D* RenderTarget = CastChecked<UTextureRenderTarget2D>( Object );

	if( RenderTarget )
	{
		TArray<FColor> SurfData;
		UINT Width, Height;
        GRenderDevice->ReadRenderTargetSurfacePixels( RenderTarget, CubeTargetFace_PosX, SurfData, Width, Height );

		if( Width > 0 && 
			Height > 0 && 
			SurfData.Num() )
		{
			FPNGHelper PNG;
			PNG.InitRaw( &SurfData(0), SurfData.Num()*sizeof(FColor), Width, Height );
			TArray<BYTE> RawData = PNG.GetRawData();

			FTGAFileHeader TGA;
			appMemzero( &TGA, sizeof(TGA) );
			TGA.ImageTypeCode = 2;
			TGA.BitsPerPixel = 32;
			TGA.Width = Width;
			TGA.Height = Height;		

			Ar.Serialize( &TGA, sizeof(TGA) );

			for( UINT Y=0;Y < Height;Y++ )
				Ar.Serialize( &RawData( (Height - Y - 1) * Width * 4 ), Width * 4 );

			FTGAFileFooter Ftr;
			appMemzero( &Ftr, sizeof(Ftr) );
			appMemcpy( Ftr.Signature, "TRUEVISION-XFILE", 16 );
			Ftr.TrailingPeriod = '.';
			Ar.Serialize( &Ftr, sizeof(Ftr) );

			Result=TRUE;
		}
	}
#endif

	return Result;
}

IMPLEMENT_CLASS(URenderTargetExporterTGA)


/*------------------------------------------------------------------------------
	USpeedTreeExporter implementation .
------------------------------------------------------------------------------*/

void USpeedTreeExporter::InitializeIntrinsicPropertyValues()
{
#if WITH_SPEEDTREE_EXPORT
	SupportedClass = USpeedTree::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("SPT")) );
	new(FormatDescription)FString(TEXT("SpeedTree"));
#endif
}
UBOOL USpeedTreeExporter::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UBOOL Result=FALSE;

#if WITH_SPEEDTREE_EXPORT

	USpeedTree* SpeedTree = CastChecked<USpeedTree>( Object );

	if( SpeedTree && SpeedTree->IsInitialized() )
	{
		CSpeedTreeRT* ActualSpeedTree = SpeedTree->SRH->SpeedTree;

		UINT NumBytes = 0;
		BYTE* Data = ActualSpeedTree->SaveTree(NumBytes);

		if (Data && NumBytes > 0)
		{
			// mangle it
			#if WITH_SPEEDTREE_MANGLE
				unsigned int KeyLength = appStrlen(SPEEDTREE_MANGLE_KEY);
				for (UINT i = 0; i < NumBytes; ++i)
				{	
					INT Byte = Data[i] + SPEEDTREE_MANGLE_KEY[i % KeyLength];
					if (Byte > 255)
						Byte -= 256;
					Data[i] = (char)Byte;
				}
			#endif

			Ar.Serialize(Data, NumBytes);
			//@todo SpeedTree: route allocations through our allocator
			//delete [] Data;
			Result=TRUE;
		}
	}
#endif

	return Result;
}

IMPLEMENT_CLASS(USpeedTreeExporter)

/*------------------------------------------------------------------------------
	URenderTargetCubeExporterTGA implementation .
------------------------------------------------------------------------------*/

void URenderTargetCubeExporterTGA::StaticConstructor()
{
	UEnum* CubeFaceEnum = new(GetClass(),TEXT("CubeFace"),RF_Public) UEnum(NULL);

	TArray<FName> EnumNames;
#if GEMINI_TODO
	for( BYTE Idx=0; Idx < CubeTargetFace_MAX; Idx++ )
	{
		switch( (ECubeTargetFace)Idx )
		{
		case CubeTargetFace_PosX:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_PosX")) );
			break;
		case CubeTargetFace_NegX:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_NegX")) );
			break;
		case CubeTargetFace_PosY:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_PosY")) );
			break;
		case CubeTargetFace_NegY:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_NegY")) );
			break;
		case CubeTargetFace_PosZ:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_PosZ")) );
			break;
		case CubeTargetFace_NegZ:
			EnumNames.AddItem( FName(TEXT("CubeTargetFace_NegZ")) );
			break;
		}
	}
#endif
	CubeFaceEnum->SetEnums( EnumNames );

	new(GetClass(),TEXT("CubeFace"), RF_Public)UByteProperty (CPP_PROPERTY(CubeFace), TEXT("CubeFace"), CPF_Edit, CubeFaceEnum	);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void URenderTargetCubeExporterTGA::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UTextureRenderTargetCube::StaticClass();
	PreferredFormatIndex = FormatExtension.AddItem( FString(TEXT("TGA")) );
	new(FormatDescription)FString(TEXT("Targa"));
#if GEMINI_TODO
	CubeFace = (BYTE)CubeTargetFace_PosX;
#endif
}
UBOOL URenderTargetCubeExporterTGA::ExportBinary( UObject* Object, const TCHAR* Type, FArchive& Ar, FFeedbackContext* Warn, INT FileIndex, DWORD PortFlags )
{
	UBOOL Result=FALSE;

#if GEMINI_TODO
	// kind of a weird exporter, but it needs a render device to access render targets
	if( !GRenderDevice )
	{
		return Result;
	}

	UTextureRenderTargetCube* RenderTarget = CastChecked<UTextureRenderTargetCube>( Object );

	if( RenderTarget )
	{
		TArray<FColor> SurfData;
		UINT Width, Height;
		check(CubeFace < CubeTargetFace_MAX);
		GRenderDevice->ReadRenderTargetSurfacePixels( RenderTarget, ECubeTargetFace(CubeFace), SurfData, Width, Height );

		if( Width > 0 && 
			Height > 0 && 
			SurfData.Num() )
		{
			FPNGHelper PNG;
			PNG.InitRaw( &SurfData(0), SurfData.Num()*sizeof(FColor), Width, Height );
			TArray<BYTE> RawData = PNG.GetRawData();

			FTGAFileHeader TGA;
			appMemzero( &TGA, sizeof(TGA) );
			TGA.ImageTypeCode = 2;
			TGA.BitsPerPixel = 32;
			TGA.Width = Width;
			TGA.Height = Height;		

			Ar.Serialize( &TGA, sizeof(TGA) );

			for( UINT Y=0;Y < Height;Y++ )
				Ar.Serialize( &RawData( (Height - Y - 1) * Width * 4 ), Width * 4 );

			FTGAFileFooter Ftr;
			appMemzero( &Ftr, sizeof(Ftr) );
			appMemcpy( Ftr.Signature, "TRUEVISION-XFILE", 16 );
			Ftr.TrailingPeriod = '.';
			Ar.Serialize( &Ftr, sizeof(Ftr) );

			Result=TRUE;
		}
	}
#endif

	return Result;
}

IMPLEMENT_CLASS(URenderTargetCubeExporterTGA)

/*------------------------------------------------------------------------------
	UFontFactory.
------------------------------------------------------------------------------*/

//
//	Fast pixel-lookup.
//
static inline BYTE AT( BYTE* Screen, UINT SXL, UINT X, UINT Y )
{
	return Screen[X+Y*SXL];
}

//
// Codepage 850 -> Latin-1 mapping table:
//
BYTE FontRemap[256] = 
{
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,

	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,

	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,000,
	032,173,184,156,207,190,124,245,034,184,166,174,170,196,169,238,
	248,241,253,252,239,230,244,250,247,251,248,175,172,171,243,168,

	183,181,182,199,142,143,146,128,212,144,210,211,222,214,215,216,
	209,165,227,224,226,229,153,158,157,235,233,234,154,237,231,225,
	133,160,131,196,132,134,145,135,138,130,136,137,141,161,140,139,
	208,164,149,162,147,228,148,246,155,151,163,150,129,236,232,152,
};

//
//	Find the border around a font glyph that starts at x,y (it's upper
//	left hand corner).  If it finds a glyph box, it returns 0 and the
//	glyph 's length (xl,yl).  Otherwise returns -1.
//
static UBOOL ScanFontBox( BYTE* Data, INT X, INT Y, INT& XL, INT& YL, INT SizeX )
{
	INT FontXL = SizeX;

	// Find x-length.
	INT NewXL = 1;
	while ( AT(Data,FontXL,X+NewXL,Y)==255 && AT(Data,FontXL,X+NewXL,Y+1)!=255 )
	{
		NewXL++;
	}

	if( AT(Data,FontXL,X+NewXL,Y)!=255 )
	{
		return 0;
	}

	// Find y-length.
	INT NewYL = 1;
	while( AT(Data,FontXL,X,Y+NewYL)==255 && AT(Data,FontXL,X+1,Y+NewYL)!=255 )
	{
		NewYL++;
	}

	if( AT(Data,FontXL,X,Y+NewYL)!=255 )
	{
		return 0;
	}

	XL = NewXL - 1;
	YL = NewYL - 1;

	return 1;
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UFontFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = UFont::StaticClass();

	bEditorImport = 0;

	// Default font textures to use the 'UI' LOD group
	LODGroup = TEXTUREGROUP_UI;
}


UFontFactory::UFontFactory()
{
}

#define NUM_FONT_CHARS 256

UObject* UFontFactory::FactoryCreateBinary
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const BYTE*&		Buffer,
	const BYTE*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	check(Class==UFont::StaticClass());
	UFont* Font = new( InParent, Name, Flags )UFont;
	// note RF_Public because font textures can be referenced direclty by material expressions
	UTexture2D* Tex = CastChecked<UTexture2D>( UTextureFactory::FactoryCreateBinary( 
		UTexture2D::StaticClass(), Font, NAME_None, RF_Public, Context, Type, Buffer, BufferEnd, Warn ) );

	if( Tex != NULL )
	{
		Tex->LODGroup = TEXTUREGROUP_UI;  // set the LOD group otherwise this will be in the World Group

		// Also, we never want to stream in font textures since that always looks awful
		Tex->NeverStream = TRUE;

		Font->Textures.AddItem(Tex);

		// Init.
		BYTE* TextureData = (BYTE*) Tex->Mips(0).Data.Lock(LOCK_READ_WRITE);
		Font->Characters.AddZeroed( NUM_FONT_CHARS );

		// Scan in all fonts, starting at glyph 32.
		UINT i = 32;
		INT Y = 0;
		do
		{
			INT X = 0;
			while( AT(TextureData,Tex->SizeX,X,Y)!=255 && Y<Tex->SizeY )
			{
				X++;
				if( X >= Tex->SizeX )
				{
					X = 0;
					if( ++Y >= Tex->SizeY )
						break;
				}
			}

			// Scan all glyphs in this row.
			if( Y < Tex->SizeY )
			{
				INT XL=0, YL=0, MaxYL=0;
				while( i<(UINT)Font->Characters.Num() && ScanFontBox(TextureData,X,Y,XL,YL,Tex->SizeX) )
				{
					Font->Characters(i).StartU = X+1;
					Font->Characters(i).StartV = Y+1;
					Font->Characters(i).USize  = XL;
					Font->Characters(i).VSize  = YL;
					Font->Characters(i).TextureIndex = 0;
					Font->Characters(i).VerticalOffset = 0;
					X += XL + 1;
					i++;
					if( YL > MaxYL )
						MaxYL = YL;
				}
				Y += MaxYL + 1;
			}
		} while( i<(UINT)Font->Characters.Num() && Y<Tex->SizeY );

		Tex->Mips(0).Data.Unlock();

		// Cleanup font data.
		for( i=0; i<(UINT)Tex->Mips.Num(); i++ )
		{
			BYTE* MipData = (BYTE*) Tex->Mips(i).Data.Lock(LOCK_READ_WRITE);
			for( INT j=0; j<Tex->Mips(i).Data.GetBulkDataSize(); j++ )
			{
				if( MipData[j]==255 )
				{
					MipData[j] = 0;
				}
			}
			Tex->Mips(i).Data.Unlock();
		}

		// Remap old fonts.
		TArray<FFontCharacter> Old = Font->Characters;
		for( i=0; i<(UINT)Font->Characters.Num(); i++ )
		{
			Font->Characters(i) = Old(FontRemap[i]);
		}

		Font->CacheCharacterCountAndMaxCharHeight();

		return Font;
	}
	else 
	{
		return NULL;
	}
}
IMPLEMENT_CLASS(UFontFactory);

/*------------------------------------------------------------------------------
	USequenceFactory implementation.
------------------------------------------------------------------------------*/
/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void USequenceFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = USequence::StaticClass();
	new(Formats)FString(TEXT("t3d;Unreal Sequence"));
	bCreateNew = FALSE;
	bText = 1;
	bEditorImport = 1;
}
USequenceFactory::USequenceFactory()
{
	bEditorImport = 1;
}

/**
 *	Create a USequence from text.
 *
 *	@param	InParent	Usually the parent sequence, but might be a package for example. Used as outer for created SequenceObjects.
 *	@param	Flags		Flags used when creating SequenceObjects
 *	@param	Type		If "paste", the newly created SequenceObjects are added to the selected object set.
 *	@param	Buffer		Text buffer with description of sequence
 *	@param	BufferEnd	End of text info buffer
 *	@param	Warn		Device for squirting warnings to
 *
 *	
 *	Note that we assume that all subobjects of a USequence are SequenceObjects, and will be explicitly added to its SequenceObjects array ( SequenceObjects(x)=... lines are ignored )
 *	This is because objects may already exist in parent sequence etc. It does mean that if we ever add non-SequenceObject subobjects to Sequence it won't work. Hmm..
 */
UObject* USequenceFactory::FactoryCreateText
(
	UClass*				UnusedClass,
	UObject*			InParent,
	FName				UnusedName,
	EObjectFlags		Flags,
	UObject*			UnusedContext,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
	USequence* ParentSeq = Cast<USequence>(InParent);

	// We keep a mapping of new, empty sequence objects to their property text.
	// We want to create all new SequenceObjects first before importing their properties (which will create links)
	TArray<USequenceObject*>		NewSeqObjs;
	TMap<USequenceObject*,FString>	PropMap;
	TMap<USequence*,FString>		SubObjMap;

	const UBOOL bIsPasting = (appStricmp(Type,TEXT("paste"))==0);
	const UBOOL bIsUISequence = ParentSeq ? ParentSeq->IsA(UUISequence::StaticClass()) : FALSE;

	USequence* OutSequence = NULL;

	ParseNext( &Buffer );

	FString StrLine;
	while( ParseLine(&Buffer,StrLine) )
	{
		const TCHAR* Str = *StrLine;
		if( GetBEGIN(&Str,TEXT("OBJECT")) )
		{
			UClass* SeqObjClass;
			if( ParseObject<UClass>( Str, TEXT("CLASS="), SeqObjClass, ANY_PACKAGE ) )
			{
				//check( SeqObjClass->IsChildOf(USequenceObject::StaticClass()) );
				if (!SeqObjClass->IsChildOf(USequenceObject::StaticClass()))
				{
					continue;
				}

				USequenceObject* SeqObjCDO = SeqObjClass->GetDefaultObject<USequenceObject>();
				if ( (bIsUISequence && !SeqObjCDO->eventIsPastingIntoUISequenceAllowed())
				||	(!bIsUISequence && !SeqObjCDO->eventIsPastingIntoLevelSequenceAllowed()) )
				{
					continue;
				}



				FName SeqObjName(NAME_None);
				Parse( Str, TEXT("NAME="), SeqObjName );

				// Setup archetype
				FString SeqObjArchetypeName;
				Parse( Str, TEXT("ARCHETYPE="), SeqObjArchetypeName );
				UObject* SeqObjArchetype;
				SeqObjArchetype = LoadObject<UObject>(NULL, *SeqObjArchetypeName, NULL, LOAD_None, NULL);

				// Make sure this name is not used by anything else. Will rename other stuff if necessary
				ParentSeq->ClearNameUsage(SeqObjName);

				USequenceObject* NewSeqObj = ConstructObject<USequenceObject>( SeqObjClass, InParent, SeqObjName, Flags, SeqObjArchetype );
				USequence* NewSeq = Cast<USequence>(NewSeqObj);

				// Reset archetype for new component
				NewSeqObj->SetArchetype(NewSeqObj->GetClass()->GetDefaultObject<UObject>());

				// If our text contains multiple top-level Sequences, we will just return the last one parsed.
				if(NewSeq)
				{
					OutSequence = NewSeq;
				}


				// Get property text for the new sequence object.
				FString PropText, PropLine;
				FString SubObjText;
				INT ObjDepth = 1;
				while ( ParseLine( &Buffer, PropLine ) )
				{
					const TCHAR* PropStr = *PropLine;

					// Ignore lines that assign the objects to SequenceObjects.
					// We just add all SequenceObjects within this sequence to the SequenceObjects array. Thats because we may need to rename them etc.
					if( appStrstr(PropStr, TEXT(" SequenceObjects(")) )
						continue;

					// Track how deep we are in contained sets of sub-objects.
					UBOOL bEndLine = false;
					if( GetBEGIN(&PropStr, TEXT("OBJECT")) )
					{
						ObjDepth++;
					}
					else if( GetEND(&PropStr, TEXT("OBJECT")) )
					{
						bEndLine = true;

						// When close out our initial BEGIN OBJECT, we are done with this object.
						if(ObjDepth == 1)
						{
							break;
						}
					}

					// If this is a sequence object, we stick its sub-object properties in a separate string.
					if(NewSeq && ObjDepth > 1)
					{
						SubObjText += *PropLine;
						SubObjText += TEXT("\r\n");
					}
					else
					{
						PropText += *PropLine;
						PropText += TEXT("\r\n");
					}

					if(bEndLine)
					{
						ObjDepth--;
					}
				}

				// Save property text and possibly sub-object text in the case of sub-sequence.
				PropMap.Set( NewSeqObj, *PropText );
				if(NewSeq)
				{
					SubObjMap.Set( NewSeq, *SubObjText );
				}

				NewSeqObjs.AddItem(NewSeqObj);

				// Add sequence object to the parent sequence, and set the ParentSequence pointer.
				if(ParentSeq)
				{
					ParentSeq->AddSequenceObject(NewSeqObj);
				}
			}
		}
	}

	if(bIsPasting)
	{
		GEditor->GetSelectedObjects()->SelectNone( USequenceObject::StaticClass() );
	}

	for(INT i=0; i<NewSeqObjs.Num(); i++)
	{
		USequenceObject* NewSeqObj = NewSeqObjs(i);
		FString* PropText = PropMap.Find(NewSeqObj);
		check(PropText); // Every new object should have property text.

		// First, if this is a sub-sequence, use this factory again and feed in the sub object text to create all the sequence sub-objects.
		USequence* NewSeq = Cast<USequence>(NewSeqObj);
		if(NewSeq)
		{
			FString* SubObjText = SubObjMap.Find(NewSeq);
			check(SubObjText);

			const TCHAR* SubObjStr = **SubObjText;

			//debugf( TEXT("-----SUBOBJECT STRING------") );
			//debugf( TEXT("%s"), SubObjStr );

			USequenceFactory* TempSebSeqFactor = new USequenceFactory;
			TempSebSeqFactor->FactoryCreateText( NULL, NewSeqObj, NAME_None, Flags, NULL, TEXT(""), SubObjStr, SubObjStr+appStrlen(SubObjStr), GWarn );
		}

		// Now import properties into newly created sequence object. Should create links correctly...
		const TCHAR* PropStr = **PropText;

		//debugf( TEXT("-----PROP STRING------") );
		//debugf( TEXT("%s"), PropStr );

		NewSeqObj->PreEditChange(NULL);

		UInterpData* IData = Cast<UInterpData>(NewSeqObj);
		if(IData)
		{
			ImportObjectProperties( (BYTE*)NewSeqObj, PropStr, NewSeqObj->GetClass(), NewSeqObj, NewSeqObj, Warn, 0 );
		}
		else
		{
			ImportObjectProperties( (BYTE*)NewSeqObj, PropStr, NewSeqObj->GetClass(), NewSeqObj, InParent, Warn, 0 );
		}

		// If this is Matinee data - clear the CurveEdSetup as the references to tracks get screwed up by text export/import.
		if(IData)
		{
			IData->CurveEdSetup = NULL;
		}
	}

	// Now we do a final cleanup/update pass.
	// We do this afterwards, because things like CleanupConnections looks at numbe of inputs of other Ops,
	// so they all need to be imported first.
	for(INT i=0; i<NewSeqObjs.Num(); i++)
	{
		USequenceObject* NewSeqObj = NewSeqObjs(i);

		// If this is a sequence Op, ensure that no output, var or event links point to an SequenceObject with a different Outer ie. is in a different SubSequence.
		USequenceOp* SeqOp = Cast<USequenceOp>(NewSeqObj);
		if(SeqOp)
		{
			SeqOp->CleanupConnections();
		}

		// If this is a paste, add the newly created sequence objects to the selection list.
		if(bIsPasting)
		{
			GEditor->GetSelectedObjects()->Select(NewSeqObj);
		}

		NewSeqObj->PostEditChange(NULL);
	}

	return OutSequence;
}

IMPLEMENT_CLASS(USequenceFactory);


/*-----------------------------------------------------------------------------
UReimportTextureFactory.
-----------------------------------------------------------------------------*/
void UReimportTextureFactory::StaticConstructor()
{

	// This needs to be mirrored in UnTex.h, Texture.uc and UnEdFact.cpp.
	UEnum* Enum = new(GetClass(),TEXT("TextureCompressionSettings"),RF_Public) UEnum(NULL);
	TArray<FName> EnumNames;
	EnumNames.AddItem( FName(TEXT("TC_Default")) );
	EnumNames.AddItem( FName(TEXT("TC_Normalmap")) );
	EnumNames.AddItem( FName(TEXT("TC_Displacementmap")) );
	EnumNames.AddItem( FName(TEXT("TC_NormalmapAlpha")) );
	EnumNames.AddItem( FName(TEXT("TC_Grayscale")) );
	EnumNames.AddItem( FName(TEXT("TC_HighDynamicRange")) );
	Enum->SetEnums( EnumNames );

	new(GetClass()->HideCategories) FName(NAME_Object);
	new(GetClass(),TEXT("NoCompression")			,RF_Public) UBoolProperty(CPP_PROPERTY(NoCompression			),TEXT("Compression"),0							);
	new(GetClass(),TEXT("CompressionNoAlpha")		,RF_Public)	UBoolProperty(CPP_PROPERTY(NoAlpha					),TEXT("Compression"),CPF_Edit					);
	new(GetClass(),TEXT("CompressionSettings")		,RF_Public) UByteProperty(CPP_PROPERTY(CompressionSettings		),TEXT("Compression"),CPF_Edit, Enum			);	
	new(GetClass(),TEXT("DeferCompression")			,RF_Public) UBoolProperty(CPP_PROPERTY(bDeferCompression		),TEXT("Compression"),CPF_Edit					);
	new(GetClass(),TEXT("Create Material?")			,RF_Public) UBoolProperty(CPP_PROPERTY(bCreateMaterial			),TEXT("Compression"),CPF_Edit					);

	// This needs to be mirrored with Material.uc::EBlendMode
	UEnum* BlendEnum = new(GetClass(),TEXT("Blending"),RF_Public) UEnum(NULL);
	EnumNames.Empty();
	EnumNames.AddItem( FName(TEXT("BLEND_Opaque")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Masked")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Translucent")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Additive")) );
	EnumNames.AddItem( FName(TEXT("BLEND_Modulate")) );
	BlendEnum->SetEnums( EnumNames );

	// This needs to be mirrored with Material.uc::EMaterialLightingModel
	UEnum* LightingModelEnum = new(GetClass(),TEXT("LightingModel"),RF_Public) UEnum(NULL);
	EnumNames.Empty();
	EnumNames.AddItem( FName(TEXT("MLM_Phong")) );
	EnumNames.AddItem( FName(TEXT("MLM_NonDirectional")) );
	EnumNames.AddItem( FName(TEXT("MLM_Unlit")) );
	EnumNames.AddItem( FName(TEXT("MLM_Custom")) );
	LightingModelEnum->SetEnums( EnumNames );

	new(GetClass(),TEXT("RGB To Diffuse")			,RF_Public) UBoolProperty(CPP_PROPERTY(bRGBToDiffuse			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("RGB To Emissive")			,RF_Public) UBoolProperty(CPP_PROPERTY(bRGBToEmissive			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Specular")		,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToSpecular			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Emissive")		,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToEmissive			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Opacity")			,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToOpacity			),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Alpha To Opacity Mask")	,RF_Public) UBoolProperty(CPP_PROPERTY(bAlphaToOpacityMask		),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Two Sided?")				,RF_Public) UBoolProperty(CPP_PROPERTY(bTwoSided				),TEXT("Create Material"),CPF_Edit				);
	new(GetClass(),TEXT("Blending")					,RF_Public) UByteProperty(CPP_PROPERTY(Blending					),TEXT("Create Material"),CPF_Edit, BlendEnum	);
	new(GetClass(),TEXT("Lighting Model")			,RF_Public) UByteProperty(CPP_PROPERTY(LightingModel			),TEXT("Create Material"),CPF_Edit, LightingModelEnum	);	
	new(GetClass(),TEXT("FlipBook")					,RF_Public) UBoolProperty(CPP_PROPERTY(bFlipBook				),TEXT("FlipBook"),		  CPF_Edit				);	
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UReimportTextureFactory::InitializeIntrinsicPropertyValues()
{
	// The texture reimport factory declares the supported class to
	// be UTextureFlipBookUReimportTexture (a child of UTexture2D) so that
	// flip-book textures can reimport.
	SupportedClass = UTextureFlipBook::StaticClass();
	new(Formats)FString(TEXT("bmp;Texture"));
	new(Formats)FString(TEXT("pcx;Texture"));
	new(Formats)FString(TEXT("tga;Texture"));
	new(Formats)FString(TEXT("float;Texture"));
	new(Formats)FString(TEXT("psd;Texture")); 
	bCreateNew = FALSE;
}
UReimportTextureFactory::UReimportTextureFactory()
{
	pOriginalTex = NULL;
}

UTexture2D* UReimportTextureFactory::CreateTexture( UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags )
{
	if (pOriginalTex)
	{
		// Prepare texture to be modified. We're going to clobber the bulk data so we can't continue to load it from the
		// persistent archive that might be attached.
		pOriginalTex->bHasBeenLoadedFromPersistentArchive = FALSE;
		// Update with new settings, which should disable streaming...
		pOriginalTex->UpdateResource();
		// ... and for good measure flush rendering commands.
		FlushRenderingCommands();
		return pOriginalTex;
	}
	else
	{
		return Super::CreateTexture( Class, InParent, Name, Flags );
	}
}

/**
* Reimports specified texture from its source material, if the meta-data exists
*/
UBOOL UReimportTextureFactory::Reimport( UObject* Obj )
{
	if(!Obj || !Obj->IsA(UTexture2D::StaticClass()))
		return FALSE;

	UTexture2D* pTex = Cast<UTexture2D>(Obj);
	
	pOriginalTex = pTex;

	if (!(pTex->SourceFilePath.Len()))
	{
		// Since this is a new system most textures don't have paths, so logigng has been commented out
		//GWarn->Log( TEXT("-- cannot reimport: texture resource does not have path stored."));
		return FALSE;
	}

	GWarn->Log( FString::Printf(TEXT("Performing atomic reimport of [%s]"),*(pTex->SourceFilePath)) );

	FFileManager::timestamp  TS,MyTS;
	if (!GFileManager->GetTimestamp( *(pTex->SourceFilePath), TS ))
	{
		GWarn->Log( TEXT("-- cannot reimport: source file cannot be found."));
		return FALSE;
	}

	// Pull the timestamp from the user readable string.
	// It would be nice if this was stored directly, and maybe it will be if
	// its decided that UTC dates are too confusing to the users.
	appMemzero(&MyTS,sizeof(MyTS));
	MyTS.Year   = appAtoi(*(pTex->SourceFileTimestamp.Mid(0,4)));
	MyTS.Month  = appAtoi(*(pTex->SourceFileTimestamp.Mid(5,2)))-1;
	MyTS.Day    = appAtoi(*(pTex->SourceFileTimestamp.Mid(8,2)));
	MyTS.Hour   = appAtoi(*(pTex->SourceFileTimestamp.Mid(11,2)));
	MyTS.Minute = appAtoi(*(pTex->SourceFileTimestamp.Mid(14,2)));
	MyTS.Second = appAtoi(*(pTex->SourceFileTimestamp.Mid(17,2)));    

	if (MyTS < TS)
	{        
		GWarn->Log( TEXT("-- file on disk exists and is newer.  Performing import."));

		// We use this reimport factory to skip the object creation process
		// which obliterates all of the properties of the texture.
		// Also preset the factory with the settings of the current texture.
		// These will be used during the import and compression process.        
		CompressionSettings   = pTex->CompressionSettings;
		NoCompression         = pTex->CompressionNone;
		NoAlpha               = pTex->CompressionNoAlpha;
		bDeferCompression     = pTex->DeferCompression;
#if GEMINI_TODO
		bFlipBook             = pTex->bFlipBook;
#endif
		// Cache the width/height so we can see if we need to re-evaluate the
		// compression settings
		INT Width = pTex->SizeX;
		INT Height = pTex->SizeY;

		// Handle flipbook textures as well...
#if GEMINI_TODO
		if (pTex->IsA(UTextureFlipBook::StaticClass()))
		{
			bFlipBook	= TRUE;
		}
#endif

		// Suppress the import overwrite dialog because we know that for explicitly re-importing we want to preserve existing settings
		UTextureFactory::SuppressImportOverwriteDialog();

		if (UFactory::StaticImportObject(pTex->GetClass(), pTex->GetOuter(), *pTex->GetName(), RF_Public|RF_Standalone, *(pTex->SourceFilePath), NULL, this))
		{
			GWarn->Log( TEXT("-- imported successfully") );
			// Try to find the outer package so we can dirty it up
			if (pTex->GetOuter())
			{
				pTex->GetOuter()->MarkPackageDirty();
			}
			else
			{
				pTex->MarkPackageDirty();
			}
			// Check for a size change that might require a compression change
			if (NoCompression == TRUE &&
				(Width != pTex->SizeX || Height != pTex->SizeY))
			{
				// Re-enable compression
				pTex->CompressionNone = FALSE;
				pTex->DeferCompression = TRUE;
			}
		}
		else
		{
			GWarn->Log( TEXT("-- import failed") );
		}
	}
	else
	{
		GWarn->Log( TEXT("-- Texture is already up to date."));
	}
	return TRUE;
}

IMPLEMENT_CLASS(UReimportTextureFactory);

///////////////////////////////////////////////////////////////////////////////////////////////
//
// UColladaFactory
//
///////////////////////////////////////////////////////////////////////////////////////////////

UColladaFactory::UColladaFactory()
{
	bEditorImport			= 1;
	bImportAsSkeletalMesh	= FALSE;
}

void UColladaFactory::StaticConstructor()
{
	// COLLADA factory.
	new(GetClass(), TEXT("bImportAsSkeletalMesh"), RF_Public) UBoolProperty(CPP_PROPERTY(bImportAsSkeletalMesh), TEXT("COLLADA"), CPF_Edit);

	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UColladaFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass = NULL;

	new(Formats)FString(TEXT("dae;Collada Skeletal Mesh"));

	bCreateNew	= FALSE;
	bText		= TRUE;

	bEditorImport			= 1;
	bImportAsSkeletalMesh	= FALSE;
}

UClass* UColladaFactory::ResolveSupportedClass()
{
	UClass* ImportClass = NULL;
	if ( bImportAsSkeletalMesh )
	{
		ImportClass = USkeletalMesh::StaticClass();
	}
	else
	{
		ImportClass = UStaticMesh::StaticClass();
	}
	return ImportClass;
}

UObject* UColladaFactory::FactoryCreateText
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	EObjectFlags		Flags,
	UObject*			Context,
	const TCHAR*		Type,
	const TCHAR*&		Buffer,
	const TCHAR*		BufferEnd,
	FFeedbackContext*	Warn
)
{
	UObject* NewObject = NULL;

	UnCollada::CImporter* ColladaImporter = UnCollada::CImporter::GetInstance();

	Warn->BeginSlowTask( TEXT("Importing COLLADA mesh"), TRUE );
	if ( !ColladaImporter->ImportFromText( UFactory::CurrentFilename, Buffer, BufferEnd ) )
	{
		// Log the error message and fail the import.
		Warn->Log( NAME_Error, ColladaImporter->GetErrorMessage() );
	}
	else
	{
		TArray<const TCHAR*> AllNames;

		ColladaImporter->RetrieveEntityList(AllNames, TRUE);
		ColladaImporter->RetrieveEntityList(AllNames, FALSE);

		if (AllNames.Num() > 0)
		{
			// Log the import message and import the mesh.
			Warn->Log( ColladaImporter->GetErrorMessage() );

			WxDlgCOLLADAResourceList Dialog;
			Dialog.FillResourceList( AllNames );
			if ( Dialog.ShowModal() == wxID_OK )
			{
				const FScopedBusyCursor BusyCursor;

				TArray<FString> NamesToImport;
				Dialog.GetSelectedResources( NamesToImport );

				FName NameToImport( Name );
				if ( bImportAsSkeletalMesh )
				{
					NewObject = ColladaImporter->ImportSkeletalMesh( InParent, NamesToImport, NameToImport, Flags );
				}
				else
				{
					for ( INT NameIndex = 0 ; NameIndex < NamesToImport.Num() ; ++NameIndex )
					{
						const FString& ResourceName = NamesToImport(NameIndex);

						// If importing multiple objects, mangle the object name with the COLLADA resource name.
						if ( NamesToImport.Num() > 0 )
						{
							NameToImport = FName( *FString::Printf(TEXT("%s_%s"), *Name.ToString(), *ResourceName ) );
						}

						Warn->StatusUpdatef( NameIndex, NamesToImport.Num(), *FString::Printf(LocalizeSecure(LocalizeUnrealEd("Importingf"), NameIndex, NamesToImport.Num())) );
						NewObject = ColladaImporter->ImportStaticMesh( InParent, *ResourceName, NameToImport, Flags );
					}
				}
			}
		}
	}
	ColladaImporter->CloseDocument();
	Warn->EndSlowTask();

	return NewObject;
}

IMPLEMENT_CLASS(UColladaFactory);

/*------------------------------------------------------------------------------
	UCurveEdPresetCurveFactoryNew implementation.
------------------------------------------------------------------------------*/

//
//	UCurveEdPresetCurveFactoryNew::StaticConstructor
//
void UCurveEdPresetCurveFactoryNew::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UCurveEdPresetCurveFactoryNew::InitializeIntrinsicPropertyValues()
{
	SupportedClass	= UCurveEdPresetCurve::StaticClass();
	bCreateNew		= TRUE;
	bEditAfterNew   = TRUE;
	Description		= TEXT("CurveEdPresetCurve");
}
//
//	UCurveEdPresetCurveFactoryNew::FactoryCreateNew
//
UObject* UCurveEdPresetCurveFactoryNew::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	return StaticConstructObject(Class,InParent,Name,Flags);
}

IMPLEMENT_CLASS(UCurveEdPresetCurveFactoryNew);




/*------------------------------------------------------------------------------
	UCameraAnimFactory implementation.
------------------------------------------------------------------------------*/

//
//	UCameraAnimFactory::StaticConstructor
//
void UCameraAnimFactory::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UCameraAnimFactory::InitializeIntrinsicPropertyValues()
{
	SupportedClass	= UCameraAnim::StaticClass();
	bCreateNew		= 1;
	Description		= TEXT("Camera Animation");
}
//
//	UCameraAnimFactory::FactoryCreateNew
//
UObject* UCameraAnimFactory::FactoryCreateNew(UClass* Class,UObject* InParent,FName Name,EObjectFlags Flags,UObject* Context,FFeedbackContext* Warn)
{
	UCameraAnim* NewCamAnim = CastChecked<UCameraAnim>(StaticConstructObject(Class,InParent,Name,Flags));
	NewCamAnim->CameraInterpGroup = CastChecked<UInterpGroup>(StaticConstructObject(UInterpGroup::StaticClass(), NewCamAnim));
	return NewCamAnim;
}

IMPLEMENT_CLASS(UCameraAnimFactory);


IMPLEMENT_CLASS(USpeedTreeFactory);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	USpeedTreeFactory::StaticConstructor

void USpeedTreeFactory::StaticConstructor( )
{
#if WITH_SPEEDTREE
	Description	= TEXT("SpeedTree");
	RandomSeed = 1;

	new(GetClass( )->HideCategories) FName(TEXT("Object"));
	new(GetClass( ), TEXT("RandomSeed"), RF_Public) UIntProperty(CPP_PROPERTY(RandomSeed), TEXT("SpeedTree"), CPF_Edit);
#endif
}

USpeedTreeFactory::USpeedTreeFactory()
{
#if WITH_SPEEDTREE
	bEditorImport = 1;
	RandomSeed = 1;
#endif
}

void USpeedTreeFactory::InitializeIntrinsicPropertyValues( )
{
#if WITH_SPEEDTREE
	SupportedClass = USpeedTree::StaticClass( );
	new(Formats)FString(TEXT("spt;SpeedTree"));
	bCreateNew = 0;
	bText = 0;
#endif
}

UObject* USpeedTreeFactory::FactoryCreateBinary(UClass* Class,
												UObject* Outer,
												FName Name,
												EObjectFlags Flags,
												UObject* Context,
												const TCHAR* Type,
												const BYTE*& Buffer,
												const BYTE* BufferEnd,
												FFeedbackContext* Warn)
{
#if WITH_SPEEDTREE
	if( appStricmp(Type, TEXT("SPT")) == 0 )
	{
		INT	RandomSeed = 1;		

		UMaterialInterface* BranchMaterial = NULL;			
		UMaterialInterface* FrondMaterial = NULL;			
		UMaterialInterface* LeafMaterial = NULL;			
		UMaterialInterface* BillboardMaterial = NULL;

		USpeedTree* ExistingSpeedTree = FindObject<USpeedTree>(Outer, *Name.ToString( ));
		if( ExistingSpeedTree != NULL )
		{
			RandomSeed			= ExistingSpeedTree->RandomSeed;
			BranchMaterial		= ExistingSpeedTree->BranchMaterial;		
			FrondMaterial		= ExistingSpeedTree->FrondMaterial;		
			LeafMaterial		= ExistingSpeedTree->LeafMaterial;			
			BillboardMaterial	= ExistingSpeedTree->BillboardMaterial;
		}

		USpeedTree* SpeedTree = CastChecked<USpeedTree>(StaticConstructObject(Class, Outer, Name, Flags));

		SpeedTree->RandomSeed			= RandomSeed;
		SpeedTree->BranchMaterial		= BranchMaterial;
		SpeedTree->FrondMaterial		= FrondMaterial;
		SpeedTree->LeafMaterial			= LeafMaterial;
		SpeedTree->BillboardMaterial	= BillboardMaterial;

		UINT NumBytes = BufferEnd - Buffer;
		SpeedTree->SRH = new FSpeedTreeResourceHelper( SpeedTree );

		#if WITH_SPEEDTREE_MANGLE
			BYTE* Data = (BYTE*) appMalloc( NumBytes );
			appMemcpy(Data, Buffer, NumBytes);
			unsigned int KeyLength = appStrlen(SPEEDTREE_MANGLE_KEY);
			for (UINT i = 0; i < NumBytes; ++i)
			{	
				INT Byte = Data[i] - SPEEDTREE_MANGLE_KEY[i % KeyLength];
				if (Byte < 0)
					Byte += 256;
				Data[i] = (char)Byte;
			}
			SpeedTree->SRH->Load(Data, NumBytes);
			appFree(Data);

			if( !SpeedTree->IsInitialized() )
			{
				// tree failed to load, try unencrypted
				SpeedTree->SRH->Load(Buffer, NumBytes);
			}
		#else
			SpeedTree->SRH->Load(Buffer, NumBytes);
		#endif

		if( SpeedTree->IsInitialized() )
		{
			return SpeedTree;
		}
	}
#endif
	return NULL;
}
