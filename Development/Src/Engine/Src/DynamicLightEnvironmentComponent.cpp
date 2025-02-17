/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "DynamicLightEnvironmentComponent.h"

IMPLEMENT_CLASS(UDynamicLightEnvironmentComponent);

/** Compute the direction which the spherical harmonic is highest at. */
static FVector SHGetMaximumDirection(const FSHVector& SH,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	// This is an approximation which only takes into account first and second order spherical harmonics.
	FLOAT Z = SH.V[2];
	if(!bLowerHemisphere)
	{
		Z = Max(Z,0.0f);
	}
	if(!bUpperHemisphere)
	{
		Z = Min(Z,0.0f);
	}
	return FVector(
		-SH.V[3],
		-SH.V[1],
		Z
		);
}

/** Compute the direction which the spherical harmonic is lowest at. */
static FVector SHGetMinimumDirection(const FSHVector& SH,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	// This is an approximation which only takes into account first and second order spherical harmonics.
	FLOAT Z = -SH.V[2];
	if(!bLowerHemisphere)
	{
		Z = Max(Z,0.0f);
	}
	if(!bUpperHemisphere)
	{
		Z = Min(Z,0.0f);
	}
	return FVector(
		+SH.V[3],
		+SH.V[1],
		Z
		);
}

/** Clamps each color component above 0. */
static FLinearColor GetPositiveColor(const FLinearColor& Color)
{
	return FLinearColor(
		Max(Color.R,0.0f),
		Max(Color.G,0.0f),
		Max(Color.B,0.0f),
		Color.A
		);
}

/**
 * Calculates the intensity to use for a light that minimizes Dot(RemainingLightEnvironment,RemainingLightEnvironment),
 * given RemainingLightEnvironment = LightEnvironment - UnitLightFunction * <resulting intensity>.
 * In other words, it tries to set a light intensity that accounts for as much of the light environment as possible given UnitLightFunction.
 * @param LightEnvironment - The light environment to subtract the light function from.
 * @param UnitLightFunction - The incident lighting that would result from a light intensity of 1.
 * @return The light intensity that minimizes the light remaining in the environment.
 */
static FLinearColor GetLightIntensity(const FSHVectorRGB& LightEnvironment,const FSHVector& UnitLightFunction)
{
	return GetPositiveColor(Dot(LightEnvironment,UnitLightFunction) / Dot(UnitLightFunction,UnitLightFunction));
}

/**
 * Extracts the dominant lighting from a composite light environment.
 * @param InOutLightEnvironment - The composite light environment.  The dominant light is subtracted from it.
 * @param OutDirection - On successful return, the direction to the dominant light.
 * @param OutIntensity - On successful return, the intensity of the dominant light.
 * @return TRUE if the light environment had light to extract.
 */
static UBOOL ExtractDominantLight(FSHVectorRGB& InOutLightEnvironment,FVector& OutDirection,FLinearColor& OutIntensity)
{
	// Find the direction in the light environment with the highest luminance.
	const FSHVector EnvironmentLuminance = InOutLightEnvironment.GetLuminance();
	OutDirection = SHGetMaximumDirection(EnvironmentLuminance,TRUE,TRUE);
	if(OutDirection.SizeSquared() >= Square(DELTA))
	{
		OutDirection.Normalize();

		// Calculate the light intensity for this direction.
		const FSHVector UnitLightSH = SHBasisFunction(OutDirection);

		OutIntensity = GetLightIntensity(InOutLightEnvironment,UnitLightSH);

		if(OutIntensity.R > 0.0f || OutIntensity.G > 0.0f || OutIntensity.B > 0.0f)
		{
			// Remove the dominant light from the environment.
			InOutLightEnvironment -= UnitLightSH * OutIntensity;

			return TRUE;
		}
	}

	return FALSE;
}

/** Remove skylighting from a light environment. */
static void ExtractEnvironmentSkyLight(FSHVectorRGB& LightEnvironment,FLinearColor& OutSkyColor,UBOOL bLowerHemisphere,UBOOL bUpperHemisphere)
{
	// Set up SH coefficients representing incident lighting from a sky light of unit brightness.
	FSHVector SkyFunction;
	if(bLowerHemisphere)
	{
		SkyFunction += FSHVector::LowerSkyFunction();
	}
	if(bUpperHemisphere)
	{
		SkyFunction += FSHVector::UpperSkyFunction();
	}

	// Calculate the sky light intensity.
	const FLinearColor Intensity = GetLightIntensity(LightEnvironment,SkyFunction);
	if(Intensity.R > 0.0f || Intensity.G > 0.0f || Intensity.B > 0.0f)
	{
		OutSkyColor += Intensity;
		LightEnvironment -= SkyFunction * Intensity;
	}
}

/** Adds a skylight to a light environment. */
static FSHVectorRGB GetSkyLightEnvironment(const UDynamicLightEnvironmentComponent* LightEnvironment,const USkyLightComponent* SkyLight)
{
	const FLinearColor DirectUpperColor = FLinearColor(SkyLight->LightColor) * SkyLight->Brightness;
	const FLinearColor DirectLowerColor = FLinearColor(SkyLight->LowerColor) * SkyLight->LowerBrightness;
	const FLinearColor BouncedLightColor =
		FLinearColor(SkyLight->LightEnv_BouncedModulationColor) *
		SkyLight->LightEnv_BouncedLightBrightness *
		LightEnvironment->BouncedLightingFactor;
	const FLinearColor TotalUpperColor = DirectUpperColor + DirectLowerColor * BouncedLightColor;
	const FLinearColor TotalLowerColor = DirectLowerColor + DirectUpperColor * BouncedLightColor;

	return	FSHVector::UpperSkyFunction() * TotalUpperColor +
			FSHVector::LowerSkyFunction() * TotalLowerColor;
}

