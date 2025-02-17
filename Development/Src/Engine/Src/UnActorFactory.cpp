/*=============================================================================
	UnActorFactory.cpp: 
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAIClasses.h"
#include "EngineParticleClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineAnimClasses.h"
#include "EngineDecalClasses.h"
#include "EngineMaterialClasses.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "EngineSequenceClasses.h"
#include "LensFlare.h"
#include "EngineFogVolumeClasses.h"

IMPLEMENT_CLASS(UActorFactory);
IMPLEMENT_CLASS(UActorFactoryStaticMesh);
IMPLEMENT_CLASS(UActorFactoryFracturedStaticMesh);
IMPLEMENT_CLASS(UActorFactoryPhysXDestructible);
IMPLEMENT_CLASS(UActorFactoryLight);
IMPLEMENT_CLASS(UActorFactoryTrigger);
IMPLEMENT_CLASS(UActorFactoryPathNode);
IMPLEMENT_CLASS(UActorFactoryPhysicsAsset);
IMPLEMENT_CLASS(UActorFactoryRigidBody);
IMPLEMENT_CLASS(UActorFactoryMover);
IMPLEMENT_CLASS(UActorFactoryEmitter);
IMPLEMENT_CLASS(UActorFactoryAI);
IMPLEMENT_CLASS(UActorFactoryVehicle);
IMPLEMENT_CLASS(UActorFactorySkeletalMesh);
IMPLEMENT_CLASS(UActorFactoryPlayerStart);
IMPLEMENT_CLASS(UActorFactoryDynamicSM);
IMPLEMENT_CLASS(UActorFactoryAmbientSound);
IMPLEMENT_CLASS(UActorFactoryAmbientSoundMovable);
IMPLEMENT_CLASS(UActorFactoryAmbientSoundSimple);
IMPLEMENT_CLASS(UActorFactoryDecal);
IMPLEMENT_CLASS(UActorFactoryCoverLink);
IMPLEMENT_CLASS(UActorFactoryArchetype);
IMPLEMENT_CLASS(UActorFactoryLensFlare);
IMPLEMENT_CLASS(UActorFactoryFogVolumeConstantDensityInfo);
IMPLEMENT_CLASS(UActorFactoryFogVolumeLinearHalfspaceDensityInfo);
IMPLEMENT_CLASS(UActorFactoryFogVolumeSphericalDensityInfo);

/*-----------------------------------------------------------------------------
	UActorFactory
-----------------------------------------------------------------------------*/


void UActorFactory::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );
}

AActor* UActorFactory::GetDefaultActor()
{
	check( NewActorClass );

	// if the default class is requested during gameplay, but it's bNoDelete, replace it with GameplayActorClass
	if (GWorld->HasBegunPlay() && NewActorClass == GetClass()->GetDefaultObject<UActorFactory>()->NewActorClass && NewActorClass->GetDefaultActor()->bNoDelete)
	{
		if ( GameplayActorClass == NULL || GameplayActorClass->GetDefaultActor()->bNoDelete )
		{
			appErrorf(TEXT("Actor factories of type %s cannot be used in-game"), *GetClass()->GetName() );
		}
		NewActorClass = GameplayActorClass;
	}
	check( !(NewActorClass->ClassFlags & CLASS_Abstract) );

	return NewActorClass->GetDefaultActor();
}

AActor* UActorFactory::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	GetDefaultActor();

	check(Location);

	// Don't try and spawn bStatic things once we have started playing level.
	const UBOOL bBegunPlay = GWorld->HasBegunPlay();
	if( bBegunPlay && (GetDefaultActor()->bStatic || GetDefaultActor()->bNoDelete) )
	{
		debugf( TEXT("Cannot spawn class '%s' because it is bStatic/bNoDelete and HasBegunPlay is true."), *NewActorClass->GetName() );
		return NULL;
	}

	FRotator NewRotation;
	if(Rotation)
		NewRotation = *Rotation;
	else
		NewRotation = GetDefaultActor()->Rotation;

	AActor* NewActor = GWorld->SpawnActor(NewActorClass, NAME_None, *Location, NewRotation);

	return NewActor;
}


/**
* This will check whether there is enough space to spawn an character.
* Additionally it will check the ActorFactoryData to for any overrides 
* ( e.g. bCheckSpawnCollision )
*
* @return if there is enough space to spawn character at this location
**/
UBOOL UActorFactory::IsEnoughRoomToSpawnPawn( const FVector* const Location, const USeqAct_ActorFactory* const ActorFactoryData ) const
{
	UBOOL Retval = FALSE;

	// check that the area around the location is clear of other characters
	UBOOL bHitPawn = FALSE;

	FMemMark Mark( GMainThreadMemStack );

	FCheckResult* checkResult = NULL;
	
	// if we don't have an param data then default to checking for collision	
	if( ( ActorFactoryData == NULL )
 		|| ( ActorFactoryData->bCheckSpawnCollision == TRUE )
	   )
	{
		checkResult = GWorld->MultiPointCheck( GMainThreadMemStack, *Location, FVector(36,36,78), TRACE_AllColliding );
	}
	else
	{
		checkResult = GWorld->MultiPointCheck( GMainThreadMemStack, *Location, FVector(36,36,78), TRACE_World );
	}


	for( FCheckResult* testResult = checkResult; testResult != NULL; testResult = testResult->GetNext() )
	{
		if( ( testResult->Actor != NULL )
			&& ( testResult->Actor->IsA( APawn::StaticClass() ))
			)
		{
			bHitPawn = TRUE;
			break;
		}
	}

	Mark.Pop();

	Retval = bHitPawn;

	return Retval;
}




