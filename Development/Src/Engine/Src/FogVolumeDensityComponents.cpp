/*=============================================================================
	FogVolumeDensityComponents.cpp: Native implementations of fog volume components.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "EngineFogVolumeClasses.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(AFogVolumeDensityInfo);

void AFogVolumeDensityInfo::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.Ver() < VER_AUTOMATIC_FOGVOLUME_COMPONENT)
	{
		// Remove the reference to AutomaticMeshComponent for fog volume actors that were added before the automatic component change
		AutomaticMeshComponent = NULL;
	}
}

void AFogVolumeDensityInfo::PostLoad()
{
	Super::PostLoad();

	// AutomaticMeshComponent can be NULL if the user cleared it or if the Fog Volume actor was placed in a level before the automatic component change
	if (AutomaticMeshComponent == NULL)
	{
		// if AutomaticMeshComponent is NULL, make sure there are no static meshes in the Components array
		UStaticMeshComponent* AutomaticComponent;
		INT AutomaticComponentIndex = INDEX_NONE;
		if (Components.FindItemByClass<UStaticMeshComponent>(&AutomaticComponent, &AutomaticComponentIndex))
		{
			Components.Remove(AutomaticComponentIndex);
		}
	}
}

IMPLEMENT_CLASS(UFogVolumeDensityComponent);

/** Adds the fog volume components to the scene */
void UFogVolumeDensityComponent::AddFogVolumeComponents()
{
	for (INT ActorIndex = 0; ActorIndex < FogVolumeActors.Num(); ActorIndex++)
	{
		AActor* CurrentActor = FogVolumeActors(ActorIndex);
		if (CurrentActor)
		{
			for (INT ComponentIndex = 0; ComponentIndex < CurrentActor->Components.Num(); ComponentIndex++)
			{
				// AActor::Components can contain NULL entries in game
				if (CurrentActor->Components(ComponentIndex) != NULL && CurrentActor->Components(ComponentIndex)->IsA(UPrimitiveComponent::StaticClass()))
				{
					UPrimitiveComponent* CurrentComponent = Cast<UPrimitiveComponent>(CurrentActor->Components(ComponentIndex));
					Scene->AddFogVolume(this, CurrentComponent);
					CurrentComponent->FogVolumeComponent = this;
				}
			}
		}
	}
}

/** Removes the fog volume components from the scene */
void UFogVolumeDensityComponent::RemoveFogVolumeComponents()
{
	for (INT ActorIndex = 0; ActorIndex < FogVolumeActors.Num(); ActorIndex++)
	{
		AActor* CurrentActor = FogVolumeActors(ActorIndex);
		if (CurrentActor)
		{
			for (INT ComponentIndex = 0; ComponentIndex < CurrentActor->Components.Num(); ComponentIndex++)
			{
				if (CurrentActor->Components(ComponentIndex) != NULL && CurrentActor->Components(ComponentIndex)->IsA(UPrimitiveComponent::StaticClass()))
				{
					UPrimitiveComponent* CurrentComponent = Cast<UPrimitiveComponent>(CurrentActor->Components(ComponentIndex));
					Scene->RemoveFogVolume(CurrentComponent);
					CurrentComponent->FogVolumeComponent = NULL;
				}
			}
		}
	}
}

/** 
 * Sets up FogVolumeActors's mesh components to defaults that are common usage with fog volumes.  
 * Collision is disabled for the actor, each component gets assigned the default fog volume material,
 * lighting, shadowing, decal accepting, and occluding are disabled.
 */
