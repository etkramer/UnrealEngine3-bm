//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "EngineAnimClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameAIClasses.h"
#include "UnPath.h"

IMPLEMENT_CLASS(AUTWeapon);
IMPLEMENT_CLASS(AUTWeaponShield);
IMPLEMENT_CLASS(AUTWeaponPickupFactory);
IMPLEMENT_CLASS(AUTProjectile);
IMPLEMENT_CLASS(UUTExplosionLight);
IMPLEMENT_CLASS(AUTWeap_ImpactHammer);
IMPLEMENT_CLASS(AUTVWeap_TowCable);
IMPLEMENT_CLASS(AUTWeap_FlakCannon);
IMPLEMENT_CLASS(AUTProj_ScorpionGlob_Base);
IMPLEMENT_CLASS(AUTProj_Rocket);
IMPLEMENT_CLASS(AUTProj_SeekingRocket);
IMPLEMENT_CLASS(AUTWeap_RocketLauncher);
IMPLEMENT_CLASS(AUTProj_FlakShard);
IMPLEMENT_CLASS(AUTAmmo_BioRifle);
IMPLEMENT_CLASS(AUTBeamWeapon);
IMPLEMENT_CLASS(AUTWeap_LinkGun);

UBOOL AUTWeapon::Tick( FLOAT DeltaSeconds, ELevelTick TickType )
{
	CustomTimeDilation = Instigator ? Instigator->CustomTimeDilation : 1.f;
	return Super::Tick(DeltaSeconds, TickType);
}

/** Util that makes sure the overlay component is last in the AllComponents and Components arrays. */
void AUTWeapon::EnsureWeaponOverlayComponentLast()
{
	// Iterate over Components array looking for OverlayMesh
	for(INT i=0; i<Components.Num(); i++)
	{
		if(Components(i) == OverlayMesh)
		{
			// When/if we find it, remove it and add to the end instead.
			Components.Remove(i);
			Components.AddItem(OverlayMesh);
			continue;
		}
	}

	// Iterate over AllComponents array looking for OverlayMesh
	for(INT i=0; i<AllComponents.Num(); i++)
	{
		if(AllComponents(i) == OverlayMesh)
		{
			// When/if we find it, remove it and add to the end instead.
			AllComponents.Remove(i);
			AllComponents.AddItem(OverlayMesh);
			continue;
		}
	}
}

UBOOL AUTWeaponShield::IgnoreBlockingBy(const AActor* Other) const
{
	return Other->GetAProjectile() ? (bIgnoreFlaggedProjectiles && Other->GetAProjectile()->bNotBlockedByShield) : TRUE;
}

UBOOL AUTWeaponShield::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	return (SourceActor != NULL && ((TraceFlags & TRACE_ComplexCollision) || SourceActor->GetAProjectile() || SourceActor->IsA(AWeapon::StaticClass())) && !IsOwnedBy(SourceActor)) ? TRUE : FALSE;
}

void AUTWeap_ImpactHammer::TickSpecial( FLOAT DeltaTime )
{
	Super::TickSpecial(DeltaTime);

	if ( bIsCurrentlyCharging && (WorldInfo->TimeSeconds - ChargeTime > MinChargeTime) )
	{
		// @todo fixmesteve - move the trace to TickSpecial as well
		eventImpactAutoFire();
	}
}

