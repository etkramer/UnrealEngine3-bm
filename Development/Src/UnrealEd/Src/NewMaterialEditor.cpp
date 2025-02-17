/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineMaterialClasses.h"
#include "GenericBrowser.h"
#include "MaterialEditorBase.h"
#include "NewMaterialEditor.h"
#include "MaterialEditorContextMenus.h"
#include "MaterialEditorPreviewScene.h"
#include "MaterialEditorToolBar.h"
#include "UnLinkedObjDrawUtils.h"
#include "BusyCursor.h"
#include "ScopedTransaction.h"
#include "Properties.h"

IMPLEMENT_CLASS( UMaterialEditorOptions );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static helpers.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

/** Location of the realtime preview icon, relative to the upper-hand corner of the material expression node. */
static const FIntPoint PreviewIconLoc( 2, 6 );

/** Width of the realtime preview icon. */
static const INT PreviewIconWidth = 10;

static const FColor ConnectionNormalColor(0,0,0);
static const FColor ConnectionSelectedColor(255,128,0);

} // namespace

/**
 * An input to a compound material expression.
 */
class FCompoundInput
{
public:
	UMaterialExpression*	Expression;
	UStructProperty*		ExpressionInput;
};

/**
 * An output from a compound material expression.
 */
class FCompoundOutput
{
public:
	UMaterialExpression*	Expression;
	FExpressionOutput		ExpressionOutput;
	FCompoundOutput():
		ExpressionOutput(FALSE)
	{}
};

/**
 * Information 
 */
class FCompoundInfo
{
public:
	/** Inputs to the compound expression. */
	TArray<FCompoundInput> CompoundInputs;
	/** Outputs to the compound expression. */
	TArray<FCompoundOutput> CompoundOutputs;
};

/**
 * Class for rendering previews of material expressions in the material editor's linked object viewport.
 */
class FExpressionPreview : public FMaterial, public FMaterialRenderProxy
{
public:
	FExpressionPreview()
	{}

	FExpressionPreview(UMaterialExpression* InExpression)
		:	Expression( InExpression )
	{
		CacheShaders();
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
	virtual UBOOL ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const
	{
		if(VertexFactoryType == FindVertexFactoryType(FName(TEXT("FLocalVertexFactory"), FNAME_Find)))
		{
			// we only need the non-light-mapped, base pass, local vertex factory shaders for drawing an opaque Material Tile
			// @todo: Added a FindShaderType by fname or something"
			if(appStristr(ShaderType->GetName(), TEXT("BasePassVertexShaderFNoLightMapPolicyFNoDensityPolicy")))
			{
				return TRUE;
			}
			else if(appStristr(ShaderType->GetName(), TEXT("BasePassPixelShaderFNoLightMapPolicy")))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	////////////////
	// FMaterialRenderProxy interface.

	virtual const FMaterial* GetMaterial() const
	{
		if(GetShaderMap())
		{
			return this;
		}
		else
		{
			return GEngine->DefaultMaterial->GetRenderProxy(FALSE)->GetMaterial();
		}
	}

	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
	{
		return Expression->GetOuterUMaterial()->GetRenderProxy(0)->GetVectorValue(ParameterName, OutValue, Context);
	}

	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
	{
		return Expression->GetOuterUMaterial()->GetRenderProxy(0)->GetScalarValue(ParameterName, OutValue, Context);
	}

	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
	{
		return Expression->GetOuterUMaterial()->GetRenderProxy(0)->GetTextureValue(ParameterName,OutValue,Context);
	}

	
	// Material properties.
	virtual INT CompileProperty(EMaterialShaderPlatform MatPlatform,EMaterialProperty Property,FMaterialCompiler* Compiler) const
	{
		if( Property == MP_EmissiveColor )
		{
			return Expression->Compile(Compiler);
		}
		else
		{
			return Compiler->Constant(1.0f);
		}
	}

	virtual UBOOL IsTwoSided() const { return FALSE; }
	virtual UBOOL NeedsDepthTestDisabled() const { return FALSE; }
	virtual UBOOL IsLightFunction() const { return FALSE; }
	virtual UBOOL IsUsedWithFogVolumes() const { return FALSE; }
	virtual UBOOL IsSpecialEngineMaterial() const { return FALSE; }
	virtual UBOOL IsTerrainMaterial() const { return FALSE; }
	virtual UBOOL IsDecalMaterial() const { return FALSE; }
	virtual UBOOL IsWireframe() const { return FALSE; }
	virtual UBOOL IsDistorted() const { return FALSE; }
	virtual UBOOL IsMasked() const { return FALSE; }
	virtual enum EBlendMode GetBlendMode() const { return BLEND_Opaque; }
	virtual enum EMaterialLightingModel GetLightingModel() const { return MLM_Unlit; }
	virtual FLOAT GetOpacityMaskClipValue() const { return 0.5f; }
	virtual FString GetFriendlyName() const { return TEXT("Expression preview"); }
	/**
	 * Should shaders compiled for this material be saved to disk?
	 */
	virtual UBOOL IsPersistent() const { return FALSE; }

	const UMaterialExpression* GetExpression() const
	{
		return Expression;
	}

	friend FArchive& operator<< ( FArchive& Ar, FExpressionPreview& V )
	{
		return Ar << V.Expression;
	}

private:
	UMaterialExpression* Expression;
};

namespace {

/**
 * Class for rendering the material on the preview mesh in the Material Editor
 */
class FPreviewMaterial : public FMaterialResource, public FMaterialRenderProxy
{
public:
	FPreviewMaterial(UMaterial* InMaterial)
	:	FMaterialResource(InMaterial)
	{
		CacheShaders();
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
	virtual UBOOL ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const
	{
		// only generate the needed shaders (which should be very restrictive)
		// @todo: Add a FindShaderType by fname or something

		// Always allow HitProxy shaders.
		if (appStristr(ShaderType->GetName(), TEXT("HitProxy")))
		{
			return TRUE;
		}

		// we only need local vertex factory for the preview static mesh
		if (VertexFactoryType != FindVertexFactoryType(FName(TEXT("FLocalVertexFactory"), FNAME_Find)))
		{
			//cache for gpu skinned vertex factory if the material allows it
			//this way we can have a preview skeletal mesh
			if (!IsUsedWithSkeletalMesh() || VertexFactoryType != FindVertexFactoryType(FName(TEXT("FGPUSkinVertexFactory"), FNAME_Find)))
			{
				return FALSE;
			}
		}

		// look for any of the needed type
		UBOOL bShaderTypeMatches = FALSE;
		if (appStristr(ShaderType->GetName(), TEXT("BasePassPixelShaderFNoLightMapPolicy")))
		{
			bShaderTypeMatches = TRUE;
		}
		//used for instruction count on lit materials
		else if (appStristr(ShaderType->GetName(), TEXT("TBasePassPixelShaderFDirectionalLightMapTexturePolicyNoSkyLight")))
		{
			bShaderTypeMatches = TRUE;
		}
		//used for instruction count on lit materials
		else if (appStristr(ShaderType->GetName(), TEXT("TLightPixelShaderFPointLightPolicyFNoStaticShadowingPolicy")))
		{
			bShaderTypeMatches = TRUE;
		}
		else if (appStristr(ShaderType->GetName(), TEXT("BasePassVertexShaderFNoLightMapPolicyFNoDensityPolicy")))
		{
			bShaderTypeMatches = TRUE;
		}
		else if (appStristr(ShaderType->GetName(), TEXT("TDistortion")))
		{
			bShaderTypeMatches = TRUE;
		}
		else if (appStristr(ShaderType->GetName(), TEXT("TLight")))
		{
			if (appStristr(ShaderType->GetName(), TEXT("FDirectionalLightPolicyFShadowTexturePolicy")) ||
				appStristr(ShaderType->GetName(), TEXT("FDirectionalLightPolicyFNoStaticShadowingPolicy")))
			{
				bShaderTypeMatches = TRUE;
			}
		}
		else if (IsUsedWithFogVolumes() 
			&& (appStristr(ShaderType->GetName(), TEXT("FFogVolumeApply"))
			|| appStristr(ShaderType->GetName(), TEXT("TFogIntegral")) && appStristr(ShaderType->GetName(), TEXT("FConstantDensityPolicy"))))
		{
			bShaderTypeMatches = TRUE;
		}
		else if (appStristr(ShaderType->GetName(), TEXT("DepthOnly")))
		{
			bShaderTypeMatches = TRUE;
		}
		else if (appStristr(ShaderType->GetName(), TEXT("FShadowDepth")))
		{
			bShaderTypeMatches = TRUE;
		}

		return bShaderTypeMatches;
	}


	// FMaterialRenderProxy interface
	virtual const FMaterial* GetMaterial() const
	{
		if(GetShaderMap())
		{
			return this;
		}
		else
		{
			return GEngine->DefaultMaterial->GetRenderProxy(FALSE)->GetMaterial();
		}
	}

	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
	{
		return Material->GetRenderProxy(0)->GetVectorValue(ParameterName, OutValue, Context);
	}

	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
	{
		return Material->GetRenderProxy(0)->GetScalarValue(ParameterName, OutValue, Context);
	}

	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
	{
		return Material->GetRenderProxy(0)->GetTextureValue(ParameterName,OutValue,Context);
	}
};

} // namespace

/**
 * Helper class to use on the Material Editor preview mesh to use a FPreviewMaterial
 * instead of FMaterialResource
 */
class UPreviewMaterial : public UMaterial
{
	DECLARE_CLASS(UPreviewMaterial,UMaterial,CLASS_Intrinsic,UnrealEd)

	/**
	 * Allocates a material resource off the heap to be stored in MaterialResource.
	 */
	virtual FMaterialResource* AllocateResource()
	{
		return new FPreviewMaterial(this);
	}
};

IMPLEMENT_CLASS(UPreviewMaterial);

namespace {

/**
 * Hitproxy for the realtime preview icon on material expression nodes.
 */
struct HRealtimePreviewProxy : public HHitProxy
{
	DECLARE_HIT_PROXY( HRealtimePreviewProxy, HHitProxy );

	UMaterialExpression*	MaterialExpression;

	HRealtimePreviewProxy(UMaterialExpression* InMaterialExpression)
		:	HHitProxy( HPP_UI )
		,	MaterialExpression( InMaterialExpression )
	{}
	virtual void Serialize(FArchive& Ar)
	{
		Ar << MaterialExpression;
	}
};

/**
 * Returns a reference to the material to be used as the material expression copy-paste buffer.
 */
static UMaterial* GetCopyPasteBuffer()
{
	if ( !GUnrealEd->MaterialCopyPasteBuffer )
	{
		GUnrealEd->MaterialCopyPasteBuffer = ConstructObject<UMaterial>( UMaterial::StaticClass() );
	}
	return GUnrealEd->MaterialCopyPasteBuffer;
}

/**
 * Returns the UStruct corrsponding to UMaterialExpression::ExpressionInput.
 */
static const UStruct* GetExpressionInputStruct()
{
	static const UStruct* ExpressionInputStruct = FindField<UStruct>( UMaterialExpression::StaticClass(), TEXT("ExpressionInput") );
	check( ExpressionInputStruct );
	return ExpressionInputStruct;
}

/**
 * Allows access to a material's input by index.
 */
static FExpressionInput* GetMaterialInput(UMaterial* Material, INT Index)
{
	FExpressionInput* ExpressionInput = NULL;
	switch( Index )
	{
	case 0: ExpressionInput = &Material->DiffuseColor ; break;
	case 1: ExpressionInput = &Material->DiffusePower ; break;
	case 2: ExpressionInput = &Material->EmissiveColor ; break;
	case 3: ExpressionInput = &Material->SpecularColor ; break;
	case 4: ExpressionInput = &Material->SpecularPower ; break;
	case 5: ExpressionInput = &Material->Opacity ; break;
	case 6: ExpressionInput = &Material->OpacityMask ; break;
	case 7: ExpressionInput = &Material->Distortion ; break;
	case 8: ExpressionInput = &Material->TwoSidedLightingMask ; break;
	case 9: ExpressionInput = &Material->TwoSidedLightingColor ; break;
	case 10: ExpressionInput = &Material->Normal ; break;
	case 11: ExpressionInput = &Material->CustomLighting ; break;
	default: appErrorf( TEXT("%i: Invalid material input index"), Index );
	}
	return ExpressionInput;
}

/**
 * Connects the specified input expression to the specified output expression.
 */
static void ConnectExpressions(FExpressionInput& Input, const FExpressionOutput& Output, UMaterialExpression* Expression)
{
	Input.Expression = Expression;
	Input.Mask = Output.Mask;
	Input.MaskR = Output.MaskR;
	Input.MaskG = Output.MaskG;
	Input.MaskB = Output.MaskB;
	Input.MaskA = Output.MaskA;
}

/**
 * Connects the MaterialInputIndex'th material input to the MaterialExpressionOutputIndex'th material expression output.
 */
static void ConnectMaterialToMaterialExpression(UMaterial* Material,
												INT MaterialInputIndex,
												UMaterialExpression* MaterialExpression,
												INT MaterialExpressionOutputIndex)
{
	// Assemble a list of outputs this material expression has.
	TArray<FExpressionOutput> Outputs;
	MaterialExpression->GetOutputs(Outputs);

	const FExpressionOutput& ExpressionOutput = Outputs( MaterialExpressionOutputIndex );
	FExpressionInput* MaterialInput = GetMaterialInput( Material, MaterialInputIndex );

	ConnectExpressions( *MaterialInput, ExpressionOutput, MaterialExpression );
}

/**
 * Breaks all liks to the specified expression.
 */
static void BreakLinksToExpression(UMaterialExpression* InMaterialExpression, UMaterial* Material)
{
	// Iterate over all expressions in the material and clear any references they have to the specified material expression.
	for ( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		MaterialExpression->Modify();
		MaterialExpression->SwapReferenceTo( InMaterialExpression, NULL );
	}

	Material->Modify();
#define __CLEAR_REFERENCE_TO_EXPRESSION( MatExpr, Mat, Prop ) \
	if ( Mat->Prop.Expression == InMaterialExpression ) { Mat->Prop.Expression = NULL; }

	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, DiffuseColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, DiffusePower );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, EmissiveColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, SpecularColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, SpecularPower );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Opacity );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, OpacityMask );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Distortion );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, TwoSidedLightingMask );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, TwoSidedLightingColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Normal );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, CustomLighting );

#undef __CLEAR_REFERENCE_TO_EXPRESSION
}

/**
 * Assembles a list of FExpressionInput objects that refer to the specified FExpressionOutput.
 */
static void GetListOfReferencingInputs(const UMaterialExpression* InMaterialExpression, UMaterial* Material, const FExpressionOutput& MaterialExpressionOutput, TArray<FExpressionInput*>& OutReferencingInputs)
{
	OutReferencingInputs.Empty();

	for ( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		const TArray<UStructProperty*>& ExpressionInputs = WxMaterialEditor::GetMaterialExpressionInputs( MaterialExpression );

		for ( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ExpressionInputIndex )
		{
			UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
			if ( Input->Expression == InMaterialExpression &&
				Input->Mask == MaterialExpressionOutput.Mask &&
				Input->MaskR == MaterialExpressionOutput.MaskR &&
				Input->MaskG == MaterialExpressionOutput.MaskG &&
				Input->MaskB == MaterialExpressionOutput.MaskB &&
				Input->MaskA == MaterialExpressionOutput.MaskA )
			{
				OutReferencingInputs.AddItem( Input );
			}
		}
	}

#define __CLEAR_REFERENCE_TO_EXPRESSION( MatExpr, Mat, Prop ) \
	if ( Mat->Prop.Expression == InMaterialExpression && \
	Mat->Prop.Mask == MaterialExpressionOutput.Mask && \
	Mat->Prop.MaskR == MaterialExpressionOutput.MaskR && \
	Mat->Prop.MaskG == MaterialExpressionOutput.MaskG && \
	Mat->Prop.MaskB == MaterialExpressionOutput.MaskB && \
	Mat->Prop.MaskA == MaterialExpressionOutput.MaskA ) { OutReferencingInputs.AddItem( &(Mat->Prop) ); }

	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, DiffuseColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, DiffusePower );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, EmissiveColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, SpecularColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, SpecularPower );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Opacity );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, OpacityMask );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Distortion );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, TwoSidedLightingMask );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, TwoSidedLightingColor );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, Normal );
	__CLEAR_REFERENCE_TO_EXPRESSION( InMaterialExpression, Material, CustomLighting );
#undef __CLEAR_REFERENCE_TO_EXPRESSION
}

/**
 * Copies the SrcExpressions into the specified material.  Preserves internal references.
 * New material expressions are created within the specified material.
 */
static void CopyMaterialExpressions(const TArray<UMaterialExpression*>& SrcExpressions, const TArray<UMaterialExpressionComment*>& SrcExpressionComments, 
									UMaterial* Material, TArray<UMaterialExpression*>& OutNewExpressions, TArray<UMaterialExpression*>& OutNewComments)
{
	OutNewExpressions.Empty();
	OutNewComments.Empty();

	TMap<UMaterialExpression*,UMaterialExpression*> SrcToDestMap;

	// Duplicate source expressions into the editor's material copy buffer.
	for( INT SrcExpressionIndex = 0 ; SrcExpressionIndex < SrcExpressions.Num() ; ++SrcExpressionIndex )
	{
		UMaterialExpression*	SrcExpression		= SrcExpressions(SrcExpressionIndex);
		UMaterialExpression*	NewExpression		= Cast<UMaterialExpression>(UObject::StaticDuplicateObject( SrcExpression, SrcExpression, Material, NULL, RF_Transactional ));

		SrcToDestMap.Set( SrcExpression, NewExpression );

		// Add to list of material expressions associated with the copy buffer.
		Material->Expressions.AddItem( NewExpression );

		// Generate GUID if we are copying a parameter expression
		UMaterialExpressionParameter* ParameterExpression = Cast<UMaterialExpressionParameter>( NewExpression );
		if( ParameterExpression )
		{
			ParameterExpression->ConditionallyGenerateGUID(TRUE);
		}

		UMaterialExpressionTextureSampleParameter* TextureParameterExpression = Cast<UMaterialExpressionTextureSampleParameter>( NewExpression );
		if( TextureParameterExpression )
		{
			TextureParameterExpression->ConditionallyGenerateGUID(TRUE);
		}

		// Record in output list.
		OutNewExpressions.AddItem( NewExpression );
	}

	// Fix up internal references.  Iterate over the inputs of the new expressions, and for each input that refers
	// to an expression that was duplicated, point the reference to that new expression.  Otherwise, clear the input.
	for( INT NewExpressionIndex = 0 ; NewExpressionIndex < OutNewExpressions.Num() ; ++NewExpressionIndex )
	{
		UMaterialExpression* NewExpression = OutNewExpressions(NewExpressionIndex);

		const TArray<UStructProperty*>& ExpressionInputs = WxMaterialEditor::GetMaterialExpressionInputs( NewExpression );
		for ( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ExpressionInputIndex )
		{
			const UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)NewExpression + ExpressionInput->Offset);
			UMaterialExpression* InputExpression = Input->Expression;
			if ( InputExpression )
			{
				UMaterialExpression** NewExpression = SrcToDestMap.Find( InputExpression );
				if ( NewExpression )
				{
					check( *NewExpression );
					Input->Expression = *NewExpression;
				}
				else
				{
					Input->Expression = NULL;
				}
			}
		}
	}

	// Copy Selected Comments
	for( INT CommentIndex=0; CommentIndex<SrcExpressionComments.Num(); CommentIndex++)
	{
		UMaterialExpressionComment* ExpressionComment = SrcExpressionComments(CommentIndex);
		UMaterialExpressionComment* NewComment = Cast<UMaterialExpressionComment>(UObject::StaticDuplicateObject(ExpressionComment, ExpressionComment, Material, NULL));

		// Add reference to the material
		Material->EditorComments.AddItem(NewComment);

		// Add to the output array.
		OutNewComments.AddItem(NewComment);
	}
}

/**
 * Populates the specified material's Expressions array (eg if cooked out or old content).
 * Also ensures materials and expressions are RF_Transactional for undo/redo support.
 */
