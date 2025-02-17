/*=============================================================================
	UCContentCommandlets.cpp: Various commmandlets.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EditorPrivate.h"
#include "EngineMaterialClasses.h"

#include "../../UnrealEd/Inc/SourceControlIntegration.h"

/** A FMaterialCompiler implementation which is used to traverse the material's expression tree. */
class FGatherExpressionsCompiler : public FMaterialCompiler
{
public:

	TArray<UMaterialExpression*> Expressions;
	TArray<UMaterialExpression*> ExpressionStack;

	virtual INT Error(const TCHAR* Text)
	{
		return INDEX_NONE;
	}

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler)
	{
		if(ExpressionStack.FindItemIndex(MaterialExpression) == INDEX_NONE)
		{
			Expressions.AddItem(MaterialExpression);
			ExpressionStack.Push(MaterialExpression);
			MaterialExpression->Compile(this);
			check(ExpressionStack.Pop() == MaterialExpression);
		}
		return INDEX_NONE;
	}

	virtual EMaterialValueType GetType(INT Code)
	{
		return MCT_Unknown;
	}
	virtual INT ForceCast(INT Code,EMaterialValueType DestType)
	{
		return INDEX_NONE;
	}

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue)
	{
		return INDEX_NONE;
	}
	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue)
	{
		return INDEX_NONE;
	}

	virtual INT FlipBookOffset(UTexture* InFlipBook)
	{
		return INDEX_NONE;
	}

	virtual INT Constant(FLOAT X)
	{
		return INDEX_NONE;
	}
	virtual INT Constant2(FLOAT X,FLOAT Y)
	{
		return INDEX_NONE;
	}
	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z)
	{
		return INDEX_NONE;
	}
	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W)
	{
		return INDEX_NONE;
	}

	virtual INT GameTime()
	{
		return INDEX_NONE;
	}
	virtual INT RealTime()
	{
		return INDEX_NONE;
	}
	virtual INT PeriodicHint(INT PeriodicCode) { return PeriodicCode; }

	virtual INT Sine(INT X)
	{
		return INDEX_NONE;
	}
	virtual INT Cosine(INT X)
	{
		return INDEX_NONE;
	}

	virtual INT Floor(INT X)
	{
		return INDEX_NONE;
	}
	virtual INT Ceil(INT X)
	{
		return INDEX_NONE;
	}
	virtual INT Frac(INT X)
	{
		return INDEX_NONE;
	}
	virtual INT Abs(INT X)
	{
		return INDEX_NONE;
	}

	virtual INT ReflectionVector()
	{
		return INDEX_NONE;
	}
	virtual INT CameraVector()
	{
		return INDEX_NONE;
	}
	virtual INT CameraWorldPosition()
	{
		return INDEX_NONE;
	}
	virtual INT LightVector()
	{
		return INDEX_NONE;
	}

	virtual INT ScreenPosition( UBOOL bScreenAlign )
	{
		return INDEX_NONE;
	}

	virtual INT WorldPosition()
	{
		return INDEX_NONE;
	}

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB)
	{
		return INDEX_NONE;
	}

	virtual INT TextureCoordinate(UINT CoordinateIndex)
	{
		return INDEX_NONE;
	}
	virtual INT TextureSample(INT Texture,INT Coordinate)
	{
		return INDEX_NONE;
	}

	virtual INT Texture(UTexture* Texture)
	{
		return INDEX_NONE;
	}
	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture)
	{
		return INDEX_NONE;
	}

	virtual	INT SceneTextureSample( BYTE TexType, INT CoordinateIdx, UBOOL ScreenAlign )
	{
		return INDEX_NONE;
	}
	virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)
	{
		return INDEX_NONE;
	}
	virtual	INT PixelDepth(UBOOL bNormalize)
	{
		return INDEX_NONE;
	}
	virtual	INT DestColor()
	{
		return INDEX_NONE;
	}
	virtual	INT DestDepth(UBOOL bNormalize)
	{
		return INDEX_NONE;
	}
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx )
	{
		return INDEX_NONE;
	}
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx )
	{
		return INDEX_NONE;
	}

	virtual INT VertexColor()
	{
		return INDEX_NONE;
	}

	virtual INT Add(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Sub(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Mul(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Div(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Dot(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Cross(INT A,INT B)
	{
		return INDEX_NONE;
	}

	virtual INT Power(INT Base,INT Exponent)
	{
		return INDEX_NONE;
	}
	virtual INT SquareRoot(INT X)
	{
		return INDEX_NONE;
	}

	virtual INT Lerp(INT X,INT Y,INT A)
	{
		return INDEX_NONE;
	}
	virtual INT Min(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Max(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT Clamp(INT X,INT A,INT B)
	{
		return INDEX_NONE;
	}

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A)
	{
		return INDEX_NONE;
	}
	virtual INT AppendVector(INT A,INT B)
	{
		return INDEX_NONE;
	}
	virtual INT TransformVector(BYTE CoordType,INT A)
	{
		return INDEX_NONE;
	}
	virtual INT TransformPosition(BYTE CoordType,INT A)
	{
		return INDEX_NONE;
	}

	INT LensFlareIntesity()
	{
		return INDEX_NONE;
	}

	INT LensFlareOcclusion()
	{
		return INDEX_NONE;
	}

	INT LensFlareRadialDistance()
	{
		return INDEX_NONE;
	}

	INT LensFlareRayDistance()
	{
		return INDEX_NONE;
	}

	INT LensFlareSourceDistance()
	{
		return INDEX_NONE;
	}

	INT DynamicParameter()
	{
		return INDEX_NONE;
	}
};

/*-----------------------------------------------------------------------------
	UFixAmbiguousMaterialParameters commandlet.
-----------------------------------------------------------------------------*/

//
INT UFixAmbiguousMaterialParametersCommandlet::Main( const FString& Params )
{
	const TCHAR* Parms = *Params;

	// Retrieve list of all packages in .ini paths.
	TArray<FString> PackageList;

	FString PackageWildcard;
	FString PackagePrefix;
	if(ParseToken(Parms,PackageWildcard,FALSE))
	{
		GFileManager->FindFiles(PackageList,*PackageWildcard,TRUE,FALSE);
		PackagePrefix = FFilename(PackageWildcard).GetPath() * TEXT("");
	}
	else
	{
		PackageList = GPackageFileCache->GetPackageFileList();
	}
	if( !PackageList.Num() )
		return 0;

#if HAVE_SCC
	FSourceControlIntegration* SCC = new FSourceControlIntegration;
#endif

	// Iterate over all packages.
	for( INT PackageIndex = 0; PackageIndex < PackageList.Num(); PackageIndex++ )
	{
		FFilename Filename = PackagePrefix * PackageList(PackageIndex);

		if( Filename.GetExtension() == TEXT("U") )
		{
			warnf(NAME_Log, TEXT("Skipping script file %s"), *Filename);
		}
		else
		{
			warnf(NAME_Log, TEXT("Loading %s"), *Filename);

			// Assert if package couldn't be opened so we have no chance of messing up saving later packages.
			UPackage* Package = UObject::LoadPackage( NULL, *Filename, 0 );
			UBOOL bPackageDirty = FALSE;
			if (Package != NULL)
			{
				for(TObjectIterator<UMaterial> MaterialIt;MaterialIt;++MaterialIt)
				{
					if(MaterialIt->IsIn(Package))
					{
						UBOOL bMaterialDirty = FALSE;
						// Traverse the material expressions in the order they would have been used pre-Gemini.
						FGatherExpressionsCompiler GatherExpressionsCompiler;
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_DiffuseColor,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_DiffusePower,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_SpecularColor,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_Normal,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_EmissiveColor,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_Opacity,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_OpacityMask,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_Distortion,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_TwoSidedLightingMask,&GatherExpressionsCompiler);
						MaterialIt->MaterialResources[MSP_SM3]->CompileProperty(MSP_SM3,MP_CustomLighting,&GatherExpressionsCompiler);

						// Use the order of the expressions to build an unambiguous parameter value map.
						TMap<FName,FLinearColor> ParameterValueMap;
						for(INT ExpressionIndex = 0;ExpressionIndex < GatherExpressionsCompiler.Expressions.Num();ExpressionIndex++)
						{
							UMaterialExpression* Expression = GatherExpressionsCompiler.Expressions(ExpressionIndex);
							UMaterialExpressionVectorParameter* VectorParameter = Cast<UMaterialExpressionVectorParameter>(Expression);
							UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(Expression);

							if(VectorParameter)
							{
								ParameterValueMap.Set(VectorParameter->ParameterName,VectorParameter->DefaultValue);
							}
							else if(ScalarParameter)
							{
								ParameterValueMap.Set(ScalarParameter->ParameterName,FLinearColor(ScalarParameter->DefaultValue,0,0,0));
							}
						}

						// Set the unambiguous parameter values as the parameter default values.
						for (INT ExpressionIndex = 0;ExpressionIndex < MaterialIt->Expressions.Num();ExpressionIndex++)
						{
							UMaterialExpression* Expression = MaterialIt->Expressions(ExpressionIndex);
							UMaterialExpressionVectorParameter* VectorParameter = Cast<UMaterialExpressionVectorParameter>(Expression);
							UMaterialExpressionScalarParameter* ScalarParameter = Cast<UMaterialExpressionScalarParameter>(Expression);

							if(VectorParameter)
							{
								const FLinearColor* Value = ParameterValueMap.Find(VectorParameter->ParameterName);
								if(Value && !(VectorParameter->DefaultValue == *Value))
								{
									VectorParameter->DefaultValue = *Value;
									bMaterialDirty = TRUE;
								}
							}
							else if(ScalarParameter && ScalarParameter->ParameterName == NAME_None)
							{
								const FLinearColor* Value = ParameterValueMap.Find(ScalarParameter->ParameterName);
								if(Value && ScalarParameter->DefaultValue != Value->R)
								{
									ScalarParameter->DefaultValue = Value->R;
									bMaterialDirty = TRUE;
								}
							}
						}

						if(bMaterialDirty)
						{
							bPackageDirty = TRUE;

							// Log information about the material that was updated.
							warnf(TEXT("Fixed ambiguous parameters for material %s"),*MaterialIt->GetPathName());

							// Recompile the material.
							MaterialIt->PreEditChange(NULL);
							MaterialIt->PostEditChange(NULL);
						}
					}
				}

				if(bPackageDirty)
				{
					if (GFileManager->IsReadOnly(*Filename))
					{
#if HAVE_SCC
						SCC->CheckOut(Package);
#endif
						if (GFileManager->IsReadOnly(*Filename))
						{
							continue;
						}
					}
					// resave the package
					UWorld* World = FindObject<UWorld>( Package, TEXT("TheWorld") );
					if( World )
					{	
						UObject::SavePackage( Package, World, 0, *Filename, GWarn );
					}
					else
					{
						UObject::SavePackage( Package, NULL, RF_Standalone, *Filename, GWarn );
					}
				}
			}
		}

		UObject::CollectGarbage(RF_Native);
		SaveLocalShaderCaches();
	}

#if HAVE_SCC
	delete SCC; // clean up our allocated SCC
#endif

	return 0;
}
IMPLEMENT_CLASS(UFixAmbiguousMaterialParametersCommandlet)