void AUTWeap_FlakCannon::TickSpecial(FLOAT DeltaSeconds)
{

	INT desiredTensOdometer;
	INT desiredOnesOdometer;
	INT OdometerDiffOnes, OdometerDiffTens;
	USkelControlSingleBone* SkelControl;
	
	Super::TickSpecial(DeltaSeconds);
	if (Instigator && Instigator->Weapon == this && Instigator->IsHumanControlled() && Instigator->IsLocallyControlled())
	{
		desiredOnesOdometer = (AmmoCount%10)*-6554;
		desiredTensOdometer = (AmmoCount/10)*-6554;
		
		// if we're where we want, we're done.
		if(curTensOdometer == desiredTensOdometer && curOnesOdometer == desiredOnesOdometer)
			return;

		OdometerDiffOnes = appTrunc(OdometerMaxPerSecOnes *DeltaSeconds);
		OdometerDiffTens = appTrunc(OdometerMaxPerSecTens *DeltaSeconds);

		//wrap around (range 0 through -65535)
		if(curOnesOdometer> 0)
		{
			curOnesOdometer = (curOnesOdometer)-65535;
		}
		if(curTensOdometer > 0)
		{
			curTensOdometer = (curTensOdometer)-65535;
		}
		if(curOnesOdometer < -65535)
		{
			curOnesOdometer= (curOnesOdometer)+65535;
		}
		if(curTensOdometer < -65535)
		{
			curTensOdometer=(curTensOdometer)+65535;
		}
		
		// deal with direction
		if((desiredOnesOdometer - curOnesOdometer) > 32768)
		{
			curOnesOdometer += 65536;	
		}
		else if((desiredOnesOdometer - curOnesOdometer) < -32768)
		{
			curOnesOdometer -= 65536;
		}
		if(curOnesOdometer-desiredOnesOdometer < 0) // opposite way
		{
			OdometerDiffOnes *= -1;
		}
		
		if((desiredTensOdometer - curTensOdometer) > 32768)
		{
			curTensOdometer += 65536;	
		}
		else if((desiredTensOdometer - curTensOdometer) < -32768)
		{
			curTensOdometer -= 65536;
		}
		if(curTensOdometer-desiredTensOdometer < 0) // opposite way
		{
			OdometerDiffTens *= -1;
		}		
		// then deal with overshoot,
		if(Abs(curOnesOdometer-desiredOnesOdometer) < Abs(OdometerDiffOnes))  // if we'd overshoot, go straight there
		{
			curOnesOdometer = desiredOnesOdometer;
		}
		else // otherwise move as far as we can
		{
			curOnesOdometer -= OdometerDiffOnes;
		}
		if(Abs(curTensOdometer-desiredTensOdometer) < Abs(OdometerDiffTens))
		{
			curTensOdometer = desiredTensOdometer;
		}
		else
		{
			curTensOdometer -= OdometerDiffTens;
		}
		
		// finally, set it to the new value:
		SkelControl = (USkelControlSingleBone*)SkeletonFirstPersonMesh->FindSkelControl(OnesPlaceSkelName);
		if(SkelControl != NULL)
		{
			SkelControl->BoneRotation.Pitch = curOnesOdometer;
		}
		SkelControl = (USkelControlSingleBone*)(SkeletonFirstPersonMesh->FindSkelControl(TensPlaceSkelName));
		if(SkelControl != NULL)
		{
			SkelControl->BoneRotation.Pitch = curTensOdometer;
		}
	}
}

void AUTWeap_LinkGun::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	// auto recharge
	if ( (Role == ROLE_Authority) && (AmmoCount < MaxAmmoCount) && bAutoCharge && (WorldInfo->TimeSeconds - LastFireTime > 1.f) )
	{
		PartialCharge += DeltaSeconds * RechargeRate;
		if ( PartialCharge >= 1.f )
		{
			PartialCharge -= 1.f;
			AmmoCount += 1;
		}
	}
}

/*-----------------------------------------------------------------------------
	FUTSkeletalMeshSceneProxy
	Support for rendering weapon at different FOV than world
-----------------------------------------------------------------------------*/

/**
 * A UT skeletal mesh component scene proxy.
 */
class FUTSkeletalMeshSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	FUTSkeletalMeshSceneProxy(const USkeletalMeshComponent* Component, FLOAT InFOV )
	:	FSkeletalMeshSceneProxy(Component)
	,	FOV(InFOV)
	{
	}
	virtual ~FUTSkeletalMeshSceneProxy()
	{
	}

	/**
	 * Returns the world transform to use for drawing.
	 * @param View - Current view
	 * @param OutLocalToWorld - Will contain the local-to-world transform when the function returns.
	 * @param OutWorldToLocal - Will contain the world-to-local transform when the function returns.
	 */
	virtual void GetWorldMatrices( const FSceneView* View, FMatrix& OutLocalToWorld, FMatrix& OutWorldToLocal )
	{
		if (FOV != 0.0f)
		{
			const FMatrix LocalToView = LocalToWorld * View->ViewMatrix;
			const FMatrix ViewToWarpedView =
				FPerspectiveMatrix(FOV * PI / 360.0f, View->SizeX, View->SizeY, View->NearClippingDistance) *
				View->ProjectionMatrix.Inverse();

			OutLocalToWorld = LocalToView * ViewToWarpedView * View->ViewMatrix.Inverse();
			OutWorldToLocal = OutLocalToWorld.Inverse();
		}
		else
		{
			OutLocalToWorld = LocalToWorld;
			OutWorldToLocal = LocalToWorld.Inverse();
		}
	}

	FORCEINLINE void SetFOV(FLOAT NewFOV)
	{
		FOV = NewFOV;
	}

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FSkeletalMeshSceneProxy::GetAllocatedSize() ); }

private:
    FLOAT FOV;
};