static void InitExpressions(UMaterial* Material)
{
	FString ParmName;

	if( Material->Expressions.Num() == 0 )
	{
		for( TObjectIterator<UMaterialExpression> It; It; ++It )
		{
			UMaterialExpression* MaterialExpression = *It;
			if( MaterialExpression->GetOuter() == Material )
			{
				// Comment expressions are stored in a separate list.
				if ( MaterialExpression->IsA( UMaterialExpressionComment::StaticClass() ) )
				{
					Material->EditorComments.AddItem( static_cast<UMaterialExpressionComment*>(MaterialExpression) );
				}
				else
				{
					Material->Expressions.AddItem( MaterialExpression );
				}

				Material->AddExpressionParameter(MaterialExpression);
			}
		}
	}
	else
	{
		Material->BuildEditorParameterList();
	}

	// Propagate RF_Transactional to all referenced material expressions.
	Material->SetFlags( RF_Transactional );
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->Expressions.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( MaterialExpressionIndex );
		MaterialExpression->SetFlags( RF_Transactional );
	}
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->EditorComments.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpressionComment* Comment = Material->EditorComments( MaterialExpressionIndex );
		Comment->SetFlags( RF_Transactional );
	}
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->EditorCompounds.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpressionCompound* Compound = Material->EditorCompounds( MaterialExpressionIndex );
		Compound->SetFlags( RF_Transactional );
	}

}

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WxMaterialEditor
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/** 
 * TRUE if the list of UMaterialExpression-derived classes shared between material
 * editors has already been created.
 */
UBOOL			WxMaterialEditor::bMaterialExpressionClassesInitialized = FALSE;

/** Static array of UMaterialExpression-derived classes, shared between all material editor instances. */
TArray<UClass*> WxMaterialEditor::MaterialExpressionClasses;

/** Static array of categorized material expression classes. */
TArray<FCategorizedMaterialExpressionNode> WxMaterialEditor::CategorizedMaterialExpressionClasses;
TArray<UClass*> WxMaterialEditor::FavoriteExpressionClasses;
TArray<UClass*> WxMaterialEditor::UnassignedExpressionClasses;

/** Maps UMaterialExpression-derived classes to a list of their inputs. */
TMap<UClass*,TArray<UStructProperty*> > WxMaterialEditor::InputStructClassMap;

/**
 * Comparison function used to sort material expression classes by name.
 */
IMPLEMENT_COMPARE_POINTER( UClass, MaterialEditor, { return appStricmp( *A->GetName(), *B->GetName() ); } )

/**
 *	Grab the expression array for the given category.
 *	
 *	@param	InCategoryName	The category to retrieve
 *	@param	bCreate			If TRUE, create the entry if not found.
 *
 *	@return	The category node.
 */
FCategorizedMaterialExpressionNode* WxMaterialEditor::GetCategoryNode(FString& InCategoryName, UBOOL bCreate)
{
	for (INT CheckIndex = 0; CheckIndex < CategorizedMaterialExpressionClasses.Num(); CheckIndex++)
	{
		FCategorizedMaterialExpressionNode* CheckNode = &(CategorizedMaterialExpressionClasses(CheckIndex));
		if (CheckNode)
		{
			if (CheckNode->CategoryName == InCategoryName)
			{
				return CheckNode;
			}
		}
	}

	if (bCreate == TRUE)
	{
		FCategorizedMaterialExpressionNode* NewNode = new(CategorizedMaterialExpressionClasses)FCategorizedMaterialExpressionNode;
		check(NewNode);

		NewNode->CategoryName = InCategoryName;
		return NewNode;
	}

	return NULL;
}

/** Sorting helper... */
IMPLEMENT_COMPARE_CONSTREF(FCategorizedMaterialExpressionNode,NewMaterialEditor,{ return A.CategoryName > B.CategoryName ? 1 : -1; });

/**
 * Initializes the list of UMaterialExpression-derived classes shared between all material editor instances.
 */
void WxMaterialEditor::InitMaterialExpressionClasses()
{
	if( !bMaterialExpressionClassesInitialized )
	{
		UMaterialEditorOptions* TempEditorOptions = ConstructObject<UMaterialEditorOptions>( UMaterialEditorOptions::StaticClass() );
		UClass* BaseType = UMaterialExpression::StaticClass();
		if( BaseType )
		{
			TArray<UStructProperty*>	ExpressionInputs;
			const UStruct*				ExpressionInputStruct = GetExpressionInputStruct();

			for( TObjectIterator<UClass> It ; It ; ++It )
			{
				UClass* Class = *It;
				if( !(Class->ClassFlags & CLASS_Abstract) && !(Class->ClassFlags & CLASS_Deprecated) )
				{
					if( Class->IsChildOf(UMaterialExpression::StaticClass()) )
					{
						ExpressionInputs.Empty();

						// Exclude comments and compounds from the expression list.
						if ( Class != UMaterialExpressionComment::StaticClass() && Class != UMaterialExpressionCompound::StaticClass() )
						{
							MaterialExpressionClasses.AddItem( Class );

							// Initialize the expression class input map.							
							for( TFieldIterator<UStructProperty,CASTCLASS_UStructProperty> InputIt(Class) ; InputIt ; ++InputIt )
							{
								UStructProperty* StructProp = *InputIt;
								if( StructProp->Struct == ExpressionInputStruct )
								{
									ExpressionInputs.AddItem( StructProp );
								}
							}

							// See if it is in the favorites array...
							for (INT FavoriteIndex = 0; FavoriteIndex < TempEditorOptions->FavoriteExpressions.Num(); FavoriteIndex++)
							{
								if (Class->GetName() == TempEditorOptions->FavoriteExpressions(FavoriteIndex))
								{
									FavoriteExpressionClasses.AddUniqueItem(Class);
								}
							}

							// Category fill...
							UMaterialExpression* TempObject = Cast<UMaterialExpression>(Class->GetDefaultObject());
							if (TempObject)
							{
								if (TempObject->MenuCategories.Num() == 0)
								{
									UnassignedExpressionClasses.AddItem(Class);
								}
								else
								{
									for (INT CategoryIndex = 0; CategoryIndex < TempObject->MenuCategories.Num(); CategoryIndex++)
									{
										FString TempName = TempObject->MenuCategories(CategoryIndex).ToString();
										FCategorizedMaterialExpressionNode* CategoryNode = GetCategoryNode(
											TempName, TRUE);
										check(CategoryNode);

										CategoryNode->MaterialExpressionClasses.AddUniqueItem(Class);
									}
								}
							}
						}
						
						// map the expression class to its input structs
						InputStructClassMap.Set( Class, ExpressionInputs );
					}
				}
			}
		}

		Sort<USE_COMPARE_POINTER(UClass,MaterialEditor)>( static_cast<UClass**>(MaterialExpressionClasses.GetTypedData()), MaterialExpressionClasses.Num() );
		Sort<USE_COMPARE_CONSTREF(FCategorizedMaterialExpressionNode,NewMaterialEditor)>(&(CategorizedMaterialExpressionClasses(0)),CategorizedMaterialExpressionClasses.Num());

		bMaterialExpressionClassesInitialized = TRUE;
	}
}

/**
 * Remove the expression from the favorites menu list.
 */
void WxMaterialEditor::RemoveMaterialExpressionFromFavorites(UClass* InClass)
{
	FavoriteExpressionClasses.RemoveItem(InClass);
}

/**
 * Add the expression to the favorites menu list.
 */
void WxMaterialEditor::AddMaterialExpressionToFavorites(UClass* InClass)
{
	FavoriteExpressionClasses.AddUniqueItem(InClass);
}

/**
 * Assembles a list of inputs for the specified material expression.
 */
const TArray<UStructProperty*>& WxMaterialEditor::GetMaterialExpressionInputs(UMaterialExpression* MaterialExpression)
{
	UClass* MaterialExpressionClass				= MaterialExpression->GetClass();
	TArray<UStructProperty*>* ExpressionInputs	= InputStructClassMap.Find( MaterialExpressionClass );
	return *ExpressionInputs;
}

BEGIN_EVENT_TABLE( WxMaterialEditor, WxMaterialEditorBase )
	EVT_CLOSE( WxMaterialEditor::OnClose )
	EVT_MENU_RANGE( IDM_NEW_SHADER_EXPRESSION_START, IDM_NEW_SHADER_EXPRESSION_END, WxMaterialEditor::OnNewMaterialExpression )
	EVT_MENU( ID_MATERIALEDITOR_NEW_COMMENT, WxMaterialEditor::OnNewComment )
	EVT_MENU( ID_MATERIALEDITOR_NEW_COMPOUND_EXPRESSION, WxMaterialEditor::OnNewCompoundExpression )
	EVT_MENU( IDM_USE_CURRENT_TEXTURE, WxMaterialEditor::OnUseCurrentTexture )

	EVT_TOOL( IDM_SHOW_BACKGROUND, WxMaterialEditor::OnShowBackground )
	EVT_TOOL( ID_MATERIALEDITOR_TOGGLEGRID, WxMaterialEditor::OnToggleGrid )
	EVT_TOOL( ID_MATERIALEDITOR_SHOWHIDE_CONNECTORS, WxMaterialEditor::OnShowHideConnectors )
	EVT_TOOL( ID_MATERIALEDITOR_TOGGLE_DRAW_CURVES, WxMaterialEditor::OnToggleDrawCurves )

	EVT_TOOL( ID_MATERIALEDITOR_REALTIME_EXPRESSIONS, WxMaterialEditor::OnRealTimeExpressions )
	EVT_TOOL( ID_MATERIALEDITOR_ALWAYS_REFRESH_ALL_PREVIEWS, WxMaterialEditor::OnAlwaysRefreshAllPreviews )

	EVT_UPDATE_UI( ID_MATERIALEDITOR_REALTIME_EXPRESSIONS, WxMaterialEditor::UI_RealTimeExpressions )
	EVT_UPDATE_UI( ID_MATERIALEDITOR_SHOWHIDE_CONNECTORS, WxMaterialEditor::UI_HideUnusedConnectors )

	EVT_TOOL( ID_MATERIALEDITOR_APPLY, WxMaterialEditor::OnApply )
	EVT_UPDATE_UI( ID_MATERIALEDITOR_APPLY, WxMaterialEditor::UI_Apply )

	EVT_TOOL( ID_MATERIALEDITOR_PROPAGATE_TO_FALLBACK, WxMaterialEditor::OnPropagateToFallback )
	EVT_UPDATE_UI( ID_MATERIALEDITOR_PROPAGATE_TO_FALLBACK, WxMaterialEditor::UI_PropagateToFallback )

	EVT_TOOL( ID_MATERIALEDITOR_REGENERATE_AUTO_FALLBACK, WxMaterialEditor::OnRegenerateAutoFallback )
	EVT_UPDATE_UI( ID_MATERIALEDITOR_REGENERATE_AUTO_FALLBACK, WxMaterialEditor::UI_RegenerateAutoFallback )

	EVT_TOOL( ID_MATERIALEDITOR_OPEN_FALLBACK, WxMaterialEditor::OnOpenFallback )
	EVT_UPDATE_UI( ID_MATERIALEDITOR_OPEN_FALLBACK, WxMaterialEditor::UI_OpenFallback )

	EVT_TOOL( ID_GO_HOME, WxMaterialEditor::OnCameraHome )
	EVT_TOOL( ID_MATERIALEDITOR_CLEAN_UNUSED_EXPRESSIONS, WxMaterialEditor::OnCleanUnusedExpressions )

	EVT_LIST_BEGIN_DRAG( ID_MATERIALEDITOR_MATERIALEXPRESSIONLIST, WxMaterialEditor::OnMaterialExpressionListDrag )

	EVT_MENU( ID_MATERIALEDITOR_DUPLICATE_NODES, WxMaterialEditor::OnDuplicateObjects )
	EVT_MENU( ID_MATERIALEDITOR_DELETE_NODE, WxMaterialEditor::OnDeleteObjects )
	EVT_MENU( ID_MATERIALEDITOR_BREAK_LINK, WxMaterialEditor::OnBreakLink )
	EVT_MENU( ID_MATERIALEDITOR_BREAK_ALL_LINKS, WxMaterialEditor::OnBreakAllLinks )
	EVT_MENU( ID_MATERIALEDITOR_REMOVE_FROM_FAVORITES, WxMaterialEditor::OnRemoveFromFavorites )
	EVT_MENU( ID_MATERIALEDITOR_ADD_TO_FAVORITES, WxMaterialEditor::OnAddToFavorites )

	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_DiffuseColor, WxMaterialEditor::OnConnectToMaterial_DiffuseColor )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_DiffusePower, WxMaterialEditor::OnConnectToMaterial_DiffusePower )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_EmissiveColor, WxMaterialEditor::OnConnectToMaterial_EmissiveColor )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_SpecularColor, WxMaterialEditor::OnConnectToMaterial_SpecularColor )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_SpecularPower, WxMaterialEditor::OnConnectToMaterial_SpecularPower )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_Opacity, WxMaterialEditor::OnConnectToMaterial_Opacity )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_OpacityMask, WxMaterialEditor::OnConnectToMaterial_OpacityMask )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_Distortion, WxMaterialEditor::OnConnectToMaterial_Distortion)
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_TransmissionMask, WxMaterialEditor::OnConnectToMaterial_TransmissionMask )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_TransmissionColor, WxMaterialEditor::OnConnectToMaterial_TransmissionColor )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_Normal, WxMaterialEditor::OnConnectToMaterial_Normal )
	EVT_MENU( ID_MATERIALEDITOR_CONNECT_TO_CustomLighting, WxMaterialEditor::OnConnectToMaterial_CustomLighting )
END_EVENT_TABLE()

/**
 * A panel containing a list of material expression node types.
 */
class WxMaterialExpressionList : public wxPanel
{
public:
	WxMaterialExpressionList(wxWindow* InParent)
		: wxPanel( InParent )
	{
		MaterialExpressionList = new wxListCtrl( this, ID_MATERIALEDITOR_MATERIALEXPRESSIONLIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL );

		wxBoxSizer* Sizer = new wxBoxSizer(wxVERTICAL);
		Sizer->Add( MaterialExpressionList, 1, wxGROW|wxALL, 4 );
		SetSizer( Sizer );
		SetAutoLayout(true);
	}

	void RepopulateList()
	{
		MaterialExpressionList->Freeze();
		MaterialExpressionList->ClearAll();

		MaterialExpressionList->InsertColumn( 0, *LocalizeUnrealEd("MaterialExpressions"), wxLIST_FORMAT_CENTRE, 170 );
		check( WxMaterialEditor::bMaterialExpressionClassesInitialized );
		INT ID = IDM_NEW_SHADER_EXPRESSION_START;
		for( INT Index = 0 ; Index < WxMaterialEditor::MaterialExpressionClasses.Num() ; ++Index )
		{
			UClass* MaterialExpressionClass = WxMaterialEditor::MaterialExpressionClasses(Index);
			MaterialExpressionList->InsertItem( Index, *FString(*MaterialExpressionClass->GetName()).Mid(appStrlen(TEXT("MaterialExpression"))) );
		}
		checkMsg( ID <= IDM_NEW_SHADER_EXPRESSION_END, "Insuffcient number of material expression IDs" );

		MaterialExpressionList->Thaw();
	}

	FString GetSelectedString()
	{
		FString Result;
		const long ItemIndex = MaterialExpressionList->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
		if ( ItemIndex != -1 )
		{
			Result = FString::Printf( TEXT("%i"), ItemIndex );
		}
		return Result;
	}

private:
	wxListCtrl* MaterialExpressionList;
};

WxMaterialEditor::WxMaterialEditor(wxWindow* InParent, wxWindowID InID, UMaterial* InMaterial)
	:	WxMaterialEditorBase( InParent, InID, InMaterial )
	,  FDockingParent(this)
	,	ConnObj( NULL )
	,	ConnType( LOC_INPUT )
	,	ConnIndex( 0 )
	,	OriginalMaterial( InMaterial )
	,	ToolBar( NULL )
	,	MaterialExpressionList( NULL )
	,	EditorOptions( NULL )
	,	bMaterialDirty( FALSE )
	,	bAlwaysRefreshAllPreviews( FALSE )
	,	bUseUnsortedMenus( FALSE )
	,	bHideUnusedConnectors( FALSE )
	,	bDrawCurves( TRUE )
	,	ScopedTransaction( NULL )
{
	// Create a copy of the material for preview usage (duplicating to a different class than original!)
	// Propagate all object flags except for RF_Standalone, otherwise the preview material won't GC once
	// the material editor releases the reference.
	Material = (UMaterial*)UObject::StaticDuplicateObject(OriginalMaterial, OriginalMaterial, UObject::GetTransientPackage(), NULL, ~RF_Standalone, UPreviewMaterial::StaticClass()); 
	// copy material usage
	for( INT Usage=0; Usage < MATUSAGE_MAX; Usage++ )
	{
		const EMaterialUsage UsageEnum = (EMaterialUsage)Usage;
		if( OriginalMaterial->GetUsageByFlag(UsageEnum) )
		{
			UBOOL bNeedsRecompile=FALSE;
			Material->SetMaterialUsage(bNeedsRecompile,UsageEnum);
		}
	}
	SetPreviewMaterial( Material );

	// Mark material as being the preview material to avoid unnecessary work in UMaterial::PostEditChange. This property is duplicatetransient so we don't have
	// to worry about resetting it when propagating the preview to the original material after editing.
	// Note:  The material editor must reattach the preview meshes through RefreshPreviewViewport() after calling Material::PEC()!
	Material->bIsPreviewMaterial = TRUE;

	// Set the material editor window title to include the material being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("MaterialEditorCaption_F"), *OriginalMaterial->GetPathName()) ) );

	// Initialize the shared list of material expression classes.
	InitMaterialExpressionClasses();

	// Make sure the material is the list of material expressions it references.
	InitExpressions( Material );

	// Set default size
	SetSize(1024,768);

	// Load the desired window position from .ini file.
	FWindowUtil::LoadPosSize(TEXT("MaterialEditor"), this, 256, 256, 1024, 768);

	// Create property window.
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );
	
	// Create a material editor drop target.
	/** Drop target for the material editor's linked object viewport. */
	class WxMaterialEditorDropTarget : public wxTextDropTarget
	{
	public:
		WxMaterialEditorDropTarget(WxMaterialEditor* InMaterialEditor)
			: MaterialEditor( InMaterialEditor )
		{}
		virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text)
		{
			long MaterialExpressionIndex;
			if ( text.ToLong(&MaterialExpressionIndex) )
			{
				UClass* NewExpressionClass = MaterialEditor->MaterialExpressionClasses(MaterialExpressionIndex);
				const INT LocX = (x - MaterialEditor->LinkedObjVC->Origin2D.X)/MaterialEditor->LinkedObjVC->Zoom2D;
				const INT LocY = (y - MaterialEditor->LinkedObjVC->Origin2D.Y)/MaterialEditor->LinkedObjVC->Zoom2D;
				MaterialEditor->CreateNewMaterialExpression( NewExpressionClass, FALSE, FIntPoint(LocX, LocY) );
			}
			return true;
		}
		WxMaterialEditor* MaterialEditor;
	};
	WxMaterialEditorDropTarget* MaterialEditorDropTarget = new WxMaterialEditorDropTarget(this);

	// Create linked-object tree window.
	WxLinkedObjVCHolder* TreeWin = new WxLinkedObjVCHolder( this, -1, this );
	TreeWin->SetDropTarget( MaterialEditorDropTarget );
	LinkedObjVC = TreeWin->LinkedObjVC;
	LinkedObjVC->SetRedrawInTick( FALSE );

	// Create material expression list.
	MaterialExpressionList = new WxMaterialExpressionList( this );
	MaterialExpressionList->RepopulateList();

	// Add docking windows.
	{
		SetDockHostSize(FDockingParent::DH_Bottom, 150);
		SetDockHostSize(FDockingParent::DH_Right, 150);

		AddDockingWindow( TreeWin, FDockingParent::DH_None, NULL );

		AddDockingWindow(PropertyWindow, FDockingParent::DH_Bottom, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PropertiesCaption_F"), *OriginalMaterial->GetPathName())), *LocalizeUnrealEd("Properties"));
		AddDockingWindow((wxWindow*)PreviewWin, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("PreviewCaption_F"), *OriginalMaterial->GetPathName())), *LocalizeUnrealEd("Preview"));
		AddDockingWindow(MaterialExpressionList, FDockingParent::DH_Right, *FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialExpressionListCaption_F"), *OriginalMaterial->GetPathName())), *LocalizeUnrealEd("MaterialExpressions"));

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}
	
	wxMenuBar* MenuBar = new wxMenuBar();
	AppendWindowMenu(MenuBar);
	SetMenuBar(MenuBar);

	ToolBar = new WxMaterialEditorToolBar( this, -1 );
	SetToolBar( ToolBar );
	LinkedObjVC->Origin2D = FIntPoint(-Material->EditorX,-Material->EditorY);

	BackgroundTexture = LoadObject<UTexture2D>(NULL, TEXT("EditorMaterials.MaterialsBackground"), NULL, LOAD_None, NULL);

	// Load editor settings from disk.
	LoadEditorSettings();

	// Set the preview mesh for the material.  This call must occur after the toolbar is initialized.
	if ( !SetPreviewMesh( *Material->PreviewMesh ) )
	{
		// The material preview mesh couldn't be found or isn't loaded.  Default to the one of the primitive types.
		SetPrimitivePreview();
	}

	// Initialize the contents of the property window.
	UpdatePropertyWindow();

	// Initialize the material input list.
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Diffuse"), &Material->DiffuseColor ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("DiffusePower"), &Material->DiffusePower ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Emissive"), &Material->EmissiveColor ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Specular"), &Material->SpecularColor ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("SpecularPower"), &Material->SpecularPower ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Opacity"), &Material->Opacity ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("OpacityMask"), &Material->OpacityMask ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Distortion"), &Material->Distortion ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("TransmissionMask"), &Material->TwoSidedLightingMask ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("TransmissionColor"), &Material->TwoSidedLightingColor ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("Normal"), &Material->Normal ) );
	MaterialInputs.AddItem( FMaterialInputInfo( TEXT("CustomLighting"), &Material->CustomLighting ) );

	// Initialize expression previews.
	ForceRefreshExpressionPreviews();

	// Initialize compound expression info.
	FlushCompoundExpressionInfo( TRUE );
}

