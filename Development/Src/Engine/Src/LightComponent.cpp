/*=============================================================================
	LightComponent.cpp: LightComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(ULightComponent);
IMPLEMENT_CLASS(ULightFunction);

/**
 * Updates/ resets light GUIDs.
 */
void ULightComponent::UpdateLightGUIDs()
{
	LightGuid		= appCreateGuid();
	LightmapGuid	= appCreateGuid();
	MarkLightingRequiringRebuild();
}

/**
 * Validates light GUIDs and resets as appropriate.
 */
void ULightComponent::ValidateLightGUIDs()
{
	// Validate light guids.
	if( !LightGuid.IsValid() )
	{
		LightGuid = appCreateGuid();
	}
	if( !LightmapGuid.IsValid() )
	{
		LightmapGuid = appCreateGuid();
	}
}

UBOOL ULightComponent::AffectsPrimitive(const UPrimitiveComponent* Primitive) const
{
	ULightEnvironmentComponent* PrimitiveLightEnvironment = Primitive->LightEnvironment;
	if(PrimitiveLightEnvironment && !PrimitiveLightEnvironment->IsEnabled())
	{
		PrimitiveLightEnvironment = NULL;
	}
	if(PrimitiveLightEnvironment != LightEnvironment)
	{
		return FALSE;
	}

	if( !LightingChannels.OverlapsWith( Primitive->LightingChannels ) )
	{
		return FALSE;
	}

	if(!Primitive->bAcceptsLights)
	{
		return FALSE;
	}

	if( !Primitive->bAcceptsDynamicLights && !HasStaticShadowing() )
	{
		return FALSE;
	}

	// Check whether the light affects the primitive's bounding volume and level.
	return AffectsBounds(Primitive->Bounds) && AffectsLevel(Primitive->GetOutermost());
}