FPrimitiveSceneProxy* UUTSkeletalMeshComponent::CreateSceneProxy()
{
	FSkeletalMeshSceneProxy* Result = NULL;

	// Only create a scene proxy for rendering if properly initialized
	if( SkeletalMesh && 
		SkeletalMesh->LODModels.IsValidIndex(PredictedLODLevel) &&
		!bHideSkin &&
		MeshObject )
	{
		Result = ::new FUTSkeletalMeshSceneProxy(this, FOV);
	}

	return Result;
}

void UUTSkeletalMeshComponent::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( bForceLoadTextures && (ClearStreamingTime < GWorld->GetWorldInfo()->TimeSeconds) )
	{
		eventPreloadTextures(FALSE, 0.f);
	}
}

void UUTSkeletalMeshComponent::SetFOV(FLOAT NewFOV)
{
	if (FOV != NewFOV)
	{
		FOV = NewFOV;
		if (SceneInfo != NULL)
		{
			// tell the rendering thread to update the proxy's FOV
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER( UpdateFOVCommand, FPrimitiveSceneProxy*, Proxy, Scene_GetProxyFromInfo(SceneInfo), FLOAT, NewFOV, FOV,
														{
															((FUTSkeletalMeshSceneProxy*)Proxy)->SetFOV(NewFOV);
														} );
		}
	}
}

void UUTParticleSystemComponent::SetTransformedToWorld()
{
	// Only worry about this custom stuff if a custom FOV is set
	if (FOV != 0.0f)
	{
		if (bHasSavedScale3D)
		{
			Scale3D = SavedScale3D;
		}
		// call inherited implementation
		UParticleSystemComponent::SetTransformedToWorld();
		SavedScale3D = Scale3D;
		bHasSavedScale3D = TRUE;

		// WRH - 2007/07/25 - Assume that custom FOVs are only placed for 1st-person weapons, so we can get the
		// associated player controller and use that information to hack the local to world matrix with the custom
		// fov calculations
		if (Owner)
		{
			AActor* TopOwner = Owner->GetTopOwner();
			APlayerController* PC = Cast<APlayerController>(TopOwner);
			if (PC)
			{
				FVector ViewLocation(0,0,0);
				FRotator ViewRotation(0,0,0);
				PC->eventGetPlayerViewPoint( ViewLocation, ViewRotation );

				// WRH - 2007/07/25 - This code to calculate the View and Projection matrix is essentially copied from
				// ULocalPlayer::CalcSceneView().
				FMatrix ViewMatrix = FTranslationMatrix(-ViewLocation);
				ViewMatrix = ViewMatrix * FInverseRotationMatrix(ViewRotation);
				ViewMatrix = ViewMatrix * FMatrix(
					FPlane(0,	0,	1,	0),
					FPlane(1,	0,	0,	0),
					FPlane(0,	1,	0,	0),
					FPlane(0,	0,	0,	1));
				FLOAT PlayerFOV = PC->eventGetFOVAngle();
				
				// Don't need to consider aspect ratio when applying the custom FOV
				FPerspectiveMatrix ProjectionMatrix( PlayerFOV * (FLOAT)PI / 360.0f, 1, 1, NEAR_CLIPPING_PLANE );
				FPerspectiveMatrix WarpedProjectionMatrix( FOV * (FLOAT)PI / 360.0f, 1, 1, NEAR_CLIPPING_PLANE );

				LocalToWorld = LocalToWorld * ViewMatrix * WarpedProjectionMatrix * ProjectionMatrix.Inverse() * ViewMatrix.Inverse();
				LocalToWorldDeterminant = LocalToWorld.Determinant();

				// Determine the scale imparted by this warped matrix and set it in the component so particle updates will use it properly, since they
				// currently bypass the LocalToWorld matrix and use the Scale3D and Scale directly.
				FVector NewScale3D;
				for(INT i=0; i<3; i++)
				{
					const FLOAT SquareSum = (LocalToWorld.M[i][0] * LocalToWorld.M[i][0]) + (LocalToWorld.M[i][1] * LocalToWorld.M[i][1]) + (LocalToWorld.M[i][2] * LocalToWorld.M[i][2]);
					NewScale3D[i] = appSqrt(SquareSum);
				}
				Scale3D = NewScale3D;
			}
		}
	}
	else
	{
		UParticleSystemComponent::SetTransformedToWorld();
	}
}

/**
 * Helper function to set the IgnoreScale flag for mesh emitter instances
 */
static void SetIgnoreComponentsScale3D(const TArrayNoInit<FParticleEmitterInstance*>& EmitterInstances)
{
	for (INT i=0; i<EmitterInstances.Num(); i++)
	{
		FParticleEmitterInstance* Instance = EmitterInstances(i);
		FParticleMeshEmitterInstance* MeshInstance = CastEmitterInstance<FParticleMeshEmitterInstance>(Instance);
		if (MeshInstance)
		{
			MeshInstance->bIgnoreComponentScale = TRUE;
		}
	}
}