/*-----------------------------------------------------------------------------
	UActorFactoryStaticMesh
-----------------------------------------------------------------------------*/

AActor* UActorFactoryStaticMesh::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	if( StaticMesh )
	{
		// Term Component
		NewActor->TermRBPhys(NULL);
		NewActor->ClearComponents();

		// Change properties
		UStaticMeshComponent* StaticMeshComponent = NULL;
		for (INT Idx = 0; Idx < NewActor->Components.Num() && StaticMeshComponent == NULL; Idx++)
		{
			StaticMeshComponent = Cast<UStaticMeshComponent>(NewActor->Components(Idx));
		}

		check(StaticMeshComponent);
		StaticMeshComponent->StaticMesh = StaticMesh;
		NewActor->DrawScale3D = DrawScale3D;

		// Init Component
		NewActor->ConditionalUpdateComponents();
		NewActor->InitRBPhys();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewActor;
}

UBOOL UActorFactoryStaticMesh::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(StaticMesh)
	{
		return true;
	}
	else
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoStaticMesh");
		return false;
	}
}

void UActorFactoryStaticMesh::AutoFillFields(USelection* Selection)
{
	StaticMesh = Selection->GetTop<UStaticMesh>();
}

FString UActorFactoryStaticMesh::GetMenuName()
{
	if(StaticMesh)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *StaticMesh->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}


/*-----------------------------------------------------------------------------
	UActorFactoryFracturedStaticMesh
-----------------------------------------------------------------------------*/

AActor* UActorFactoryFracturedStaticMesh::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	if( FracturedStaticMesh )
	{
		// Term Component
		NewActor->TermRBPhys(NULL);
		NewActor->ClearComponents();

		// Change properties
		UFracturedStaticMeshComponent* FracturedStaticMeshComponent = NULL;
		for (INT Idx = 0; Idx < NewActor->Components.Num() && FracturedStaticMeshComponent == NULL; Idx++)
		{
			FracturedStaticMeshComponent = Cast<UFracturedStaticMeshComponent>(NewActor->Components(Idx));
		}

		check(FracturedStaticMeshComponent);
		FracturedStaticMeshComponent->SetStaticMesh(FracturedStaticMesh);
		NewActor->DrawScale3D = DrawScale3D;

		// Init Component
		NewActor->ConditionalUpdateComponents();
		NewActor->InitRBPhys();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewActor;
}

UBOOL UActorFactoryFracturedStaticMesh::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(FracturedStaticMesh && FracturedStaticMesh->BodySetup)
	{
		return true;
	}
	else
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoStaticMesh");
		return false;
	}
}

void UActorFactoryFracturedStaticMesh::AutoFillFields(USelection* Selection)
{
	FracturedStaticMesh = Selection->GetTop<UFracturedStaticMesh>();
}

FString UActorFactoryFracturedStaticMesh::GetMenuName()
{
	if(FracturedStaticMesh)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *FracturedStaticMesh->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}

/*-----------------------------------------------------------------------------
	UActorFactoryPhysXDestructible
-----------------------------------------------------------------------------*/

AActor* UActorFactoryPhysXDestructible::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	if( PhysXDestructible )
	{
		// Term Component
		NewActor->TermRBPhys(NULL);
		NewActor->ClearComponents();

		// Change properties
		UFracturedStaticMeshComponent* FracturedStaticMeshComponent = NULL;
		for( INT Index = 0; Index < NewActor->Components.Num() && FracturedStaticMeshComponent == NULL; ++Index )
		{
			FracturedStaticMeshComponent = Cast<UFracturedStaticMeshComponent>(NewActor->Components(Index));
		}

		check(FracturedStaticMeshComponent);
		FracturedStaticMeshComponent->SetStaticMesh(PhysXDestructible->FracturedStaticMesh);
		FracturedStaticMeshComponent->WireframeColor = FColor( 255, 128, 0, 255 );
		NewActor->DrawScale3D = DrawScale3D;

		// Set PhysXDestructibleActor-specific fields
		APhysXDestructibleActor * PDA = Cast<APhysXDestructibleActor>(NewActor);
		check( PDA != NULL );
		PDA->PhysXDestructible = PhysXDestructible; 
		PDA->DestructibleParameters = PhysXDestructible->DestructibleParameters;	// Copy so they can be modified per-actor
		PDA->LinearSize = PhysXDestructible->FracturedStaticMesh->Bounds.BoxExtent.GetMax();
//		PDA->DestructibleComponent->CreateDetailedCollisionFromDestructible( PhysXDestructible, FracturedStaticMeshComponent->StaticMesh->BodySetup );

		// Init Component
		if( NewActor->CollisionComponent )
		{
			NewActor->CollisionComponent->SetRBChannel( RBCC_EffectPhysics );
			NewActor->CollisionComponent->SetRBCollidesWithChannel( RBCC_Default, TRUE );
			NewActor->CollisionComponent->SetRBCollidesWithChannel( RBCC_BlockingVolume, TRUE );
			NewActor->CollisionComponent->SetRBCollidesWithChannel( RBCC_EffectPhysics, TRUE );
			NewActor->CollisionComponent->SetRBCollidesWithChannel( RBCC_GameplayPhysics, TRUE );
		}
		NewActor->ConditionalUpdateComponents();
		NewActor->InitRBPhys();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewActor;
}