void UFogVolumeDensityComponent::SetFogActorDefaults()
{
	for (INT ActorIndex = 0; ActorIndex < FogVolumeActors.Num(); ActorIndex++)
	{
		AActor* CurrentActor = FogVolumeActors(ActorIndex);
		if (CurrentActor)
		{
			CurrentActor->CollisionType = COLLIDE_NoCollision;
			CurrentActor->BlockRigidBody = FALSE;
			CurrentActor->bNoEncroachCheck = TRUE;
			for (INT ComponentIndex = 0; ComponentIndex < CurrentActor->Components.Num(); ComponentIndex++)
			{
				UMeshComponent* CurrentComponent = Cast<UMeshComponent>(CurrentActor->Components(ComponentIndex));
				if (CurrentComponent)
				{
					if (GEngine->DefaultFogVolumeMaterial)
					{
						CurrentComponent->SetMaterial(0, GEngine->DefaultFogVolumeMaterial);
					}

					CurrentComponent->BlockRigidBody = FALSE;
					CurrentComponent->bForceDirectLightMap = FALSE;
					CurrentComponent->bAcceptsDynamicLights = FALSE;
					CurrentComponent->bAcceptsLights = FALSE;
					CurrentComponent->bCastDynamicShadow = FALSE;
					CurrentComponent->CastShadow = FALSE;
					CurrentComponent->bUsePrecomputedShadows = FALSE;
					CurrentComponent->bAcceptsStaticDecals = FALSE;
					CurrentComponent->bAcceptsDynamicDecals = FALSE;
					CurrentComponent->bUseAsOccluder = FALSE;
				}

				UStaticMeshComponent* CurrentStaticMeshComponent = Cast<UStaticMeshComponent>(CurrentActor->Components(ComponentIndex));
				if (CurrentStaticMeshComponent)
				{
					CurrentStaticMeshComponent->WireframeColor = FColor(100,100,200,255);
				}
			}
		}
	}
}

void UFogVolumeDensityComponent::Attach()
{
	Super::Attach();

	if(bEnabled)
	{
		AFogVolumeDensityInfo* FogOwner = CastChecked<AFogVolumeDensityInfo>(Owner);
		// Setup the AutomaticMeshComponent
		if (FogOwner->AutomaticMeshComponent)
		{
			// Use the default fog volume material if none has been set
			if (FogMaterial == NULL)
			{
				FogOwner->AutomaticMeshComponent->SetMaterial(0, DefaultFogVolumeMaterial);
			}
			else
			{
				UMaterialInstance* FogMI = Cast<UMaterialInstance>(FogMaterial);
				if (FogMI)
				{
					// If FogMaterial is a UMaterialInstance, set its EmissiveColor parameter to be SimpleLightColor
					static FName SimpleColorParam = FName(TEXT("EmissiveColor"));
					FogMI->SetVectorParameterValue(SimpleColorParam, SimpleLightColor);
				}
				FogOwner->AutomaticMeshComponent->SetMaterial(0, FogMaterial);
			}

			FogOwner->AutomaticMeshComponent->FogVolumeComponent = this;
			Scene->AddFogVolume(this, FogOwner->AutomaticMeshComponent);
		}

		AddFogVolumeComponents();
	}
}

void UFogVolumeDensityComponent::UpdateTransform()
{
	Super::UpdateTransform();
	RemoveFogVolumeComponents();

	if (bEnabled)
	{
		AddFogVolumeComponents();
	}
}

void UFogVolumeDensityComponent::Detach( UBOOL bWillReattach )
{
	Super::Detach( bWillReattach );

	AFogVolumeDensityInfo* FogOwner = CastChecked<AFogVolumeDensityInfo>(Owner);
	if (FogOwner->AutomaticMeshComponent)
	{
		Scene->RemoveFogVolume(FogOwner->AutomaticMeshComponent);
		FogOwner->AutomaticMeshComponent->FogVolumeComponent = NULL;
	}

	RemoveFogVolumeComponents();
}

void UFogVolumeDensityComponent::SetEnabled(UBOOL bSetEnabled)
{
	if(bEnabled != bSetEnabled)
	{
		// Update bEnabled, and begin a deferred component reattach.
		bEnabled = bSetEnabled;
		BeginDeferredReattach();
	}
}

/** 
 * Sets defaults on all FogVolumeActors when one of the changes
 */