/** Returns the integral of the square of the difference between two spherical harmonic projected functions. */
static FLOAT GetSquaredDifferenceIntegral(const FSHVectorRGB& A,const FSHVectorRGB& B)
{
	const FSHVectorRGB Difference = A - B;
	return	Dot(Difference.R,Difference.R) +
			Dot(Difference.G,Difference.G) +
			Dot(Difference.B,Difference.B);
}

FDynamicLightEnvironmentState::FDynamicLightEnvironmentState(UDynamicLightEnvironmentComponent* InComponent):
	Component(InComponent)
,	PredictedOwnerPosition(0,0,0)
,	LastUpdateTime(0)
,	InvisibleUpdateTime(InComponent->InvisibleUpdateTime)
,	MinTimeBetweenFullUpdates(InComponent->MinTimeBetweenFullUpdates)
,	CurrentRepresentativeShadowLight(NULL)
,	bFirstFullUpdate(TRUE)
{
}

void FDynamicLightEnvironmentState::UpdateOwner()
{
	AActor* Owner = Component->GetOwner();
	check(Owner);
	check(Owner->AllComponents.ContainsItem(Component));

	// Ensure that the owner's other components have been attached before we gather their attributes.
	for(INT ComponentIndex = 0;ComponentIndex < Owner->Components.Num();ComponentIndex++)
	{
		UActorComponent* Component = Owner->Components(ComponentIndex);
		if(Component && !Component->IsAttached())
		{
			Component->ConditionalAttach(GWorld->Scene,Owner,Owner->LocalToWorld());
		}
	}

	// Find the owner's bounds and lighting channels.
	const FBoxSphereBounds PreviousOwnerBounds = OwnerBounds;
	OwnerBounds = FBoxSphereBounds(Owner->Location,FVector(0,0,0),50);
	OwnerLightingChannels.Bitfield = 0;
	OwnerLightingChannels.bInitialized = TRUE;
	for(INT ComponentIndex = 0;ComponentIndex < Owner->AllComponents.Num();ComponentIndex++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Owner->AllComponents(ComponentIndex));

		// Only look at primitives which use this light environment.
		if(Primitive && Primitive->LightEnvironment == Component)
		{
			// Add the primitive's bounds to the composite owner bounds.
			OwnerBounds = OwnerBounds + Primitive->Bounds;

			// Add the primitive's lighting channels to the composite owner lighting channels.
			OwnerLightingChannels.Bitfield |= Primitive->LightingChannels.Bitfield;
		}
	}

	if (Component->bOverrideOwnerBounds)
	{
		OwnerBounds = Component->OverriddenBounds;
	}

	if (Component->bOverrideOwnerLightingChannels)
	{
		OwnerLightingChannels = Component->OverriddenLightingChannels;
	}

	// Find the owner's package.
	OwnerPackage = Owner->GetOutermost();

	PredictedOwnerPosition = OwnerBounds.Origin;

	// Update the cached light visibility sample points if we don't have the right number.
	if(LightVisibilitySamplePoints.Num() != Component->NumVolumeVisibilitySamples)
	{
		LightVisibilitySamplePoints.Empty();

		// Initialize the random light visibility sample points.
		const INT NumLightVisibilitySamplePoints = Component->NumVolumeVisibilitySamples;
		LightVisibilitySamplePoints.Empty(NumLightVisibilitySamplePoints);
		for(INT PointIndex = 1;PointIndex < NumLightVisibilitySamplePoints;PointIndex++)
		{
			LightVisibilitySamplePoints.AddItem(
				FVector(
					-1.0f + 2.0f * appSRand(),
					-1.0f + 2.0f * appSRand(),
					-1.0f + 2.0f * appSRand()
					)
				);
		}

		// Always place one sample at the center of the owner's bounds.
		LightVisibilitySamplePoints.AddItem(FVector(0,0,0));
	}
}

void FDynamicLightEnvironmentState::UpdateStaticEnvironment()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateStaticEnvironmentTime);
	INC_DWORD_STAT(STAT_StaticEnvironmentUpdates);

	// Reset as the below code is going to accumulate from scratch.
	NewStaticLightEnvironment = FSHVectorRGB();
	NewStaticNonShadowedLightEnvironment = FSHVectorRGB();
	NewStaticShadowEnvironment	= FSHVectorRGB();

	// Iterate over static lights and update the static light environment.
	for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->StaticLightList);LightIt;++LightIt)
	{
		const ULightComponent* Light = *LightIt;

		// Add static light to static light environment
		AddLightToEnvironment(Light,NewStaticLightEnvironment,NewStaticNonShadowedLightEnvironment,NewStaticShadowEnvironment,PredictedOwnerPosition,FALSE);
	}

	// Add the ambient glow.
	NewStaticLightEnvironment += FSHVector::AmbientFunction() * (Component->AmbientGlow * 4.0f);

	// Add the ambient shadow source.
	const FSHVector AmbientShadowSH = SHBasisFunction(Component->AmbientShadowSourceDirection.SafeNormal());
	NewStaticShadowEnvironment += AmbientShadowSH * Component->AmbientShadowColor;
}