UBOOL UActorFactoryPhysXDestructible::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(PhysXDestructible && PhysXDestructible->FracturedStaticMesh && PhysXDestructible->FracturedStaticMesh->BodySetup)
	{
		return true;
	}
	else
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoPhysXDestructible");
		return false;
	}
}

void UActorFactoryPhysXDestructible::AutoFillFields(USelection* Selection)
{
	PhysXDestructible = Selection->GetTop<UPhysXDestructible>();
}

FString UActorFactoryPhysXDestructible::GetMenuName()
{
	if(PhysXDestructible)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *PhysXDestructible->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}


/*-----------------------------------------------------------------------------
	UActorFactoryDynamicSM
-----------------------------------------------------------------------------*/

AActor* UActorFactoryDynamicSM::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	ADynamicSMActor* NewSMActor = CastChecked<ADynamicSMActor>(NewActor);

	if( StaticMesh )
	{
		// Term Component
		NewSMActor->TermRBPhys(NULL);
		NewSMActor->ClearComponents();

		// Change properties
		NewSMActor->StaticMeshComponent->StaticMesh = StaticMesh;
		if (GIsGame)
		{
			NewSMActor->ReplicatedMesh = StaticMesh;
		}
		NewSMActor->StaticMeshComponent->bNotifyRigidBodyCollision = bNotifyRigidBodyCollision;
		NewSMActor->DrawScale3D = DrawScale3D;
		NewSMActor->CollisionType = CollisionType;
		NewSMActor->SetCollisionFromCollisionType();
		NewSMActor->bNoEncroachCheck = bNoEncroachCheck;
		NewSMActor->StaticMeshComponent->bUseCompartment = bUseCompartment;
		NewSMActor->StaticMeshComponent->bCastDynamicShadow = bCastDynamicShadow;

		// Init Component
		NewSMActor->ConditionalUpdateComponents();
		NewSMActor->InitRBPhys();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewSMActor;
}

UBOOL UActorFactoryDynamicSM::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(StaticMesh)
	{
		return true;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoStaticMesh");
		return false;
	}
}

void UActorFactoryDynamicSM::AutoFillFields(USelection* Selection)
{
	StaticMesh = Selection->GetTop<UStaticMesh>();
}

FString UActorFactoryDynamicSM::GetMenuName()
{
	if(StaticMesh)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *StaticMesh->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}

void UActorFactoryDynamicSM::PostLoad()
{
	Super::PostLoad();
}

/*-----------------------------------------------------------------------------
	UActorFactoryRigidBody
-----------------------------------------------------------------------------*/

void UActorFactoryRigidBody::PostLoad()
{
	Super::PostLoad();
}

AActor* UActorFactoryRigidBody::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if(NewActor)
	{
		AKActor* NewRB = CastChecked<AKActor>(NewActor);

		// Find reference frame initial velocity is applied in
		FMatrix VelFrameTM = FMatrix::Identity;
		if(bLocalSpaceInitialVelocity)
		{
			FRotator InitRot(0,0,0);
			if(Rotation)
			{
				InitRot = *Rotation;
			}

			VelFrameTM = FRotationMatrix(InitRot);
		}

		// Calculate initial angular/linear velocity
		FVector TotalInitAngVel = FVector(0,0,0);
		FVector TotalInitLinVel = InitialVelocity;

		// Add contribution from Distributions if present.
		if(AdditionalVelocity)
		{
			TotalInitLinVel += AdditionalVelocity->GetValue();
		}
		
		if(InitialAngularVelocity)
		{
			TotalInitAngVel += InitialAngularVelocity->GetValue();
		}


		FVector InitVel = VelFrameTM.TransformNormal(InitialVelocity);
	
		// Apply initial linear/angular velocity
		NewRB->StaticMeshComponent->SetRBLinearVelocity( VelFrameTM.TransformNormal(TotalInitLinVel) );
		NewRB->StaticMeshComponent->SetRBAngularVelocity( VelFrameTM.TransformNormal(TotalInitAngVel) );
		
		// Wake if desired.
		if(bStartAwake)
		{
			NewRB->StaticMeshComponent->WakeRigidBody();
		}

		NewRB->StaticMeshComponent->SetRBChannel((ERBCollisionChannel)RBChannel);

		NewRB->bDamageAppliesImpulse = bDamageAppliesImpulse;
		NewRB->ReplicatedDrawScale3D = NewRB->DrawScale3D * 1000.0f; // avoid effects of vector rounding (@warning: must match script: see PostBeginPlay() and ReplicatedEvent())

		NewRB->bEnableStayUprightSpring = bEnableStayUprightSpring;
		NewRB->StayUprightTorqueFactor = StayUprightTorqueFactor;
		NewRB->StayUprightMaxTorque = StayUprightMaxTorque;
	}

	return NewActor;
}