WxMaterialEditor::~WxMaterialEditor()
{
	// make sure rendering thread is done with the expressions, since they are about to be deleted
	FlushRenderingCommands();

	// Delete compound info.
	FlushCompoundExpressionInfo( FALSE );

	// Save editor settings to disk.
	SaveEditorSettings();

	check( !ScopedTransaction );
}

/**
 * Load editor settings from disk (docking state, window pos/size, option state, etc).
 */
void WxMaterialEditor::LoadEditorSettings()
{
	EditorOptions				= ConstructObject<UMaterialEditorOptions>( UMaterialEditorOptions::StaticClass() );
	bShowGrid					= EditorOptions->bShowGrid;
	bShowBackground				= EditorOptions->bShowBackground;
	bHideUnusedConnectors		= EditorOptions->bHideUnusedConnectors;
	bDrawCurves					= EditorOptions->bDrawCurves;
	bAlwaysRefreshAllPreviews	= EditorOptions->bAlwaysRefreshAllPreviews;
	bUseUnsortedMenus			= EditorOptions->bUseUnsortedMenus;

	if ( ToolBar )
	{
		ToolBar->SetShowGrid( bShowGrid );
		ToolBar->SetShowBackground( bShowBackground );
		ToolBar->SetHideConnectors( bHideUnusedConnectors );
		ToolBar->SetDrawCurves( bDrawCurves );
		ToolBar->SetAlwaysRefreshAllPreviews( bAlwaysRefreshAllPreviews );
		ToolBar->SetRealtimeMaterialPreview( EditorOptions->bRealtimeMaterialViewport );
		ToolBar->SetRealtimeExpressionPreview( EditorOptions->bRealtimeExpressionViewport );
	}
	if ( PreviewVC )
	{
		PreviewVC->SetShowGrid( bShowGrid );
		PreviewVC->SetRealtime( EditorOptions->bRealtimeMaterialViewport  );
	}
	if ( LinkedObjVC )
	{
		LinkedObjVC->SetRealtime( EditorOptions->bRealtimeExpressionViewport );
	}

	// Primitive type
	INT PrimType;
	if(GConfig->GetInt(TEXT("MaterialEditor"), TEXT("PrimType"), PrimType, GEditorUserSettingsIni))
	{
		PreviewPrimType = (EThumbnailPrimType)PrimType;
	}
	else
	{
		PreviewPrimType = TPT_Sphere;
	}

}

/**
 * Saves editor settings to disk (docking state, window pos/size, option state, etc).
 */
void WxMaterialEditor::SaveEditorSettings()
{
	// Save docking layout.
	SaveDockingLayout();

	// Save window position/size.
	FWindowUtil::SavePosSize(TEXT("MaterialEditor"), this);

	if ( EditorOptions )
	{
		check( PreviewVC );
		check( LinkedObjVC );
		EditorOptions->bShowGrid					= bShowGrid;
		EditorOptions->bShowBackground				= bShowBackground;
		EditorOptions->bHideUnusedConnectors		= bHideUnusedConnectors;
		EditorOptions->bDrawCurves					= bDrawCurves;
		EditorOptions->bAlwaysRefreshAllPreviews	= bAlwaysRefreshAllPreviews;
		EditorOptions->bRealtimeMaterialViewport	= PreviewVC->IsRealtime();
		EditorOptions->bRealtimeExpressionViewport	= LinkedObjVC->IsRealtime();
		EditorOptions->SaveConfig();
	}

	GConfig->SetInt(TEXT("MaterialEditor"), TEXT("PrimType"), PreviewPrimType, GEditorUserSettingsIni);

	//Material->PreviewCamPos = PreviewVC->ViewLocation;
	//Material->EditorPitch = PreviewVC->ViewRotation.Pitch;
	//Material->EditorYaw = PreviewVC->ViewRotation.Yaw;
}

/**
 * Called by SetPreviewMesh, allows derived types to veto the setting of a preview mesh.
 *
 * @return	TRUE if the specified mesh can be set as the preview mesh, FALSE otherwise.
 */
UBOOL WxMaterialEditor::ApproveSetPreviewMesh(UStaticMesh* InStaticMesh, USkeletalMesh* InSkeletalMesh)
{
	UBOOL bApproved = TRUE;
	// Only permit the use of a skeletal mesh if the material has bUsedWithSkeltalMesh.
	if ( InSkeletalMesh && !Material->bUsedWithSkeletalMesh )
	{
		appMsgf( AMT_OK, *LocalizeUnrealEd("Error_MaterialEditor_CantPreviewOnSkelMesh") );
		bApproved = FALSE;
	}
	return bApproved;
}

/**
 * Empties compound info.  If bRefresh is TRUE, all information is regenerated.
 */
void WxMaterialEditor::FlushCompoundExpressionInfo(UBOOL bRefresh)
{
	for( CompoundInfoMapType::TIterator It(CompoundInfoMap) ; It ; ++It )
	{
		FCompoundInfo* Info = It.Value();
		delete Info;
	}
	CompoundInfoMap.Empty();

	if ( bRefresh )
	{
		for( INT CompoundIndex = 0 ; CompoundIndex < Material->EditorCompounds.Num() ; ++CompoundIndex )
		{
			UMaterialExpressionCompound* Compound = Material->EditorCompounds(CompoundIndex);
			UpdateCompoundExpressionInfo( Compound );
		}
	}
}

/** Refreshes the viewport containing the material expression graph. */
void WxMaterialEditor::RefreshExpressionViewport()
{
	LinkedObjVC->Viewport->Invalidate();
}

/**
 * Refreshes the preview for the specified material expression.  Does nothing if the specified expression
 * has a bRealtimePreview of FALSE.
 *
 * @param	MaterialExpression		The material expression to update.
 */
void WxMaterialEditor::RefreshExpressionPreview(UMaterialExpression* MaterialExpression)
{
	if (MaterialExpression->bRealtimePreview)
	{
		for( INT PreviewIndex = 0 ; PreviewIndex < ExpressionPreviews.Num() ; ++PreviewIndex )
		{
			FExpressionPreview& ExpressionPreview = ExpressionPreviews(PreviewIndex);
			if( ExpressionPreview.GetExpression() == MaterialExpression )
			{
				// we need to make sure the rendering thread isn't drawing this tile
				FlushRenderingCommands();
				ExpressionPreviews.Remove( PreviewIndex );
				break;
			}
		}
	}
}

/**
 * Refreshes material expression previews.  Refreshes all previews if bAlwaysRefreshAllPreviews is TRUE.
 * Otherwise, refreshes only those previews that have a bRealtimePreview of TRUE.
 */
void WxMaterialEditor::RefreshExpressionPreviews()
{
	const FScopedBusyCursor BusyCursor;

	if ( bAlwaysRefreshAllPreviews )
	{
		// we need to make sure the rendering thread isn't drawing these tiles
		FlushRenderingCommands();

		// Refresh all expression previews.
		ExpressionPreviews.Empty();
	}
	else
	{
		// Only refresh expressions that are marked for realtime update.
		for ( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
		{
			UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
			RefreshExpressionPreview( MaterialExpression );
		}
	}
}

/**
 * Refreshes all material expression previews, regardless of whether or not realtime previews are enabled.
 */
void WxMaterialEditor::ForceRefreshExpressionPreviews()
{
	// Initialize expression previews.
	const UBOOL bOldAlwaysRefreshAllPreviews = bAlwaysRefreshAllPreviews;
	bAlwaysRefreshAllPreviews = TRUE;
	RefreshExpressionPreviews();
	bAlwaysRefreshAllPreviews = bOldAlwaysRefreshAllPreviews;
}

/**
 * @param		InMaterialExpression	The material expression to query.
 * @param		ConnType				Type of the connection (LOC_INPUT or LOC_OUTPUT).
 * @param		ConnIndex				Index of the connection to query
 * @return								A viewport location for the connection.
 */
FIntPoint WxMaterialEditor::GetExpressionConnectionLocation(UMaterialExpression* InMaterialExpression, INT ConnType, INT ConnIndex)
{
	const FMaterialNodeDrawInfo& ExpressionDrawInfo = GetDrawInfo( InMaterialExpression );

	FIntPoint Result(0,0);
	if ( ConnType == LOC_OUTPUT ) // connectors on the right side of the material
	{
		Result.X = InMaterialExpression->EditorX + ExpressionDrawInfo.DrawWidth + LO_CONNECTOR_LENGTH;
		Result.Y = ExpressionDrawInfo.RightYs(ConnIndex);
	}
	else if ( ConnType == LOC_INPUT ) // connectors on the left side of the material
	{
		Result.X = InMaterialExpression->EditorX - LO_CONNECTOR_LENGTH,
		Result.Y = ExpressionDrawInfo.LeftYs(ConnIndex);
	}
	return Result;
}

/**
 * @param		InMaterial	The material to query.
 * @param		ConnType	Type of the connection (LOC_OUTPUT).
 * @param		ConnIndex	Index of the connection to query
 * @return					A viewport location for the connection.
 */
FIntPoint WxMaterialEditor::GetMaterialConnectionLocation(UMaterial* InMaterial, INT ConnType, INT ConnIndex)
{
	FIntPoint Result(0,0);
	if ( ConnType == LOC_OUTPUT ) // connectors on the right side of the material
	{
		Result.X = InMaterial->EditorX + MaterialDrawInfo.DrawWidth + LO_CONNECTOR_LENGTH;
		Result.Y = MaterialDrawInfo.RightYs( ConnIndex );
	}
	return Result;
}

/**
 * Returns the expression preview for the specified material expression.
 */
FExpressionPreview* WxMaterialEditor::GetExpressionPreview(UMaterialExpression* MaterialExpression)
{
	if (MaterialExpression->bHidePreviewWindow == FALSE)
	{
		FExpressionPreview*	Preview = NULL;
		for( INT PreviewIndex = 0 ; PreviewIndex < ExpressionPreviews.Num() ; ++PreviewIndex )
		{
			FExpressionPreview& ExpressionPreview = ExpressionPreviews(PreviewIndex);
			if( ExpressionPreview.GetExpression() == MaterialExpression )
			{
				Preview = &ExpressionPreviews(PreviewIndex);
				break;
			}
		}

		if( !Preview )
		{
			Preview = new(ExpressionPreviews) FExpressionPreview(MaterialExpression);
		}
		return Preview;
	}

	return NULL;
}

/**
* Returns the drawinfo object for the specified expression, creating it if one does not currently exist.
*/
WxMaterialEditor::FMaterialNodeDrawInfo& WxMaterialEditor::GetDrawInfo(UMaterialExpression* MaterialExpression)
{
	FMaterialNodeDrawInfo* ExpressionDrawInfo = MaterialNodeDrawInfo.Find( MaterialExpression );
	return ExpressionDrawInfo ? *ExpressionDrawInfo : MaterialNodeDrawInfo.Set( MaterialExpression, FMaterialNodeDrawInfo(MaterialExpression->EditorY) );
}

/**
 * Disconnects and removes the selected material expressions.
 */
void WxMaterialEditor::DeleteSelectedObjects()
{
	const UBOOL bHaveExpressionsToDelete	= SelectedExpressions.Num() > 0;
	const UBOOL bHaveCommentsToDelete		= SelectedComments.Num() > 0;
	const UBOOL bHaveCompoundsToDelete		= SelectedCompounds.Num() > 0;
	if ( bHaveExpressionsToDelete || bHaveCommentsToDelete || bHaveCompoundsToDelete )
	{
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorDelete")) );
			Material->Modify();
			
			// Whack selected expressions.
			for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < SelectedExpressions.Num() ; ++MaterialExpressionIndex )
			{
				UMaterialExpression* MaterialExpression = SelectedExpressions( MaterialExpressionIndex );
				MaterialExpression->Modify();
				BreakLinksToExpression( MaterialExpression, Material );
				Material->Expressions.RemoveItem( MaterialExpression );
				Material->RemoveExpressionParameter(MaterialExpression);
			}

			// Whack selected comments.
			for( INT CommentIndex = 0 ; CommentIndex < SelectedComments.Num() ; ++CommentIndex )
			{
				UMaterialExpressionComment* Comment = SelectedComments( CommentIndex );
				Comment->Modify();
				Material->EditorComments.RemoveItem( Comment );
			}

			// Whack selected compounds.
			for( INT CompoundIndex = 0 ; CompoundIndex < SelectedCompounds.Num() ; ++CompoundIndex )
			{
				UMaterialExpressionCompound* Compound = SelectedCompounds( CompoundIndex );
				Compound->Modify();
				Material->EditorCompounds.RemoveItem( Compound );
				for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Compound->MaterialExpressions.Num() ; ++MaterialExpressionIndex )
				{
					UMaterialExpression* MaterialExpression = Compound->MaterialExpressions( MaterialExpressionIndex );
					check( MaterialExpression->Compound == Compound );
					MaterialExpression->Modify();
					MaterialExpression->Compound = NULL;
				}
				FCompoundInfo** CompoundInfo = CompoundInfoMap.Find( Compound );
				check( CompoundInfo );
				delete *CompoundInfo;
				CompoundInfoMap.Remove( Compound );
			}
		} // ScopedTransaction

		// Deselect all expressions, comments and compounds.
		EmptySelection();

		if ( bHaveExpressionsToDelete )
		{
			Material->PreEditChange( NULL );
			Material->PostEditChange( NULL );
		}
		Material->MarkPackageDirty();
		bMaterialDirty = TRUE;

		UpdatePropertyWindow();

		if ( bHaveExpressionsToDelete )
		{
			RefreshExpressionPreviews();
			RefreshPreviewViewport();
		}
		RefreshExpressionViewport();
	}
}

/**
 * Pastes into this material from the editor's material copy buffer.
 *
 * @param	PasteLocation	If non-NULL locate the upper-left corner of the new nodes' bound at this location.
 */
void WxMaterialEditor::PasteExpressionsIntoMaterial(const FIntPoint* PasteLocation)
{
	if ( GetCopyPasteBuffer()->Expressions.Num() > 0 || GetCopyPasteBuffer()->EditorComments.Num() > 0 )
	{
		// Empty the selection set, as we'll be selecting the newly created expressions.
		EmptySelection();

		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorPaste")) );
			Material->Modify();

			// Copy the expressions in the material copy buffer into this material.
			TArray<UMaterialExpression*> NewExpressions;
			TArray<UMaterialExpression*> NewComments;

			CopyMaterialExpressions( GetCopyPasteBuffer()->Expressions, GetCopyPasteBuffer()->EditorComments, Material, NewExpressions, NewComments );

			// Append the comments list to the expressions list so we can position all of the items at once.
			NewExpressions.Append(NewComments);
				
			// Locate and select the newly created nodes.
			const FIntRect NewExpressionBounds( GetBoundingBoxOfExpressions( NewExpressions ) );
			for ( INT ExpressionIndex = 0 ; ExpressionIndex < NewExpressions.Num() ; ++ExpressionIndex )
			{
				UMaterialExpression* NewExpression = NewExpressions( ExpressionIndex );
				if ( PasteLocation )
				{
					// We're doing a paste here.
					NewExpression->EditorX += -NewExpressionBounds.Min.X + PasteLocation->X;
					NewExpression->EditorY += -NewExpressionBounds.Min.Y + PasteLocation->Y;
				}
				else
				{
					// We're doing a duplicate or straight-up paste; offset the nodes by a fixed amount.
					const INT DuplicateOffset = 30;
					NewExpression->EditorX += DuplicateOffset;
					NewExpression->EditorY += DuplicateOffset;
				}
				AddToSelection( NewExpression );
				Material->AddExpressionParameter(NewExpression);
			}
		}

		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();

		UpdatePropertyWindow();

		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Duplicates the selected material expressions.  Preserves internal references.
 */
void WxMaterialEditor::DuplicateSelectedObjects()
{
	// Clear the material copy buffer and copy the selected expressions into it.
	TArray<UMaterialExpression*> NewExpressions;
	TArray<UMaterialExpression*> NewComments;

	GetCopyPasteBuffer()->Expressions.Empty();
	GetCopyPasteBuffer()->EditorComments.Empty();
	CopyMaterialExpressions( SelectedExpressions, SelectedComments, GetCopyPasteBuffer(), NewExpressions, NewComments );

	// Paste the material copy buffer into this material.
	PasteExpressionsIntoMaterial( NULL );
}

/**
 * Deletes any disconnected material expressions.
 */
void WxMaterialEditor::CleanUnusedExpressions()
{
	EmptySelection();

	// The set of material expressions to visit.
	TArray<UMaterialExpression*> Stack;

	// Populate the stack with inputs to the material.
	for ( INT MaterialInputIndex = 0 ; MaterialInputIndex < MaterialInputs.Num() ; ++MaterialInputIndex )
	{
		const FMaterialInputInfo& MaterialInput = MaterialInputs(MaterialInputIndex);
		UMaterialExpression* Expression = MaterialInput.Input->Expression;
		if ( Expression )
		{
			Stack.Push( Expression );
		}
	}

	// Depth-first traverse the material expression graph.
	TArray<UMaterialExpression*>	NewExpressions;
	TMap<UMaterialExpression*, INT> ReachableExpressions;
	while ( Stack.Num() > 0 )
	{
		UMaterialExpression* MaterialExpression = Stack.Pop();
		INT *AlreadyVisited = ReachableExpressions.Find( MaterialExpression );
		if ( !AlreadyVisited )
		{
			// Mark the expression as reachable.
			ReachableExpressions.Set( MaterialExpression, 0 );
			NewExpressions.AddItem( MaterialExpression );

			// Iterate over the expression's inputs and add them to the pending stack.
			const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( MaterialExpression );

			for( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ ExpressionInputIndex )
			{
				const UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
				FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
				UMaterialExpression* InputExpression = Input->Expression;
				if ( InputExpression )
				{
					Stack.Push( InputExpression );
				}
			}
		}
	}

	// Kill off expressions referenced by the material that aren't reachable.
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorCleanUnusedExpressions")) );
		Material->Modify();
		Material->Expressions = NewExpressions;
	}

	RefreshExpressionViewport();
	bMaterialDirty = TRUE;
}

/** Draws the root node -- that is, the node corresponding to the final material. */
void WxMaterialEditor::DrawMaterialRoot(UBOOL bIsSelected, FCanvas* Canvas)
{
	// Construct the FLinkedObjDrawInfo for use by the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;
	ObjInfo.ObjObject = Material;

	const FColor MaterialInputColor( 0, 0, 0 );
	for ( INT MaterialInputIndex = 0 ; MaterialInputIndex < MaterialInputs.Num() ; ++MaterialInputIndex )
	{
		const FMaterialInputInfo& MaterialInput = MaterialInputs(MaterialInputIndex);
		const UBOOL bShouldAddInputConnector = !bHideUnusedConnectors || MaterialInput.Input->Expression;
		if ( bShouldAddInputConnector )
		{
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(MaterialInput.Name, MaterialInputColor) );
		}
	}

	// Generate border color
	const FColor BorderColor( bIsSelected ? FColor( 255, 255, 0 ) : FColor( 0, 0, 0 ) );

	// Highlight the currently moused over connector
	HighlightActiveConnectors( ObjInfo );

	// Use util to draw box with connectors, etc.
	const FIntPoint MaterialPos( Material->EditorX, Material->EditorY );
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, *Material->GetName(), NULL, BorderColor, FColor(112,112,112), MaterialPos );

	// Copy off connector values for use
	MaterialDrawInfo.DrawWidth	= ObjInfo.DrawWidth;
	MaterialDrawInfo.RightYs	= ObjInfo.OutputY;
	check( bHideUnusedConnectors || MaterialDrawInfo.RightYs.Num() == MaterialInputs.Num() );
}