void FDynamicLightEnvironmentState::UpdateDynamicEnvironment()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDynamicEnvironmentTime);
	INC_DWORD_STAT(STAT_DynamicEnvironmentUpdates);
	
	DynamicLightEnvironment = FSHVectorRGB();
	DynamicNonShadowedLightEnvironment = FSHVectorRGB();
	DynamicShadowEnvironment = FSHVectorRGB();

	// Iterate over dynamic lights and update the dynamic light environment.
	for(TSparseArray<ULightComponent*>::TConstIterator LightIt(GWorld->DynamicLightList);LightIt;++LightIt)
	{
		ULightComponent* Light = *LightIt;

		// Add the dynamic light to the light environment.
		AddLightToEnvironment(Light,DynamicLightEnvironment,DynamicNonShadowedLightEnvironment,DynamicShadowEnvironment,OwnerBounds.Origin,TRUE);
	}
}

void FDynamicLightEnvironmentState::UpdateEnvironmentInterpolation(FLOAT DeltaTime,FLOAT TimeBetweenUpdates)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateEnvironmentInterpolationTime);

	// Smoothly interpolate the static light set.
	const FLOAT RemainingTransitionTime = Max(DELTA,LastUpdateTime + TimeBetweenUpdates - GWorld->GetTimeSeconds());
	const FLOAT TransitionAlpha = Clamp(DeltaTime / RemainingTransitionTime,0.0f,1.0f);
	StaticLightEnvironment				*= (1.0f - TransitionAlpha);
	StaticNonShadowedLightEnvironment	*= (1.0f - TransitionAlpha);
	StaticShadowEnvironment				*= (1.0f - TransitionAlpha);
	StaticLightEnvironment				+= TransitionAlpha * NewStaticLightEnvironment;
	StaticNonShadowedLightEnvironment	+= TransitionAlpha * NewStaticNonShadowedLightEnvironment;
	StaticShadowEnvironment				+= TransitionAlpha * NewStaticShadowEnvironment;
}

void FDynamicLightEnvironmentState::Update()
{
	UpdateOwner();
	UpdateStaticEnvironment();
	UpdateDynamicEnvironment();

	// Immediately transition to the newly computed light environment.
	StaticLightEnvironment = NewStaticLightEnvironment;
	StaticNonShadowedLightEnvironment = NewStaticNonShadowedLightEnvironment;
	StaticShadowEnvironment = NewStaticShadowEnvironment;

	// Update the lights from the environment.
	CreateEnvironmentLightList();
}

void FDynamicLightEnvironmentState::Tick(FLOAT DeltaTime)
{
	INC_DWORD_STAT(STAT_NumEnvironments);

	const FVector PreviousPredictedOwnerPosition = PredictedOwnerPosition;
	const FBoxSphereBounds PreviousOwnerBounds = OwnerBounds;

	if(bFirstFullUpdate || Component->bRequiresNonLatentUpdates)
	{
		// The first time a light environment is ticked, perform a full update.
		Update();
		bFirstFullUpdate = FALSE;
	}
	else if(!bFirstFullUpdate)
	{
		FLOAT LastRenderTime = 0.0f;
		for (INT ComponentIndex = 0; ComponentIndex < Component->AffectedComponents.Num(); ComponentIndex++)
		{
			UPrimitiveComponent* CurrentComponent = Component->AffectedComponents(ComponentIndex);
			if (CurrentComponent && CurrentComponent->LastRenderTime > LastRenderTime)
			{
				LastRenderTime = CurrentComponent->LastRenderTime;
			}
		}
		const UBOOL bVisible = (GWorld->GetTimeSeconds() - LastRenderTime) < 1.0f;
		UBOOL bForceDynamicLightUpdate = FALSE;

		if(Component->bDynamic)
		{
			// Update the owner bounds and other info.
			UpdateOwner();

			// Determine the distance of the light environment's owner from the closest local player's view.
			FLOAT MinPlayerDistanceSquared = 0.0f;
			if(GIsGame)
			{
				MinPlayerDistanceSquared = Square(WORLD_MAX);
				for(INT PlayerIndex = 0;PlayerIndex < GEngine->GamePlayers.Num();PlayerIndex++)
				{
					if(GEngine->GamePlayers(PlayerIndex))
					{
						const FVector& PlayerViewLocation = GEngine->GamePlayers(PlayerIndex)->LastViewLocation;
						MinPlayerDistanceSquared = Min(MinPlayerDistanceSquared,(PlayerViewLocation - OwnerBounds.Origin).SizeSquared());
					}
				}
			}
			const FLOAT FullSpeedUpdateMaxDistance = Max(DELTA,OwnerBounds.SphereRadius * 4.0f);
			const FLOAT TimeBetweenUpdatesDistanceFactor = Clamp(appSqrt(MinPlayerDistanceSquared) / FullSpeedUpdateMaxDistance,1.0f,10.0f);

			// Determine if the light environment's primitives have been rendered in the last second.
			const FLOAT TimeSinceLastUpdate	= GWorld->GetTimeSeconds() - LastUpdateTime;
			const FLOAT TimeBetweenUpdatesVisibilityFactor = bVisible ?
				MinTimeBetweenFullUpdates :
				InvisibleUpdateTime;

			// Only update the light environment if it's visible, or it hasn't been updated for the last InvisibleUpdateTime seconds.
			const FLOAT TimeBetweenUpdates = TimeBetweenUpdatesVisibilityFactor * TimeBetweenUpdatesDistanceFactor;
			const UBOOL bDynamicUpdateNeeded = TimeSinceLastUpdate > TimeBetweenUpdates;
			UBOOL bPerformFullUpdate = FALSE;
			if(bDynamicUpdateNeeded)
			{
				LastUpdateTime = GWorld->GetTimeSeconds();

				// Only perform an update of the static light environment after the first update if the primitive has moved.
				if( (PredictedOwnerPosition != PreviousPredictedOwnerPosition) 
				||	(OwnerBounds.SphereRadius != PreviousOwnerBounds.SphereRadius) )
				{
					UpdateStaticEnvironment();
				}

				// Spread out updating light environments over several frames to avoid frame time spikes for level
				// placed light environments that start being ticked on the same frame.
				InvisibleUpdateTime = Component->InvisibleUpdateTime * (0.8 + 0.4 * appSRand());
				MinTimeBetweenFullUpdates = Component->MinTimeBetweenFullUpdates * (0.8 + 0.4 * appSRand());
			}

			// Interpolate toward the previously calculated light environment state.
			UpdateEnvironmentInterpolation(DeltaTime,TimeBetweenUpdates);

			// Also update the environment if it's within the full-speed update radius.
			if(MinPlayerDistanceSquared < Square(FullSpeedUpdateMaxDistance))
			{
				bForceDynamicLightUpdate = TRUE;
			}
		}

		// Skip updating the dynamic environment or synthesized lights for light environments on primitives that aren't visible.
		if(bVisible || bForceDynamicLightUpdate)
		{
			// Update the effect of dynamic lights on the light environment.
			UpdateDynamicEnvironment();

			// Create lights to represent the environment.
			CreateEnvironmentLightList();
		}
	}
}