UBOOL UActorFactoryRigidBody::CanCreateActor(FString& OutErrorMsg)
{
	if(StaticMesh && StaticMesh->BodySetup)
	{
		return true;
	}
	else 
	{
		if ( !StaticMesh )
		{
			OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoStaticMesh");
		}
		else if ( !StaticMesh->BodySetup )
		{
			OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoRigidBodySetup");
		}

		return false;
	}
}

/*-----------------------------------------------------------------------------
	UActorFactoryEmitter
-----------------------------------------------------------------------------*/

AActor* UActorFactoryEmitter::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	AEmitter* NewEmitter = CastChecked<AEmitter>(NewActor);

	if( ParticleSystem )
	{
		// Term Component
		NewEmitter->ClearComponents();

		// Change properties
		NewEmitter->ParticleSystemComponent->Template = ParticleSystem;

		// if we're created by Kismet on the server during gameplay, we need to replicate the emitter
		if (GWorld->HasBegunPlay() && GWorld->GetNetMode() != NM_Client && ActorFactoryData != NULL)
		{
			NewEmitter->RemoteRole = ROLE_SimulatedProxy;
			NewEmitter->bAlwaysRelevant = TRUE;
			NewEmitter->NetUpdateFrequency = 0.1f; // could also set bNetTemporary but LD might further trigger it or something
			// call into gameplay code with template so it can set up replication
			NewEmitter->eventSetTemplate(ParticleSystem, NewEmitter->bDestroyOnSystemFinish);
		}

		// Init Component
		NewEmitter->ConditionalUpdateComponents();
	}

	return NewEmitter;
}

UBOOL UActorFactoryEmitter::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(ParticleSystem)
	{
		return true;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoParticleSystem");
		return false;
	}
}

void UActorFactoryEmitter::AutoFillFields(USelection* Selection)
{
	ParticleSystem = Selection->GetTop<UParticleSystem>();
}

FString UActorFactoryEmitter::GetMenuName()
{
	if(ParticleSystem)
		return FString::Printf( TEXT("%s: %s"), *MenuName, *ParticleSystem->GetName() );
	else
		return MenuName;
}

/*-----------------------------------------------------------------------------
	UActorFactoryPhysicsAsset
-----------------------------------------------------------------------------*/

void UActorFactoryPhysicsAsset::PreSave()
{
	Super::PreSave();

	// Because PhysicsAsset->DefaultSkelMesh is editor only, we need to keep a reference to the SkeletalMesh we want to spawn here.
	if(!IsTemplate() && SkeletalMesh == NULL && PhysicsAsset && PhysicsAsset->DefaultSkelMesh)
	{
		SkeletalMesh = PhysicsAsset->DefaultSkelMesh;
	}
}

AActor* UActorFactoryPhysicsAsset::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	if(!PhysicsAsset)
		return NULL;

	// If a specific mesh is supplied, use it. Otherwise, use default from PhysicsAsset.
	USkeletalMesh* UseSkelMesh = SkeletalMesh;
	if(!UseSkelMesh)
	{
		UseSkelMesh = PhysicsAsset->DefaultSkelMesh;
	}
	if(!UseSkelMesh)
	{
		return NULL;
	}

	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if(!NewActor)
	{
		return NULL;
	}

	AKAsset* NewAsset = CastChecked<AKAsset>(NewActor);

	// Term Component
	NewAsset->TermRBPhys(NULL);
	NewAsset->ClearComponents();

	// Change properties
	NewAsset->SkeletalMeshComponent->SkeletalMesh = UseSkelMesh;
	if (GIsGame)
	{
		NewAsset->ReplicatedMesh = UseSkelMesh;
		NewAsset->ReplicatedPhysAsset = PhysicsAsset;
	}
	NewAsset->SkeletalMeshComponent->PhysicsAsset = PhysicsAsset;
	NewAsset->SkeletalMeshComponent->bNotifyRigidBodyCollision = bNotifyRigidBodyCollision;
	NewAsset->SkeletalMeshComponent->bUseCompartment = bUseCompartment;
	NewAsset->SkeletalMeshComponent->bCastDynamicShadow = bCastDynamicShadow;
	NewAsset->DrawScale3D = DrawScale3D;

	// Init Component
	NewAsset->ConditionalUpdateComponents();
	NewAsset->InitRBPhys();

	// Call other functions
	NewAsset->SkeletalMeshComponent->SetRBLinearVelocity(InitialVelocity);

	if(bStartAwake)
	{
		NewAsset->SkeletalMeshComponent->WakeRigidBody();
	}

	NewAsset->bDamageAppliesImpulse = bDamageAppliesImpulse;
		
	return NewAsset;
}