/**
 * Render connections between the material's inputs and the material expression outputs connected to them.
 */
void WxMaterialEditor::DrawMaterialRootConnections(UBOOL bInDrawCurves, FCanvas* Canvas)
{
	// Compensates for the difference between the number of rendered inputs
	// (based on bHideUnusedConnectors) and the true number of inputs.
	INT ActiveInputCounter = -1;

	TArray<FExpressionInput*> ReferencingInputs;
	for ( INT MaterialInputIndex = 0 ; MaterialInputIndex < MaterialInputs.Num() ; ++MaterialInputIndex )
	{
		const FMaterialInputInfo& MaterialInput = MaterialInputs(MaterialInputIndex);
		UMaterialExpression* MaterialExpression = MaterialInput.Input->Expression;
		if ( MaterialExpression )
		{
			++ActiveInputCounter;

			TArray<FExpressionOutput>	Outputs;
			MaterialExpression->GetOutputs(Outputs);

			INT ActiveOutputCounter = -1;
			INT OutputIndex = 0;
			for( ; OutputIndex < Outputs.Num() ; ++OutputIndex )
			{
				const FExpressionOutput& Output = Outputs(OutputIndex);

				// If unused connectors are hidden, the material expression output index needs to be transformed
				// to visible index rather than absolute.
				if ( bHideUnusedConnectors )
				{
					// Get a list of inputs that refer to the selected output.
					GetListOfReferencingInputs( MaterialExpression, Material, Output, ReferencingInputs );
					if ( ReferencingInputs.Num() > 0 )
					{
						++ActiveOutputCounter;
					}
				}

				if(	Output.Mask == MaterialInput.Input->Mask
					&& Output.MaskR == MaterialInput.Input->MaskR
					&& Output.MaskG == MaterialInput.Input->MaskG
					&& Output.MaskB == MaterialInput.Input->MaskB
					&& Output.MaskA == MaterialInput.Input->MaskA )
				{
					break;
				}
			}
			check( OutputIndex < Outputs.Num() );

			const INT MaterialInputLookupIndex			= bHideUnusedConnectors ? ActiveInputCounter : MaterialInputIndex;
			const FIntPoint Start( GetMaterialConnectionLocation(Material,LOC_OUTPUT,MaterialInputLookupIndex) );

			const INT ExpressionOutputLookupIndex		= bHideUnusedConnectors ? ActiveOutputCounter : OutputIndex;
			const FIntPoint End( GetExpressionConnectionLocation(MaterialExpression,LOC_INPUT,ExpressionOutputLookupIndex) );

			// If either of the connection ends are highlighted then highlight the line.
			FColor Color;

			if(IsConnectorHighlighted(Material, LOC_OUTPUT, MaterialInputLookupIndex) || IsConnectorHighlighted(MaterialExpression, LOC_INPUT, ExpressionOutputLookupIndex))
			{
				Color = ConnectionSelectedColor;
			}
			else
			{
				Color = ConnectionNormalColor;
			}

			if ( bInDrawCurves )
			{
				const FLOAT Tension = Abs<INT>( Start.X - End.X );
				FLinkedObjDrawUtils::DrawSpline( Canvas, End, -Tension*FVector2D(1,0), Start, -Tension*FVector2D(1,0), Color, TRUE );
			}
			else
			{
				DrawLine2D( Canvas, Start, End, FColor(0,0,0) );
				const FVector2D Dir( FVector2D(Start) - FVector2D(End) );
				FLinkedObjDrawUtils::DrawArrowhead( Canvas, Start, Dir.SafeNormal(), Color );
			}
		}
	}
}

void WxMaterialEditor::DrawMaterialExpressionLinkedObj(FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const TCHAR* Name, const TCHAR* Comment, const FColor& BorderColor, const FColor& TitleBkgColor, UMaterialExpression* MaterialExpression, UBOOL bRenderPreview)
{
	static const INT ExpressionPreviewSize = 96;
	static const INT ExpressionPreviewBorder = 1;
	//static const INT ExpressionPreviewSize = ExpressionPreviewBaseSize + ExpressionPreviewBorder;

	const FIntPoint Pos( MaterialExpression->EditorX, MaterialExpression->EditorY );
#if 0
	FLinkedObjDrawUtils::DrawLinkedObj( Canvas, ObjInfo, Name, Comment, BorderColor, TitleBkgColor, Pos );
#else
	const UBOOL bHitTesting = Canvas->IsHitTesting();

	const FIntPoint TitleSize	= FLinkedObjDrawUtils::GetTitleBarSize(Canvas, Name);
	const FIntPoint LogicSize	= FLinkedObjDrawUtils::GetLogicConnectorsSize(Canvas, ObjInfo);

	// Includes one-pixel border on left and right and a one-pixel border between the preview icon and the title text.
	ObjInfo.DrawWidth	= 2 + 1 + PreviewIconWidth + Max(Max(TitleSize.X, LogicSize.X), ExpressionPreviewSize+2*ExpressionPreviewBorder);
	const INT BodyHeight = 2 + Max(LogicSize.Y, ExpressionPreviewSize+2*ExpressionPreviewBorder);

	// Includes one-pixel spacer between title and body.
	ObjInfo.DrawHeight	= TitleSize.Y + 1 + BodyHeight;

	if(bHitTesting) Canvas->SetHitProxy( new HLinkedObjProxy(ObjInfo.ObjObject) );

	FLinkedObjDrawUtils::DrawTitleBar(Canvas, Pos, FIntPoint(ObjInfo.DrawWidth, TitleSize.Y), BorderColor, TitleBkgColor, Name, Comment);

	FLinkedObjDrawUtils::DrawTile( Canvas, Pos.X,		Pos.Y + TitleSize.Y + 1,	ObjInfo.DrawWidth,		BodyHeight,		0.0f,0.0f,0.0f,0.0f, BorderColor );
	FLinkedObjDrawUtils::DrawTile( Canvas, Pos.X + 1,	Pos.Y + TitleSize.Y + 2,	ObjInfo.DrawWidth - 2,	BodyHeight-2,	0.0f,0.0f,0.0f,0.0f, FColor(140,140,140) );

	if ( bRenderPreview )
	{
		FExpressionPreview* ExpressionPreview = GetExpressionPreview( MaterialExpression );
		FLinkedObjDrawUtils::DrawTile( Canvas, Pos.X + 1 + ExpressionPreviewBorder,	Pos.Y + TitleSize.Y + 2 + ExpressionPreviewBorder,	ExpressionPreviewSize,	ExpressionPreviewSize,	0.0f,0.0f,1.0f,1.0f, ExpressionPreview );
	}

	if(bHitTesting) Canvas->SetHitProxy( NULL );

	// Highlight the currently moused over connector
	HighlightActiveConnectors( ObjInfo );

	//const FLinearColor ConnectorTileBackgroundColor( 0.f, 0.f, 0.f, 0.5f );
	FLinkedObjDrawUtils::DrawLogicConnectors(Canvas, ObjInfo, Pos + FIntPoint(0, TitleSize.Y + 1), FIntPoint(ObjInfo.DrawWidth, LogicSize.Y), NULL);//&ConnectorTileBackgroundColor);
#endif
}

/** Draws the specified material expression node. */
void WxMaterialEditor::DrawMaterialExpression(UMaterialExpression* MaterialExpression, UBOOL bExpressionSelected, FCanvas* Canvas)
{
	// Don't render the expression if it is contained by a compound that is visible.
	const UBOOL bCompoundVisible = MaterialExpression->Compound && !MaterialExpression->Compound->bExpanded;
	if ( bCompoundVisible )
	{
		return;
	}

	// Construct the FLinkedObjDrawInfo for use by the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;
	ObjInfo.ObjObject = MaterialExpression;

	// Material expression inputs, drawn on the right side of the node.
	const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( MaterialExpression );
	for( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ ExpressionInputIndex )
	{
		const UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
		FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
		UMaterialExpression* InputExpression = Input->Expression;
		const UBOOL bShouldAddInputConnector = !bHideUnusedConnectors || InputExpression;
		if ( bShouldAddInputConnector )
		{
			FString InputName = ExpressionInput->GetName();
			// Shorten long expression input names.
			if ( !appStricmp( *InputName, TEXT("Coordinates") ) )
			{
				InputName = TEXT("UVs");
			}
			else if ( !appStricmp( *InputName, TEXT("Input") ) )
			{
				InputName = TEXT("");
			}
			else if ( !appStricmp( *InputName, TEXT("Exponent") ) )
			{
				InputName = TEXT("Exp");
			}
			else if ( !appStricmp( *InputName, TEXT("AGreaterThanB") ) )
			{
				InputName = TEXT("A>B");
			}
			else if ( !appStricmp( *InputName, TEXT("AEqualsB") ) )
			{
				InputName = TEXT("A=B");
			}
			else if ( !appStricmp( *InputName, TEXT("ALessThanB") ) )
			{
				InputName = TEXT("A<B");
			}
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*InputName, FColor(0,0,0)) );
		}
	}

	// Material expression outputs, drawn on the left side of the node
	TArray<FExpressionOutput> Outputs;
	MaterialExpression->GetOutputs(Outputs);

	TArray<FExpressionInput*> ReferencingInputs;
	for( INT OutputIndex = 0 ; OutputIndex < Outputs.Num() ; ++OutputIndex )
	{
		const FExpressionOutput& ExpressionOutput = Outputs(OutputIndex);
		UBOOL bShouldAddOutputConnector = TRUE;
		if ( bHideUnusedConnectors )
		{
			// Get a list of inputs that refer to the selected output.
			GetListOfReferencingInputs( MaterialExpression, Material, ExpressionOutput, ReferencingInputs );
			bShouldAddOutputConnector = ReferencingInputs.Num() > 0;
		}

		if ( bShouldAddOutputConnector )
		{
			const TCHAR* OutputName = TEXT("");
			FColor OutputColor( 0, 0, 0 );

			if( ExpressionOutput.Mask )
			{
				if		( ExpressionOutput.MaskR && !ExpressionOutput.MaskG && !ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// R
					OutputColor = FColor( 255, 0, 0 );
					//OutputName = TEXT("R");
				}
				else if	(!ExpressionOutput.MaskR &&  ExpressionOutput.MaskG && !ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// G
					OutputColor = FColor( 0, 255, 0 );
					//OutputName = TEXT("G");
				}
				else if	(!ExpressionOutput.MaskR && !ExpressionOutput.MaskG &&  ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// B
					OutputColor = FColor( 0, 0, 255 );
					//OutputName = TEXT("B");
				}
				else if	(!ExpressionOutput.MaskR && !ExpressionOutput.MaskG && !ExpressionOutput.MaskB &&  ExpressionOutput.MaskA)
				{
					// A
					OutputColor = FColor( 255, 255, 255 );
					//OutputName = TEXT("A");
				}
				else
				{
					// RGBA
					//OutputName = TEXT("RGBA");
				}
			}

			if (MaterialExpression->bShowOutputNameOnPin == TRUE)
			{
				OutputName = *(ExpressionOutput.Name);
			}

			// We use the "Inputs" array so that the connectors are drawn on the left side of the node.
			ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(OutputName, OutputColor) );
		}
	}

	// Determine the texture dependency length for the material and the expression.
	const FMaterialResource * MaterialResource = Material->GetMaterialResource();
	INT MaxTextureDependencyLength = MaterialResource->GetMaxTextureDependencyLength();
	const INT* TextureDependencyLength = MaterialResource->GetTextureDependencyLengthMap().Find(MaterialExpression);

	// Generate border color
	FColor BorderColor;
	if(bExpressionSelected)
	{
		BorderColor = FColor( 255, 255, 0 );
	}
	else if(TextureDependencyLength && *TextureDependencyLength == MaxTextureDependencyLength && MaxTextureDependencyLength > 1)
	{
		BorderColor = FColor( 255, 0, 255 );
	}
	else if(UMaterial::IsParameter(MaterialExpression))
	{
		if(Material->HasDuplicateParameters(MaterialExpression))
		{
			BorderColor = FColor( 0, 255, 255 );
		}
		else
		{
			BorderColor = FColor( 0, 128, 128 );
		}
	}
	else if (UMaterial::IsDynamicParameter(MaterialExpression))
	{
		if (Material->HasDuplicateDynamicParameters(MaterialExpression))
		{
			BorderColor = FColor( 0, 255, 255 );
		}
		else
		{
			BorderColor = FColor( 0, 128, 128 );
		}
	}
	else
	{
		BorderColor = FColor( 0, 0, 0 );
	}

	// Use util to draw box with connectors, etc.
	DrawMaterialExpressionLinkedObj( Canvas, ObjInfo, *MaterialExpression->GetCaption(), NULL, BorderColor, FColor(112,112,112), MaterialExpression, !(MaterialExpression->bHidePreviewWindow) );

	// Read back the height of the first connector on the left side of the node,
	// for use later when drawing connections to this node.
	FMaterialNodeDrawInfo& ExpressionDrawInfo	= GetDrawInfo( MaterialExpression );
	ExpressionDrawInfo.LeftYs					= ObjInfo.InputY;
	ExpressionDrawInfo.RightYs					= ObjInfo.OutputY;
	ExpressionDrawInfo.DrawWidth				= ObjInfo.DrawWidth;

	check( bHideUnusedConnectors || ExpressionDrawInfo.RightYs.Num() == ExpressionInputs.Num() );

	// Draw realtime preview indicator above the node.
	if (MaterialExpression->bHidePreviewWindow == FALSE)
	{
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, MaterialExpression->EditorX+PreviewIconLoc.X, MaterialExpression->EditorY+PreviewIconLoc.Y, PreviewIconWidth, PreviewIconWidth ) )
		{
			const UBOOL bHitTesting = Canvas->IsHitTesting();
			if( bHitTesting )  Canvas->SetHitProxy( new HRealtimePreviewProxy( MaterialExpression ) );

			// Draw black background icon.
			FLinkedObjDrawUtils::DrawTile( Canvas, MaterialExpression->EditorX+PreviewIconLoc.X,		MaterialExpression->EditorY+PreviewIconLoc.Y, PreviewIconWidth,	PreviewIconWidth, 0.f, 0.f, 1.f, 1.f, FColor(0,0,0) );

			// Draw yellow fill if realtime preview is enabled for this node.
			if( MaterialExpression->bRealtimePreview )
			{
				FLinkedObjDrawUtils::DrawTile( Canvas, MaterialExpression->EditorX+PreviewIconLoc.X+1,	MaterialExpression->EditorY+PreviewIconLoc.Y+1, PreviewIconWidth-2,	PreviewIconWidth-2,	0.f, 0.f, 1.f, 1.f, FColor(255,215,0) );
			}

			// Draw a small red icon above the node if realtime preview is enabled for all nodes.
			if ( bAlwaysRefreshAllPreviews )
			{
				FLinkedObjDrawUtils::DrawTile( Canvas, MaterialExpression->EditorX+PreviewIconLoc.X+2,	MaterialExpression->EditorY+PreviewIconLoc.Y+2, PreviewIconWidth-4,	PreviewIconWidth-4,	0.f, 0.f, 1.f, 1.f, FColor(255,0,0) );
			}
			if( bHitTesting )  Canvas->SetHitProxy( NULL );
		}
	}
}

/**
 * Render connectors between this material expression's inputs and the material expression outputs connected to them.
 */
void WxMaterialEditor::DrawMaterialExpressionConnections(UMaterialExpression* MaterialExpression, UBOOL bInDrawCurves, FCanvas* Canvas)
{
	// Compensates for the difference between the number of rendered inputs
	// (based on bHideUnusedConnectors) and the true number of inputs.
	INT ActiveInputCounter = -1;

	TArray<FExpressionInput*> ReferencingInputs;

	const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( MaterialExpression );
	for ( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ExpressionInputIndex )
	{
		const UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
		FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
		UMaterialExpression* InputExpression = Input->Expression;
		if ( InputExpression )
		{
			++ActiveInputCounter;

			// Find the matching output.
			TArray<FExpressionOutput> Outputs;
			InputExpression->GetOutputs(Outputs);
			INT ActiveOutputCounter = -1;
			INT OutputIndex = 0;
			for( ; OutputIndex < Outputs.Num() ; ++OutputIndex )
			{
				const FExpressionOutput& Output = Outputs(OutputIndex);

				// If unused connectors are hidden, the material expression output index needs to be transformed
				// to visible index rather than absolute.
				if ( bHideUnusedConnectors )
				{
					// Get a list of inputs that refer to the selected output.
					GetListOfReferencingInputs( InputExpression, Material, Output, ReferencingInputs );
					if ( ReferencingInputs.Num() > 0 )
					{
						++ActiveOutputCounter;
					}
				}

				if(	Output.Mask == Input->Mask
					&& Output.MaskR == Input->MaskR
					&& Output.MaskG == Input->MaskG
					&& Output.MaskB == Input->MaskB
					&& Output.MaskA == Input->MaskA )
				{
					break;
				}
			}
			check( OutputIndex < Outputs.Num() );

			const INT ExpressionInputLookupIndex		= bHideUnusedConnectors ? ActiveInputCounter : ExpressionInputIndex;
			const FIntPoint Start( GetExpressionConnectionLocation(MaterialExpression,LOC_OUTPUT,ExpressionInputLookupIndex) );
			const INT ExpressionOutputLookupIndex		= bHideUnusedConnectors ? ActiveOutputCounter : OutputIndex;
			const FIntPoint End( GetExpressionConnectionLocation(InputExpression,LOC_INPUT,ExpressionOutputLookupIndex) );


			// If either of the connection ends are highlighted then highlight the line.
			FColor Color;

			if(IsConnectorHighlighted(MaterialExpression, LOC_OUTPUT, ExpressionInputLookupIndex) || IsConnectorHighlighted(InputExpression, LOC_INPUT, ExpressionOutputLookupIndex))
			{
				Color = ConnectionSelectedColor;
			}
			else
			{
				Color = ConnectionNormalColor;
			}
			

			if ( bInDrawCurves )
			{
				const FLOAT Tension = Abs<INT>( Start.X - End.X );
				FLinkedObjDrawUtils::DrawSpline( Canvas, End, -Tension*FVector2D(1,0), Start, -Tension*FVector2D(1,0), Color, TRUE );
			}
			else
			{
				DrawLine2D( Canvas, Start, End, FColor(0,0,0) );
				const FVector2D Dir( FVector2D(Start) - FVector2D(End) );
				FLinkedObjDrawUtils::DrawArrowhead( Canvas, Start, Dir.SafeNormal(), Color );
			}
		}
	}
}

/** Draws comments for the specified material expression node. */
void WxMaterialEditor::DrawMaterialExpressionComments(UMaterialExpression* MaterialExpression, FCanvas* Canvas)
{
	// Don't render the expression if it is contained by a compound that is visible.
	const UBOOL bCompoundVisible = MaterialExpression->Compound && !MaterialExpression->Compound->bExpanded;
	if ( bCompoundVisible )
	{
		return;
	}

	// Draw the material expression comment string unzoomed.
	if( MaterialExpression->Desc.Len() > 0 )
	{
		const FLOAT OldZoom2D = FLinkedObjDrawUtils::GetUniformScaleFromMatrix(Canvas->GetFullTransform());

		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *MaterialExpression->Desc );

		// We always draw comment-box text at normal size (don't scale it as we zoom in and out.)
		const INT x = appTrunc( MaterialExpression->EditorX*OldZoom2D );
		const INT y = appTrunc( MaterialExpression->EditorY*OldZoom2D - YL );

		// Viewport cull at a zoom of 1.0, because that's what we'll be drawing with.
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, MaterialExpression->EditorX, MaterialExpression->EditorY - YL, XL, YL ) )
		{
			Canvas->PushRelativeTransform(FScaleMatrix(1.0f / OldZoom2D));
			{
				FLinkedObjDrawUtils::DrawShadowedString( Canvas, x, y, *MaterialExpression->Desc, GEngine->SmallFont, FColor(64,64,192) );
			}
			Canvas->PopTransform();
		}
	}
}