ULightComponent* FDynamicLightEnvironmentState::CreateRepresentativeLight(const FVector& Direction,const FLinearColor& Intensity)
{
	// Construct a point light to represent the brightest direction of the remaining light environment.
	UDirectionalLightComponent* Light = AllocateLight<UDirectionalLightComponent>();
	const FVector LightDirection = Direction.SafeNormal();
	Light->LightingChannels = OwnerLightingChannels;
	Light->LightEnvironment = Component;
	Light->bCastCompositeShadow = TRUE;

	ComputeAndFixedColorAndIntensity(Intensity,Light->LightColor,Light->Brightness);

	Light->CastShadows = FALSE;

	return Light;
}

UPointLightComponent* FDynamicLightEnvironmentState::CreateRepresentativeShadowLight(const FVector& Direction,const FLinearColor& Intensity)
{
	// Construct a point light to represent the brightest direction of the remaining light environment.
	UPointLightComponent* Light = AllocateLight<UPointLightComponent>();
	Light->LightingChannels = OwnerLightingChannels;
	Light->LightEnvironment = Component;

	Light->CastShadows = TRUE;
	Light->LightShadowMode = Component->LightShadowMode;
	Light->ModShadowFadeoutTime = Component->ModShadowFadeoutTime;
	Light->ModShadowFadeoutExponent = Component->ModShadowFadeoutExponent;
	Light->ShadowFilterQuality = Component->ShadowFilterQuality;
	Light->MinShadowResolution = Component->MinShadowResolution;
	Light->MaxShadowResolution = Component->MaxShadowResolution;
	Light->ShadowFadeResolution = Component->ShadowFadeResolution;

	Light->Brightness = 0.0f;
	Light->ModShadowColor.R = Intensity.R;
	Light->ModShadowColor.G = Intensity.G;
	Light->ModShadowColor.B = Intensity.B;

	return Light;
}

void FDynamicLightEnvironmentState::DetachRepresentativeLights()
{
	CurrentRepresentativeShadowLight = NULL;

	// Detach the environment's representative lights.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		RepresentativeLightPool(LightIndex)->ConditionalDetach();
	}
}