UBOOL UActorFactoryPhysicsAsset::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(PhysicsAsset)
	{
		return true;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoPhysicsAsset");
		return false;
	}
}

void UActorFactoryPhysicsAsset::AutoFillFields(USelection* Selection)
{
	PhysicsAsset = Selection->GetTop<UPhysicsAsset>();
}

FString UActorFactoryPhysicsAsset::GetMenuName()
{
	if(PhysicsAsset)
		return FString::Printf( TEXT("%s: %s"), *MenuName, *PhysicsAsset->GetName() );
	else
		return MenuName;
}

/*-----------------------------------------------------------------------------
	UActorFactoryAI
-----------------------------------------------------------------------------*/
AActor* UActorFactoryAI::GetDefaultActor()
{
	if ( PawnClass )
	{
		NewActorClass = PawnClass;
	}

	check( NewActorClass );
	check( !(NewActorClass->ClassFlags & CLASS_Abstract) );

	return NewActorClass->GetDefaultActor();
}

AActor* UActorFactoryAI::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	// first create the pawn
	APawn* newPawn = NULL;
	if (PawnClass != NULL)
	{
		// check that the area around the location is clear of other characters
		UBOOL bHitPawn = IsEnoughRoomToSpawnPawn( Location, ActorFactoryData );

		if (!bHitPawn)
		{
			newPawn = (APawn*)Super::CreateActor( Location, Rotation, ActorFactoryData );
			if (newPawn != NULL)
			{
				// create the controller
				if (ControllerClass != NULL)
				{
					// If no pointer for rotation supplied, use default rotation.
					FRotator NewRotation;
					if(Rotation)
						NewRotation = *Rotation;
					else
						NewRotation = ControllerClass->GetDefaultActor()->Rotation;

					check(Location);
					AAIController* newController = (AAIController*)GWorld->SpawnActor(ControllerClass, NAME_None, *Location, NewRotation);
					if (newController != NULL)
					{
						// handle the team assignment
						newController->eventSetTeam(TeamIndex);
						// force the controller to possess, etc
						newController->eventPossess(newPawn, false);


						if (newController && newController->PlayerReplicationInfo && PawnName != TEXT("") )
							newController->PlayerReplicationInfo->eventSetPlayerName(PawnName);
					}
				}
				if (bGiveDefaultInventory && newPawn->WorldInfo->Game != NULL)
				{
					newPawn->WorldInfo->Game->eventAddDefaultInventory(newPawn);
				}
				// create any inventory
				for (INT idx = 0; idx < InventoryList.Num(); idx++)
				{
					newPawn->eventCreateInventory( InventoryList(idx), false  );
				}
			}
		}
	}
	return newPawn;
}

/*-----------------------------------------------------------------------------
	UActorFactoryVehicle
-----------------------------------------------------------------------------*/
AActor* UActorFactoryVehicle::GetDefaultActor()
{
	if ( VehicleClass )
	{
		NewActorClass = VehicleClass;
	}

	check( NewActorClass );
	check( !(NewActorClass->ClassFlags & CLASS_Abstract) );

	return NewActorClass->GetDefaultActor();
}

AActor* UActorFactoryVehicle::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	// first create the pawn
	AVehicle* NewVehicle = NULL;

	if (VehicleClass != NULL)
	{
		// check that the area around the location is clear of other characters
		UBOOL bHitPawn = IsEnoughRoomToSpawnPawn( Location, ActorFactoryData );

		if( !bHitPawn )
		{
			NewVehicle = (AVehicle*)Super::CreateActor( Location, Rotation, ActorFactoryData );
		}
	}
	return NewVehicle;
}

/*-----------------------------------------------------------------------------
	UActorFactorySkeletalMesh
-----------------------------------------------------------------------------*/

AActor* UActorFactorySkeletalMesh::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if(!NewActor)
	{
		return NULL;
	}

	ASkeletalMeshActor* NewSMActor = CastChecked<ASkeletalMeshActor>(NewActor);

	if( SkeletalMesh )
	{
		// Term Component
		NewSMActor->ClearComponents();

		// Change properties
		NewSMActor->SkeletalMeshComponent->SkeletalMesh = SkeletalMesh;
		if (GIsGame)
		{
			NewSMActor->ReplicatedMesh = SkeletalMesh;
		}
		if(AnimSet)
		{
			NewSMActor->SkeletalMeshComponent->AnimSets.AddItem( AnimSet );
		}

		UAnimNodeSequence* SeqNode = Cast<UAnimNodeSequence>(NewSMActor->SkeletalMeshComponent->Animations);
		if(SeqNode)
		{
			SeqNode->AnimSeqName = AnimSequenceName;
		}

		// Init Component
		NewSMActor->ConditionalUpdateComponents();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewSMActor;
}