void UFogVolumeDensityComponent::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0)
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if (PropertyName == TEXT("FogVolumeActors"))
			{
				//unfortunately there's no way with the current property system to only set the defaults on the changed actor!
				//property to actual variable mapping has to be done with name comparisons, and no index into arrays is stored
				SetFogActorDefaults();
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/** 
 * Checks for partial fog volume setup that will not render anything.
 */
void UFogVolumeDensityComponent::CheckForErrors()
{
	Super::CheckForErrors();

	for (INT ActorIndex = 0; ActorIndex < FogVolumeActors.Num(); ActorIndex++)
	{
		AActor* CurrentActor = FogVolumeActors(ActorIndex);
		if (CurrentActor)
		{
			for (INT ComponentIndex = 0; ComponentIndex < CurrentActor->Components.Num(); ComponentIndex++)
			{
				UMeshComponent* CurrentComponent = Cast<UMeshComponent>(CurrentActor->Components(ComponentIndex));
				if (CurrentComponent && CurrentComponent->Materials.Num() > 0 && CurrentComponent->Materials(0) != NULL)
				{
					UMaterial* BaseMaterial = CurrentComponent->Materials(0)->GetMaterial(MSP_SM3);
					if (BaseMaterial)
					{
						if (!BaseMaterial->bUsedWithFogVolumes
							|| BaseMaterial->LightingModel != MLM_Unlit
							|| (BaseMaterial->BlendMode != BLEND_Translucent && BaseMaterial->BlendMode != BLEND_Additive && BaseMaterial->BlendMode != BLEND_Modulate)
							|| (!BaseMaterial->EmissiveColor.UseConstant && BaseMaterial->EmissiveColor.Expression == NULL))
						{
							GWarn->MapCheck_Add( 
								MCTYPE_WARNING, 
								CurrentActor, 
								*FString::Printf(TEXT("FogVolumeActor's Material is not setup to be used with Fog Volumes!")), 
								MCACTION_NONE, 
								TEXT("FogVolumeMaterialNotSetupCorrectly") );
						}
					}
					
				}
			}
		}
	}
}

UMaterialInterface*	UFogVolumeDensityComponent::GetMaterial() const
{
	if (FogMaterial == NULL)
	{
		return DefaultFogVolumeMaterial;
	}
	else
	{
		return FogMaterial;
	}
}

IMPLEMENT_CLASS(AFogVolumeConstantDensityInfo);
IMPLEMENT_CLASS(UFogVolumeConstantDensityComponent);

/** 
* Creates a copy of the data stored in this component to be used by the rendering thread. 
* This memory is released in FScene::RemoveFogVolume by the rendering thread. 
*/
FFogVolumeDensitySceneInfo* UFogVolumeConstantDensityComponent::CreateFogVolumeDensityInfo(const UPrimitiveComponent* MeshComponent) const
{
	// Only create a density info if there is work to be done.
	if( Density > 0 )
	{
		return new FFogVolumeConstantDensitySceneInfo(this, MeshComponent->Bounds.GetBox(), MeshComponent->GetStaticDepthPriorityGroup());
	}
	else
	{
		return NULL;
	}
}

IMPLEMENT_CLASS(AFogVolumeLinearHalfspaceDensityInfo);
IMPLEMENT_CLASS(UFogVolumeLinearHalfspaceDensityComponent);

void UFogVolumeLinearHalfspaceDensityComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	Super::SetParentToWorld(ParentToWorld);
	//get the plane height
	FLOAT W = -ParentToWorld.GetOrigin().Z;
	//get the plane normal
	FVector4 T = ParentToWorld.TransformNormal(FVector(0.0f,0.0f,1.0f));
	FVector PlaneNormal = FVector(T);
	PlaneNormal.Normalize();
	HalfspacePlane = FPlane(PlaneNormal, W);
}

/** 
* Creates a copy of the data stored in this component to be used by the rendering thread. 
* This memory is released in FScene::RemoveFogVolume by the rendering thread. 
*/
FFogVolumeDensitySceneInfo* UFogVolumeLinearHalfspaceDensityComponent::CreateFogVolumeDensityInfo(const UPrimitiveComponent* MeshComponent) const
{
	// Only create a density info if there is work to be done.
	if( PlaneDistanceFactor > 0 )
	{
		return new FFogVolumeLinearHalfspaceDensitySceneInfo(this, MeshComponent->Bounds.GetBox(), MeshComponent->GetStaticDepthPriorityGroup());
	}
	else
	{
		return NULL;
	}
}