void FDynamicLightEnvironmentState::CreateEnvironmentLightList(UBOOL bForceUpdate)
{
	SCOPE_CYCLE_COUNTER(STAT_CreateLightsTime);

	// If we're using composite dynamic lights, include their lighting contribution in the composite light environment.
	FSHVectorRGB CompositeLightEnvironment = StaticLightEnvironment + DynamicLightEnvironment;
	FSHVectorRGB CompositeNonShadowedLightEnvironment = StaticNonShadowedLightEnvironment + DynamicNonShadowedLightEnvironment;

	// If the light environment is set to composite shadows from dynamic lights, include their lighting in the composite shadow environment.
	FSHVectorRGB CompositeShadowEnvironment = StaticShadowEnvironment;
	if(Component->bCompositeShadowsFromDynamicLights)
	{
		CompositeShadowEnvironment += DynamicShadowEnvironment;
	}

	// Only update the representative lights if the environment has changed substantially from the last representative light update.
	static const FLOAT ErrorThreshold = Square(1.0f / 256.0f);
	const FLOAT LightError = GetSquaredDifferenceIntegral(CompositeLightEnvironment,CurrentRepresentativeLightEnvironment);
	const FLOAT NonShadowedLightError = GetSquaredDifferenceIntegral(CompositeNonShadowedLightEnvironment,CurrentRepresentativeLightEnvironment);
	const FLOAT ShadowError = GetSquaredDifferenceIntegral(CompositeShadowEnvironment,CurrentRepresentativeShadowEnvironment);
	if(	LightError > ErrorThreshold ||
		NonShadowedLightError > ErrorThreshold || 
		ShadowError > ErrorThreshold ||
		bForceUpdate)
	{
		CurrentRepresentativeLightEnvironment = CompositeLightEnvironment;
		CurrentRepresentativeNonShadowedLightEnvironment = CompositeNonShadowedLightEnvironment;
		CurrentRepresentativeShadowEnvironment = CompositeShadowEnvironment;

		// Detach the old representative lights.
		DetachRepresentativeLights();

		if(Component->bSynthesizeDirectionalLight)
		{
			FVector DominantLightDirection;
			FLinearColor DominantLightIntensity;
			if(ExtractDominantLight(CompositeLightEnvironment,DominantLightDirection,DominantLightIntensity))
			{
				// Create a directional light that is representative of the light environment.
				ULightComponent* Light = CreateRepresentativeLight(
					DominantLightDirection,
					DominantLightIntensity.Desaturate(Component->LightDesaturation)
					);

				// Attach the light after it is associated with the light environment to ensure it is only attached once.
				Light->ConditionalAttach(Component->GetScene(),NULL,FRotationMatrix((-DominantLightDirection).Rotation()));
			}
		}

		// Include the remaining shadowed light that couldn't be represented by the directional light in the secondary lighting,
		// in addition to all the lights that aren't modulated by the composite shadow.
		FSHVectorRGB SecondaryLightEnvironment = CompositeLightEnvironment + CompositeNonShadowedLightEnvironment;

		if(Component->bSynthesizeSHLight && (GWorld->GetWorldInfo()->GetSHLightsAllowed() || Component->bForceAllowLightEnvSphericalHarmonicLights) )
		{
			// Create a SH light for the lights not represented by the directional lights.
			USphericalHarmonicLightComponent* SHLight = AllocateLight<USphericalHarmonicLightComponent>();
			SHLight->LightingChannels = OwnerLightingChannels;
			SHLight->LightEnvironment = Component;
			SHLight->WorldSpaceIncidentLighting = SecondaryLightEnvironment;
			SHLight->bCastCompositeShadow = FALSE;

			// Combine the SH light into the base pass if the light environment is not casting shadows
			// Otherwise mod shadows would darken the SH light
			SHLight->bRenderBeforeModShadows = !(Component->bCastShadows && GSystemSettings.bAllowLightEnvironmentShadows);
	    
			// Attach the SH light after it is associated with the light environment to ensure it is only attached once.
			SHLight->ConditionalAttach(Component->GetScene(),NULL,FMatrix::Identity);
		}
		else
		{
			// Move as much light as possible into the sky light.
			FLinearColor LowerSkyLightColor(FLinearColor::Black);
			FLinearColor UpperSkyLightColor(FLinearColor::Black);
			ExtractEnvironmentSkyLight(SecondaryLightEnvironment,UpperSkyLightColor,FALSE,TRUE);
			ExtractEnvironmentSkyLight(SecondaryLightEnvironment,LowerSkyLightColor,TRUE,FALSE);

			// Create a sky light for the lights not represented by the directional lights.
			USkyLightComponent* SkyLight = AllocateLight<USkyLightComponent>();
			SkyLight->LightingChannels = OwnerLightingChannels;
			SkyLight->LightEnvironment = Component;
			SkyLight->bCastCompositeShadow = FALSE;
	    
			// Desaturate sky light color and add ambient glow afterwards.
			UpperSkyLightColor = UpperSkyLightColor.Desaturate( Component->LightDesaturation );
			LowerSkyLightColor = LowerSkyLightColor.Desaturate( Component->LightDesaturation );
	    
			// Convert linear color to color and brightness pair.
			ComputeAndFixedColorAndIntensity(UpperSkyLightColor,SkyLight->LightColor,SkyLight->Brightness);
			ComputeAndFixedColorAndIntensity(LowerSkyLightColor,SkyLight->LowerColor,SkyLight->LowerBrightness);
	    
			// Attach the skylight after it is associated with the light environment to ensure it is only attached once.
			SkyLight->ConditionalAttach(Component->GetScene(),NULL,FMatrix::Identity);
		}

		// Create a shadow-only point light that is representative of the shadow-casting light environment.
		if( Component->bCastShadows && GSystemSettings.bAllowLightEnvironmentShadows )
		{
			FLinearColor DominantShadowIntensity;
			if(ExtractDominantLight(CompositeShadowEnvironment,CurrentShadowDirection,DominantShadowIntensity))
			{
				// Use a shadow color that lets through light proportional to the shadowing not represented by the dominant shadow direction.
				const FLinearColor RemainingShadowIntensity = GetLightIntensity(CompositeShadowEnvironment,FSHVector::AmbientFunction());
				const FLinearColor TotalLightingIntensity = RemainingShadowIntensity + DominantShadowIntensity;
				const FLinearColor DominantShadowIntensityRatio(
					Min(1.0f,(TotalLightingIntensity.R - DominantShadowIntensity.R) / Max(TotalLightingIntensity.R,DELTA)),
					Min(1.0f,(TotalLightingIntensity.G - DominantShadowIntensity.G) / Max(TotalLightingIntensity.G,DELTA)),
					Min(1.0f,(TotalLightingIntensity.B - DominantShadowIntensity.B) / Max(TotalLightingIntensity.B,DELTA))
					);

				CurrentRepresentativeShadowLight = CreateRepresentativeShadowLight(CurrentShadowDirection,DominantShadowIntensityRatio);
			}
		}
	}

	// Attach the shadow light at the appropriate position.
	if(CurrentRepresentativeShadowLight)
	{
		// Compute the light's position and transform.
		const FLOAT LightDistance = OwnerBounds.SphereRadius * Component->LightDistance;
		const FVector LightPosition = OwnerBounds.Origin + CurrentShadowDirection.SafeNormal() * LightDistance;
		const FMatrix LightToWorld = FTranslationMatrix(LightPosition);

		// Update the light's radius for the owner's current bounding radius.
		CurrentRepresentativeShadowLight->Radius = OwnerBounds.SphereRadius * (Component->LightDistance + Component->ShadowDistance + 2);
		CurrentRepresentativeShadowLight->MinShadowFalloffRadius = OwnerBounds.SphereRadius * (Component->LightDistance + 1);

		if(CurrentRepresentativeShadowLight->IsAttached())
		{
			// Move the light to the new position.
			CurrentRepresentativeShadowLight->ConditionalUpdateTransform(LightToWorld);
			
			// Update the light's color and brightness.
			Component->GetScene()->UpdateLightColorAndBrightness(CurrentRepresentativeShadowLight);
		}
		else
		{
			// Attach the light after it is associated with the light environment to ensure it is only attached once.
			CurrentRepresentativeShadowLight->ConditionalAttach(Component->GetScene(),NULL,LightToWorld);
		}
	}
}