UBOOL UActorFactorySkeletalMesh::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(SkeletalMesh)
	{	
		return true;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoSkeletalMesh");
		return false;
	}
}

void UActorFactorySkeletalMesh::AutoFillFields(USelection* Selection)
{
	SkeletalMesh = Selection->GetTop<USkeletalMesh>();
}

FString UActorFactorySkeletalMesh::GetMenuName()
{
	if(SkeletalMesh)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *SkeletalMesh->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}

/*-----------------------------------------------------------------------------
	UActorFactoryAmbientSound
-----------------------------------------------------------------------------*/

AActor* UActorFactoryAmbientSound::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	AAmbientSound* NewSound = CastChecked<AAmbientSound>( NewActor );

	if( AmbientSoundCue )
	{
		// Term Component
		NewSound->ClearComponents();

		// Change properties
		NewSound->AudioComponent->SoundCue = AmbientSoundCue;

		// Init Component
		NewSound->ConditionalUpdateComponents();

		// propagate the actor
		GObjectPropagator->PropagateActor( NewActor );
	}

	return NewSound;
}

UBOOL UActorFactoryAmbientSound::CanCreateActor( FString& OutErrorMsg ) 
{ 
	if( !AmbientSoundCue )
	{
		OutErrorMsg = TEXT( "Error_CouldNotCreateActor_NoSoundCue" );
		return false;
	}

	return true;
}

void UActorFactoryAmbientSound::AutoFillFields( USelection* Selection )
{
	AmbientSoundCue = Selection->GetTop<USoundCue>();
}

FString UActorFactoryAmbientSound::GetMenuName( void )
{
	if( AmbientSoundCue )
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *AmbientSoundCue->GetName() );
	}

	return MenuName;
}

/*-----------------------------------------------------------------------------
	UActorFactoryAmbientSoundMovable
-----------------------------------------------------------------------------*/

AActor* UActorFactoryAmbientSoundMovable::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	AAmbientSoundMovable* NewSound = CastChecked<AAmbientSoundMovable>( NewActor );

	if( AmbientSoundCue )
	{
		// Term Component
		NewSound->ClearComponents();

		// Change properties
		NewSound->AudioComponent->SoundCue = AmbientSoundCue;

		// Init Component
		NewSound->ConditionalUpdateComponents();

		// propagate the actor
		GObjectPropagator->PropagateActor( NewActor );
	}

	return NewSound;
}

UBOOL UActorFactoryAmbientSoundMovable::CanCreateActor( FString& OutErrorMsg ) 
{ 
	if( !AmbientSoundCue )
	{
		OutErrorMsg = TEXT( "Error_CouldNotCreateActor_NoSoundCue" );
		return false;
	}

	return true;
}

void UActorFactoryAmbientSoundMovable::AutoFillFields( USelection* Selection )
{
	AmbientSoundCue = Selection->GetTop<USoundCue>();
}

FString UActorFactoryAmbientSoundMovable::GetMenuName( void )
{
	if( AmbientSoundCue )
	{
		return FString::Printf( TEXT( "%s: %s" ), *MenuName, *AmbientSoundCue->GetName() );
	}

	return MenuName;
}

/*-----------------------------------------------------------------------------
	UActorFactoryAmbientSoundSimple
-----------------------------------------------------------------------------*/

AActor* UActorFactoryAmbientSoundSimple::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	AAmbientSoundSimple* NewSound = CastChecked<AAmbientSoundSimple>( NewActor );

	if( SoundNodeWave )
	{
		// Term Component
		NewSound->ClearComponents();

		// Change properties
		NewSound->AmbientProperties->Wave = SoundNodeWave;

		// Init Component
		NewSound->ConditionalUpdateComponents();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewSound;
}

UBOOL UActorFactoryAmbientSoundSimple::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(SoundNodeWave)
	{
		return TRUE;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoSoundWave");
		return FALSE;
	}
}

void UActorFactoryAmbientSoundSimple::AutoFillFields(USelection* Selection)
{
	SoundNodeWave = Selection->GetTop<USoundNodeWave>();
}

FString UActorFactoryAmbientSoundSimple::GetMenuName()
{
	if( SoundNodeWave )
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *SoundNodeWave->GetName() );
	}
	else
	{
		return MenuName;
	}
}

/*-----------------------------------------------------------------------------
	UActorFactoryDecal
-----------------------------------------------------------------------------*/

/**
 * @return	TRUE if the specified material instance is a valid decal material.
 */
static inline UBOOL IsValidDecalMaterial(UMaterialInterface* MaterialInterface)
{
	UBOOL bResult = FALSE;
	if ( MaterialInterface )
	{
		bResult=TRUE;
	}
	return bResult;
}