UBOOL ULightComponent::AffectsBounds(const FBoxSphereBounds& Bounds) const
{
	// Use inclusion/ exclusion volumes to determine whether primitive is affected by light.
	if( bUseVolumes )
	{
		// Exclusion volumes have higher priority. Return FALSE if primitive intersects or is
		// encompassed by any exclusion volume.
		for( INT VolumeIndex=0; VolumeIndex<ExclusionConvexVolumes.Num(); VolumeIndex++ )
		{
			const FConvexVolume& ExclusionConvexVolume	= ExclusionConvexVolumes(VolumeIndex);
			if( ExclusionConvexVolume.IntersectBox( Bounds.Origin, Bounds.BoxExtent ) )
			{
				return FALSE;	
			}
		}

		// Check whether primitive intersects or is encompassed by any inclusion volume.
		UBOOL bIntersectsInclusionVolume = FALSE;
		for( INT VolumeIndex=0; VolumeIndex<InclusionConvexVolumes.Num(); VolumeIndex++ )
		{
			const FConvexVolume& InclusionConvexVolume	= InclusionConvexVolumes(VolumeIndex);
			if( InclusionConvexVolume.IntersectBox( Bounds.Origin, Bounds.BoxExtent ) )
			{
				bIntersectsInclusionVolume = TRUE;
				break;
			}
		}
		// No interaction unless there is intersection or there are no inclusion volumes, in which case
		// we assume that the whole world is included unless explicitly excluded.
		if( !bIntersectsInclusionVolume && InclusionConvexVolumes.Num() )
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL ULightComponent::AffectsLevel(const UPackage* LevelPackage) const
{
	check(LevelPackage);

	// Cached outermost used by level/ level light/ primitive interaction code.
	UPackage* LightOutermost = GetOutermost();

	// Check whether the package the level resides in is set to have self contained lighting.
	if( (LevelPackage->PackageFlags & PKG_SelfContainedLighting) && HasStaticShadowing() )
	{
		// Only allow interaction in the case of self contained lighting if the light is in the level.
		if( LightOutermost != LevelPackage )
		{
			return FALSE;
		}
	}

	if( bOnlyAffectSameAndSpecifiedLevels )
	{
		// Retrieve the FName of the light's and primtive's level.
		FName LightLevelName			= LightOutermost->GetFName();
		FName LevelName					= LevelPackage->GetFName();
		UBOOL bShouldAffectPrimitive	= FALSE;

		// Check whether the light's level matches the primitives.
		if( LightLevelName == LevelName )
		{
			bShouldAffectPrimitive = TRUE;
		}
		// Check whether the primitve's level is in the OtherLevelsToAffect array.
		else
		{
			// Iterate over all levels and look for match.
			for( INT LevelIndex=0; LevelIndex<OtherLevelsToAffect.Num(); LevelIndex++ )
			{
				const FName& OtherLevelName = OtherLevelsToAffect(LevelIndex);
				if( OtherLevelName == LevelName )
				{
					// Match found, early out.
					bShouldAffectPrimitive = TRUE;
					break;
				}
			}
		}

		// Light should not affect primitive, early out.
		if( !bShouldAffectPrimitive )
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL ULightComponent::IsShadowCast(UPrimitiveComponent* Primitive) const
{
	if(Primitive->HasStaticShadowing())
	{
		return CastShadows && CastStaticShadows;
	}
	else
	{
		return CastShadows && CastDynamicShadows;
	}
}

UBOOL ULightComponent::HasStaticShadowing() const
{
	// Skylights don't support non-light-mapped static shadowing.
	const UBOOL bSupportsStaticShadowing =
		IsA(USkyLightComponent::StaticClass()) ?
			HasStaticLighting() :
			TRUE;

	return (!Owner || Owner->HasStaticShadowing()) && !bForceDynamicLight && !LightEnvironment && bSupportsStaticShadowing;
}

UBOOL ULightComponent::HasProjectedShadowing() const
{
	return (!Owner || Owner->HasStaticShadowing()) && !bForceDynamicLight;
}

UBOOL ULightComponent::HasStaticLighting() const
{
	return (!Owner || Owner->bStatic) && !Function && !bForceDynamicLight && !LightEnvironment;
}

/**
 * Returns whether static lighting, aka lightmaps, is being used for primitive/ light
 * interaction.
 *
 * @param bForceDirectLightMap	Whether primitive is set to force lightmaps
 * @return TRUE if lightmaps/ static lighting is being used, FALSE otherwise
 */
UBOOL ULightComponent::UseStaticLighting( UBOOL bForceDirectLightMap ) const
{
	return HasStaticLighting() && (UseDirectLightMap || bForceDirectLightMap || !HasStaticShadowing());
}

//
//	ULightComponent::SetParentToWorld
//

void ULightComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	FMatrix OrthonormalParentToWorld = ParentToWorld;
	OrthonormalParentToWorld.RemoveScaling();

	const FVector LightOrigin = OrthonormalParentToWorld.InverseTransformFVectorNoScale(FVector4(0,0,0));

	static const FMatrix LightToParent(
		FPlane(+0,+0,+1,+0),
		FPlane(+0,+1,+0,+0),
		FPlane(+1,+0,+0,+0),
		FPlane(+0,+0,+0,+1)
		);
	static const FMatrix ParentToLight = LightToParent.Inverse();

	LightToWorld = LightToParent * OrthonormalParentToWorld;
	WorldToLight = FTranslationMatrix(LightOrigin) * OrthonormalParentToWorld.RemoveTranslation().Inverse() * ParentToLight;
}


UBOOL ULightComponent::IsLACDynamicAffecting()
{
	UBOOL Retval = FALSE;

	if(	   ( CastShadows				== TRUE )
		&& ( CastStaticShadows			== FALSE )
		&& ( CastDynamicShadows			== TRUE )
		&& ( bForceDynamicLight			== FALSE )
		&& ( UseDirectLightMap			== FALSE )

		&& ( LightingChannels.BSP		== FALSE )
		&& ( LightingChannels.Dynamic	== TRUE )
		&& ( LightingChannels.Static	== FALSE )
		)
	{
		Retval = TRUE;
	}

	return Retval;
}


UBOOL ULightComponent::IsLACStaticAffecting()
{
	UBOOL Retval = FALSE;

	// here we need to check our outer and see if it is a light 
	// and if it is a toggleable light
	UBOOL bIsToggleableLight = FALSE;
	ALight* LightActor = Cast<ALight>(GetOuter());

	if( LightActor != NULL )
	{
		// now check to see if it is a toggleable light.  We can can just check the flags of the
		// the LightActor to see if it has the "toggleable" set.    Could also cast each to the 
		// three types of lights we have and check those

		if( ( LightActor->bMovable == FALSE )
			&& (LightActor->bStatic == FALSE )
			&& ( LightActor->bHardAttach == TRUE )
			)
		{
			bIsToggleableLight = TRUE;
		}
	}

	if( ( CastShadows == TRUE )
		&& ( CastStaticShadows == TRUE )
		&& ( CastDynamicShadows == FALSE )
		&& ( bForceDynamicLight == FALSE )

		// not certain we need to have the UseDirectLightMap check here?   should be an optional 
		// as objects can be forced lightmapped (tho that is the "Wrong" way to do things
		&& ( ( ( bIsToggleableLight == FALSE ) && ( UseDirectLightMap == TRUE ) )
		   || ( ( bIsToggleableLight == TRUE ) && ( UseDirectLightMap == FALSE ) )// toggleable lights can't have direct light maps
		   )

		   // Lots of wonderful Lighting Channels exist so S lights are basically any light
		   // that meets the above values  And doesn't have the dynamic channel set
		&& ( LightingChannels.BSP == TRUE )
		&& ( LightingChannels.Dynamic == FALSE )
		&& ( LightingChannels.Static == TRUE )
		)
	{
		Retval = TRUE;
	}

	return Retval;
}


UBOOL ULightComponent::IsLACDynamicAndStaticAffecting()
{
	UBOOL Retval = FALSE;

	if( ( CastShadows == TRUE )
		&& ( CastStaticShadows == TRUE )
		&& ( CastDynamicShadows == TRUE )
		&& ( bForceDynamicLight == FALSE )
		&& ( UseDirectLightMap == FALSE )

		&& ( LightingChannels.BSP == TRUE )
		&& ( LightingChannels.Dynamic == TRUE )
		&& ( LightingChannels.Static == TRUE )
		)
	{
		Retval = TRUE;
	}

	return Retval;
}


/**
 * Called after this UObject has been serialized
 */
void ULightComponent::PostLoad()
{
	Super::PostLoad();

#if !SUPPORTS_VSM
	// use default shadow projection if code doesn't support VSM shadows 
	if( ShadowProjectionTechnique == ShadowProjTech_VSM )
	{
		ShadowProjectionTechnique = ShadowProjTech_Default;
	}
#endif

	if ( Function != NULL && Function->GetOuter() != this && !IsTemplate() )
	{
		// this is the culprit behind PointLightComponents that have a NULL PreviewLightRadius;  basically, this PLC's Function is pointing to the Function object owned by a
		// PLC that has been removed from the level....fix up these guys now
		ULightFunction* NewFunction = Cast<ULightFunction>(StaticDuplicateObject(Function, Function, this, *Function->GetName()));
		if ( NewFunction != NULL )
		{
			debugf(NAME_Warning, TEXT("Invalid LightFunction detected for %s: %s.  Replacing existing value with fresh copy %s"), *GetFullName(), *Function->GetFullName(), *NewFunction->GetFullName());
			Function = NewFunction;
		}
	}

	if( RequireDynamicShadows )
	{
		bForceDynamicLight			= TRUE;
		RequireDynamicShadows		= FALSE;
	}

	if(GetLinker() && GetLinker()->Ver() < VER_BOUNCEDLIGHTING_DIRECTMODULATION)
	{
		// Set a bounced light intensity and modulation color to mimic the old behavior of light environments:
		// Light environments previously added bounced lighting of 10% of the slightly saturated direct light color.
		const FLinearColor LinearLightColor = FLinearColor(LightColor) * Brightness;
		const FLinearColor DesaturatedLightColor = LinearLightColor.Desaturate(-1.0f);
		const FLinearColor DesaturatedLightColorRatio(
			DesaturatedLightColor.R / Max(DELTA,LinearLightColor.R),
			DesaturatedLightColor.G / Max(DELTA,LinearLightColor.G),
			DesaturatedLightColor.B / Max(DELTA,LinearLightColor.B),
			1.0f
			);
		const FLinearColor BouncedLightColor = DesaturatedLightColorRatio * 0.1f;
		ComputeAndFixedColorAndIntensity(BouncedLightColor,LightEnv_BouncedModulationColor,LightEnv_BouncedLightBrightness);
	}

	// so we have loaded up a map we want to make certain all of the lights have the most
    // recent light classification icon
	SetLightAffectsClassificationBasedOnSettings();
}

void ULightComponent::PreEditUndo()
{
	// Directly call UActorComponent::PreEditChange to avoid ULightComponent::PreEditChange being called for transactions.
	UActorComponent::PreEditChange(NULL);
}

void ULightComponent::PostEditUndo()
{
	// Directly call UActorComponent::PostEditChange to avoid ULightComponent::PostEditChange being called for transactions.
	UActorComponent::PostEditChange(NULL);
}

void ULightComponent::PostEditChange(UProperty* PropertyThatChanged)
{
	// depending on what has changed we will possibly change the icon on the light
	FName PropertyName = PropertyThatChanged ? PropertyThatChanged->GetFName() : NAME_None;

	MinShadowResolution = Max(MinShadowResolution,0);
	MaxShadowResolution = Max(MaxShadowResolution,0);

#if !SUPPORTS_VSM
	// use default shadow projection if code doesn't support VSM shadows 
	if( ShadowProjectionTechnique == ShadowProjTech_VSM )
	{
		ShadowProjectionTechnique = ShadowProjTech_Default;
	}
#endif

	// Enabling/ using a light function wins over using lightmaps. We disable UseDirectLightMap to make it clear what
	// goes on and what to expect when rebuilding lighting.
	if( Function )
	{
		UseDirectLightMap = FALSE;
	}

	// check for the things we care about for light classification
	if(		( PropertyName == NAME_None )
		||	( PropertyName == TEXT("CastShadows") )
		||	( PropertyName == TEXT("CastStaticShadows") )
		||	( PropertyName == TEXT("CastDynamicShadows") )
		||	( PropertyName == TEXT("bForceDynamicLight") )
		||	( PropertyName == TEXT("UseDirectLightMap") )
		||	( PropertyName == TEXT("BSP") ) 
		||	( PropertyName == TEXT("Dynamic") ) 
		||	( PropertyName == TEXT("Static") ) 
		)
	{
		SetLightAffectsClassificationBasedOnSettings();
	}

	// Check for properties that don't require any static lighting to be rebuilt.
	if( PropertyName != TEXT("CastDynamicShadows") &&
		PropertyName != TEXT("bCastCompositeShadow") &&
		PropertyName != TEXT("bAffectCompositeShadowDirection") &&
		PropertyName != TEXT("LightEnv_BouncedLightBrightness") &&
		PropertyName != TEXT("LightEnv_BouncedModulationColor") &&
		PropertyName != TEXT("LightShadowMode") &&
		PropertyName != TEXT("ModShadowColor") &&
		PropertyName != TEXT("ModShadowFadeoutTime") &&
		PropertyName != TEXT("ModShadowColor") &&
		PropertyName != TEXT("ModShadowFadeoutTime") &&
		PropertyName != TEXT("ModShadowFadeoutExponent") &&
		PropertyName != TEXT("ShadowProjectionTechnique") &&
		PropertyName != TEXT("ShadowFilterQuality") &&
		PropertyName != TEXT("MinShadowResolution") &&
		PropertyName != TEXT("MaxShadowResolution") &&
		PropertyName != TEXT("ShadowFadeResolution"))
	{
		// Check for properties that only require light-maps to be rebuilt, and not shadow-maps.
		if(	PropertyName == TEXT("LightColor") ||
			PropertyName == TEXT("Brightness") ||
			PropertyName == TEXT("bEnabled"))
		{
			// Create a new GUID for lightmap usage.
			LightmapGuid = appCreateGuid();
			// Invalidate just lightmap data.
			InvalidateLightmapData();
			// Mark level as requiring lighting to be rebuilt.
			MarkLightingRequiringRebuild();		
		}
		// If the property isn't explicitly defined as not needing a static lighting rebuild, then discard the light's current
		// static lighting data.
		else
		{
			// Fully invalidate lighting cache.
			InvalidateLightingCache();
		}
	}

	// Update inclusion/ exclusion light volumes.
	UpdateVolumes();

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Updates the inclusion/ exclusion volumes.
 */
void ULightComponent::UpdateVolumes()
{
	// Clear any references to brushes in a different level from the light.
	for( INT VolumeIndex=0; VolumeIndex<ExclusionVolumes.Num(); VolumeIndex++ )
	{
		ABrush*& Brush = ExclusionVolumes(VolumeIndex);
		if( Brush && Brush->GetOutermost() != GetOutermost() )
		{
			Brush = NULL;
		}
	}
	for( INT VolumeIndex=0; VolumeIndex<InclusionVolumes.Num(); VolumeIndex++ )
	{
		ABrush*& Brush = InclusionVolumes(VolumeIndex);
		if( Brush && Brush->GetOutermost() != GetOutermost() )
		{
			Brush = NULL;
		}
	}

	// Re-populate convex exclusion volumes.
	GetConvexVolumesFromBrushes(ExclusionVolumes,ExclusionConvexVolumes);

	// Re-populate convex inclusion volumes.
	GetConvexVolumesFromBrushes(InclusionVolumes,InclusionConvexVolumes);
}

/**
 * Serialize function.
 */
void ULightComponent::Serialize(FArchive& Ar)
{
	Super::Serialize( Ar );

	// Serialize inclusion/ exclusion volumes.
	Ar << InclusionConvexVolumes;
	Ar << ExclusionConvexVolumes;
}

/**
 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure GUIDs remains globally unique.
 */
void ULightComponent::PostDuplicate()
{
	Super::PostDuplicate();
	// Create new guids for light.
	UpdateLightGUIDs();
	// This is basically a new object so we don't have to worry about invalidating lightmap data.
	bHasLightEverBeenBuiltIntoLightMap	= FALSE;
}

/**
 * Called after importing property values for this object (paste, duplicate or .t3d import)
 * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
 * are unsupported by the script serialization
 */
void ULightComponent::PostEditImport()
{
	Super::PostEditImport();
	// Create new guids for light.
	UpdateLightGUIDs();
	// This is basically a new object so we don't have to worry about invalidating lightmap data.
	bHasLightEverBeenBuiltIntoLightMap	= FALSE;
}

void ULightComponent::SetLightAffectsClassificationBasedOnSettings()
{
	ALight* LightActor = Cast<ALight>(GetOuter());

    // check to make certain out parent is a light otherwise we probably don't want
    // to override the current icon
	if( ( LightActor != NULL ) && ( LightActor->LightComponent == this ) )
	{
		if( IsLACDynamicAffecting() == TRUE )
		{
			LightAffectsClassification = LAC_DYNAMIC_AFFECTING;
		}
		else if( IsLACStaticAffecting() == TRUE )
		{
			LightAffectsClassification = LAC_STATIC_AFFECTING;
		}
		else if( IsLACDynamicAndStaticAffecting() == TRUE )
		{
			LightAffectsClassification = LAC_DYNAMIC_AND_STATIC_AFFECTING;
		}
		else
		{
			LightAffectsClassification = LAC_USER_SELECTED;
		}

		// DetermineAndSetEditorIcon depends on the LAC being set
		LightActor->DetermineAndSetEditorIcon();
	}
}

/**
 * Update of all light environments in the given light's world.
 */
static void UpdateLightEnvironments(const ULightComponent* Light)
{
	UWorld* World = Light->GetScene()->GetWorld();
	if(World)
	{
		// Make a local copy of the light environment list, since the reattachments will modify the world's list.
		TSparseArray<ULightEnvironmentComponent*> LocalLightEnvironmentList = World->LightEnvironmentList;
		for(TSparseArray<ULightEnvironmentComponent*>::TConstIterator EnvironmentIt(LocalLightEnvironmentList);EnvironmentIt;++EnvironmentIt)
		{
			ULightEnvironmentComponent* LightEnvironmentComponent = *EnvironmentIt;
			if(!LightEnvironmentComponent->HasAnyFlags(RF_Unreachable))
			{
				LightEnvironmentComponent->UpdateLight(Light);
			}
		}
	}
}

/**
 * Adds this light to the world's dynamic or static light list.
 */
void ULightComponent::AddToLightList()
{
	UWorld* World = Scene->GetWorld();

	// Add the light to the world's light set.
	if( World )
	{
		// Insert the light into the correct list
		if( HasStaticLighting() )
		{
			SetStaticLightListIndex( World->StaticLightList.AddItem(this) );
		}
		else
		{
			SetDynamicLightListIndex( World->DynamicLightList.AddItem(this) );
		}
	}
}

void ULightComponent::Attach()
{
	// Update GUIDs on attachment if they are not valid.
	ValidateLightGUIDs();

	Super::Attach();

	if (bEnabled)
	{
		// Add the light to the scene.
		Scene->AddLight(this);

		if(!LightEnvironment)
		{
			// Add the light to the world's light set.
			AddToLightList();
			
			// In the editor, update light environments to include the changed static lighting.
			if(!GIsGame)
			{
				UpdateLightEnvironments(this);
			}
		}
	}
}

void ULightComponent::UpdateTransform()
{
	Super::UpdateTransform();

	// Update the scene info's transform for this light.
	Scene->UpdateLightTransform(this);

	if( bEnabled )
	{
		if(!LightEnvironment)
		{
			// Add the light to the world's light set, if it hasn't already been added.
			if( !IsInLightList() )
			{
				AddToLightList();
			}
			
			// In the editor, update light environments to include the changed static lighting.
			if(!GIsGame)
			{
				UpdateLightEnvironments(this);
			}
		}
	}
}

/**
 * Sets bEnabled and handles updating the component.
 */
void ULightComponent::execSetEnabled(FFrame &Stack,RESULT_DECL)
{
	P_GET_UBOOL(bSetEnabled);
	P_FINISH;
	SetEnabled( bSetEnabled );
}
IMPLEMENT_FUNCTION(ULightComponent,INDEX_NONE,execSetEnabled);



/**
 * Toggles the light on or off
 *
 * @param bSetEnabled TRUE to enable the light or FALSE to disable it
 */
void ULightComponent::SetEnabled( UBOOL bSetEnabled )
{
	if (bEnabled != bSetEnabled)
	{
		bEnabled = bSetEnabled;

		// Request a deferred component update.
		BeginDeferredReattach();
	}
}


void ULightComponent::execSetLightProperties(FFrame& Stack, RESULT_DECL)
{
	P_GET_FLOAT_OPTX(NewBrightness, Brightness);
	P_GET_STRUCT_OPTX(FColor, NewLightColor, LightColor);
	P_GET_OBJECT_OPTX(ULightFunction, NewLightFunction, Function);
	P_FINISH;

	// If we're not changing anything, then bail out.
	if( Brightness == NewBrightness &&
		LightColor == NewLightColor &&
		Function == NewLightFunction )
	{
		return;
	}

	Brightness	= NewBrightness;
	LightColor	= NewLightColor;
	Function	= NewLightFunction;

	// Request a deferred component update.
	BeginDeferredReattach();
}
IMPLEMENT_FUNCTION(ULightComponent,INDEX_NONE,execSetLightProperties);

void ULightComponent::execGetOrigin(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;
	*(FVector*)Result = GetOrigin();
}
IMPLEMENT_FUNCTION(ULightComponent,INDEX_NONE,execGetOrigin);

void ULightComponent::execGetDirection(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;
	*(FVector*)Result = GetDirection();
}
IMPLEMENT_FUNCTION(ULightComponent,INDEX_NONE,execGetDirection);

void ULightComponent::execUpdateColorAndBrightness(FFrame& Stack, RESULT_DECL)
{
	P_FINISH;
	if( Scene )
	{
		Scene->UpdateLightColorAndBrightness( this );
	}
}
IMPLEMENT_FUNCTION(ULightComponent,INDEX_NONE,execUpdateColorAndBrightness);

void ULightComponent::Detach( UBOOL bWillReattach )
{
	Super::Detach( bWillReattach );
	Scene->RemoveLight(this);

	UWorld* World = Scene->GetWorld();
	if( World )
	{
		if(!LightEnvironment)
		{
			// Remove light from the correct list.
			if( IsInDynamicLightList() )
			{
				World->DynamicLightList.Remove( GetLightListIndex() );
			}
			else if( IsInStaticLightList() )
			{
				World->StaticLightList.Remove( GetLightListIndex() );
			}
			// Update light environments to include the updated lighting.
			if(!GIsGame)
			{
				UpdateLightEnvironments(this);
			}
			// Invalidate index.
			InvalidateLightListIndex();
		}
	}
}

//
//	ULightComponent::InvalidateLightingCache
//

void ULightComponent::InvalidateLightingCache()
{
	// Save the light state for transactions.
	Modify();

	// Create new guids for light.
	UpdateLightGUIDs();
	// Invalidate lightmap data.
	InvalidateLightmapData();
	// reattach the invalidated component
	// this has to be done after the above calls so that the Owner remains valid
	FComponentReattachContext ReattachContext( this );
}

/**
 * Invalidates lightmap data of affected primitives if this light has ever been built 
 * into a lightmap.
 */
void ULightComponent::InvalidateLightmapData()
{
	// If the light has been built into any lightmaps, try to invalidate the lightmaps by invalidating the cached lighting for the
	// primitives the light is attached to.  This isn't perfect, but will catch most cases.
	if(bHasLightEverBeenBuiltIntoLightMap)
	{
		bHasLightEverBeenBuiltIntoLightMap = FALSE;

		// Iterate over primitives which the light affects.
		for( TObjectIterator<UPrimitiveComponent> It; It; ++It )
		{
			UPrimitiveComponent* Primitive = *It;
			// If the primitive is attached, affected and the interaction is lightmapped, invalidate the lighting cache
			// of the primitive, which in turn will invalidate its lightmaps.
			if( Primitive->IsAttached() 
			&&	AffectsPrimitive(Primitive) 
			&&	UseStaticLighting(Primitive->bForceDirectLightMap) )
			{
				// Invalidate the primitive's lighting cache.
				Primitive->InvalidateLightingCache();
			}
		}
	}
}

/**
 * Computes the intensity of the direct lighting from this light on a specific point.
 */
FLinearColor ULightComponent::GetDirectIntensity(const FVector& Point) const 
{ 
	if(bEnabled)
	{
		return FLinearColor(LightColor) * Brightness;
	}
	else
	{
		return FLinearColor::Black;
	}
}

FLinearColor ULightComponent::GetBouncedIntensity(const FVector& Point) const 
{ 
	return GetDirectIntensity(Point) * FLinearColor(LightEnv_BouncedModulationColor) * LightEnv_BouncedLightBrightness;
}