void FDynamicLightEnvironmentState::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	// Add the light environment's representative lights.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		UObject::AddReferencedObject(ObjectArray,RepresentativeLightPool(LightIndex));
	}
}

/** Forces a full update on the next Tick. */
void FDynamicLightEnvironmentState::ResetEnvironment()
{
	bFirstFullUpdate = TRUE;
}

UBOOL FDynamicLightEnvironmentState::IsLightVisible(const ULightComponent* Light,const FVector& OwnerPosition,FLOAT& OutVisibilityFactor)
{
	SCOPE_CYCLE_COUNTER(STAT_LightVisibilityTime);

	// Sky lights are always visible.
	if(Light->IsA(USkyLightComponent::StaticClass()))
	{
		OutVisibilityFactor = 1.0f;
		return TRUE;
	}

	// Lights which don't cast static shadows are always visible.
	if(!Light->CastShadows || !Light->CastStaticShadows)
	{
		OutVisibilityFactor = 1.0f;
		return TRUE;
	}

	// Get the owning actor
	AActor* OwnerActor = Component ? Component->GetOwner() : NULL;

	// Compute light visibility for one or more points within the owner's bounds.
	INT NumVisibleSamples = 0;
	for(INT SampleIndex = 0;SampleIndex < LightVisibilitySamplePoints.Num();SampleIndex++)
	{
		FVector StartPosition = PredictedOwnerPosition;
		FVector4 LightPosition = Light->GetPosition();
		if (Component->bTraceFromClosestBoundsPoint)
		{
			FVector LightVector = (FVector)LightPosition - PredictedOwnerPosition * LightPosition.W;
			LightVector.Normalize();
			StartPosition = PredictedOwnerPosition + LightVector * OwnerBounds.SphereRadius;
		}
		// Determine a random point to test visibility for in the owner's bounds.
		const FVector VisibilityTestPoint = StartPosition + LightVisibilitySamplePoints(SampleIndex) * OwnerBounds.BoxExtent;

		// Determine the direction from the primitive to the light.
		FVector LightVector = (FVector)LightPosition - VisibilityTestPoint * LightPosition.W;

		// Check the line between the light and the primitive's origin for occlusion.
		FCheckResult Hit(1.0f);
		const UBOOL bPointIsLit = GWorld->SingleLineCheck(
			Hit,
			OwnerActor,
			VisibilityTestPoint,
			VisibilityTestPoint + LightVector,
			TRACE_Level|TRACE_Actors|TRACE_ShadowCast|TRACE_StopAtAnyHit,
			FVector(0,0,0),
			const_cast<ULightComponent*>(Light)
			);
		if(bPointIsLit)
		{
			NumVisibleSamples++;
		}
	}

	OutVisibilityFactor = (FLOAT)NumVisibleSamples / (FLOAT)LightVisibilitySamplePoints.Num();

	return OutVisibilityFactor > 0.0f;
}