AActor* UActorFactoryDecal::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor;

	if ( Rotation )
	{
		// Assuming the rotation is a surface orientation, we hand in the inverted rotation 
		// to orient the decal towards the surface.
		const FRotator InvertedRotation( (-Rotation->Vector()).Rotation() );
		if ( Location )
		{
			// A location was specified; back the decal off the surface a bit.
			const FVector OffsetLocation( *Location + Rotation->Vector() );
			NewActor = Super::CreateActor( &OffsetLocation, &InvertedRotation, ActorFactoryData );
		}
		else
		{
			NewActor = Super::CreateActor( NULL, &InvertedRotation, ActorFactoryData );
		}
	}
	else
	{
		// No rotation was specified, so we can't orient the decal or back it off the surface.
		NewActor = Super::CreateActor( Location, NULL, ActorFactoryData );
	}

	if( !NewActor )
	{
		return NULL;
	}

	ADecalActorBase* NewDecalActor = CastChecked<ADecalActorBase>( NewActor );

	if( NewDecalActor && 
		IsValidDecalMaterial( DecalMaterial ) )
	{
		// Term Component
		NewDecalActor->ClearComponents();

		// Change properties
		NewDecalActor->Decal->SetDecalMaterial(DecalMaterial);

		// Call PostEditMove to force the decal actor's position/orientation on the decal component.
		//NewDecalActor->PostEditMove( TRUE );

		// Init Component
		NewDecalActor->ConditionalUpdateComponents();

		// propagate the actor
		GObjectPropagator->PropagateActor(NewActor);
	}

	return NewDecalActor;
}

UBOOL UActorFactoryDecal::CanCreateActor(FString& OutErrorMsg) 
{ 
	return TRUE;
}

void UActorFactoryDecal::AutoFillFields(USelection* Selection)
{
	// use selected decal material
	if( Selection )
	{
		for( FSelectionIterator It( *Selection ); It; ++It )
		{
			UMaterialInterface* Mat = Cast<UMaterialInterface>(*It);
			if( Mat )
			{
				DecalMaterial = Mat;
				break;
			}
		}
	}
}

FString UActorFactoryDecal::GetMenuName()
{
	if( IsValidDecalMaterial( DecalMaterial ) )
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *DecalMaterial->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}

/*-----------------------------------------------------------------------------
	UActorFactoryArchetype
-----------------------------------------------------------------------------*/

AActor* UActorFactoryArchetype::GetDefaultActor()
{
	return ArchetypeActor;
}

AActor* UActorFactoryArchetype::CreateActor( const FVector* const Location, const FRotator* const Rotation, const USeqAct_ActorFactory* const ActorFactoryData )
{
	check(Location);

	// Invalid if there is no Archetype, or  the Archetype is not one...
	if(!ArchetypeActor || !ArchetypeActor->HasAnyFlags(RF_ArchetypeObject))
	{
		return NULL;
	}

	UClass* NewClass = ArchetypeActor->GetClass();

	FRotator NewRotation;
	if(Rotation)
		NewRotation = *Rotation;
	else
		NewRotation = NewClass->GetDefaultActor()->Rotation;

	AActor* NewActor = GWorld->SpawnActor(NewClass, NAME_None, *Location, NewRotation, ArchetypeActor);

	return NewActor;
}

UBOOL UActorFactoryArchetype::CanCreateActor(FString& OutErrorMsg) 
{ 
	if(ArchetypeActor && ArchetypeActor->HasAnyFlags(RF_ArchetypeObject))
	{
		return TRUE;
	}
	else
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoArchetype");
		return FALSE;
	}
}

void UActorFactoryArchetype::AutoFillFields(USelection* Selection)
{
	ArchetypeActor = NULL;

	for( USelection::TObjectIterator It( Selection->ObjectItor() ) ; It && !ArchetypeActor; ++It )
	{
		UObject* Object = *It;
		AActor* Actor = Cast<AActor>(Object);
		if(Actor && Actor->HasAnyFlags(RF_ArchetypeObject) )
		{
			ArchetypeActor = Actor;
		}
	}
}

FString UActorFactoryArchetype::GetMenuName()
{
	if(ArchetypeActor)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *ArchetypeActor->GetFullName() );
	}
	else
	{
		return MenuName;
	}
}

/**
 *	UActorFactoryLensFlare
 */
    //## BEGIN PROPS ActorFactoryLensFlare
//    class ULensFlare* LensFlareObject;
    //## END PROPS ActorFactoryLensFlare
AActor* UActorFactoryLensFlare::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	ALensFlareSource* NewLensFlare = CastChecked<ALensFlareSource>(NewActor);

	if( LensFlareObject )
	{
		// Term Component
		NewLensFlare->ClearComponents();

		// Change properties
		NewLensFlare->RemoteRole = ROLE_None;
		NewLensFlare->bAlwaysRelevant = FALSE;
		NewLensFlare->NetUpdateFrequency = 0.0f; // could also set bNetTemporary but LD might further trigger it or something
		NewLensFlare->SetTemplate(LensFlareObject);

		// Init Component
		NewLensFlare->ConditionalUpdateComponents();
	}

	return NewLensFlare;
}