/**
 * Special version of init particles that ensures that meshes
 * spawned by this particle system will not inherit the custom FOV
 * of the component, since that is not desired.
 */
void UUTParticleSystemComponent::InitParticles()
{
	UParticleSystemComponent::InitParticles();
	if (FOV != 0.0f)
	{
		SetIgnoreComponentsScale3D(EmitterInstances);
	}
}


void UUTParticleSystemComponent::SetFOV(FLOAT NewFOV)
{
	FOV = NewFOV;
	if (FOV != 0.0f)
	{
		SetIgnoreComponentsScale3D(EmitterInstances);
	}
}

//--------------------------------------------------------------


void AUTWeaponPickupFactory::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
}

void AUTWeaponPickupFactory::CheckForErrors()
{
	Super::CheckForErrors();

	if (WeaponPickupClass == NULL)
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("UTWeaponPickupFactory with no weapon type"), MCACTION_NONE);
	}
	else if (WeaponPickupClass->ClassFlags & CLASS_Deprecated)
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Weapon type '%s' is deprecated"), *WeaponPickupClass->GetName()));
	}
	//@FIXME: remove after all maps have switched to content version of bio
	else if (WeaponPickupClass->GetFName() == FName(TEXT("UTWeap_BioRifle")))
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("UTWeap_BioRifle should be replaced with UTWeap_BioRifle_Content"));
	}

	// Check for a pickup light being nearby

	UBOOL bHasLight = FALSE;

	for( FActorIterator It; It; ++It )
	{
		AActor* actor = Cast<AActor>( *It );

		if( Cast<AUTWeaponPickupLight>( actor ) && FPointsAreNear( actor->Location, Location, 16.0f ) )
		{
			bHasLight = TRUE;
		}
	}

	if( !bHasLight )
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("%s doesn't have a pickup light near it"), *GetName()));
	}
}

//--------------------------------------------------------------
// Projectiles

FLOAT AUTProjectile::GetGravityZ()
{
	return Super::GetGravityZ() * CustomGravityScaling;
}

void AUTProjectile::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	if (bWideCheck && bCollideActors && (Instigator != NULL || InstigatorBaseVehicle != NULL))
	{
		APlayerReplicationInfo* InstigatorPRI = (Instigator != NULL) ? Instigator->PlayerReplicationInfo : InstigatorBaseVehicle->PlayerReplicationInfo;
		// hit enemy if just nearby (for console games)
		FMemMark Mark(GMainThreadMemStack);
		FCheckResult* Link = GWorld->Hash->PawnOverlapCheck(GMainThreadMemStack, this, Location, CheckRadius);
		for( FCheckResult* result=Link; result; result=result->GetNext())
		{
			// FIXMESTEVE - is hero clean up cast
			APawn* TargetPawn = Link->Actor ? Link->Actor->GetAPawn() : NULL;
			if ( TargetPawn && !IgnoreBlockingBy(TargetPawn) && TargetPawn->IsValidEnemyTargetFor(InstigatorPRI, FALSE) 
				&& (!Cast<AUTPawn>(TargetPawn) || !Cast<AUTPawn>(TargetPawn)->bIsHero)  )
			{
				UBOOL bDoTouch = TRUE;
				if ( TargetPawn->Velocity.IsNearlyZero() )
				{
					// reduce effective radius if target not moving
					const FLOAT EffectiveRadius = 0.3f * CheckRadius;

					// find nearest point projectile will pass
					if ( Abs(Location.Z - TargetPawn->Location.Z) > TargetPawn->CylinderComponent->CollisionHeight + EffectiveRadius )
					{
						bDoTouch = FALSE;
					}
					else
					{
						FVector ClosestPoint;
						PointDistToLine(TargetPawn->Location, Velocity, Location, ClosestPoint);
						bDoTouch = (ClosestPoint - TargetPawn->Location).Size2D() < TargetPawn->CylinderComponent->CollisionRadius + EffectiveRadius;
					}
				}
				if ( bDoTouch )
				{
					eventTouch( TargetPawn, TargetPawn->CylinderComponent, Location, (Location - TargetPawn->Location).SafeNormal() );
					break;
				}
			}
		}
		Mark.Pop();
	}
	if ( bCheckProjectileLight && Instigator )
	{
		bCheckProjectileLight = FALSE;
		if ( Instigator->IsHumanControlled() && Instigator->IsLocallyControlled() )
		{
			eventCreateProjectileLight();
		}
	}
}