IMPLEMENT_CLASS(AFogVolumeSphericalDensityInfo);
IMPLEMENT_CLASS(UFogVolumeSphericalDensityComponent);

void UFogVolumeSphericalDensityComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	Super::SetParentToWorld(ParentToWorld);
	SphereCenter = ParentToWorld.GetOrigin();
}

void UFogVolumeSphericalDensityComponent::Attach()
{
	// Use the owner's scale to calculate SphereRadius
	SphereRadius = 600.0f * Owner->DrawScale * (Owner->DrawScale3D.X + Owner->DrawScale3D.Y + Owner->DrawScale3D.Z) / 3.0f;
	Super::Attach();

	if ( PreviewSphereRadius )
	{
		PreviewSphereRadius->SphereRadius = SphereRadius;
	}
}

/** 
* Creates a copy of the data stored in this component to be used by the rendering thread. 
* This memory is released in FScene::RemoveFogVolume by the rendering thread. 
*/
FFogVolumeDensitySceneInfo* UFogVolumeSphericalDensityComponent::CreateFogVolumeDensityInfo(const UPrimitiveComponent* MeshComponent) const
{
	// Only create a density info if there is work to be done.
	if( MaxDensity > 0 )
	{
		return new FFogVolumeSphericalDensitySceneInfo(this, MeshComponent->Bounds.GetBox(), MeshComponent->GetStaticDepthPriorityGroup());
	}
	else
	{
		return NULL;
	}
}

IMPLEMENT_CLASS(AFogVolumeConeDensityInfo);

void AFogVolumeConeDensityInfo::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	UFogVolumeConeDensityComponent* FogDensityComponent = Cast<UFogVolumeConeDensityComponent>( DensityComponent );
	check( FogDensityComponent );
	
	if (bCtrlDown || bAltDown)
	{
		const FVector ModifiedScale = DeltaScale;
		const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;
		FogDensityComponent->ConeMaxAngle += Multiplier * ModifiedScale.Size();
		FogDensityComponent->ConeMaxAngle = Max( 0.0f, FogDensityComponent->ConeMaxAngle );
		FogDensityComponent->ConeMaxAngle = Min( 89.0f, FogDensityComponent->ConeMaxAngle );
	}
	else
	{
		const FVector ModifiedScale = DeltaScale * 500.0f;
		const FLOAT Multiplier = ( ModifiedScale.X > 0.0f || ModifiedScale.Y > 0.0f || ModifiedScale.Z > 0.0f ) ? 1.0f : -1.0f;
		FogDensityComponent->ConeRadius += Multiplier * ModifiedScale.Size();
		FogDensityComponent->ConeRadius = Max( 0.0f, FogDensityComponent->ConeRadius );
	}
	PostEditChange( NULL );
}

IMPLEMENT_CLASS(UFogVolumeConeDensityComponent);

void UFogVolumeConeDensityComponent::SetParentToWorld(const FMatrix& ParentToWorld)
{
	Super::SetParentToWorld(ParentToWorld);
	ConeVertex = ParentToWorld.GetOrigin();
	FVector4 T = ParentToWorld.TransformNormal(FVector(1.0f,0.0f,0.0f));
	ConeAxis = FVector(T);
	ConeAxis.Normalize();
}

void UFogVolumeConeDensityComponent::Attach()
{
	Super::Attach();
	if ( PreviewCone )
	{
		PreviewCone->ConeRadius = ConeRadius;
	}
}

/** 
* Creates a copy of the data stored in this component to be used by the rendering thread. 
* This memory is released in FScene::RemoveFogVolume by the rendering thread. 
*/
FFogVolumeDensitySceneInfo* UFogVolumeConeDensityComponent::CreateFogVolumeDensityInfo(const UPrimitiveComponent* MeshComponent) const
{
	// Only create a density info if there is work to be done.
	if( MaxDensity > 0 )
	{
		return new FFogVolumeConeDensitySceneInfo(this, MeshComponent->Bounds.GetBox(), MeshComponent->GetStaticDepthPriorityGroup());
	}
	else
	{
		return NULL;
	}
}