template<typename LightType>
LightType* FDynamicLightEnvironmentState::AllocateLight() const
{
	// Try to find an unattached light of matching type in the representative light pool.
	for(INT LightIndex = 0;LightIndex < RepresentativeLightPool.Num();LightIndex++)
	{
		ULightComponent* Light = RepresentativeLightPool(LightIndex);
		if(Light && !Light->IsAttached() && Light->IsA(LightType::StaticClass()))
		{
			return CastChecked<LightType>(Light);
		}
	}

	// Create a new light.
	LightType* NewLight = ConstructObject<LightType>(LightType::StaticClass(),Component);
	RepresentativeLightPool.AddItem(NewLight);
	return NewLight;
}

UBOOL FDynamicLightEnvironmentState::DoesLightAffectOwner(const ULightComponent* Light,const FVector& OwnerPosition)
{
	// Skip disabled lights.
	if(!Light->bEnabled)
	{
		return FALSE;
	}

	// Use the CompositeDynamic lighting channel as the Dynamic lighting channel. 
	FLightingChannelContainer ConvertedLightingChannels = Light->LightingChannels;
	ConvertedLightingChannels.Dynamic = FALSE;
	if(ConvertedLightingChannels.CompositeDynamic)
	{
		ConvertedLightingChannels.CompositeDynamic = FALSE;
		ConvertedLightingChannels.Dynamic = TRUE;
	}

	// Skip lights which don't affect the owner's lighting channels.
	if(!ConvertedLightingChannels.OverlapsWith(OwnerLightingChannels))
	{
		return FALSE;
	}

	// Skip lights which don't affect the owner's level.
	if(!Light->AffectsLevel(OwnerPackage))
	{
		return FALSE;
	}

	// Skip lights which don't affect the owner's predicted bounds.
	if(!Light->AffectsBounds(FBoxSphereBounds(OwnerPosition,OwnerBounds.BoxExtent,OwnerBounds.SphereRadius)))
	{
		return FALSE;
	}

	return TRUE;
}

void FDynamicLightEnvironmentState::AddLightToEnvironment(
	const ULightComponent* Light, 
	FSHVectorRGB& ShadowedLightEnvironment,
	FSHVectorRGB& NonShadowedLightEnvironment,
	FSHVectorRGB& ShadowEnvironment,
	const FVector& OwnerPosition,
	UBOOL bIsDynamic
	)
{
	// Determine whether the light affects the owner, and its visibility factor.
	FLOAT VisibilityFactor;
	if(DoesLightAffectOwner(Light,OwnerPosition) && IsLightVisible(Light,OwnerPosition,VisibilityFactor))
	{
		// If the light doesn't cast composite shadows, add its lighting to the light environment SH that isn't modulated by the composite shadow.
		FSHVectorRGB& LightEnvironment =
			Light->bCastCompositeShadow ? 
				ShadowedLightEnvironment :
				NonShadowedLightEnvironment;

		if(Light->IsA(USkyLightComponent::StaticClass()))
		{
			const USkyLightComponent* SkyLight = ConstCast<USkyLightComponent>(Light);

			// Compute the sky light's effect on the environment SH.
			const FSHVectorRGB IndividualSkyLightEnvironment = GetSkyLightEnvironment(Component,SkyLight);

			// Add the sky light to the light environment SH.
			LightEnvironment += IndividualSkyLightEnvironment;

			if(Light->bCastCompositeShadow)
			{
				FSHVectorRGB IndividualShadowEnvironment;
				if(!Light->bAffectCompositeShadowDirection)
				{
					// If the sky light is set to not affect the shadow direction, add it to the shadow environment as an ambient light.
					IndividualShadowEnvironment = FSHVector::AmbientFunction() * IndividualSkyLightEnvironment.CalcIntegral();
				}
				else
				{
					// Add the sky light to the shadow environment SH.
					IndividualShadowEnvironment = IndividualSkyLightEnvironment;
				}

				// Use the ModShadowColor to blend between adding the light directly to the light environment or as an ambient shadow source that
				// lightens the composite shadow.
				ShadowEnvironment += FSHVector::AmbientFunction() * (IndividualShadowEnvironment.CalcIntegral() * Light->ModShadowColor);
				ShadowEnvironment += IndividualShadowEnvironment * (FLinearColor::White - Light->ModShadowColor);
			}
		}
		else
		{
			// Determine the direction from the primitive to the light.
			const FVector4 LightPosition = Light->GetPosition();
			const FVector LightVector = (FVector)LightPosition - OwnerPosition * LightPosition.W;

			// Compute the light's intensity at the actor's origin.
			const FLinearColor Intensity = Light->GetDirectIntensity(OwnerPosition) * VisibilityFactor;
			const FLinearColor BouncedIntensity = Light->GetBouncedIntensity(OwnerPosition) * VisibilityFactor;

			const UBOOL bUseCompositeDynamicLights = (GSystemSettings.bUseCompositeDynamicLights && !Component->bForceNonCompositeDynamicLights);
			if(!bIsDynamic || bUseCompositeDynamicLights)
			{
				// Add the light to the light environment SH.
				const FSHVectorRGB IndividualLightEnvironment = SHBasisFunction( LightVector.SafeNormal()) * Intensity;
				LightEnvironment += IndividualLightEnvironment;
			}

			// Add the bounced light to the non-shadowed light environment SH.
			const FSHVectorRGB IndividualBouncedLightEnvironment =
				SHBasisFunction(-LightVector.SafeNormal()) * BouncedIntensity * Component->BouncedLightingFactor;
			NonShadowedLightEnvironment += IndividualBouncedLightEnvironment;

			if(Light->bCastCompositeShadow)
			{
				FSHVectorRGB IndividualShadowEnvironment;
				if(Light->bAffectCompositeShadowDirection)
				{
					// Don't allow the shadow to come from below the light environment's minimum shadow angle.
					FVector ShadowVector = LightVector.SafeNormal();
					const FLOAT MinShadowZ = appSin(Component->MinShadowAngle * PI / 180.0f);
					if(ShadowVector.Z < MinShadowZ)
					{
						ShadowVector.Z = 0.0f;
						ShadowVector.Normalize();
						ShadowVector.Z = appTan(Component->MinShadowAngle * PI / 180.0f);
					}

					// Add the light to the shadow casting environment SH.
					IndividualShadowEnvironment = SHBasisFunction(ShadowVector.SafeNormal()) * Intensity;
				}
				else
				{
					// Add the light to the shadow casting environment SH as an ambient light.
					IndividualShadowEnvironment = FSHVector::AmbientFunction() * (Intensity * (1.0f / (2.0f * appSqrt(PI))));
				}

				// Use the ModShadowColor to blend between adding the light directly to the light environment or as an ambient shadow source that
				// lightens the composite shadow.
				ShadowEnvironment += FSHVector::AmbientFunction() * (IndividualShadowEnvironment.CalcIntegral() * Light->ModShadowColor);
				ShadowEnvironment += IndividualShadowEnvironment * (FLinearColor::White - Light->ModShadowColor);
			}
		}
	}
}