UBOOL AUTProjectile::IgnoreBlockingBy(const AActor* Other) const
{
	return ((!bBlockedByInstigator && InstigatorBaseVehicle == Other) || Super::IgnoreBlockingBy(Other));
}

void UUTExplosionLight::ResetLight()
{
	if ( !bEnabled)
	{
		bEnabled = TRUE;
		// flag as dirty to guarantee an update this frame
		BeginDeferredReattach();
	}

	TimeShiftIndex = 0;
	Lifetime = 0.f;
}

void UUTExplosionLight::Attach()
{
	if (!bInitialized)
	{
		// pull initial light values from first TimeShift entry
		if (TimeShift.Num() > 0)
		{
			Radius = TimeShift(0).Radius;
			Brightness = TimeShift(0).Brightness;
			LightColor = TimeShift(0).LightColor;
		}
		bInitialized = TRUE;
	}

	Super::Attach();
}

/**
 * Updates time dependent state for this component.
 * Requires bAttached == true.
 * @param DeltaTime - The time since the last tick.
 */
void UUTExplosionLight::Tick(FLOAT DeltaTime)
{
	DeltaTime *= GetOwner() ? GetOwner()->CustomTimeDilation : 1.f;

	Super::Tick(DeltaTime);

	if ( bEnabled )
	{
		if ( TimeShift.Num() <= TimeShiftIndex + 1 )
		{
			bEnabled = FALSE;
		}
		else
		{
			Lifetime += DeltaTime;
			if ( Lifetime > TimeShift(TimeShiftIndex+1).StartTime )
			{
				TimeShiftIndex++;
				if ( TimeShift.Num() <= TimeShiftIndex + 1 )
				{
					bEnabled = FALSE;
				}
			}
			if ( bEnabled )
			{
				// fade and color shift
				FLOAT InterpFactor = (Lifetime - TimeShift(TimeShiftIndex).StartTime)/(TimeShift(TimeShiftIndex+1).StartTime - TimeShift(TimeShiftIndex).StartTime);
				Radius = TimeShift(TimeShiftIndex).Radius * (1.f - InterpFactor) + TimeShift(TimeShiftIndex+1).Radius * InterpFactor;
				Brightness = TimeShift(TimeShiftIndex).Brightness * (1.f - InterpFactor) + TimeShift(TimeShiftIndex+1).Brightness * InterpFactor;
				LightColor.R = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.R) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.R) * InterpFactor);
				LightColor.G = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.G) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.G) * InterpFactor);
				LightColor.B = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.B) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.B) * InterpFactor);
				LightColor.A = (BYTE)appTrunc(FLOAT(TimeShift(TimeShiftIndex).LightColor.A) * (1.f - InterpFactor) + FLOAT(TimeShift(TimeShiftIndex+1).LightColor.A) * InterpFactor);
			}
		}
		BeginDeferredReattach();

		if (!bEnabled && DELEGATE_IS_SET(OnLightFinished))
		{
			delegateOnLightFinished(this);
		}
	}
}

/** returns terminal velocity (max speed while falling) for this actor.  Unless overridden, it returns the TerminalVelocity of the PhysicsVolume in which this actor is located.
*/
FLOAT AUTProjectile::GetTerminalVelocity()
{
	return (PhysicsVolume && PhysicsVolume->bWaterVolume) ? PhysicsVolume->TerminalVelocity : TerminalVelocity;
}


/*
GetNetBuoyancy()
determine how deep in water actor is standing:
0 = not in water,
1 = fully in water
*/
void AUTProjectile::GetNetBuoyancy(FLOAT &NetBuoyancy, FLOAT &NetFluidFriction)
{
	if ( PhysicsVolume->bWaterVolume )
	{
		NetBuoyancy = Buoyancy;
		NetFluidFriction = PhysicsVolume->FluidFriction;
	}
}