/** Draws UMaterialExpressionComment nodes. */
void WxMaterialEditor::DrawCommentExpressions(FCanvas* Canvas)
{
	const UBOOL bHitTesting = Canvas->IsHitTesting();
	for ( INT CommentIndex = 0 ; CommentIndex < Material->EditorComments.Num() ; ++CommentIndex )
	{
		UMaterialExpressionComment* Comment = Material->EditorComments(CommentIndex);

		const UBOOL bSelected = SelectedComments.ContainsItem( Comment );
		const FColor FrameColor = bSelected ? FColor(255,255,0) : FColor(0, 0, 0);

		for(INT i=0; i<1; i++)
		{
			DrawLine2D(Canvas, FVector2D(Comment->EditorX,						Comment->EditorY + i),					FVector2D(Comment->EditorX + Comment->SizeX,		Comment->EditorY + i),					FrameColor );
			DrawLine2D(Canvas, FVector2D(Comment->EditorX + Comment->SizeX - i,	Comment->EditorY),						FVector2D(Comment->EditorX + Comment->SizeX - i,	Comment->EditorY + Comment->SizeY),		FrameColor );
			DrawLine2D(Canvas, FVector2D(Comment->EditorX + Comment->SizeX,		Comment->EditorY + Comment->SizeY - i),	FVector2D(Comment->EditorX,						Comment->EditorY + Comment->SizeY - i),	FrameColor );
			DrawLine2D(Canvas, FVector2D(Comment->EditorX + i,					Comment->EditorY + Comment->SizeY),		FVector2D(Comment->EditorX + i,					Comment->EditorY - 1),					FrameColor );
		}

		// Draw little sizing triangle in bottom left.
		const INT HandleSize = 16;
		const FIntPoint A(Comment->EditorX + Comment->SizeX,				Comment->EditorY + Comment->SizeY);
		const FIntPoint B(Comment->EditorX + Comment->SizeX,				Comment->EditorY + Comment->SizeY - HandleSize);
		const FIntPoint C(Comment->EditorX + Comment->SizeX - HandleSize,	Comment->EditorY + Comment->SizeY);
		const BYTE TriangleAlpha = (bSelected) ? 255 : 32; // Make it more transparent if comment is not selected.

		if(bHitTesting)  Canvas->SetHitProxy( new HLinkedObjProxySpecial(Comment, 1) );
		DrawTriangle2D( Canvas, A, FVector2D(0,0), B, FVector2D(0,0), C, FVector2D(0,0), FColor(0,0,0,TriangleAlpha) );
		if(bHitTesting)  Canvas->SetHitProxy( NULL );

		// Check there are some valid chars in string. If not - we can't select it! So we force it back to default.
		UBOOL bHasProperChars = FALSE;
		for( INT i = 0 ; i < Comment->Text.Len() ; ++i )
		{
			if ( Comment->Text[i] != ' ' )
			{
				bHasProperChars = TRUE;
				break;
			}
		}
		if ( !bHasProperChars )
		{
			Comment->Text = TEXT("Comment");
		}

		const FLOAT OldZoom2D = FLinkedObjDrawUtils::GetUniformScaleFromMatrix(Canvas->GetFullTransform());

		INT XL, YL;
		StringSize( GEngine->SmallFont, XL, YL, *Comment->Text );

		// We always draw comment-box text at normal size (don't scale it as we zoom in and out.)
		const INT x = appTrunc(Comment->EditorX*OldZoom2D + 2);
		const INT y = appTrunc(Comment->EditorY*OldZoom2D - YL - 2);

		// Viewport cull at a zoom of 1.0, because that's what we'll be drawing with.
		if ( FLinkedObjDrawUtils::AABBLiesWithinViewport( Canvas, Comment->EditorX+2, Comment->EditorY-YL-2, XL, YL ) )
		{
			Canvas->PushRelativeTransform(FScaleMatrix(1.0f / OldZoom2D));
			{
				// We only set the hit proxy for the comment text.
				if ( bHitTesting ) Canvas->SetHitProxy( new HLinkedObjProxy(Comment) );
				DrawShadowedString(Canvas, x, y, *Comment->Text, GEngine->SmallFont, FColor(64,64,192) );
				if ( bHitTesting ) Canvas->SetHitProxy( NULL );
			}
			Canvas->PopTransform();
		}
	}
}

/**
 * Draws the specified material expression compound node.
 */