UBOOL UActorFactoryLensFlare::CanCreateActor(FString& OutErrorMsg)
{
	if(LensFlareObject)
	{
		return true;
	}
	else 
	{
		OutErrorMsg = TEXT("Error_CouldNotCreateActor_NoLensFlare");
		return false;
	}
}

void UActorFactoryLensFlare::AutoFillFields(class USelection* Selection)
{
	LensFlareObject = Selection->GetTop<ULensFlare>();
}

FString UActorFactoryLensFlare::GetMenuName()
{
	if (LensFlareObject)
	{
		return FString::Printf( TEXT("%s: %s"), *MenuName, *LensFlareObject->GetName() );
	}
	else
	{
		return MenuName;
	}
}

/*
 * 
 */
static void SetupFogVolumeActor(AActor* NewActor, UMaterialInterface* SelectedMaterial)
{
	AFogVolumeDensityInfo* NewFogVolume = CastChecked<AFogVolumeDensityInfo>(NewActor);

	// Find an unused name in the level
	INT NameIndex = 0;
	UObject* ExistingObject = NULL;
	FString PotentialMIName;
	do 
	{
		PotentialMIName = FString::Printf(TEXT("FogVolumeMI_%i"), NameIndex);
		ExistingObject = FindObject<UObject>(NewFogVolume->GetOutermost(), *PotentialMIName);
		NameIndex++;
	} 
	while (ExistingObject != NULL);

	// Create a new material instance, whose parent is the selected material in the generic browser, or EngineMaterials.FogVolumeMaterial if nothing is selected
	UMaterialInstanceConstant* NewMI = 
		ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), NewFogVolume->GetOutermost(), *PotentialMIName, RF_Transactional);
	NewMI->MarkPackageDirty(TRUE);

	// If a material was selected in the generic browser, use that as the parent, otherwise use the default fog volume material
	if (SelectedMaterial)
	{
		NewMI->SetParent(SelectedMaterial);
	}
	else
	{
		UMaterialInterface* DefaultFogVolumeMaterial = 
			(UMaterialInterface*)UObject::StaticLoadObject( UMaterialInterface::StaticClass(),NULL,TEXT("EngineMaterials.FogVolumeMaterial"),NULL,LOAD_None,NULL );
		NewMI->SetParent(DefaultFogVolumeMaterial);
	}

	// Set the new material instance on the fog volume
	NewFogVolume->DensityComponent->FogMaterial = NewMI;
}

/**
*	UActorFactoryFogVolumeConstantDensityInfo
*/
AActor* UActorFactoryFogVolumeConstantDensityInfo::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	SetupFogVolumeActor(NewActor, SelectedMaterial);
	
	return NewActor;
}

void UActorFactoryFogVolumeConstantDensityInfo::AutoFillFields(class USelection* Selection)
{
	SelectedMaterial = Selection->GetTop<UMaterialInterface>();
}

/**
*	UActorFactoryFogVolumeLinearHalfspaceDensityInfo
*/
AActor* UActorFactoryFogVolumeLinearHalfspaceDensityInfo::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	SetupFogVolumeActor(NewActor, SelectedMaterial);

	return NewActor;
}

void UActorFactoryFogVolumeLinearHalfspaceDensityInfo::AutoFillFields(class USelection* Selection)
{
	SelectedMaterial = Selection->GetTop<UMaterialInterface>();
}

/**
*	UActorFactoryFogVolumeSphericalDensityInfo
*/
AActor* UActorFactoryFogVolumeSphericalDensityInfo::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	AActor* NewActor = Super::CreateActor( Location, Rotation, ActorFactoryData );
	if( !NewActor )
	{
		return NULL;
	}

	SetupFogVolumeActor(NewActor, SelectedMaterial);

	AFogVolumeDensityInfo* NewFogVolume = CastChecked<AFogVolumeDensityInfo>(NewActor);

	if (NewFogVolume->AutomaticMeshComponent && NewFogVolume->AutomaticMeshComponent->StaticMesh)
	{
		UFogVolumeSphericalDensityComponent* SphericalComponent = CastChecked<UFogVolumeSphericalDensityComponent>(NewFogVolume->DensityComponent);
		const FLOAT ComponentBoundsRadius = NewFogVolume->AutomaticMeshComponent->StaticMesh->Bounds.SphereRadius;
		// Set the AutomaticMeshComponent's scale so that it will always bound the fog volume tightly
		NewFogVolume->AutomaticMeshComponent->Scale = (600.0f + 5.0f) / ComponentBoundsRadius;
	}

	return NewActor;
}

void UActorFactoryFogVolumeSphericalDensityInfo::AutoFillFields(class USelection* Selection)
{
	SelectedMaterial = Selection->GetTop<UMaterialInterface>();
}