static UBOOL VehicleHasFreeHoverboardSocket(AUTVehicle* UTV)
{
	check(UTV->HoverBoardAttachSockets.Num() == UTV->HoverBoardSocketInUse.Num());

	for(INT i=0; i<UTV->HoverBoardSocketInUse.Num(); i++)
	{
		if(!UTV->HoverBoardSocketInUse(i))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*-----------------------------------------------------------------------------
Check to see if we can link to someone
-----------------------------------------------------------------------------*/
void AUTVWeap_TowCable::TickSpecial( FLOAT DeltaTime )
{
	Super::TickSpecial(DeltaTime);

	if (WorldInfo->NetMode != NM_DedicatedServer)
	{
		if (CrossScaleTime > 0.0f)
		{
			CrossScaler += (1.f - CrossScaler) * (DeltaTime / CrossScaleTime);
			CrossScaleTime -= DeltaTime;

			if (CrossScaleTime <= 0.0f)
			{
				CrossScaler = 1.f;
			}
		}
	}
	if  ( (Role == ROLE_Authority) && MyHoverboard && Instigator )
	{
		AUTGame* Game = Cast<AUTGame>(WorldInfo->Game);
		AUTVehicle* BestPick = NULL;
		if ( Game )
		{
			FLOAT BestDot = 0.0f;
			const FVector XAxis = FRotationMatrix(MyHoverboard->Rotation).GetAxis(0);
			const FVector HoverboardDir = MyHoverboard->Rotation.Vector();
			const FLOAT MaxAttachRangeSq = Square(MaxAttachRange);
			const BYTE HoverboardTeam = Instigator->GetTeamNum();

			for (AUTVehicle* Vehicle = Game->VehicleList; Vehicle != NULL; Vehicle = Vehicle->NextVehicle)
			{
				if ( Vehicle->Driver && Vehicle->Health > 0 && Vehicle->HoverBoardAttachSockets.Num() > 0 && 
						Vehicle->Team == HoverboardTeam &&
						(Vehicle->Location - MyHoverboard->Location).SizeSquared() <= MaxAttachRangeSq &&
						VehicleHasFreeHoverboardSocket(Vehicle) )
				{
					const FLOAT DotA = XAxis | (Vehicle->Location - MyHoverboard->Location).SafeNormal();
					if ( DotA > 0.7f )
					{
						const FLOAT DotB = HoverboardDir | Vehicle->Rotation.Vector();
						if ( DotB > 0.f || Vehicle->bAllowTowFromAllDirections)
						{
							if (!BestPick || DotA < BestDot)
							{
								FCheckResult Hit(1.0f);
								if (GWorld->SingleLineCheck(Hit, MyHoverboard, Vehicle->Location, MyHoverboard->Location, TRACE_AllBlocking) || Hit.Actor == Vehicle)
								{
									BestPick = Vehicle;
									BestDot = DotA;
								}
							}
						}
					}
				}
			}
		}
		if (BestPick != PotentialTowTruck)
		{
			PotentialTowTruck = BestPick;
			bNetDirty = TRUE;
		}
	}
}

/*
  Scorpion ball seek accel update
*/
void AUTProj_ScorpionGlob_Base::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	if ( bShuttingDown )
	{
		return;
	}

	// seek out hover vehicles
	if ( SeekTarget )
	{
		bRotationFollowsVelocity = TRUE;
		Acceleration = SeekAccel * (SeekTarget->Location - Location).SafeNormal();
	}
	else if ( (Role == ROLE_Authority) && (WorldInfo->TimeSeconds - LastTraceTime > 0.2f) && InstigatorController )
	{
		FLOAT BestDistSq = SeekRangeSq;
		for (APawn *P=WorldInfo->PawnList; P!=NULL; P=P->NextPawn )
		{
			AUTHoverVehicle *V = Cast<AUTHoverVehicle>(P);
			if ( V && V->bDriving && WorldInfo->GRI && !WorldInfo->GRI->OnSameTeam(InstigatorController, V) )
			{
				if ( ((Location - V->Location).SizeSquared() < BestDistSq) )
				{
					FCheckResult Hit(1.f);
					GWorld->SingleLineCheck( Hit, this, V->Location, Location, TRACE_World|TRACE_StopAtAnyHit, FVector(0.f,0.f,0.f) );
					if ( Hit.Time == 1.f )
					{
						SeekTarget = V;
						BestDistSq = (Location - V->Location).SizeSquared();
					}
				}
			}
		}
		if ( SeekTarget )
		{
			// seek it!!!
			bNetDirty = TRUE;
			bRotationFollowsVelocity = TRUE;
			Acceleration = SeekAccel * (SeekTarget->Location - Location).SafeNormal();
		}
	}
}

/** Seeking rocket seeking */
void AUTProj_SeekingRocket::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	if ( bShuttingDown || !Seeking )
	{
		return;
	}
	if ( Seeking == Instigator )
	{
		Seeking = NULL;
		return;
	}
    if ( InitialDir.IsZero() )
	{
        InitialDir = Velocity.SafeNormal();
	}

	FVector	SeekingVector = Seeking->GetTargetLocation(this);
	FVector ForceDir = (SeekingVector - Location).SafeNormal();

	if( (ForceDir | InitialDir) > 0.f )
	{
		FLOAT TrackingStrength = BaseTrackingStrength;
		AUTVehicle* SuperSeekVehicle = bSuperSeekAirTargets ? Cast<AUTVehicle>(Seeking) : NULL;
		if ( SuperSeekVehicle )
		{
			SuperSeekVehicle = SuperSeekVehicle->bHomingTarget ? SuperSeekVehicle : NULL;
		}
		if ( SuperSeekVehicle )
		{
			// track better if following from behind (locking onto engines)
			TrackingStrength = BaseTrackingStrength * ((ForceDir | Seeking->Rotation.Vector()) > 0.7f) ? 20.f : 12.f;
		}
		else
		{
			TrackingStrength = BaseTrackingStrength * (Cast<AUTHoverVehicle>(Seeking) ? 8.f : 6.f);
		}
		FLOAT VelMag = Velocity.Size();
		Acceleration = TrackingStrength * VelMag * ForceDir;

		if ( SuperSeekVehicle && SuperSeekVehicle->IsLocallyControlled() && SuperSeekVehicle->IsHumanControlled() )
		{
			// possibly warn target
			if ( WorldInfo->TimeSeconds - LastLockWarningTime > LockWarningInterval )
			{
				LastLockWarningTime = WorldInfo->TimeSeconds;

				// warning message for players
				SuperSeekVehicle->eventLockOnWarning(this);
				
				// update LockWarningInterval based on target proximity
				LockWarningInterval = Clamp(0.25f*(Location - SuperSeekVehicle->Location).Size()/VelMag, 0.1f, 1.5f);
			}
		}
	}
	else
	{
		Acceleration = FVector(0.f, 0.f, 0.f);
	}
}