void UDynamicLightEnvironmentComponent::FinishDestroy()
{
	Super::FinishDestroy();

	// Clean up the light environment's state.
	delete State;
	State = NULL;
}

void UDynamicLightEnvironmentComponent::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects(ObjectArray);

	if(State)
	{
		State->AddReferencedObjects(ObjectArray);
	}
}

void UDynamicLightEnvironmentComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if(!Ar.IsSaving() && !Ar.IsLoading())
	{
		// If serialization is being used to find references for garbage collection, use AddReferencedObjects to gather a list to serialize.
		TArray<UObject*> ReferencedObjects;
		AddReferencedObjects(ReferencedObjects);
		Ar << ReferencedObjects;
	}
}

void UDynamicLightEnvironmentComponent::Tick(FLOAT DeltaTime)
{
	SCOPE_CYCLE_COUNTER( STAT_DynamicLightEnvironmentComponentTickTime );

	Super::Tick(DeltaTime);

	if(bEnabled)
	{
#if !FINAL_RELEASE
		extern UBOOL GShouldLofOutAFrameOfLightEnvTick;
		if(GShouldLofOutAFrameOfLightEnvTick)
		{
			debugf(TEXT("LE: %s %s %d"), *GetPathName(), *GetDetailedInfo(), bDynamic);
		}
#endif

		// If the light environment requires non-latent updates, ensure that the light environment is
		// ticked after everything is in its final location for the frame.
		if(bRequiresNonLatentUpdates && TickGroup != TG_PostUpdateWork)
		{
			SetTickGroup(TG_PostUpdateWork);
		}

		// Update the light environment's state.
		check(State);
		State->Tick(DeltaTime);
	}
}

void UDynamicLightEnvironmentComponent::Attach()
{
	Super::Attach();

	if(bEnabled)
	{
		// Initialize the light environment's state the first time it's attached.
		if(!State)
		{
			State = new FDynamicLightEnvironmentState(this);
		}

		// Outside the game we're not ticked, so update the light environment on attach.
		if(!GIsGame)
		{
			State->Update();
		}

		// Add the light environment to the world's list, so it can be updated when static lights change.
		if(!GIsGame && Scene->GetWorld())
		{
			Scene->GetWorld()->LightEnvironmentList.AddItem(this);
		}

		// Recreate the lights.
		State->CreateEnvironmentLightList(TRUE);
	}
}

void UDynamicLightEnvironmentComponent::UpdateTransform()
{
	Super::UpdateTransform();

	if(bEnabled)
	{
		// Outside the game we're not ticked, so update the light environment on attach.
		if(!GIsGame)
		{
			State->Update();
		}
	}
}

void UDynamicLightEnvironmentComponent::Detach( UBOOL bWillReattach )
{
	Super::Detach( bWillReattach );

	// Remove the light environment from the world's list.
	if(!GIsGame && Scene->GetWorld())
	{
		for(TSparseArray<ULightEnvironmentComponent*>::TIterator It(Scene->GetWorld()->LightEnvironmentList);It;++It)
		{
			if(*It == this)
			{
				Scene->GetWorld()->LightEnvironmentList.Remove(It.GetIndex());
				break;
			}
		}
	}

	if(State)
	{
		// Detach the light environment's representative lights.
		State->DetachRepresentativeLights();
	}
}

void UDynamicLightEnvironmentComponent::UpdateLight(const ULightComponent* Light)
{
	if(bEnabled && IsAttached())
	{
		if(!GIsGame)
		{
			// Outside the game, update the environment if any light changes.
			BeginDeferredUpdateTransform();
		}
	}
}

/** Forces a full update on the next Tick. */
void UDynamicLightEnvironmentComponent::ResetEnvironment()
{
	if (State)
	{
		State->ResetEnvironment();
	}
}
