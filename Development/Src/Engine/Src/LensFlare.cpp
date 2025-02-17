/**
 *	LensFlare.cpp: LensFlare implementations.
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "LensFlare.h"

IMPLEMENT_CLASS(ULensFlare);
IMPLEMENT_CLASS(ULensFlareComponent);
IMPLEMENT_CLASS(ALensFlareSource);

/**
 *	LensFlare class
 */
/**
 *	Retrieve the curve objects associated with this element.
 *
 *	@param	OutCurves	The array to fill in with the curve pairs.
 */
void FLensFlareElement::GetCurveObjects(TArray<FLensFlareElementCurvePair>& OutCurves)
{
	FLensFlareElementCurvePair* NewCurve;

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = LFMaterialIndex.Distribution;
	NewCurve->CurveName = FString(TEXT("LFMaterialIndex"));

    NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = Scaling.Distribution;
	NewCurve->CurveName = FString(TEXT("Scaling"));
	
	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = AxisScaling.Distribution;
	NewCurve->CurveName = FString(TEXT("AxisScaling"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = Rotation.Distribution;
	NewCurve->CurveName = FString(TEXT("Rotation"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = Color.Distribution;
	NewCurve->CurveName = FString(TEXT("Color"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = Alpha.Distribution;
	NewCurve->CurveName = FString(TEXT("Alpha"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = Offset.Distribution;
	NewCurve->CurveName = FString(TEXT("Offset"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = DistMap_Scale.Distribution;
	NewCurve->CurveName = FString(TEXT("DistMap_Scale"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = DistMap_Color.Distribution;
	NewCurve->CurveName = FString(TEXT("DistMap_Color"));

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = DistMap_Alpha.Distribution;
	NewCurve->CurveName = FString(TEXT("DistMap_Alpha"));
}

void FLensFlareElement::DuplicateDistribution_Float(const FRawDistributionFloat& SourceDist, UObject* Outer, FRawDistributionFloat& NewDist)
{
	NewDist = SourceDist;
	if (SourceDist.Distribution != NULL)
	{
		NewDist.Distribution = Cast<UDistributionFloat>(UObject::StaticDuplicateObject(
			SourceDist.Distribution, SourceDist.Distribution, Outer, TEXT("None")));
		NewDist.Distribution->bIsDirty = TRUE;
	}
}

void FLensFlareElement::DuplicateDistribution_Vector(const FRawDistributionVector& SourceDist, UObject* Outer, FRawDistributionVector& NewDist)
{
	NewDist = SourceDist;
	if (SourceDist.Distribution != NULL)
	{
		NewDist.Distribution = Cast<UDistributionVector>(UObject::StaticDuplicateObject(
			SourceDist.Distribution, SourceDist.Distribution, Outer, TEXT("None")));
		NewDist.Distribution->bIsDirty = TRUE;
	}
}

UBOOL FLensFlareElement::DuplicateFromSource(const FLensFlareElement& InSource, UObject* Outer)
{
	ElementName = InSource.ElementName;
	RayDistance =  InSource.RayDistance;

	bIsEnabled = InSource.bIsEnabled;
	bUseSourceDistance = InSource.bUseSourceDistance;
	bNormalizeRadialDistance = InSource.bNormalizeRadialDistance;
	bModulateColorBySource = InSource.bModulateColorBySource;
	Size = InSource.Size;

	LFMaterials.Empty();
	for (INT MatIndex = 0; MatIndex < InSource.LFMaterials.Num(); MatIndex++)
	{
		LFMaterials.AddItem(InSource.LFMaterials(MatIndex));
	}

	// We need to duplicate each distribution...
	DuplicateDistribution_Float(InSource.LFMaterialIndex, Outer, LFMaterialIndex);
	DuplicateDistribution_Float(InSource.Scaling, Outer, Scaling);
	DuplicateDistribution_Vector(InSource.AxisScaling, Outer, AxisScaling);
	DuplicateDistribution_Float(InSource.Rotation, Outer, Rotation);
	DuplicateDistribution_Vector(InSource.Color, Outer, Color);
	DuplicateDistribution_Float(InSource.Alpha, Outer, Alpha);
	DuplicateDistribution_Vector(InSource.Offset, Outer, Offset);
	DuplicateDistribution_Vector(InSource.DistMap_Scale, Outer, DistMap_Scale);
	DuplicateDistribution_Vector(InSource.DistMap_Color, Outer, DistMap_Color);
	DuplicateDistribution_Float(InSource.DistMap_Alpha, Outer, DistMap_Alpha);

	return TRUE;
}

/** 
 *	Retrieve the curve of the given name...
 *
 *	@param	CurveName		The name of the curve to retrieve.
 *
 *	@return	UObject*		The curve, if found; NULL if not.
 */
UObject* FLensFlareElement::GetCurve(FString& CurveName)
{
	if (CurveName != TEXT("ScreenPercentageMap"))
	{
		TArray<FLensFlareElementCurvePair> OutCurves;
		GetCurveObjects(OutCurves);

		for (INT CurveIndex = 0; CurveIndex < OutCurves.Num(); CurveIndex++)
		{
			if (OutCurves(CurveIndex).CurveName == CurveName)
			{
				// We found it... return it
				return OutCurves(CurveIndex).CurveObject;
			}
		}
	}
	return NULL;
}

// UObject interface.
/**
 *	Called when a property is about to change in the property window.
 *
 *	@param	PropertyAboutToChange	The property that is about to be modified.
 */
void ULensFlare::PreEditChange(UProperty* PropertyAboutToChange)
{
	ReflectionCount = Reflections.Num();
}

/**
 *	Called when a property has been changed in the property window.
 *
 *	@param	PropertyThatChanged		The property that was modified.
 */
void ULensFlare::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if (PropertyThatChanged)
	{
		if (appStrstr(*(PropertyThatChanged->GetName()), TEXT("Reflections")) != NULL)
		{
			INT NewReflectionCount = Reflections.Num();
			if (NewReflectionCount > ReflectionCount)
			{
				// Initialize the new element...
				// It's not guaranteed that the element is at the end though, so find the 'empty' element
				for (INT ElementIndex = 0; ElementIndex < Reflections.Num(); ElementIndex++)
				{
					InitializeElement(ElementIndex);
				}
			}
		}
		else
		if (appStrstr(*(PropertyThatChanged->GetName()), TEXT("RayDistance")) != NULL)
		{
			// You are NOT allowed to change the ray distance of the source
			if (1)
			{

			}
		}

		MarkPackageDirty();
	}

	for (TObjectIterator<ALensFlareSource> It;It;++It)
	{
		if (It->LensFlareComp && (It->LensFlareComp->Template == this))
		{
			UBOOL bSavedStatic = It->bStatic;
			It->bStatic = TRUE;
			It->ForceUpdateComponents();
			It->bStatic = bSavedStatic;
		}
	}
}

/**
 *	Called when an object has been loaded...
 */
void ULensFlare::PostLoad()
{
	Super::PostLoad();

	ULinkerLoad* LFLinkerLoad = GetLinker();
	if (LFLinkerLoad && (LFLinkerLoad->Ver() < VER_LENSFLARE_SCREENPERCENTAGEMAP_FIXUP))
	{
		// Convert the ScreenPercentageMap to a linear 0..1,0..1 mapping
		UDistributionFloatConstantCurve* NewCurve = ConstructObject<UDistributionFloatConstantCurve>(UDistributionFloatConstantCurve::StaticClass(), this);
		check(NewCurve);
		INT KeyIndex;
		KeyIndex = NewCurve->CreateNewKey(0.0f);
		NewCurve->SetKeyOut(0, KeyIndex, 0.0f);
		NewCurve->SetKeyInterpMode(KeyIndex, CIM_Linear);
		KeyIndex = NewCurve->CreateNewKey(1.0f);
		NewCurve->SetKeyOut(0, KeyIndex, 1.0f);
		NewCurve->SetKeyInterpMode(KeyIndex, CIM_Linear);

		ScreenPercentageMap.Distribution = NewCurve;
	}
}

// CurveEditor helper interface
/**
 *	Add the curves for the given element to the curve editor.
 *
 *	@param	ElementIndex	The index of the element whose curves should be sent.
 *							-1 indicates the source element.
 *	@param	EdSetup			The curve ed setup for the object.
 */
void ULensFlare::AddElementCurvesToEditor(INT ElementIndex, UInterpCurveEdSetup* EdSetup)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if (LFElement == NULL)
	{
		return;
	}

	TArray<FLensFlareElementCurvePair> OutCurves;

	LFElement->GetCurveObjects(OutCurves);

	for (INT CurveIndex = 0; CurveIndex < OutCurves.Num(); CurveIndex++)
	{
		UObject* Distribution = OutCurves(CurveIndex).CurveObject;
		if (Distribution)
		{
			EdSetup->AddCurveToCurrentTab(Distribution, OutCurves(CurveIndex).CurveName, FColor(255,0,0), TRUE, TRUE);
		}
	}
}

/**
 *	Removes the curves for the given element from the curve editor.
 *
 *	@param	ElementIndex	The index of the element whose curves should be sent.
 *							-1 indicates the source element.
 *	@param	EdSetup			The curve ed setup for the object.
 */
void ULensFlare::RemoveElementCurvesFromEditor(INT ElementIndex, UInterpCurveEdSetup* EdSetup)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if (LFElement == NULL)
	{
		return;
	}

	TArray<FLensFlareElementCurvePair> OutCurves;

	LFElement->GetCurveObjects(OutCurves);

	for (INT CurveIndex = 0; CurveIndex < OutCurves.Num(); CurveIndex++)
	{
		UObject* Distribution = OutCurves(CurveIndex).CurveObject;
		if (Distribution)
		{
			EdSetup->RemoveCurve(Distribution);
		}
	}
}

/**
 *	Add the curve of the given name from the given element to the curve editor.
 *
 *	@param	ElementIndex	The index of the element of interest.
 *							-1 indicates the source element.
 *	@param	CurveName		The name of the curve to add
 *	@param	EdSetup			The curve ed setup for the object.
 */
void ULensFlare::AddElementCurveToEditor(INT ElementIndex, FString& CurveName, UInterpCurveEdSetup* EdSetup)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if ((LFElement == NULL) && (CurveName != TEXT("ScreenPercentageMap")))
	{
		return;
	}

	TArray<FLensFlareElementCurvePair> OutCurves;

	if (CurveName == TEXT("ScreenPercentageMap"))
	{
		GetCurveObjects(OutCurves);
	}
	else
	{
		LFElement->GetCurveObjects(OutCurves);
	}

	for (INT CurveIndex = 0; CurveIndex < OutCurves.Num(); CurveIndex++)
	{
		if (OutCurves(CurveIndex).CurveName == CurveName)
		{
			// We found it... send it to the curve editor
			UObject* Distribution = OutCurves(CurveIndex).CurveObject;
			if (Distribution)
			{
				EdSetup->AddCurveToCurrentTab(Distribution, OutCurves(CurveIndex).CurveName, FColor(255,0,0), TRUE, TRUE);
			}
		}
	}
}

/**
 *	Retrieve the curve of the given name from the given element.
 *
 *	@param	ElementIndex	The index of the element of interest.
 *							-1 indicates the source element.
 *	@param	CurveName		The name of the curve to add
 *	@param	EdSetup			The curve ed setup for the object.
 */
UObject* ULensFlare::GetElementCurve(INT ElementIndex, FString& CurveName)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if ((LFElement == NULL) && (CurveName != TEXT("ScreenPercentageMap")))
	{
		return NULL;
	}

	TArray<FLensFlareElementCurvePair> OutCurves;

	if (CurveName == TEXT("ScreenPercentageMap"))
	{
		GetCurveObjects(OutCurves);
	}
	else
	{
		LFElement->GetCurveObjects(OutCurves);
	}

	for (INT CurveIndex = 0; CurveIndex < OutCurves.Num(); CurveIndex++)
	{
		if (OutCurves(CurveIndex).CurveName == CurveName)
		{
			// We found it... return it
			return OutCurves(CurveIndex).CurveObject;
		}
	}

	return NULL;
}

/**
 *	Retrieve the element at the given index
 *
 *	@param	ElementIndex		The index of the element of interest.
 *								-1 indicates the Source element.
 *	
 *	@return	FLensFlareElement*	Pointer to the element if found.
 *								Otherwise, NULL.
 */
const FLensFlareElement* ULensFlare::GetElement(INT ElementIndex) const
{
	if (ElementIndex == -1)
	{
		return &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		return &Reflections(ElementIndex);
	}

	return NULL;
}

/**
 *	Set the given element to the given enabled state.
 *
 *	@param	ElementIndex	The index of the element of interest.
 *							-1 indicates the Source element.
 *	@param	bInIsEnabled	The enabled state to set it to.
 *
 *	@return UBOOL			TRUE if element was found and bIsEnabled was set to given value.
 */
UBOOL ULensFlare::SetElementEnabled(INT ElementIndex, UBOOL bInIsEnabled)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if (LFElement == NULL)
	{
		return FALSE;
	}

	LFElement->bIsEnabled = bInIsEnabled;
	return TRUE;
}

/** 
 *	Initialize the element at the given index
 *
 *	@param	ElementIndex	The index of the element of interest.
 *							-1 indicates the Source element.
 *
 *	@return	UBOOL			TRUE if successful. FALSE otherwise.
 */
UBOOL ULensFlare::InitializeElement(INT ElementIndex)
{
	FLensFlareElement* LFElement = NULL;
	if (ElementIndex == -1)
	{
		LFElement = &SourceElement;
	}
	else
	if ((ElementIndex >= 0) && (ElementIndex < Reflections.Num()))
	{
		LFElement = &(Reflections(ElementIndex));
	}

	if (LFElement == NULL)
	{
		return FALSE;
	}

	LFElement->bNormalizeRadialDistance = TRUE;

	LFElement->bIsEnabled = TRUE;

	/** Size */
	LFElement->Size = FVector(0.2f, 0.2f, 0.0f);

	UDistributionFloatConstant* DFConst;
	UDistributionVectorConstant* DVConst;

	/** Index of the material to use from the LFMaterial array. */
	LFElement->LFMaterialIndex.Distribution = ConstructObject<UDistributionFloatConstant>(UDistributionFloatConstant::StaticClass(), this);
	DFConst = CastChecked<UDistributionFloatConstant>(LFElement->LFMaterialIndex.Distribution);
	DFConst->Constant = 0.0f;
		 
	/**	Global scaling.	 */
	LFElement->Scaling.Distribution = ConstructObject<UDistributionFloatConstant>(UDistributionFloatConstant::StaticClass(), this);
	DFConst = CastChecked<UDistributionFloatConstant>(LFElement->Scaling.Distribution);
	DFConst->Constant = 1.0f;
	
	/**	Anamorphic scaling.	*/
	LFElement->AxisScaling.Distribution = ConstructObject<UDistributionVectorConstant>(UDistributionVectorConstant::StaticClass(), this);
	DVConst = CastChecked<UDistributionVectorConstant>(LFElement->AxisScaling.Distribution);
	DVConst->Constant = FVector(1.0f);
	
	/** Rotation. */
	LFElement->Rotation.Distribution = ConstructObject<UDistributionFloatConstant>(UDistributionFloatConstant::StaticClass(), this);
	DFConst = CastChecked<UDistributionFloatConstant>(LFElement->Rotation.Distribution);
	DFConst->Constant = 0.0f;

	/** Color. */
	LFElement->Color.Distribution = ConstructObject<UDistributionVectorConstant>(UDistributionVectorConstant::StaticClass(), this);
	DVConst = CastChecked<UDistributionVectorConstant>(LFElement->Color.Distribution);
	DVConst->Constant = FVector(1.0f);
	LFElement->Alpha.Distribution = ConstructObject<UDistributionFloatConstant>(UDistributionFloatConstant::StaticClass(), this);
	DFConst = CastChecked<UDistributionFloatConstant>(LFElement->Alpha.Distribution);
	DFConst->Constant = 1.0f;

	/** Offset. */
	LFElement->Offset.Distribution = ConstructObject<UDistributionVectorConstant>(UDistributionVectorConstant::StaticClass(), this);
	DVConst = CastChecked<UDistributionVectorConstant>(LFElement->Offset.Distribution);
	DVConst->Constant = FVector(0.0f);

	/** Distance to camera scaling. */
	LFElement->DistMap_Scale.Distribution = ConstructObject<UDistributionVectorConstant>(UDistributionVectorConstant::StaticClass(), this);
	DVConst = CastChecked<UDistributionVectorConstant>(LFElement->DistMap_Scale.Distribution);
	DVConst->Constant = FVector(1.0f);
	LFElement->DistMap_Color.Distribution = ConstructObject<UDistributionVectorConstant>(UDistributionVectorConstant::StaticClass(), this);
	DVConst = CastChecked<UDistributionVectorConstant>(LFElement->DistMap_Color.Distribution);
	DVConst->Constant = FVector(1.0f);
	LFElement->DistMap_Alpha.Distribution = ConstructObject<UDistributionFloatConstant>(UDistributionFloatConstant::StaticClass(), this);
	DFConst = CastChecked<UDistributionFloatConstant>(LFElement->DistMap_Alpha.Distribution);
	DFConst->Constant = 1.0f;

	return TRUE;
}

/** Get the curve objects associated with the LensFlare itself */
void ULensFlare::GetCurveObjects(TArray<FLensFlareElementCurvePair>& OutCurves)
{
	FLensFlareElementCurvePair* NewCurve;

	NewCurve = new(OutCurves)FLensFlareElementCurvePair;
	check(NewCurve);
	NewCurve->CurveObject = ScreenPercentageMap.Distribution;
	NewCurve->CurveName = FString(TEXT("ScreenPercentageMap"));
}

/**
 *	LensFlareComponent class
 */
void ULensFlareComponent::SetSourceColor(FLinearColor InSourceColor)
{
	if (SourceColor != InSourceColor)
	{
		SourceColor = InSourceColor;
		// Need to detach/re-attach
		BeginDeferredReattach();
	}
}

void ULensFlareComponent::SetTemplate(class ULensFlare* NewTemplate)
{
	if (NewTemplate != Template)
	{
		Template = NewTemplate;
		if (Template)
		{
			OuterCone = Template->OuterCone;
			InnerCone = Template->InnerCone;
			ConeFudgeFactor = Template->ConeFudgeFactor;
			Radius = Template->Radius;
			if (bAutoActivate == TRUE)
			{
				bIsActive = TRUE;
			}
		}
		// Need to detach/re-attach
		BeginDeferredReattach();
	}
}

void ULensFlareComponent::SetIsActive(UBOOL bInIsActive)
{
	if (bIsActive != bInIsActive)
	{
		bIsActive = bInIsActive;
		FLensFlareSceneProxy* SceneProxy = (FLensFlareSceneProxy*)Scene_GetProxyFromInfo(SceneInfo);
		if (SceneProxy)
		{
			SceneProxy->SetIsActive(bIsActive);
		}
	}
}

// UObject interface
void ULensFlareComponent::BeginDestroy()
{
	Super::BeginDestroy();

	ReleaseResourcesFence = new FRenderCommandFence();
	check(ReleaseResourcesFence);
	ReleaseResourcesFence->BeginFence();
}

UBOOL ULensFlareComponent::IsReadyForFinishDestroy()
{
	UBOOL bSuperIsReady = Super::IsReadyForFinishDestroy();

	UBOOL bIsReady = TRUE;
	if (ReleaseResourcesFence)
	{
		bIsReady = (ReleaseResourcesFence->GetNumPendingFences() == 0);
	}
	return (bSuperIsReady && bIsReady);
}

void ULensFlareComponent::FinishDestroy()
{
	if (ReleaseResourcesFence)
	{
		delete ReleaseResourcesFence;
		ReleaseResourcesFence = FALSE;
	}
	Super::FinishDestroy();
}

void ULensFlareComponent::PreEditChange(UProperty* PropertyAboutToChange)
{
}

void ULensFlareComponent::PostEditChange(UProperty* PropertyThatChanged)
{
	BeginDeferredReattach();
}

void ULensFlareComponent::PostLoad()
{
	Super::PostLoad();
}

// UActorComponent interface.
void ULensFlareComponent::Attach()
{
	Super::Attach();

	if (GIsEditor)
	{
		if ( PreviewInnerCone )
		{
			PreviewInnerCone->ConeRadius = Radius;
			PreviewInnerCone->ConeAngle = InnerCone;
			PreviewInnerCone->Translation = Translation;
		}

		if ( PreviewOuterCone )
		{
			PreviewOuterCone->ConeRadius = Radius;
			PreviewOuterCone->ConeAngle = OuterCone;
			PreviewOuterCone->Translation = Translation;
		}

		if (PreviewRadius)
		{
			PreviewRadius->SphereRadius = Radius;
			PreviewRadius->Translation = Translation;
		}
	}
}

// UPrimitiveComponent interface
void ULensFlareComponent::UpdateBounds()
{
	if (Template && (Template->bUseFixedRelativeBoundingBox == TRUE))
	{
		FBox BoundingBox;
		BoundingBox.Init();

		// Use hardcoded relative bounding box from template.
		FVector BoundingBoxOrigin = LocalToWorld.GetOrigin();
		BoundingBox		 = Template->FixedRelativeBoundingBox;
		BoundingBox.Min += BoundingBoxOrigin;
		BoundingBox.Max += BoundingBoxOrigin;

		Bounds = FBoxSphereBounds(BoundingBox);
	}
	else
	{
		Super::UpdateBounds();
	}
}

/** Returns true if the prim is using a material with unlit distortion */
UBOOL ULensFlareComponent::HasUnlitDistortion() const
{
	if (Template == NULL)
	{
		return FALSE;
	}

	UBOOL bHasDistortion= FALSE;
	INT MaterialIndex;
	UMaterialInterface* MaterialInterface;
	UMaterial* Material;

	// Check the source element
	if (Template->SourceElement.bIsEnabled == TRUE)
	{
		for (MaterialIndex = 0; MaterialIndex < Template->SourceElement.LFMaterials.Num(); MaterialIndex++)
		{
			MaterialInterface = Template->SourceElement.LFMaterials(MaterialIndex);
			if (MaterialInterface)
			{
				Material = MaterialInterface->GetMaterial();
				if (Material)
				{
					if (Material->HasDistortion() == TRUE)
					{
						bHasDistortion = TRUE;
						break;
					}
				}
			}
		}
	}

	if (bHasDistortion == FALSE)
	{
		for (INT ElementIndex = 0; ElementIndex < Template->Reflections.Num(); ElementIndex++)
		{
			const FLensFlareElement& Element = Template->Reflections(ElementIndex);
			if (Element.bIsEnabled == TRUE)
			{
				for (MaterialIndex = 0; MaterialIndex < Element.LFMaterials.Num(); MaterialIndex++)
				{
					MaterialInterface = Element.LFMaterials(MaterialIndex);
					if (MaterialInterface)
					{
						Material = MaterialInterface->GetMaterial();
						if (Material)
						{
							if (Material->HasDistortion() == TRUE)
							{
								bHasDistortion = TRUE;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	return bHasDistortion;
}

/** Returns true if the prim is using a material with unlit translucency */
UBOOL ULensFlareComponent::HasUnlitTranslucency() const
{
	if (Template == NULL)
	{
		return FALSE;
	}

	UBOOL bHasTranslucency = FALSE;
	INT MaterialIndex;
	UMaterialInterface* MaterialInterface;
	UMaterial* Material;

	// Check the source element
	if (Template->SourceElement.bIsEnabled == TRUE)
	{
		for (MaterialIndex = 0; MaterialIndex < Template->SourceElement.LFMaterials.Num(); MaterialIndex++)
		{
			MaterialInterface = Template->SourceElement.LFMaterials(MaterialIndex);
			if (MaterialInterface)
			{
				Material = MaterialInterface->GetMaterial();
				if (Material)
				{
					if (Material->LightingModel == MLM_Unlit)
					{
						// check for unlit translucency
						if (IsTranslucentBlendMode((EBlendMode)Material->BlendMode))
						{
							bHasTranslucency = TRUE;
							break;
						}
					}
					else
					if (Material->LightingModel == MLM_Phong)
					{
						if (IsTranslucentBlendMode((EBlendMode)Material->BlendMode))
						{
							bHasTranslucency = TRUE;
							break;
						}
					}
				}
			}
		}
	}

	if (bHasTranslucency == FALSE)
	{
		for (INT ElementIndex = 0; ElementIndex < Template->Reflections.Num(); ElementIndex++)
		{
			const FLensFlareElement& Element = Template->Reflections(ElementIndex);
			if (Element.bIsEnabled == TRUE)
			{
				for (MaterialIndex = 0; MaterialIndex < Element.LFMaterials.Num(); MaterialIndex++)
				{
					MaterialInterface = Element.LFMaterials(MaterialIndex);
					if (MaterialInterface)
					{
						Material = MaterialInterface->GetMaterial();
						if (Material)
						{
							if (Material->LightingModel == MLM_Unlit)
							{
								// check for unlit translucency
								if (IsTranslucentBlendMode((EBlendMode)Material->BlendMode))
								{
									bHasTranslucency = TRUE;
									break;
								}
							}
							else
							if (Material->LightingModel == MLM_Phong)
							{
								if (IsTranslucentBlendMode((EBlendMode)Material->BlendMode))
								{
									bHasTranslucency = TRUE;
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	
	return bHasTranslucency;
}

/**
 * Returns true if the prim is using a material that samples the scene color texture.
 * If true then these primitives are drawn after all other translucency
 */
UBOOL ULensFlareComponent::UsesSceneColor() const
{
	if (Template == NULL)
	{
		return FALSE;
	}

	UBOOL bUsesSceneColor = FALSE;
	INT MaterialIndex;
	UMaterialInterface* MaterialInterface;
	UMaterial* Material;

	// Check the source element
	if (Template->SourceElement.bIsEnabled == TRUE)
	{
		for (MaterialIndex = 0; MaterialIndex < Template->SourceElement.LFMaterials.Num(); MaterialIndex++)
		{
			MaterialInterface = Template->SourceElement.LFMaterials(MaterialIndex);
			if (MaterialInterface)
			{
				Material = MaterialInterface->GetMaterial();
				if (Material)
				{
					if (Material->UsesSceneColor() == TRUE)
					{
						bUsesSceneColor = TRUE;
						break;
					}
				}
			}
		}
	}

	if (bUsesSceneColor == FALSE)
	{
		for (INT ElementIndex = 0; ElementIndex < Template->Reflections.Num(); ElementIndex++)
		{
			const FLensFlareElement& Element = Template->Reflections(ElementIndex);
			if (Element.bIsEnabled == TRUE)
			{
				for (MaterialIndex = 0; MaterialIndex < Element.LFMaterials.Num(); MaterialIndex++)
				{
					MaterialInterface = Element.LFMaterials(MaterialIndex);
					if (MaterialInterface)
					{
						Material = MaterialInterface->GetMaterial();
						if (Material)
						{
							if (Material->UsesSceneColor() == TRUE)
							{
								bUsesSceneColor = TRUE;
								break;
							}
						}
					}
				}
			}
		}
	}
	
	return bUsesSceneColor;
}

FPrimitiveSceneProxy* ULensFlareComponent::CreateSceneProxy()
{
	if (Template)
	{
		DepthPriorityGroup = Template->ReflectionsDPG;
		OuterCone = Template->OuterCone;
		InnerCone = Template->InnerCone;
		ConeFudgeFactor = Template->ConeFudgeFactor;
		Radius = Template->Radius;
		if (bAutoActivate == TRUE)
		{
			bIsActive = TRUE;
		}
	}

	if (GSystemSettings.bAllowLensFlares && (DetailMode <= GSystemSettings.DetailMode))
	{
		FLensFlareSceneProxy* LFSceneProxy = new FLensFlareSceneProxy(this);
		check(LFSceneProxy);
		return LFSceneProxy;
	}

	return NULL;
}

// InstanceParameters interface
void ULensFlareComponent::AutoPopulateInstanceProperties()
{
}

/**
 *	LensFlareSource class
 */
/**
 *	Set the template of the LensFlareComponent.
 *
 *	@param	NewTemplate		The new template to use for the lens flare.
 */
void ALensFlareSource::SetTemplate(class ULensFlare* NewTemplate)
{
	if (LensFlareComp)
	{
		FComponentReattachContext ReattachContext(LensFlareComp);
		LensFlareComp->SetTemplate(NewTemplate);
	}
}

void ALensFlareSource::AutoPopulateInstanceProperties()
{
}

// AActor interface.
/**
 * Function that gets called from within Map_Check to allow this actor to check itself
 * for any potential errors and register them with map check dialog.
 */
void ALensFlareSource::CheckForErrors()
{
	Super::CheckForErrors();

	// LensFlareSources placed in a level should have a non-NULL LensFlareComponent.
	UObject* Outer = GetOuter();
	if (Cast<ULevel>(Outer))
	{
		if (LensFlareComp == NULL)
		{
			GWarn->MapCheck_Add(MCTYPE_WARNING, this, 
				*FString::Printf(TEXT("%s : LensFlareSource actor has NULL LensFlareComponent property - please delete!"), 
				*GetName() ), MCACTION_DELETE, TEXT("LensFlareComponentNull") );
		}
	}
}