void AUTWeap_RocketLauncher::TickSpecial( FLOAT DeltaTime )
{
	Super::TickSpecial(DeltaTime);

	if ( bTargetLockingActive && ( WorldInfo->TimeSeconds > LastTargetLockCheckTime + LockCheckTime ) )
	{
		LastTargetLockCheckTime = WorldInfo->TimeSeconds;
		CheckTargetLock();
	}
}

void AUTWeap_RocketLauncher::UpdateLockTarget(AActor *NewLockTarget)
{
	if ( LockedTarget != NewLockTarget )
	{
		eventAdjustLockTarget(NewLockTarget);
	}
}

/**
* The function checks to see if we are locked on a target
*/
void AUTWeap_RocketLauncher::CheckTargetLock()
{
	if ( !Instigator || !Instigator->Controller )
	{
		return;
	}

	if ( this != Instigator->Weapon )
	{
		debugf(TEXT("checktargetlock on RL when not active weapon!"));
		return;
	}

	if ( Instigator->bNoWeaponFiring || (LoadedFireMode == RFM_Grenades) )
	{
		UpdateLockTarget(NULL);
		PendingLockedTarget = NULL;
		return;
	}

	// support keeping lock as players get onto hoverboard
	if ( LockedTarget )
	{
		if ( LockedTarget->bDeleteMe )
		{
			if ( LockedTargetPRI && Cast<AUTVehicle_Hoverboard>(LockedTarget) )
			{
				// find the appropriate pawn
				for ( APawn *P=WorldInfo->PawnList; P!=NULL; P=P->NextPawn )
				{
					if ( P->PlayerReplicationInfo == LockedTargetPRI )
					{
						UpdateLockTarget(P->GetAVehicle() ? NULL : P);
						break;
					}
				}
			}
			else
			{
				UpdateLockTarget(NULL);
			}
		}
		else if ( LockedTarget->GetAPawn() && LockedTarget->GetAPawn()->DrivenVehicle )
		{
			UpdateLockTarget(Cast<AUTVehicle_Hoverboard>(LockedTarget->GetAPawn()->DrivenVehicle) ? LockedTarget->GetAPawn()->DrivenVehicle : NULL);
		}
	}

	AActor* BestTarget = NULL;
	AUTBot *BotController = Cast<AUTBot>(Instigator->Controller);
	if ( BotController )
	{
		// only try locking onto bot's target
		if ( BotController->Focus && CanLockOnTo(BotController->Focus) )
		{
			// make sure bot can hit it
			FVector StartTrace;
			FRotator AimRot;
			BotController->eventGetPlayerViewPoint( StartTrace, AimRot );
			FVector Aim = AimRot.Vector();

			if ( (Aim | (BotController->Focus->Location - StartTrace).SafeNormal()) > LockAim )
			{
				FCheckResult Hit(1.f);
				GWorld->SingleLineCheck( Hit, this, BotController->Focus->Location, StartTrace, TRACE_ProjTargets|TRACE_World|TRACE_ComplexCollision);
				if ( !Hit.Actor || (Hit.Actor == BotController->Focus) )
				{
					BestTarget = BotController->Focus;
				}
			}
		}
	}
	else
	{
		// Begin by tracing the shot to see if it hits anyone
		FVector StartTrace;
		FRotator AimRot;
		Instigator->Controller->eventGetPlayerViewPoint( StartTrace, AimRot );
		FVector Aim = AimRot.Vector();
		FVector EndTrace = StartTrace + Aim * LockRange;

		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck( Hit, this, EndTrace, StartTrace, TRACE_ProjTargets|TRACE_World|TRACE_ComplexCollision);

		// Check for a hit
		if ( !Hit.Actor || !CanLockOnTo(Hit.Actor) )
		{
			// We didn't hit a valid target, have the controller attempt to pick a good target
			FLOAT BestAim = Cast<AUTConsolePlayerController>(Instigator->Controller) ? ConsoleLockAim : LockAim;
			FLOAT BestDist = 0.f;
			AActor* TA = Instigator->Controller->PickTarget(APawn::StaticClass(), BestAim, BestDist, Aim, StartTrace, LockRange);
			if ( TA && CanLockOnTo(TA) )
			{
				BestTarget = TA;
			}
		}
		else	// We hit a valid target
		{
			BestTarget = Hit.Actor;
		}
	}

	// If we have a "possible" target, note its time mark
	if ( BestTarget )
	{
		LastValidTargetTime = WorldInfo->TimeSeconds;

		if ( BestTarget == LockedTarget )
		{
			LastLockedOnTime = WorldInfo->TimeSeconds;
		}
		else
		{
			if ( LockedTarget && ((WorldInfo->TimeSeconds - LastLockedOnTime > LockTolerance) || !CanLockOnTo(LockedTarget)) )
			{
				// Invalidate the current locked Target
				UpdateLockTarget(NULL);
			}

			// We have our best target, see if they should become our current target.
			// Check for a new Pending Lock
			if (PendingLockedTarget != BestTarget)
			{
				PendingLockedTarget = BestTarget;
				PendingLockedTargetTime = PendingLockedTarget->GetAVehicle() && Cast<AUTConsolePlayerController>(Instigator->Controller)
										? WorldInfo->TimeSeconds + 0.5f*LockAcquireTime
										: WorldInfo->TimeSeconds + LockAcquireTime;
			}

			// Otherwise check to see if we have been tracking the pending lock long enough
			else if (PendingLockedTarget == BestTarget && WorldInfo->TimeSeconds >= PendingLockedTargetTime )
			{
				UpdateLockTarget(PendingLockedTarget);
				LastLockedOnTime = WorldInfo->TimeSeconds;
				PendingLockedTarget = NULL;
				PendingLockedTargetTime = 0.f;
			}
		}
	}
	else 
	{
		if ( LockedTarget && ((WorldInfo->TimeSeconds - LastLockedOnTime > LockTolerance) || !CanLockOnTo(LockedTarget)) )
		{
			// Invalidate the current locked Target
			UpdateLockTarget(NULL);
		}

		// Next attempt to invalidate the Pending Target
		if ( PendingLockedTarget && ((WorldInfo->TimeSeconds - LastValidTargetTime > LockTolerance) || !CanLockOnTo(PendingLockedTarget)) )
		{
			PendingLockedTarget = NULL;
		}
	}
}

/**
* Given an potential target TA determine if we can lock on to it.  By default only allow locking on
* to pawns.  Some weapons may want to be able to lock on to other actors.
*/
UBOOL AUTWeap_RocketLauncher::CanLockOnTo(AActor *TA)
{
	if ( !TA || !TA->bProjTarget || TA->bDeleteMe || !TA->GetAPawn() || (TA == Instigator) || (TA->GetAPawn()->Health <= 0) )
	{
		return FALSE;
	}

	return ( !WorldInfo->Game || !WorldInfo->Game->bTeamGame || !WorldInfo->GRI || !WorldInfo->GRI->OnSameTeam(Instigator,TA) );
}

void AUTProj_FlakShard::TickSpecial( FLOAT DeltaTime )
{
	Super::TickSpecial(DeltaTime);

	if ( (Physics == PHYS_Projectile) && (LifeSpan < 1.f) )
	{
		setPhysics(PHYS_Falling);
	}
	if ( bShrinking )
	{
		ShrinkTimer -= DeltaTime;
		if ( ShrinkTimer < 0.25f )
		{
			DrawScale = ::Max(0.01f, ShrinkTimer*4.f);

			// mark components for an update
			MarkComponentsAsDirty();
		}
	}
}