void WxMaterialEditor::DrawCompoundExpression(UMaterialExpressionCompound* Compound, UBOOL bSelected, FCanvas* Canvas)
{
	// Construct the FLinkedObjDrawInfo for use by the linked-obj drawing utils.
	FLinkedObjDrawInfo ObjInfo;
	ObjInfo.ObjObject = Compound;

	FCompoundInfo** CompoundInfo = CompoundInfoMap.Find( Compound );
	check( CompoundInfo );
	FCompoundInfo* Info = *CompoundInfo;

	// Material expression inputs, drawn on the right side of the node.
	for ( INT CompoundInputIndex = 0 ; CompoundInputIndex < Info->CompoundInputs.Num() ; ++CompoundInputIndex )
	{
		const FCompoundInput& CompoundInput				= Info->CompoundInputs(CompoundInputIndex);
		const UMaterialExpression* MaterialExpression	= CompoundInput.Expression;
		const UStructProperty* ExpressionInput			= CompoundInput.ExpressionInput;
		const FExpressionInput* Input					= (const FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
		const UMaterialExpression* InputExpression		= Input->Expression;
		const UBOOL bShouldAddInputConnector			= !bHideUnusedConnectors || InputExpression;
		if ( bShouldAddInputConnector )
		{
			FString InputName = ExpressionInput->GetName();
			// Shorten long expression input names.
			if ( !appStricmp( *InputName, TEXT("Coordinates") ) )
			{
				InputName = TEXT("UVs");
			}
			else if ( !appStricmp( *InputName, TEXT("Input") ) )
			{
				InputName = TEXT("");
			}
			else if ( !appStricmp( *InputName, TEXT("Exponent") ) )
			{
				InputName = TEXT("Exp");
			}
			ObjInfo.Outputs.AddItem( FLinkedObjConnInfo(*InputName, FColor(0,0,0)) );
		}
	}

	// Material expression outputs, drawn on the left side of the node
	for ( INT CompoundOutputIndex = 0 ; CompoundOutputIndex < Info->CompoundOutputs.Num() ; ++CompoundOutputIndex )
	{
		const FCompoundOutput& Output					= Info->CompoundOutputs(CompoundOutputIndex);
		const UMaterialExpression* MaterialExpression	= Output.Expression;
		const FExpressionOutput& ExpressionOutput		= Output.ExpressionOutput;

		UBOOL bShouldAddOutputConnector					= TRUE;
		if ( bHideUnusedConnectors )
		{
			// Get a list of inputs that refer to the selected output.
			TArray<FExpressionInput*> ReferencingInputs;
			GetListOfReferencingInputs( MaterialExpression, Material, ExpressionOutput, ReferencingInputs );
			bShouldAddOutputConnector = ReferencingInputs.Num() > 0;
		}

		if ( bShouldAddOutputConnector )
		{
			const TCHAR* OutputName = TEXT("");
			FColor OutputColor( 0, 0, 0 );

			if( ExpressionOutput.Mask )
			{
				if		( ExpressionOutput.MaskR && !ExpressionOutput.MaskG && !ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// R
					OutputColor = FColor( 255, 0, 0 );
					//OutputName = TEXT("R");
				}
				else if	(!ExpressionOutput.MaskR &&  ExpressionOutput.MaskG && !ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// G
					OutputColor = FColor( 0, 255, 0 );
					//OutputName = TEXT("G");
				}
				else if	(!ExpressionOutput.MaskR && !ExpressionOutput.MaskG &&  ExpressionOutput.MaskB && !ExpressionOutput.MaskA)
				{
					// B
					OutputColor = FColor( 0, 0, 255 );
					//OutputName = TEXT("B");
				}
				else if	(!ExpressionOutput.MaskR && !ExpressionOutput.MaskG && !ExpressionOutput.MaskB &&  ExpressionOutput.MaskA)
				{
					// A
					OutputColor = FColor( 255, 255, 255 );
					//OutputName = TEXT("A");
				}
				else
				{
					// RGBA
					//OutputName = TEXT("RGBA");
				}
			}
			// We use the "Inputs" array so that the connectors are drawn on the left side of the node.
			ObjInfo.Inputs.AddItem( FLinkedObjConnInfo(OutputName, OutputColor) );
		}
	}

	// Generate border color
	const FColor BorderColor( bSelected ? FColor( 255, 255, 0 ) : FColor( 0, 0, 0 ) );

	// Highlight the currently moused over connector
	HighlightActiveConnectors( ObjInfo );

	// Use util to draw box with connectors, etc.
	DrawMaterialExpressionLinkedObj( Canvas, ObjInfo, *Compound->GetCaption(), NULL, BorderColor, FColor(112,112,112), Compound, FALSE );

	// Read back the height of the first connector on the left side of the node,
	// for use later when drawing connections to this node.
	FMaterialNodeDrawInfo& ExpressionDrawInfo	= GetDrawInfo( Compound );
	ExpressionDrawInfo.LeftYs					= ObjInfo.InputY;
	ExpressionDrawInfo.RightYs					= ObjInfo.OutputY;
	ExpressionDrawInfo.DrawWidth				= ObjInfo.DrawWidth;
	check( bHideUnusedConnectors || ExpressionDrawInfo.RightYs.Num() == Info->CompoundInputs.Num() );
}

/**
 * Highlights any active logic connectors in the specified linked object.
 */
void WxMaterialEditor::HighlightActiveConnectors(FLinkedObjDrawInfo &ObjInfo)
{
	const FColor HighlightedColor(255, 255, 0);

	if(LinkedObjVC->MouseOverConnType==LOC_INPUT)
	{
		for(INT InputIdx=0; InputIdx<ObjInfo.Inputs.Num(); InputIdx++)
		{
			if(IsConnectorHighlighted(ObjInfo.ObjObject, LOC_INPUT, InputIdx))
			{
				ObjInfo.Inputs(InputIdx).Color = HighlightedColor;
			}
		}
	}
	else if(LinkedObjVC->MouseOverConnType==LOC_OUTPUT)
	{
		for(INT OutputIdx=0; OutputIdx<ObjInfo.Outputs.Num(); OutputIdx++)
		{
			if(IsConnectorHighlighted(ObjInfo.ObjObject, LOC_OUTPUT, OutputIdx))
			{
				ObjInfo.Outputs(OutputIdx).Color = HighlightedColor;
			}
		}
	}
}

/** @return Returns whether the specified connector is currently highlighted or not. */
UBOOL WxMaterialEditor::IsConnectorHighlighted(UObject* Object, BYTE ConnType, INT ConnIndex)
{
	UBOOL bResult = FALSE;

	if(Object==LinkedObjVC->MouseOverObject && ConnIndex==LinkedObjVC->MouseOverConnIndex && ConnType==LinkedObjVC->MouseOverConnType)
	{
		bResult = TRUE;
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FLinkedObjViewportClient interface
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxMaterialEditor::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{
	if (BackgroundTexture != NULL)
	{
		Clear( Canvas, FColor(161,161,161) );

		Canvas->PushAbsoluteTransform(FMatrix::Identity);

		const INT ViewWidth = LinkedObjVC->Viewport->GetSizeX();
		const INT ViewHeight = LinkedObjVC->Viewport->GetSizeY();

		// draw the texture to the side, stretched vertically
		FLinkedObjDrawUtils::DrawTile( Canvas, ViewWidth - BackgroundTexture->SizeX, 0,
			BackgroundTexture->SizeX, ViewHeight,
			0.f, 0.f,
			1.f, 1.f,
			FLinearColor::White,
			BackgroundTexture->Resource );

		// stretch the left part of the texture to fill the remaining gap
		if (ViewWidth > BackgroundTexture->SizeX)
		{
			FLinkedObjDrawUtils::DrawTile( Canvas, 0, 0,
				ViewWidth - BackgroundTexture->SizeX, ViewHeight,
				0.f, 0.f,
				0.1f, 0.1f,
				FLinearColor::White,
				BackgroundTexture->Resource );
		}

		Canvas->PopTransform();
	}

	// Draw comments.
	DrawCommentExpressions( Canvas );

	// Draw the root node -- that is, the node corresponding to the final material.
	DrawMaterialRoot( FALSE, Canvas );

	// Draw the material expression nodes.
	for( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		// @hack DB: For some reason, materials that were resaved in gemini have comment expressions in the material's
		// @hack DB: expressions list.  The check below is required to prevent them from rendering as normal expression nodes.
		if ( !MaterialExpression->IsA(UMaterialExpressionComment::StaticClass()) )
		{
			const UBOOL bExpressionSelected = SelectedExpressions.ContainsItem( MaterialExpression );
			DrawMaterialExpression( MaterialExpression, bExpressionSelected, Canvas );
		}
	}
#if 0
	// Draw the compound expressions.
	for( INT ExpressionIndex = 0 ; ExpressionIndex < Material->EditorCompounds.Num() ; ++ExpressionIndex )
	{
		UMaterialExpressionCompound* CompoundExpression = Material->EditorCompounds( ExpressionIndex );
		const UBOOL bExpressionSelected = SelectedCompounds.ContainsItem( CompoundExpression );
		DrawCompoundExpression( CompoundExpression, bExpressionSelected, RI );
	}
#endif
	const UBOOL bShouldDrawCurves = DrawCurves();

	// Render connections between the material's inputs and the material expression outputs connected to them.
	DrawMaterialRootConnections( bShouldDrawCurves, Canvas );

	// Render connectors between material expressions' inputs and the material expression outputs connected to them.
	for( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		const UBOOL bExpressionSelected = SelectedExpressions.ContainsItem( MaterialExpression );
		DrawMaterialExpressionConnections( MaterialExpression, bShouldDrawCurves,  Canvas );
	}

	// Draw the material expression comments.
	for( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		DrawMaterialExpressionComments( MaterialExpression, Canvas );
	}

	const EMaterialShaderPlatform RequestingPlatform = Material->bIsFallbackMaterial ? MSP_SM2 : MSP_SM3;
	const FMaterialResource* MaterialResource = Material->GetMaterialResource(RequestingPlatform);
	
	INT DrawPositionY = 5;
	
	Canvas->PushAbsoluteTransform(FMatrix::Identity);
	DrawMaterialInfoStrings(Canvas, MaterialResource, DrawPositionY);
	DrawFallbackMaterialMessages(Canvas, DrawPositionY);
	Canvas->PopTransform();
}

/**
 * Draw status messages for the fallback material
 * @param DrawPositionY - the vertical position to place the messages, and will be updated with the next open slot for text
 */
void WxMaterialEditor::DrawFallbackMaterialMessages(FCanvas* Canvas, INT &DrawPositionY)
{
	if (!OriginalMaterial->bIsFallbackMaterial)
	{
		const FMaterialResource* FallbackMaterialResource = OriginalMaterial->GetMaterialResource(MSP_SM2);
		if (FallbackMaterialResource)
		{
			DWORD DroppedFallbackComponents = FallbackMaterialResource->GetDroppedFallbackComponents();
			if (DroppedFallbackComponents != DroppedFallback_None)
			{
				FString FallbackMessage = TEXT("Fallback Dropped Inputs: ");
				UBOOL bRequiresFallback = FALSE;
				if (DroppedFallbackComponents & DroppedFallback_Failed)
				{
					FallbackMessage = TEXT("Automatic Fallback Failed!");
					bRequiresFallback = TRUE;
				}
				else 
				{
					if (DroppedFallbackComponents & DroppedFallback_Specular)
					{
						FallbackMessage += TEXT("S ");
					}
					if (DroppedFallbackComponents & DroppedFallback_Normal)
					{
						FallbackMessage += TEXT("N ");
					}
					if (DroppedFallbackComponents & DroppedFallback_Diffuse)
					{
						FallbackMessage += TEXT("D ");
						bRequiresFallback = TRUE;
					}
					if (DroppedFallbackComponents & DroppedFallback_Emissive)
					{
						FallbackMessage += TEXT("E ");
						bRequiresFallback = TRUE;
					}
				}

				FLinearColor MessageColor = FLinearColor(1,1,0);
				if (bRequiresFallback)
				{
					FallbackMessage += TEXT(" Please provide a Fallback Material.");
					MessageColor = FLinearColor(1,0,0);
				}

				FLinkedObjDrawUtils::DrawShadowedString(
					Canvas,
					5,
					DrawPositionY,
					*FallbackMessage,
					GEngine->SmallFont,
					MessageColor
					);
				DrawPositionY += 20;
			}
		}
	}
}

/**
 * Called when the user right-clicks on an empty region of the expression viewport.
 */
void WxMaterialEditor::OpenNewObjectMenu()
{
	// The user has clicked on the background, so clear the last connector object reference so that
	// any newly created material expression node will not be automatically connected to the
	// connector last clicked upon.
	ConnObj = NULL;

	WxMaterialEditorContextMenu_NewNode Menu( this );
	FTrackPopupMenu tpm( this, &Menu );
	tpm.Show();
}


/**
 * Called when the user right-clicks on an object in the viewport.
 */
void WxMaterialEditor::OpenObjectOptionsMenu()
{
	WxMaterialEditorContextMenu_NodeOptions Menu( this );
	FTrackPopupMenu tpm( this, &Menu );
	tpm.Show();
}

/**
 * Called when the user right-clicks on an object connector in the viewport.
 */
void WxMaterialEditor::OpenConnectorOptionsMenu()
{
	WxMaterialEditorContextMenu_ConnectorOptions Menu( this );
	FTrackPopupMenu tpm( this, &Menu );
	tpm.Show();
}

/**
 * Returns TRUE if links are to be drawn as splines; returns FALSE if links are to be drawn as straight lines.
 */
UBOOL WxMaterialEditor::DrawCurves()
{
	return bDrawCurves;
}

/**
 * Updates the editor's property window to contain material expression properties if any are selected.
 * Otherwise, properties for the material are displayed.
 */
void WxMaterialEditor::UpdatePropertyWindow()
{
	if( SelectedExpressions.Num() == 0 && SelectedCompounds.Num() == 0 && SelectedComments.Num() == 0 )
	{
		PropertyWindow->SetObject( Material, TRUE, FALSE, TRUE );
	}
	else
	{
		TArray<UMaterialExpression*> SelectedObjects;

		// Expressions
		for ( INT Idx = 0 ; Idx < SelectedExpressions.Num() ; ++Idx )
		{
			SelectedObjects.AddItem( SelectedExpressions(Idx) );
		}
		
		// Compounds
		for ( INT Idx = 0 ; Idx < SelectedCompounds.Num() ; ++Idx )
		{
			SelectedObjects.AddItem( (UMaterialExpression*)SelectedCompounds(Idx) );
		}

		// Comments
		for ( INT Idx = 0 ; Idx < SelectedComments.Num() ; ++Idx )
		{
			SelectedObjects.AddItem( (UMaterialExpression*)SelectedComments(Idx) );
		}

		PropertyWindow->SetObjectArray( SelectedObjects, TRUE, FALSE, TRUE );
	}
}

/**
 * Deselects all selected material expressions and comments.
 */
void WxMaterialEditor::EmptySelection()
{
	SelectedExpressions.Empty();
	SelectedComments.Empty();
	SelectedCompounds.Empty();
}

/**
 * Add the specified object to the list of selected objects
 */
void WxMaterialEditor::AddToSelection(UObject* Obj)
{
	if( Obj->IsA(UMaterialExpressionComment::StaticClass()) )
	{
		SelectedComments.AddUniqueItem( static_cast<UMaterialExpressionComment*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpressionCompound::StaticClass()) )
	{
		SelectedCompounds.AddUniqueItem( static_cast<UMaterialExpressionCompound*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpression::StaticClass()) )
	{
		SelectedExpressions.AddUniqueItem( static_cast<UMaterialExpression*>(Obj) );
	}
}

/**
 * Remove the specified object from the list of selected objects.
 */
void WxMaterialEditor::RemoveFromSelection(UObject* Obj)
{
	if( Obj->IsA(UMaterialExpressionComment::StaticClass()) )
	{
		SelectedComments.RemoveItem( static_cast<UMaterialExpressionComment*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpressionCompound::StaticClass()) )
	{
		SelectedCompounds.RemoveItem( static_cast<UMaterialExpressionCompound*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpression::StaticClass()) )
	{
		SelectedExpressions.RemoveItem( static_cast<UMaterialExpression*>(Obj) );
	}
}

/**
 * Checks whether the specified object is currently selected.
 *
 * @return	TRUE if the specified object is currently selected
 */
UBOOL WxMaterialEditor::IsInSelection(UObject* Obj) const
{
	UBOOL bIsInSelection = FALSE;
	if( Obj->IsA(UMaterialExpressionComment::StaticClass()) )
	{
		bIsInSelection = SelectedComments.ContainsItem( static_cast<UMaterialExpressionComment*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpressionCompound::StaticClass()) )
	{
		bIsInSelection = SelectedCompounds.ContainsItem( static_cast<UMaterialExpressionCompound*>(Obj) );
	}
	else if( Obj->IsA(UMaterialExpression::StaticClass()) )
	{
		bIsInSelection = SelectedExpressions.ContainsItem( static_cast<UMaterialExpression*>(Obj) );
	}
	return bIsInSelection;
}

/**
 * Returns the number of selected objects
 */
INT WxMaterialEditor::GetNumSelected() const
{
	return SelectedExpressions.Num() + SelectedComments.Num() + SelectedCompounds.Num();
}

/**
 * Called when the user clicks on an unselected link connector.
 *
 * @param	Connector	the link connector that was clicked on
 */
void WxMaterialEditor::SetSelectedConnector(FLinkedObjectConnector& Connector)
{
	ConnObj = Connector.ConnObj;
	ConnType = Connector.ConnType;
	ConnIndex = Connector.ConnIndex;
	if ( ConnObj )
	{
		debugf(TEXT("SetSelectedConnector: %s(%s) %i %i"), *ConnObj->GetName(), *ConnObj->GetClass()->GetName(), ConnType, ConnIndex);
	}
	else
	{
		debugf(TEXT("SetSelectedConnector: %i %i"),ConnType, ConnIndex);
	}
}

/**
 * Gets the position of the selected connector.
 *
 * @return	an FIntPoint that represents the position of the selected connector, or (0,0)
 *			if no connectors are currently selected
 */
FIntPoint WxMaterialEditor::GetSelectedConnLocation(FCanvas* Canvas)
{
	if( ConnObj )
	{
		UMaterialExpression* ExpressionNode = Cast<UMaterialExpression>( ConnObj );
		if( ExpressionNode )
		{
			return GetExpressionConnectionLocation( ExpressionNode, ConnType, ConnIndex );
		}

		UMaterial* MaterialNode = Cast<UMaterial>( ConnObj );
		if( MaterialNode )
		{
			return GetMaterialConnectionLocation( MaterialNode, ConnType, ConnIndex );
		}
	}

	return FIntPoint(0,0);
}

/**
 * Gets the EConnectorHitProxyType for the currently selected connector
 *
 * @return	the type for the currently selected connector, or 0 if no connector is selected
 */
INT WxMaterialEditor::GetSelectedConnectorType()
{
	return ConnType;
}

/**
 * Makes a connection between connectors.
 */
void WxMaterialEditor::MakeConnectionToConnector(FLinkedObjectConnector& Connector)
{
	// Nothing to do if object pointers are NULL.
	if ( !Connector.ConnObj || !ConnObj )
	{
		return;
	}

	// Avoid connections to yourself.
	if( Connector.ConnObj == ConnObj )
	{
		return;
	}

	UMaterialExpressionCompound* EndConnCompound	= Cast<UMaterialExpressionCompound>( Connector.ConnObj );
	UMaterialExpressionCompound* ConnCompound		= Cast<UMaterialExpressionCompound>( ConnObj );

	// @todo DB: implement connections via compounds.
	if ( EndConnCompound || ConnCompound )
	{
		return;
	}

	UBOOL bConnectionWasMade = FALSE;

	UMaterial* EndConnMaterial						= Cast<UMaterial>( Connector.ConnObj );
	UMaterialExpression* EndConnMaterialExpression	= Cast<UMaterialExpression>( Connector.ConnObj );
	check( EndConnMaterial || EndConnMaterialExpression );

	UMaterial* ConnMaterial							= Cast<UMaterial>( ConnObj );
	UMaterialExpression* ConnMaterialExpression		= Cast<UMaterialExpression>( ConnObj );
	check( ConnMaterial || ConnMaterialExpression );

	// Material to . . .
	if ( ConnMaterial )
	{
		check( ConnType == LOC_OUTPUT ); // Materials are only connected along their right side.

		// Material to expression.
		if ( EndConnMaterialExpression )
		{
			if ( Connector.ConnType == LOC_INPUT ) // Material expressions can only connect to their materials on their left side.
			{
				//debugf(TEXT("mat: %i expr: %i"), ConnIndex, Connector.ConnIndex);
				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorMakeConnection")) );
				Material->Modify();
				ConnectMaterialToMaterialExpression( ConnMaterial, ConnIndex, EndConnMaterialExpression, Connector.ConnIndex );
				bConnectionWasMade = TRUE;
			}
		}
	}
	// Expression to . . .
	else if ( ConnMaterialExpression )
	{
		// Expression to expression.
		if ( EndConnMaterialExpression )
		{
			if ( ConnType != Connector.ConnType )
			{
				//debugf(TEXT("expr1: %i expr2: %i"), ConnIndex, Connector.ConnIndex);
				UMaterialExpression* InputExpression;
				UMaterialExpression* OutputExpression;
				INT InputIndex;
				INT OutputIndex;

				if( ConnType == LOC_OUTPUT && Connector.ConnType == LOC_INPUT )
				{
					InputExpression = ConnMaterialExpression;
					InputIndex = ConnIndex;
					OutputExpression = EndConnMaterialExpression;
					OutputIndex = Connector.ConnIndex;
				}
				else
				{
					InputExpression = EndConnMaterialExpression;
					InputIndex = Connector.ConnIndex;
					OutputExpression = ConnMaterialExpression;
					OutputIndex = ConnIndex;
				}

				// Input expression.
				const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( InputExpression );
				check( InputIndex < ExpressionInputs.Num() );
				FExpressionInput* ExpressionInput = (FExpressionInput*)((BYTE*)InputExpression + ExpressionInputs( InputIndex )->Offset);

				// Output expression.
				TArray<FExpressionOutput> Outputs;
				OutputExpression->GetOutputs(Outputs);
				const FExpressionOutput& ExpressionOutput = Outputs( OutputIndex );

				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorMakeConnection")) );
				InputExpression->Modify();
				ConnectExpressions( *ExpressionInput, ExpressionOutput, OutputExpression );

				bConnectionWasMade = TRUE;
			}
			else
			{
				//debugf( TEXT("rejecting like to like") );
			}
		}
		// Expression to Material.
		else if ( EndConnMaterial )
		{
			check( Connector.ConnType == LOC_OUTPUT ); // Materials are only connected along their right side.
			if ( ConnType == LOC_INPUT ) // Material expressions can only connect to their materials on their left side.
			{
				//debugf(TEXT("expr: %i mat: %i"), ConnIndex, Connector.ConnIndex);
				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorMakeConnection")) );
				Material->Modify();
				ConnectMaterialToMaterialExpression( EndConnMaterial, Connector.ConnIndex, ConnMaterialExpression, ConnIndex );
				bConnectionWasMade = TRUE;
			}
		}
	}
	
	if ( bConnectionWasMade )
	{
		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();
		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Breaks the link currently indicated by ConnObj/ConnType/ConnIndex.
 */
void WxMaterialEditor::BreakConnLink()
{
	if ( !ConnObj )
	{
		return;
	}

	UBOOL bConnectionWasBroken = FALSE;

	UMaterialExpression* MaterialExpression	= Cast<UMaterialExpression>( ConnObj );
	UMaterial* MaterialNode					= Cast<UMaterial>( ConnObj );

	if ( ConnType == LOC_OUTPUT ) // right side
	{
		if ( MaterialNode )
		{
			// Clear the material input.
			FExpressionInput* MaterialInput = GetMaterialInput( MaterialNode, ConnIndex );
			if ( MaterialInput->Expression != NULL )
			{
				const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorBreakConnection")) );
				Material->Modify();
				MaterialInput->Expression = NULL;

				bConnectionWasBroken = TRUE;
			}
		}
		else if ( MaterialExpression )
		{
			const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( MaterialExpression );

			UStructProperty* ExpressionInput = ExpressionInputs(ConnIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);

			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorBreakConnection")) );
			MaterialExpression->Modify();
			Input->Expression = NULL;

			bConnectionWasBroken = TRUE;
		}
	}
	else if ( ConnType == LOC_INPUT ) // left side
	{
		// Only expressions can have connectors on the left side.
		check( MaterialExpression );
		TArray<FExpressionOutput> Outputs;
		MaterialExpression->GetOutputs(Outputs);
		const FExpressionOutput& ExpressionOutput = Outputs( ConnIndex );

		// Get a list of inputs that refer to the selected output.
		TArray<FExpressionInput*> ReferencingInputs;
		GetListOfReferencingInputs( MaterialExpression, Material, ExpressionOutput, ReferencingInputs );

		bConnectionWasBroken = ReferencingInputs.Num() > 0;
		if ( bConnectionWasBroken )
		{
			// Clear all referencing inputs.
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorBreakConnection")) );
			Material->Modify();///////////////
			for ( INT InputIndex = 0 ; InputIndex < ReferencingInputs.Num() ; ++InputIndex )
			{
				FExpressionInput* Input = ReferencingInputs(InputIndex);
				check( Input->Expression == MaterialExpression );
				Input->Expression = NULL;
			}
		}
	}

	if ( bConnectionWasBroken )
	{
		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();
		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Connects an expression output to the specified material input slot.
 *
 * @param	MaterialInputIndex		Index to the material input (Diffuse=0, Emissive=1, etc).
 */
void WxMaterialEditor::OnConnectToMaterial(INT MaterialInputIndex)
{
	// Checks if over expression connection.
	UMaterialExpression* MaterialExpression = NULL;
	if ( ConnObj )
	{
		MaterialExpression = Cast<UMaterialExpression>( ConnObj );
	}

	UBOOL bConnectionWasMade = FALSE;
	if ( MaterialExpression && ConnType == LOC_INPUT )
	{
		UMaterial* MaterialNode = Cast<UMaterial>( ConnObj );

		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorMakeConnection")) );
		Material->Modify();
		ConnectMaterialToMaterialExpression(Material, MaterialInputIndex, MaterialExpression, ConnIndex);
		bConnectionWasMade = TRUE;
	}

	if ( bConnectionWasMade && MaterialExpression )
	{
		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();
		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Breaks all connections to/from selected material expressions.
 */
void WxMaterialEditor::BreakAllConnectionsToSelectedExpressions()
{
	if ( SelectedExpressions.Num() > 0 )
	{
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorBreakAllConnections")) );
			for( INT ExpressionIndex = 0 ; ExpressionIndex < SelectedExpressions.Num() ; ++ExpressionIndex )
			{
				UMaterialExpression* MaterialExpression = SelectedExpressions( ExpressionIndex );
				MaterialExpression->Modify();

				// Break links to expression.
				BreakLinksToExpression( MaterialExpression, Material );

				// Break links from expression
				const TArray<UStructProperty*>& ExpressionInputs = WxMaterialEditor::GetMaterialExpressionInputs( MaterialExpression );
				for( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ ExpressionInputIndex )
				{
					UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
					FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
					Input->Expression = NULL;
				}
			}
		}

		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();
		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

/** Removes the selected expression from the favorites list. */
void WxMaterialEditor::RemoveSelectedExpressionFromFavorites()
{
	if ( SelectedExpressions.Num() == 1 )
	{
		UMaterialExpression* MaterialExpression = SelectedExpressions(0);
		if (MaterialExpression)
		{
			RemoveMaterialExpressionFromFavorites(MaterialExpression->GetClass());
			EditorOptions->FavoriteExpressions.RemoveItem(MaterialExpression->GetClass()->GetName());
			EditorOptions->SaveConfig();
		}
	}
}

/** Adds the selected expression to the favorites list. */
void WxMaterialEditor::AddSelectedExpressionToFavorites()
{
	if ( SelectedExpressions.Num() == 1 )
	{
		UMaterialExpression* MaterialExpression = SelectedExpressions(0);
		if (MaterialExpression)
		{
			AddMaterialExpressionToFavorites(MaterialExpression->GetClass());
			EditorOptions->FavoriteExpressions.AddUniqueItem(MaterialExpression->GetClass()->GetName());
			EditorOptions->SaveConfig();
		}
	}
}

/**
 * Called when the user releases the mouse over a link connector and is holding the ALT key.
 * Commonly used as a shortcut to breaking connections.
 *
 * @param	Connector	The connector that was ALT+clicked upon.
 */
void WxMaterialEditor::AltClickConnector(FLinkedObjectConnector& Connector)
{
	BreakConnLink();
}

/**
 * Called when the user performs a draw operation while objects are selected.
 *
 * @param	DeltaX	the X value to move the objects
 * @param	DeltaY	the Y value to move the objects
 */
void WxMaterialEditor::MoveSelectedObjects(INT DeltaX, INT DeltaY)
{
	const UBOOL bFirstMove = LinkedObjVC->DistanceDragged < 4;
	if ( bFirstMove )
	{
		ExpressionsLinkedToComments.Empty();
	}

	TArray<UMaterialExpression*> ExpressionsToMove;

	// Add selected expressions.
	for( INT ExpressionIndex = 0 ; ExpressionIndex < SelectedExpressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* Expression = SelectedExpressions(ExpressionIndex);
		ExpressionsToMove.AddItem( Expression );
	}

	if ( SelectedComments.Num() > 0 )
	{
		TArray<FIntRect> CommentRects;

		// Add selected comments.  At the same time, create rects for the comments.
		for( INT CommentIndex = 0 ; CommentIndex < SelectedComments.Num() ; ++CommentIndex )
		{
			UMaterialExpressionComment* Comment = SelectedComments(CommentIndex);
			ExpressionsToMove.AddItem( Comment );
			CommentRects.AddItem( FIntRect( FIntPoint(Comment->EditorX, Comment->EditorY),
											FIntPoint(Comment->EditorX+Comment->SizeX, Comment->EditorY+Comment->SizeY) ) );
		}

		// If this is the first move, generate a list of expressions that are contained by comments.
		if ( bFirstMove )
		{
			for( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
			{
				UMaterialExpression* Expression = Material->Expressions(ExpressionIndex);
				const FIntPoint ExpressionPos( Expression->EditorX, Expression->EditorY );
				for( INT CommentIndex = 0 ; CommentIndex < CommentRects.Num() ; ++CommentIndex )
				{
					const FIntRect& CommentRect = CommentRects(CommentIndex);
					if ( CommentRect.Contains(ExpressionPos) )
					{
						ExpressionsLinkedToComments.AddUniqueItem( Expression );
						break;
					}
				}
			}
		}

		// Merge the expression lists.
		for( INT ExpressionIndex = 0 ; ExpressionIndex < ExpressionsLinkedToComments.Num() ; ++ExpressionIndex )
		{
			UMaterialExpression* Expression = ExpressionsLinkedToComments(ExpressionIndex);
			ExpressionsToMove.AddUniqueItem( Expression );
		}
	}

	// Perform the move.
	if ( SelectedCompounds.Num() > 0 || ExpressionsToMove.Num() > 0 )
	{
		for( INT CompoundIndex = 0 ; CompoundIndex < SelectedCompounds.Num() ; ++CompoundIndex )
		{
			UMaterialExpressionCompound* Compound = SelectedCompounds(CompoundIndex);
			Compound->EditorX += DeltaX;
			Compound->EditorY += DeltaY;
			for( INT ExpressionIndex = 0 ; ExpressionIndex < Compound->MaterialExpressions.Num() ; ++ExpressionIndex )
			{
				UMaterialExpression* Expression = Compound->MaterialExpressions(ExpressionIndex);
				Expression->EditorX += DeltaX;
				Expression->EditorY += DeltaY;
			}
		}

		for( INT ExpressionIndex = 0 ; ExpressionIndex < ExpressionsToMove.Num() ; ++ExpressionIndex )
		{
			UMaterialExpression* Expression = ExpressionsToMove(ExpressionIndex);
			Expression->EditorX += DeltaX;
			Expression->EditorY += DeltaY;
		}
		Material->MarkPackageDirty();
		bMaterialDirty = TRUE;
	}
}

void WxMaterialEditor::EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event)
{
	if( Event == IE_Pressed )
	{
		const UBOOL bCtrlDown = Viewport->KeyState(KEY_LeftControl) || Viewport->KeyState(KEY_RightControl);
		if ( bCtrlDown )
		{
			if( Key == KEY_C )
			{
				// Clear the material copy buffer and copy the selected expressions into it.
				TArray<UMaterialExpression*> NewExpressions;
				TArray<UMaterialExpression*> NewComments;
				GetCopyPasteBuffer()->Expressions.Empty();
				GetCopyPasteBuffer()->EditorComments.Empty();

				CopyMaterialExpressions( SelectedExpressions, SelectedComments, GetCopyPasteBuffer(), NewExpressions, NewComments );
			}
			else if ( Key == KEY_V )
			{
				// Paste the material copy buffer into this material.
				PasteExpressionsIntoMaterial( NULL );
			}
			else if( Key == KEY_W )
			{
				DuplicateSelectedObjects();
			}
			else if( Key == KEY_Y )
			{
				Redo();
			}
			else if( Key == KEY_Z )
			{
				Undo();
			}
		}
		else
		{
			if( Key == KEY_Delete )
			{
				DeleteSelectedObjects();
			}
			else if( Key == KEY_C )
			{
				CreateNewCommentExpression();
			}
			else if ( Key == KEY_SpaceBar )
			{
				// Spacebar force-refreshes previews.
				ForceRefreshExpressionPreviews();
				RefreshExpressionViewport();
				RefreshPreviewViewport();
			}
			else if ( Key == KEY_LeftMouseButton )
			{
				// Check if the "Toggle realtime preview" icon was clicked on.
				Viewport->Invalidate();
				const INT HitX = Viewport->GetMouseX();
				const INT HitY = Viewport->GetMouseY();
				HHitProxy*	HitResult = Viewport->GetHitProxy( HitX, HitY );
				if ( HitResult && HitResult->IsA( HRealtimePreviewProxy::StaticGetType() ) )
				{
					// Toggle the material expression's realtime preview state.
					UMaterialExpression* MaterialExpression = static_cast<HRealtimePreviewProxy*>( HitResult )->MaterialExpression;
					check( MaterialExpression );
					{
						const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorTogglePreview")) );
						MaterialExpression->Modify();
						MaterialExpression->bRealtimePreview = !MaterialExpression->bRealtimePreview;
					}
					MaterialExpression->PreEditChange( NULL );
					MaterialExpression->PostEditChange( NULL );
					MaterialExpression->MarkPackageDirty();
					bMaterialDirty = TRUE;

					// Update the preview.
					RefreshExpressionPreview( MaterialExpression );
					RefreshPreviewViewport();
				}
			}
		}
	}
	else if ( Event == IE_Released )
	{
		// LMBRelease + hotkey places certain material expression types.
		if( Key == KEY_LeftMouseButton )
		{
			struct FMaterialExpressionHotkey
			{
				FName		Key;
				UClass*		MaterialExpressionClass;
			};
			static FMaterialExpressionHotkey MaterialExpressionHotkeys[] =
			{
				{ KEY_A, UMaterialExpressionAdd::StaticClass() },
				{ KEY_B, UMaterialExpressionBumpOffset::StaticClass() },
				{ KEY_D, UMaterialExpressionDivide::StaticClass() },
				{ KEY_I, UMaterialExpressionIf::StaticClass() },
				{ KEY_L, UMaterialExpressionLinearInterpolate::StaticClass() },
				{ KEY_M, UMaterialExpressionMultiply::StaticClass() },
				{ KEY_N, UMaterialExpressionNormalize::StaticClass() },
				{ KEY_O, UMaterialExpressionOneMinus::StaticClass() },
				{ KEY_P, UMaterialExpressionPanner::StaticClass() },
				{ KEY_S, UMaterialExpressionSubtract::StaticClass() },
				{ KEY_T, UMaterialExpressionTextureSample::StaticClass() },
				{ KEY_One, UMaterialExpressionConstant::StaticClass() },
				{ KEY_Two, UMaterialExpressionConstant2Vector::StaticClass() },
				{ KEY_Three, UMaterialExpressionConstant3Vector::StaticClass() },
				{ KEY_Four, UMaterialExpressionConstant4Vector::StaticClass() },
				{ NAME_None, NULL },
			};

			// Iterate over all expression hotkeys, reporting the first expression that has a key down.
			UClass* NewMaterialExpressionClass = NULL;
			for ( INT Index = 0 ; ; ++Index )
			{
				const FMaterialExpressionHotkey& ExpressionHotKey = MaterialExpressionHotkeys[Index];
				if ( ExpressionHotKey.MaterialExpressionClass )
				{
					if ( Viewport->KeyState(ExpressionHotKey.Key) )
					{
						NewMaterialExpressionClass = ExpressionHotKey.MaterialExpressionClass;
						break;
					}
				}
				else
				{
					// A NULL MaterialExpressionClass indicates end of list.
					break;
				}
			}

			// Create a new material expression if the associated hotkey was found to be down.
			if ( NewMaterialExpressionClass )
			{
				const INT LocX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
				const INT LocY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
				CreateNewMaterialExpression( NewMaterialExpressionClass, FALSE, FIntPoint(LocX, LocY) );
			}
		}
	}
}

/**
 * Called once the user begins a drag operation.  Transactions expression, comment and compound position.
 */
void WxMaterialEditor::BeginTransactionOnSelected()
{
	check( !ScopedTransaction );
	ScopedTransaction = new FScopedTransaction( *LocalizeUnrealEd(TEXT("LinkedObjectModify")) );

	Material->Modify();
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->Expressions.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( MaterialExpressionIndex );
		MaterialExpression->Modify();
	}
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->EditorComments.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpressionComment* Comment = Material->EditorComments( MaterialExpressionIndex );
		Comment->Modify();
	}
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Material->EditorCompounds.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpressionCompound* Compound = Material->EditorCompounds( MaterialExpressionIndex );
		Compound->Modify();
	}
}

/**
 * Called when the user releases the mouse button while a drag operation is active.
 */
void WxMaterialEditor::EndTransactionOnSelected()
{
	check( ScopedTransaction );
	delete ScopedTransaction;
	ScopedTransaction = NULL;
}


/**
 *	Handling for dragging on 'special' hit proxies. Should only have 1 object selected when this is being called. 
 *	In this case used for handling resizing handles on Comment objects. 
 *
 *	@param	DeltaX			Horizontal drag distance this frame (in pixels)
 *	@param	DeltaY			Vertical drag distance this frame (in pixels)
 *	@param	SpecialIndex	Used to indicate different classes of action. @see HLinkedObjProxySpecial.
 */
void WxMaterialEditor::SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex )
{
	// Can only 'special drag' one object at a time.
	if( SelectedComments.Num() == 1 )
	{
		if ( SpecialIndex == 1 )
		{
			// Apply dragging to the comment size.
			UMaterialExpressionComment* Comment = SelectedComments(0);
			Comment->SizeX += DeltaX;
			Comment->SizeX = ::Max<INT>(Comment->SizeX, 64);

			Comment->SizeY += DeltaY;
			Comment->SizeY = ::Max<INT>(Comment->SizeY, 64);
			Comment->MarkPackageDirty();
			bMaterialDirty = TRUE;
		}
	}
}

/**
 * Updates the original material with the changes made in the editor
 */
void WxMaterialEditor::UpdateOriginalMaterial()
{
	FString ParameterMismatches;
	Material->GetFallbackParameterInconsistencies(ParameterMismatches);

	//if a fallback material is specified and it has parameters that don't match the base material parameters,
	//warn the user of potential consequences
	if (ParameterMismatches.Len() > 0)
	{
		const FString WarningMsg = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Warning_MaterialEditorFallbackParameterMismatch"), *ParameterMismatches));
		INT YesNoReply = appMsgf(AMT_YesNo, *WarningMsg);

		if (YesNoReply == 0)
		{
			//if user selected no, don't update original material
			return;
		}
	}

	//remove any memory copies of shader files, so they will be reloaded from disk
	//this way the material editor can be used for quick shader iteration
	FlushShaderFileCache();

	const FScopedBusyCursor BusyCursor;

	const FString LocalizedMaterialEditorApply( LocalizeUnrealEd("ToolTip_MaterialEditorApply") );
	GWarn->BeginSlowTask( *LocalizedMaterialEditorApply, TRUE );
	GWarn->StatusUpdatef( 1, 1, *LocalizedMaterialEditorApply );

	// make sure that any staticmeshes, etc using this material will stop using the FMaterialResource of the original 
	// material, and will use the new FMaterialResource created when we make a new UMaterial inplace
	FGlobalComponentReattachContext RecreateComponents;

	// overwrite the original material in place by constructing a new one with the same name
	OriginalMaterial = (UMaterial*)UObject::StaticDuplicateObject(Material, Material, OriginalMaterial->GetOuter(), *OriginalMaterial->GetName(), ~0, OriginalMaterial->GetClass());
	// Restore RF_Standalone on the original material, as it had been removed from the preview material so that it could be GC'd.
	OriginalMaterial->SetFlags( RF_Standalone );

	// let the material update itself if necessary
	OriginalMaterial->PreEditChange(NULL);
	OriginalMaterial->PostEditChange(NULL);
	OriginalMaterial->MarkPackageDirty();

	//copy the compile errors from the original material to the preview material
	//this is necessary since some errors are not encountered while compiling the preview material, but only when compiling the full material
	//without this copy the errors will only be reported after the material editor is closed and reopened
	const EMaterialShaderPlatform RequestingPlatform = Material->bIsFallbackMaterial ? MSP_SM2 : MSP_SM3;
	const FMaterialResource* OriginalMaterialResource = OriginalMaterial->GetMaterialResource(RequestingPlatform);
	FMaterialResource* PreviewMaterialResource = Material->GetMaterialResource(RequestingPlatform);
	if (OriginalMaterialResource && PreviewMaterialResource)
	{
		PreviewMaterialResource->SetCompileErrors(OriginalMaterialResource->GetCompileErrors());
	}

	// clear the dirty flag
	bMaterialDirty = FALSE;

	// update the world's viewports
	GCallbackEvent->Send(CALLBACK_RefreshEditor);
	GCallbackEvent->Send(CALLBACK_RedrawAllViewports);

	// Loop through all loaded material instance constants and update their parameter names since they may have changed.
	for (TObjectIterator<UMaterialInstanceConstant> It; It; ++It)
	{
		It->UpdateParameterNames();
	}

	// Go through all loaded material instances and recompile their static permutation resources if needed
	// This is necessary since the parent UMaterial stores information about how it should be rendered, (eg bUsesDistortion)
	// but the child can have its own shader map which may not contain all the shaders that the parent's settings indicate that it should.
	// this code is duplicated in Material.cpp UMaterial::SetMaterialUsage
	for (TObjectIterator<UMaterialInstance> It; It; ++It)
	{
		UMaterialInstance* CurrentMaterialInstance = *It;
		UMaterial* BaseMaterial = CurrentMaterialInstance->GetMaterial();
		//only recompile if the instance is a child of the material that got updated
		if (OriginalMaterial == BaseMaterial)
		{
			CurrentMaterialInstance->InitStaticPermutation();
		}
	}

	GWarn->EndSlowTask();
}

/**
 * Uses the global Undo transactor to reverse changes, update viewports etc.
 */
void WxMaterialEditor::Undo()
{
	EmptySelection();

	INT NumExpressions = Material->Expressions.Num();
	GEditor->UndoTransaction();

	if(NumExpressions != Material->Expressions.Num())
	{
		Material->BuildEditorParameterList();
	}

	UpdatePropertyWindow();

	RefreshExpressionPreviews();
	RefreshExpressionViewport();
	RefreshPreviewViewport();
	bMaterialDirty = TRUE;
}

/**
 * Uses the global Undo transactor to redo changes, update viewports etc.
 */
void WxMaterialEditor::Redo()
{
	EmptySelection();

	INT NumExpressions = Material->Expressions.Num();
	GEditor->RedoTransaction();

	if(NumExpressions != Material->Expressions.Num())
	{
		Material->BuildEditorParameterList();
	}

	UpdatePropertyWindow();

	RefreshExpressionPreviews();
	RefreshExpressionViewport();
	RefreshPreviewViewport();
	bMaterialDirty = TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FSerializableObject interface
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxMaterialEditor::Serialize(FArchive& Ar)
{
	WxMaterialEditorBase::Serialize(Ar);

	Ar << Material;
	Ar << SelectedExpressions;
	Ar << SelectedComments;
	Ar << SelectedCompounds;
	Ar << ExpressionPreviews;
	Ar << ExpressionsLinkedToComments;
	Ar << EditorOptions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FNotifyHook interface
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxMaterialEditor::NotifyPreChange(void* Src, UProperty* PropertyAboutToChange)
{
	check( !ScopedTransaction );
	ScopedTransaction = new FScopedTransaction( *LocalizeUnrealEd(TEXT("MaterialEditorEditProperties")) );
	FlushRenderingCommands();
}

void WxMaterialEditor::NotifyPostChange(void* Src, UProperty* PropertyThatChanged)
{
	check( ScopedTransaction );

	if ( PropertyThatChanged )
	{
		const FName NameOfPropertyThatChanged( *PropertyThatChanged->GetName() );
		if ( NameOfPropertyThatChanged == FName(TEXT("PreviewMesh")) ||
			 NameOfPropertyThatChanged == FName(TEXT("bUsedWithSkeletalMesh")) )
		{
			// SetPreviewMesh will return FALSE if the material has bUsedWithSkeletalMesh and
			// a skeleton was requested, in which case revert to a sphere static mesh.
			if ( !SetPreviewMesh( *Material->PreviewMesh ) )
			{
				SetPreviewMesh( GUnrealEd->GetThumbnailManager()->TexPropSphere, NULL );
			}
		}

		WxPropertyWindow_Base* PropertyWindowItem = static_cast<WxPropertyWindow_Base*>( Src );
		WxPropertyWindow* PropertyWindow = PropertyWindowItem->GetPropertyWindow();
		for ( WxPropertyWindow::TObjectIterator Itor( PropertyWindow->ObjectIterator() ) ; Itor ; ++Itor )
		{
			UMaterialExpression* Expression = Cast< UMaterialExpression >( *Itor );
			if(Expression)
			{
				if(NameOfPropertyThatChanged == FName(TEXT("ParameterName")))
				{
					Material->UpdateExpressionParameterName(Expression);
				}
				else if (NameOfPropertyThatChanged == FName(TEXT("ParamNames")))
				{
					Material->UpdateExpressionDynamicParameterNames(Expression);
				}
				else
				{
					Material->PropagateExpressionParameterChanges(Expression);
				}
			}
		}
	}

	delete ScopedTransaction;
	ScopedTransaction = NULL;

	Material->MarkPackageDirty();
	RefreshExpressionPreviews();
	RefreshPreviewViewport();
	bMaterialDirty = TRUE;
}

void WxMaterialEditor::NotifyExec(void* Src, const TCHAR* Cmd)
{
	GUnrealEd->NotifyExec( Src, Cmd );
}

/** 
 * Computes a bounding box for the selected material expressions.  Output is not sensible if no expressions are selected.
 */
FIntRect WxMaterialEditor::GetBoundingBoxOfSelectedExpressions()
{
	return GetBoundingBoxOfExpressions( SelectedExpressions );
}

/** 
 * Computes a bounding box for the specified set of material expressions.  Output is not sensible if the set is empty.
 */
FIntRect WxMaterialEditor::GetBoundingBoxOfExpressions(const TArray<UMaterialExpression*>& Expressions)
{
	FIntRect Result(0, 0, 0, 0);
	UBOOL bResultValid = FALSE;

	for( INT ExpressionIndex = 0 ; ExpressionIndex < Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* Expression = Expressions(ExpressionIndex);
		// @todo DB: Remove these hardcoded extents.
		const FIntRect ObjBox( Expression->EditorX-30, Expression->EditorY-20, Expression->EditorX+150, Expression->EditorY+150 );

		if( bResultValid )
		{
			// Expand result box to encompass
			Result.Min.X = ::Min(Result.Min.X, ObjBox.Min.X);
			Result.Min.Y = ::Min(Result.Min.Y, ObjBox.Min.Y);

			Result.Max.X = ::Max(Result.Max.X, ObjBox.Max.X);
			Result.Max.Y = ::Max(Result.Max.Y, ObjBox.Max.Y);
		}
		else
		{
			// Use this objects bounding box to initialise result.
			Result = ObjBox;
			bResultValid = TRUE;
		}
	}

	return Result;
}

/**
 * Creates a new material expression comment frame.
 */
void WxMaterialEditor::CreateNewCommentExpression()
{
	WxDlgGenericStringEntry dlg;
	const INT Result = dlg.ShowModal( TEXT("NewComment"), TEXT("CommentText"), TEXT("Comment") );
	if ( Result == wxID_OK )
	{
		UMaterialExpressionComment* NewComment = NULL;
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorNewComment")) );
			Material->Modify();

			NewComment = ConstructObject<UMaterialExpressionComment>( UMaterialExpressionComment::StaticClass(), Material, NAME_None, RF_Transactional );

			// Add to the list of comments associated with this material.
			Material->EditorComments.AddItem( NewComment );

			if ( SelectedExpressions.Num() > 0 )
			{
				const FIntRect SelectedBounds = GetBoundingBoxOfSelectedExpressions();
				NewComment->EditorX = SelectedBounds.Min.X;
				NewComment->EditorY = SelectedBounds.Min.Y;
				NewComment->SizeX = SelectedBounds.Max.X - SelectedBounds.Min.X;
				NewComment->SizeY = SelectedBounds.Max.Y - SelectedBounds.Min.Y;
			}
			else
			{

				NewComment->EditorX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
				NewComment->EditorY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
				NewComment->SizeX = 128;
				NewComment->SizeY = 128;
			}

			NewComment->Text = dlg.GetEnteredString();
		}

		// Select the new comment.
		EmptySelection();
		AddToSelection( NewComment );

		Material->MarkPackageDirty();
		RefreshExpressionViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Updates the CompoundInfoMap entry for the specified compound.
 */
void WxMaterialEditor::UpdateCompoundExpressionInfo(UMaterialExpressionCompound* Compound)
{
	FCompoundInfo* Info;
	FCompoundInfo** OldInfo = CompoundInfoMap.Find( Compound );
	if ( OldInfo )
	{
		// Clear out old info.
		Info = *OldInfo;
		Info->CompoundInputs.Empty();
		Info->CompoundOutputs.Empty();
	}
	else
	{
		// No old info found; allocate new info.
		Info = new FCompoundInfo;
		CompoundInfoMap.Set( Compound, Info );
	}

	// Make a list of compound inputs (expression inputs that are connected to expressions outside of the compound).
	for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Compound->MaterialExpressions.Num() ; ++MaterialExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Compound->MaterialExpressions( MaterialExpressionIndex );

		const TArray<UStructProperty*>& ExpressionInputs = GetMaterialExpressionInputs( MaterialExpression );
		for( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ ExpressionInputIndex )
		{
			UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
			UMaterialExpression* InputExpression = Input->Expression;

			// Store the input if it refers to an expression outside of the compound.
			if ( InputExpression && !Compound->MaterialExpressions.ContainsItem( InputExpression ) )
			{
				FCompoundInput* CompoundInput = new(Info->CompoundInputs) FCompoundInput;
				CompoundInput->Expression = MaterialExpression;
				CompoundInput->ExpressionInput = ExpressionInput;
			}
		}
	}

	// Make a list of compound outputs (expression outputs that are referred to by expressions outside of the compound).
	for ( INT ExpressionIndex = 0 ; ExpressionIndex < Material->Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpression* MaterialExpression = Material->Expressions( ExpressionIndex );
		if ( Compound->MaterialExpressions.ContainsItem( MaterialExpression ) )
		{
			continue;
		}

		// See if this expression takes input from anything in the compound.
		const TArray<UStructProperty*>& ExpressionInputs = WxMaterialEditor::GetMaterialExpressionInputs( MaterialExpression );
		for ( INT ExpressionInputIndex = 0 ; ExpressionInputIndex < ExpressionInputs.Num() ; ++ExpressionInputIndex )
		{
			UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
			UMaterialExpression* InputExpression = Input->Expression;
			if ( InputExpression && Compound->MaterialExpressions.ContainsItem( InputExpression ) )
			{
				// Determine which output is connected to the external object.
				TArray<FExpressionOutput> Outputs;
				InputExpression->GetOutputs( Outputs );
				for( INT OutputIndex = 0 ; OutputIndex < Outputs.Num() ; ++OutputIndex )
				{
					FExpressionOutput& Output = Outputs(OutputIndex);
					if(	Output.Mask == Input->Mask
						&& Output.MaskR == Input->MaskR
						&& Output.MaskG == Input->MaskG
						&& Output.MaskB == Input->MaskB
						&& Output.MaskA == Input->MaskA )
					{
						FCompoundOutput* CompoundOutput = new(Info->CompoundOutputs) FCompoundOutput;
						CompoundOutput->Expression = MaterialExpression;
						CompoundOutput->ExpressionOutput = Output;
					}
				}
			}
		}
	}
}

/**
 * Creates a new compound material expression from the selected material expressions.
 */
void WxMaterialEditor::CreateNewCompoundExpression()
{
	if ( SelectedExpressions.Num() > 0 )
	{
		// Make sure no selected material expressions 1) already belong to compounds; or 2) are compounds themselves.
		FString ErrorString;
		for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < SelectedExpressions.Num() ; ++MaterialExpressionIndex )
		{
			UMaterialExpression* MaterialExpression			= SelectedExpressions( MaterialExpressionIndex );
			if ( MaterialExpression->Compound )
			{
				const FString NewError( FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_ExpressionAlreadyBelongsToCompound_F"), *MaterialExpression->GetCaption(), *MaterialExpression->Compound->GetCaption()) ) );
				ErrorString += FString::Printf( TEXT("%s\n"), *NewError );
			}
			else
			{
				UMaterialExpressionCompound* CompoundExpression	= Cast<UMaterialExpressionCompound>( MaterialExpression );
				if ( CompoundExpression )
				{
					const FString NewError( FString::Printf( LocalizeSecure(LocalizeUnrealEd("Error_ExpressionCompoundsCantContainOtherCompounds_F"), *MaterialExpression->GetCaption()) ) );
					ErrorString += NewError + TEXT("\n");
				}
			}
		}

		if ( ErrorString.Len() )
		{
			FString CompleteErrorString( FString::Printf( TEXT("%s\n\n"), *LocalizeUnrealEd("Error_CantCreateMaterialExpressionCompound") ) );
			CompleteErrorString += ErrorString;
			appMsgf( AMT_OK, *CompleteErrorString );
			return;
		}

		// Create a new compound expression.
		UMaterialExpressionCompound* NewCompound = NULL;
		{
			const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorNewCompound")) );
			Material->Modify();

			NewCompound = ConstructObject<UMaterialExpressionCompound>( UMaterialExpressionCompound::StaticClass(), Material, NAME_None, RF_Transactional );
			Material->EditorCompounds.AddItem( NewCompound );

			const FIntRect SelectedBounds = GetBoundingBoxOfSelectedExpressions();
			const FIntPoint Extents( SelectedBounds.Max.X - SelectedBounds.Min.X, SelectedBounds.Max.Y - SelectedBounds.Min.Y );
			NewCompound->EditorX = SelectedBounds.Min.X + Extents.X/2;
			NewCompound->EditorY = SelectedBounds.Min.Y + Extents.Y/2;
			NewCompound->Caption = FString::Printf( TEXT("Compound Expression (%i)"), SelectedExpressions.Num() );

			// Link expressions to the compound and vice versa.
			for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < SelectedExpressions.Num() ; ++MaterialExpressionIndex )
			{
				UMaterialExpression* MaterialExpression = SelectedExpressions( MaterialExpressionIndex );
				check( MaterialExpression->Compound == NULL );
				MaterialExpression->Compound = NewCompound;
				NewCompound->MaterialExpressions.AddItem( MaterialExpression );
			}
		}

		UpdateCompoundExpressionInfo( NewCompound );

		// Select the new compound node.
		EmptySelection();
		AddToSelection( NewCompound );

		Material->MarkPackageDirty();
		RefreshExpressionViewport();
		bMaterialDirty = TRUE;
	}
}

/**
 * Creates a new material expression of the specified class.  Will automatically connect output 0
 * of the new expression to an input tab, if one was clicked on.
 *
 * @param	NewExpressionClass		The type of material expression to add.  Must be a child of UMaterialExpression.
 * @param	bAutoConnect			If TRUE, connect the new material expression to the most recently selected connector, if possible.
 * @param	NodePos					Position of the new node.
 */
void WxMaterialEditor::CreateNewMaterialExpression(UClass* NewExpressionClass, UBOOL bAutoConnect, const FIntPoint& NodePos)
{
	check( NewExpressionClass->IsChildOf(UMaterialExpression::StaticClass()) );

	// Create the new expression.
	UMaterialExpression* NewExpression = NULL;
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("MaterialEditorNewExpression")) );
		Material->Modify();

		NewExpression = ConstructObject<UMaterialExpression>( NewExpressionClass, Material, NAME_None, RF_Transactional );
		Material->Expressions.AddItem( NewExpression );

		// If the new expression is created connected to an input tab, offset it by this amount.
		INT NewConnectionOffset = 0;

		// If an input tab was clicked on, bind the new expression to that tab.
		if ( bAutoConnect && ConnType == LOC_OUTPUT && ConnObj )
		{
			UMaterial* ConnMaterial							= Cast<UMaterial>( ConnObj );
			UMaterialExpression* ConnMaterialExpression		= Cast<UMaterialExpression>( ConnObj );
			check( ConnMaterial  || ConnMaterialExpression );

			if ( ConnMaterial )
			{
				ConnectMaterialToMaterialExpression( ConnMaterial, ConnIndex, NewExpression, 0 );
			}
			else if ( ConnMaterialExpression )
			{
				UMaterialExpression* InputExpression = ConnMaterialExpression;
				UMaterialExpression* OutputExpression = NewExpression;

				INT InputIndex = ConnIndex;
				INT OutputIndex = 0;

				// Input expression.
				const TArray<UStructProperty*>& ExpressionInputs =GetMaterialExpressionInputs( InputExpression );
				check( InputIndex < ExpressionInputs.Num() );
				FExpressionInput* ExpressionInput = (FExpressionInput*)((BYTE*)InputExpression + ExpressionInputs( InputIndex )->Offset);

				// Output expression.
				TArray<FExpressionOutput> Outputs;
				OutputExpression->GetOutputs(Outputs);
				const FExpressionOutput& ExpressionOutput = Outputs( OutputIndex );

				InputExpression->Modify();
				ConnectExpressions( *ExpressionInput, ExpressionOutput, OutputExpression );
			}

			// Offset the new expression it by this amount.
			NewConnectionOffset = 30;
		}

		// Set the expression location.
		NewExpression->EditorX = NodePos.X + NewConnectionOffset;
		NewExpression->EditorY = NodePos.Y + NewConnectionOffset;

		// If the user is adding a texture sample, automatically assign the currently selected texture to it.
		UMaterialExpressionTextureSample* METextureSample = Cast<UMaterialExpressionTextureSample>( NewExpression );
		if( METextureSample )
		{
			METextureSample->Texture = GEditor->GetSelectedObjects()->GetTop<UTexture>();
		}

		// If the user is creating any parameter objects, then generate a GUID for it.
		UMaterialExpressionParameter* ParameterExpression = Cast<UMaterialExpressionParameter>( NewExpression );

		if( ParameterExpression )
		{
			ParameterExpression->ConditionallyGenerateGUID(TRUE);
		}

		UMaterialExpressionTextureSampleParameter* TextureParameterExpression = Cast<UMaterialExpressionTextureSampleParameter>( NewExpression );
		if( TextureParameterExpression )
		{
			TextureParameterExpression->ConditionallyGenerateGUID(TRUE);
		}

		Material->AddExpressionParameter(NewExpression);
	}

	// Select the new node.
	EmptySelection();
	AddToSelection( NewExpression );

	Material->PreEditChange( NULL );
	Material->PostEditChange( NULL );
	Material->MarkPackageDirty();

	UpdatePropertyWindow();


	RefreshExpressionPreviews();
	RefreshExpressionViewport();
	RefreshPreviewViewport();
	bMaterialDirty = TRUE;
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxMaterialEditor::GetDockingParentName() const
{
	return TEXT("MaterialEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxMaterialEditor::GetDockingParentVersion() const
{
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Wx event handlers.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WxMaterialEditor::OnClose(wxCloseEvent& In)
{
	// @todo DB: Store off the viewport camera position/orientation to the material.
	//AnimTree->PreviewCamPos = PreviewVC->ViewLocation;
	//AnimTree->PreviewCamRot = PreviewVC->ViewRotation;

	if (bMaterialDirty)
	{
		// find out the user wants to do with this dirty material
		INT YesNoCancelReply = appMsgf(AMT_YesNoCancel, *LocalizeUnrealEd("Prompt_MaterialEditorClose"));
		// act on it
		switch (YesNoCancelReply)
		{
			case 0: // Yes
				// update material
				UpdateOriginalMaterial();
				break;

			case 1: // No
				// do nothing
				break;

			case 2: // Cancel
				// veto closing the window
				if (In.CanVeto())
				{
					In.Veto();
				}
				// don't call destroy!
				return;
		}
	}

	// Reset the transaction buffer so that references to the preview material are cleared.
	GEditor->ResetTransaction( TEXT("CloseMaterialEditor") );
	Destroy();
}

void WxMaterialEditor::OnNewMaterialExpression(wxCommandEvent& In)
{
	const INT NewNodeClassIndex = In.GetId() - IDM_NEW_SHADER_EXPRESSION_START;

	UClass* NewExpressionClass = NULL;
	if (bUseUnsortedMenus == TRUE)
	{
	check( NewNodeClassIndex >= 0 && NewNodeClassIndex < MaterialExpressionClasses.Num() );
		NewExpressionClass = MaterialExpressionClasses(NewNodeClassIndex);
	}
	else
	{
		INT NodeClassIndex = NewNodeClassIndex;
		INT FavoriteCount = FavoriteExpressionClasses.Num();
		if (NodeClassIndex < FavoriteCount)
		{
			// It's from the favorite expressions menu...
			NewExpressionClass = FavoriteExpressionClasses(NodeClassIndex);
		}
		else
		{
			INT Count = FavoriteCount;
			for (INT CheckIndex = 0; (CheckIndex < CategorizedMaterialExpressionClasses.Num()) && (NewExpressionClass == NULL); CheckIndex++)
			{
				FCategorizedMaterialExpressionNode& CategoryNode = CategorizedMaterialExpressionClasses(CheckIndex);
				if (NodeClassIndex < (Count + CategoryNode.MaterialExpressionClasses.Num()))
				{
					NodeClassIndex -= Count;
					NewExpressionClass = CategoryNode.MaterialExpressionClasses(NodeClassIndex);
				}
				Count += CategoryNode.MaterialExpressionClasses.Num();
			}

			if (NewExpressionClass == NULL)
			{
				// Assume it is in the unassigned list...
				NodeClassIndex -= Count;
				if (NodeClassIndex < UnassignedExpressionClasses.Num())
				{
					NewExpressionClass = UnassignedExpressionClasses(NodeClassIndex);
				}
			}
		}
	}

	if (NewExpressionClass)
	{
	const INT LocX = (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D;
	const INT LocY = (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D;
	CreateNewMaterialExpression( NewExpressionClass, TRUE, FIntPoint(LocX, LocY) );
}
}

void WxMaterialEditor::OnNewComment(wxCommandEvent& In)
{
	CreateNewCommentExpression();
}

void WxMaterialEditor::OnNewCompoundExpression(wxCommandEvent& In)
{
	CreateNewCompoundExpression();
}

void WxMaterialEditor::OnUseCurrentTexture(wxCommandEvent& In)
{
	// Set the currently selected texture in the generic browser
	// as the texture to use in all selected texture sample expressions.
	UTexture* SelectedTexture = GEditor->GetSelectedObjects()->GetTop<UTexture>();
	if ( SelectedTexture )
	{
		const FScopedTransaction Transaction( *LocalizeUnrealEd(TEXT("UseCurrentTexture")) );
		for( INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < SelectedExpressions.Num() ; ++MaterialExpressionIndex )
		{
			UMaterialExpression* MaterialExpression = SelectedExpressions( MaterialExpressionIndex );
			if ( MaterialExpression->IsA(UMaterialExpressionTextureSample::StaticClass()) )
			{
				UMaterialExpressionTextureSample* TextureSample = static_cast<UMaterialExpressionTextureSample*>(MaterialExpression);
				TextureSample->Modify();
				TextureSample->Texture = SelectedTexture;
			}
		}
		Material->PreEditChange( NULL );
		Material->PostEditChange( NULL );
		Material->MarkPackageDirty();
		UpdatePropertyWindow();
		RefreshExpressionPreviews();
		RefreshExpressionViewport();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

void WxMaterialEditor::OnDuplicateObjects(wxCommandEvent& In)
{
	DuplicateSelectedObjects();
}

void WxMaterialEditor::OnPasteHere(wxCommandEvent& In)
{
	const FIntPoint ClickLocation( (LinkedObjVC->NewX - LinkedObjVC->Origin2D.X)/LinkedObjVC->Zoom2D, (LinkedObjVC->NewY - LinkedObjVC->Origin2D.Y)/LinkedObjVC->Zoom2D );
	PasteExpressionsIntoMaterial( &ClickLocation );
}

void WxMaterialEditor::OnDeleteObjects(wxCommandEvent& In)
{
	DeleteSelectedObjects();
}

void WxMaterialEditor::OnBreakLink(wxCommandEvent& In)
{
	BreakConnLink();
}

void WxMaterialEditor::OnBreakAllLinks(wxCommandEvent& In)
{
	BreakAllConnectionsToSelectedExpressions();
}

void WxMaterialEditor::OnRemoveFromFavorites(wxCommandEvent& In)
{
	RemoveSelectedExpressionFromFavorites();
}

void WxMaterialEditor::OnAddToFavorites(wxCommandEvent& In)
{
	AddSelectedExpressionToFavorites();
}

void WxMaterialEditor::OnConnectToMaterial_DiffuseColor(wxCommandEvent& In) { OnConnectToMaterial(0); }
void WxMaterialEditor::OnConnectToMaterial_DiffusePower(wxCommandEvent& In) { OnConnectToMaterial(1); }
void WxMaterialEditor::OnConnectToMaterial_EmissiveColor(wxCommandEvent& In) { OnConnectToMaterial(2); }
void WxMaterialEditor::OnConnectToMaterial_SpecularColor(wxCommandEvent& In) { OnConnectToMaterial(3); }
void WxMaterialEditor::OnConnectToMaterial_SpecularPower(wxCommandEvent& In) { OnConnectToMaterial(4); }
void WxMaterialEditor::OnConnectToMaterial_Opacity(wxCommandEvent& In) { OnConnectToMaterial(5); }
void WxMaterialEditor::OnConnectToMaterial_OpacityMask(wxCommandEvent& In) { OnConnectToMaterial(6); }
void WxMaterialEditor::OnConnectToMaterial_Distortion(wxCommandEvent& In) { OnConnectToMaterial(7);	}
void WxMaterialEditor::OnConnectToMaterial_TransmissionMask(wxCommandEvent& In) { OnConnectToMaterial(8);	}
void WxMaterialEditor::OnConnectToMaterial_TransmissionColor(wxCommandEvent& In) { OnConnectToMaterial(9);	}
void WxMaterialEditor::OnConnectToMaterial_Normal(wxCommandEvent& In) { OnConnectToMaterial(10);	}
void WxMaterialEditor::OnConnectToMaterial_CustomLighting(wxCommandEvent& In) { OnConnectToMaterial(11);	}

void WxMaterialEditor::OnShowHideConnectors(wxCommandEvent& In)
{
	bHideUnusedConnectors = !bHideUnusedConnectors;
	RefreshExpressionViewport();
}

void WxMaterialEditor::OnToggleDrawCurves(wxCommandEvent& In)
{
	bDrawCurves = !bDrawCurves;
	RefreshExpressionViewport();
}

void WxMaterialEditor::OnRealTimeExpressions(wxCommandEvent& In)
{
	LinkedObjVC->ToggleRealtime();
}

void WxMaterialEditor::OnAlwaysRefreshAllPreviews(wxCommandEvent& In)
{
	bAlwaysRefreshAllPreviews = In.IsChecked() ? TRUE : FALSE;
	if ( bAlwaysRefreshAllPreviews )
	{
		RefreshExpressionPreviews();
	}
	RefreshExpressionViewport();
}

void WxMaterialEditor::OnApply(wxCommandEvent& In)
{
	UpdateOriginalMaterial();
}

void WxMaterialEditor::OnPropagateToFallback(wxCommandEvent& In)
{
	
	FString FallbackName = OriginalMaterial->GetName() + TEXT("_Fallback");
	
	if (bMaterialDirty)
	{
		UpdateOriginalMaterial();
	}

	if (OriginalMaterial->FallbackMaterial)
	{
		FallbackName = OriginalMaterial->FallbackMaterial->GetName();
	}

	OriginalMaterial->FallbackMaterial = (UMaterial*)UObject::StaticDuplicateObject(OriginalMaterial, OriginalMaterial, OriginalMaterial->GetOuter(), *FallbackName, ~0, OriginalMaterial->GetClass());
	check(OriginalMaterial->FallbackMaterial);
	OriginalMaterial->FallbackMaterial->bIsFallbackMaterial = TRUE;
	OriginalMaterial->FallbackMaterial->FallbackMaterial = NULL;

	OriginalMaterial->FallbackMaterial->PreEditChange(NULL);
	OriginalMaterial->FallbackMaterial->PostEditChange(NULL);
	OriginalMaterial->FallbackMaterial->MarkPackageDirty();
	
	WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	check(GenericBrowser);
	GenericBrowser->Update();
}

void WxMaterialEditor::OnRegenerateAutoFallback(wxCommandEvent& In)
{
	if (bMaterialDirty)
	{
		UpdateOriginalMaterial();
	}

	OriginalMaterial->CacheResourceShaders(GRHIShaderPlatform, FALSE, TRUE);
}

void WxMaterialEditor::OnOpenFallback(wxCommandEvent& In)
{
	check(OriginalMaterial->FallbackMaterial);
	wxFrame* NewMaterialEditor = new WxMaterialEditor( (wxWindow*)GApp->EditorFrame,-1,OriginalMaterial->FallbackMaterial );
	NewMaterialEditor->Show();
}

void WxMaterialEditor::UI_RealTimeExpressions(wxUpdateUIEvent& In)
{
	In.Check( LinkedObjVC->IsRealtime() == TRUE );
}

void WxMaterialEditor::UI_AlwaysRefreshAllPreviews(wxUpdateUIEvent& In)
{
	In.Check( bAlwaysRefreshAllPreviews == TRUE );
}

void WxMaterialEditor::UI_HideUnusedConnectors(wxUpdateUIEvent& In)
{
	In.Check( bHideUnusedConnectors == TRUE );
}

void WxMaterialEditor::UI_DrawCurves(wxUpdateUIEvent& In)
{
	In.Check( bDrawCurves == TRUE );
}

void WxMaterialEditor::UI_Apply(wxUpdateUIEvent& In)
{
	In.Enable(bMaterialDirty == TRUE);
}

void WxMaterialEditor::UI_PropagateToFallback(wxUpdateUIEvent& In)
{
	In.Enable(OriginalMaterial->bIsFallbackMaterial == FALSE);
}

void WxMaterialEditor::UI_RegenerateAutoFallback(wxUpdateUIEvent& In)
{
	In.Enable(OriginalMaterial->FallbackMaterial == NULL && !OriginalMaterial->bIsFallbackMaterial);
}

void WxMaterialEditor::UI_OpenFallback(wxUpdateUIEvent& In)
{
	In.Enable(OriginalMaterial->FallbackMaterial != NULL && !OriginalMaterial->bIsFallbackMaterial);
}

void WxMaterialEditor::OnCameraHome(wxCommandEvent& In)
{
	LinkedObjVC->Origin2D = FIntPoint(-Material->EditorX,-Material->EditorY);
	RefreshExpressionViewport();
}

void WxMaterialEditor::OnCleanUnusedExpressions(wxCommandEvent& In)
{
	CleanUnusedExpressions();
}

void WxMaterialEditor::OnSetPreviewMeshFromSelection(wxCommandEvent& In)
{
	UBOOL bFoundPreviewMesh = FALSE;

	// Look for a selected static mesh.
	UStaticMesh* SelectedStaticMesh = GEditor->GetSelectedObjects()->GetTop<UStaticMesh>();
	if ( SelectedStaticMesh )
	{
		SetPreviewMesh( SelectedStaticMesh, NULL );
		Material->PreviewMesh = SelectedStaticMesh->GetPathName();
		bFoundPreviewMesh = TRUE;
	}
	else
	{
		// No static meshes were selected; look for a selected skeletal mesh.
		USkeletalMesh* SelectedSkeletalMesh = GEditor->GetSelectedObjects()->GetTop<USkeletalMesh>();
		if ( SelectedSkeletalMesh && SetPreviewMesh( NULL, SelectedSkeletalMesh ) )
		{
			Material->PreviewMesh = SelectedSkeletalMesh->GetPathName();
			bFoundPreviewMesh = TRUE;
		}
	}

	if ( bFoundPreviewMesh )
	{
		Material->MarkPackageDirty();
		RefreshPreviewViewport();
		bMaterialDirty = TRUE;
	}
}

void WxMaterialEditor::OnMaterialExpressionListDrag(wxListEvent& In)
{
	const FString SelectedString( MaterialExpressionList->GetSelectedString() );
	if ( SelectedString.Len() > 0 )
	{
		wxTextDataObject DataObject( *SelectedString );
		wxDropSource DragSource( this );
		DragSource.SetData( DataObject );
		DragSource.DoDragDrop( TRUE );
	}
}

/**
 * Retrieves all visible parameters within the material.
 *
 * @param	Material			The material to retrieve the parameters from.
 * @param	MaterialInstance	The material instance that contains all parameter overrides.
 * @param	VisisbleExpressions	The array that will contain the id's of the visible parameter expressions.
 */
void WxMaterialEditor::GetVisibleMaterialParameters(const UMaterial *Material, UMaterialInstance *MaterialInstance, TArray<FGuid> &VisibleExpressions)
{
	VisibleExpressions.Empty();

	InitMaterialExpressionClasses();
	GetVisibleMaterialParametersFromExpression(Material->DiffuseColor.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->DiffusePower.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->SpecularColor.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->SpecularPower.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->Normal.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->EmissiveColor.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->Opacity.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->OpacityMask.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->Distortion.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->CustomLighting.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->TwoSidedLightingMask.Expression, MaterialInstance, VisibleExpressions);
	GetVisibleMaterialParametersFromExpression(Material->TwoSidedLightingColor.Expression, MaterialInstance, VisibleExpressions);
}

/**
 *	Checks if the given expression is in the favorites list...
 *
 *	@param	InExpression	The expression to check for.
 *
 *	@return	UBOOL			TRUE if it's in the list, FALSE if not.
 */
UBOOL WxMaterialEditor::IsMaterialExpressionInFavorites(UMaterialExpression* InExpression)
{
	for (INT CheckIndex = 0; CheckIndex < FavoriteExpressionClasses.Num(); CheckIndex++)
	{
		if (FavoriteExpressionClasses(CheckIndex) == InExpression->GetClass())
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Recursively walks the expression tree and parses the visible expression branches.
 *
 * @param	MaterialExpression	The expression to parse.
 * @param	MaterialInstance	The material instance that contains all parameter overrides.
 * @param	VisisbleExpressions	The array that will contain the id's of the visible parameter expressions.
 */
void WxMaterialEditor::GetVisibleMaterialParametersFromExpression(UMaterialExpression *MaterialExpression, UMaterialInstance *MaterialInstance, TArray<FGuid> &VisibleExpressions)
{
	if(!MaterialExpression)
	{
		return;
	}

	check(MaterialInstance);

	const TArray<UStructProperty*>& ExpressionInputs = WxMaterialEditor::GetMaterialExpressionInputs(MaterialExpression);

	// if it's a material parameter it must be visible so add it to the map
	UMaterialExpressionParameter *Param = Cast<UMaterialExpressionParameter>(MaterialExpression);
	UMaterialExpressionTextureSampleParameter *TexParam = Cast<UMaterialExpressionTextureSampleParameter>(MaterialExpression);
	UMaterialExpressionFontSampleParameter *FontParam = Cast<UMaterialExpressionFontSampleParameter>(MaterialExpression);
	if(Param)
	{
		VisibleExpressions.AddUniqueItem(Param->ExpressionGUID);
	}
	else if(TexParam)
	{
		VisibleExpressions.AddUniqueItem(TexParam->ExpressionGUID);
	}
	else if(FontParam)
	{
		VisibleExpressions.AddUniqueItem(FontParam->ExpressionGUID);
	}

	// check if it's a switch expression and branch according to its value
	UMaterialExpressionStaticSwitchParameter *Switch = Cast<UMaterialExpressionStaticSwitchParameter>(MaterialExpression);
	if(Switch)
	{
		UBOOL Value = FALSE;
		FGuid ExpressionID;
		MaterialInstance->GetStaticSwitchParameterValue(Switch->ParameterName, Value, ExpressionID);

		if(Value)
		{
			GetVisibleMaterialParametersFromExpression(Switch->A.Expression, MaterialInstance, VisibleExpressions);
		}
		else
		{
			GetVisibleMaterialParametersFromExpression(Switch->B.Expression, MaterialInstance, VisibleExpressions);
		}
	}
	else
	{
		for(INT ExpressionInputIndex = 0; ExpressionInputIndex < ExpressionInputs.Num(); ExpressionInputIndex++)
		{
			//retrieve the expression input and then start parsing its children
			UStructProperty* ExpressionInput = ExpressionInputs(ExpressionInputIndex);
			FExpressionInput* Input = (FExpressionInput*)((BYTE*)MaterialExpression + ExpressionInput->Offset);
			GetVisibleMaterialParametersFromExpression(Input->Expression, MaterialInstance, VisibleExpressions);
		}
	}
}
