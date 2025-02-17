/*=============================================================================
	PhysXDestructible.cpp: Destructible Vertical Component.
	Copyright 2007-2008 AGEIA Technologies.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnFracturedStaticMesh.h"
#include "EngineMeshClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineParticleClasses.h"

#if WITH_NOVODEX
	#include "UnNovodexSupport.h"
	#include "PhysXDestructible.h"
#endif // WITH_NOVODEX

/*
 *	Local functions and definitions
 */

const FLOAT MaxForceNormalizationTime = 1.0f/250.0f;

#if WITH_NOVODEX
static UBOOL
CalculateDamageFromImpact( AActor * Actor, const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData,
						   FPhysXDestructibleParameters & Parameters, FLOAT & Damage, FVector & Position, INT & ThisIndex )
{
	const FRigidBodyCollisionInfo * Info[2] = { &Info0, &Info1 };

	ThisIndex = (INT)(Info1.Actor == Actor);

	if( ThisIndex == (INT)(Info0.Actor == Actor) )
	{
		return FALSE; // don't do self collision
	}

	NxActor * ThisActor = Info[ThisIndex]->Component->GetIndexedNxActor( Info[ThisIndex]->BodyIndex );
	if( !ThisActor )
	{
		return FALSE;	// Chunk was probably destroyed
	}

	FLOAT Force = RigidCollisionData.TotalNormalForceVector.Size();
	Force *= MaxForceNormalizationTime;
	Damage = Force*Parameters.ForceToDamage;

	if( Parameters.DamageCap > 0.0f )
	{
		Damage = Min( Damage, Parameters.DamageCap );
	}

	Position = FVector(0,0,0);
	for( INT InfoNum = 0; InfoNum < RigidCollisionData.ContactInfos.Num(); ++InfoNum )
	{
		const FRigidBodyContactInfo & ContactInfo = RigidCollisionData.ContactInfos( InfoNum );
		Position += ContactInfo.ContactPosition;
	}
	Position *= 1.0f/(FLOAT)RigidCollisionData.ContactInfos.Num();

	return TRUE;
}
#endif

static void HideMesh( USkeletalMeshComponent* MeshComp )
{
	// execActorCollision(false, false)
	if (FALSE != MeshComp->CollideActors)
	{
		MeshComp->CollideActors = FALSE;
		MeshComp->BeginDeferredReattach();
	}
	MeshComp->BlockActors = FALSE;

	// execSetTraceBlocking(false, false)
	MeshComp->BlockZeroExtent = FALSE;
	MeshComp->BlockNonZeroExtent = FALSE;

	// execSetHidden(frue)
	MeshComp->SetHiddenGame( TRUE );
	
	MeshComp->SetBlockRigidBody( FALSE );
}

static void ShowMesh( USkeletalMeshComponent* MeshComp )
{
	// execActorCollision(true, true)
	if (TRUE != MeshComp->CollideActors)
	{
		MeshComp->CollideActors = TRUE;
		MeshComp->BeginDeferredReattach();
	}
	//MeshComp->BlockActors = bBlockActors;
	MeshComp->BlockActors = TRUE;

	// execSetTraceBlocking(true, true)
	MeshComp->BlockZeroExtent = TRUE;
	MeshComp->BlockNonZeroExtent = TRUE;

	// execSetHidden(false)
	MeshComp->SetHiddenGame( FALSE );

	//MeshComp->SetBlockRigidBody(BlockRigidBody);
	MeshComp->SetBlockRigidBody( TRUE );
}

// Returns the number of chunks hidden
static INT SwitchToHidden( UPhysXDestructibleStructure * Structure, INT ChunkIndex )
{
	INT NumHidden = 0;
	FPhysXDestructibleChunk & Parent = Structure->Chunks( ChunkIndex );
	for( INT ChildIndex = Parent.FirstChildIndex; ChildIndex < Parent.FirstChildIndex + Parent.NumChildren; ++ChildIndex )
	{
		FPhysXDestructibleChunk & Child = Structure->Chunks( ChildIndex );
		if( Child.CurrentState == DCS_StaticChild || Child.CurrentState == DCS_DynamicChild )
		{
			Child.CurrentState = DCS_Hidden;
			NumHidden += 1 + SwitchToHidden( Structure, ChildIndex );
		}
	}
	return NumHidden;
}

static INT IntPairCmp( const FIntPair * A, const FIntPair * B )
{
	return (A->A != B->A) ? (A->A - B->A) : (A->B - B->B);
}

static UBOOL AggregateConvexOverlap( FKAggregateGeom & Geom1, const FMatrix & LocalToWorld1, const FVector & Scale3D1,
									 FKAggregateGeom & Geom2, const FMatrix & LocalToWorld2, const FVector & Scale3D2,
									 FLOAT Padding )
{
	for( INT ConvexIndex1 = 0; ConvexIndex1 < Geom1.ConvexElems.Num(); ++ConvexIndex1 )
	{
		FKConvexElem & Elem1 = Geom1.ConvexElems( ConvexIndex1 );
		for( INT ConvexIndex2 = 0; ConvexIndex2 < Geom2.ConvexElems.Num(); ++ConvexIndex2 )
		{
			FKConvexElem & Elem2 = Geom2.ConvexElems( ConvexIndex2 );
			if( ConvexOverlap( Elem1, LocalToWorld1, Scale3D1, Elem2, LocalToWorld2, Scale3D2, Padding ) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

static UBOOL ActorConvexOverlap( APhysXDestructibleActor & Actor1, APhysXDestructibleActor & Actor2, FLOAT Padding )
{
	FMatrix TM1 = FRotationTranslationMatrix( Actor1.Rotation, Actor1.Location );
	FMatrix TM2 = FRotationTranslationMatrix( Actor2.Rotation, Actor2.Location );
	FVector Scale1 = Actor1.DrawScale*Actor1.DrawScale3D;
	FVector Scale2 = Actor2.DrawScale*Actor2.DrawScale3D;

	for( INT FragmentIndex1 = 0; FragmentIndex1 < Actor1.FracturedStaticMeshComponent->GetNumFragments(); ++FragmentIndex1 )
	{
		UPhysXDestructibleAsset * DestAsset1 = Actor1.PhysXDestructible->DestructibleAssets( FragmentIndex1 );
		for( INT MeshIndex1 = 0; MeshIndex1 < DestAsset1->Assets.Num(); ++MeshIndex1 )
		{
			UPhysicsAsset * Asset1 = DestAsset1->Assets( MeshIndex1 );
			for( INT GeomIndex1 = 0; GeomIndex1 < Asset1->BodySetup.Num(); ++GeomIndex1 )
			{
				FKAggregateGeom & Geom1 = Asset1->BodySetup( GeomIndex1 )->AggGeom;
				for( INT FragmentIndex2 = 0; FragmentIndex2 < Actor2.FracturedStaticMeshComponent->GetNumFragments(); ++FragmentIndex2 )
				{
					UPhysXDestructibleAsset * DestAsset2 = Actor2.PhysXDestructible->DestructibleAssets( FragmentIndex2 );
					for( INT MeshIndex2 = 0; MeshIndex2 < DestAsset2->Assets.Num(); ++MeshIndex2 )
					{
						UPhysicsAsset * Asset2 = DestAsset2->Assets( MeshIndex2 );
						for( INT GeomIndex2 = 0; GeomIndex2 < Asset2->BodySetup.Num(); ++GeomIndex2 )
						{
							FKAggregateGeom & Geom2 = Asset2->BodySetup( GeomIndex2 )->AggGeom;
							if( AggregateConvexOverlap( Geom1, TM1, Scale1, Geom2, TM2, Scale2, Padding ) )
							{
								return TRUE;
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

/*
 *	APhysXEmitterSpawnable
 *
 *	Derived from AEmitter.	Just like AEmitterSpawnable, but contains RB volume fill data
 */
IMPLEMENT_CLASS(APhysXEmitterSpawnable);

void APhysXEmitterSpawnable::Term()
{
	if( VolumeFill )
	{
		delete VolumeFill;
		VolumeFill = NULL;
	}
}

/*
 *	UPhysXDestructibleComponent
 *
 *	Derived from UFracturedStaticMeshComponent.  Allows an alternative collision represtation.
 */
IMPLEMENT_CLASS(UPhysXDestructibleComponent);

UBOOL UPhysXDestructibleComponent::CreateDetailedCollisionFromDestructible( UPhysXDestructible * Destructible, URB_BodySetup * Template )
{
	if( Destructible == NULL )
	{
		return FALSE;
	}

	DetailedCollision = ConstructObject<URB_BodySetup>(URB_BodySetup::StaticClass(), this);
	if( Template )
	{
		DetailedCollision->CopyBodyPropertiesFrom( Template );
	}
	DetailedCollision->AggGeom.EmptyElements();

	BoxElemStart.Empty();
	BoxElemStart.AddZeroed( Destructible->DestructibleAssets.Num() );
	ConvexElemStart.Empty();
	ConvexElemStart.AddZeroed( Destructible->DestructibleAssets.Num() );
	Fragmented.Empty();
	Fragmented.AddZeroed( Destructible->DestructibleAssets.Num() );

	for( INT AssetIndex = 0; AssetIndex < Destructible->DestructibleAssets.Num(); ++AssetIndex )
	{
		UPhysXDestructibleAsset * DestructibleAsset = Destructible->DestructibleAssets( AssetIndex );
		INT Index = DetailedCollision->AggGeom.ConvexElems.AddZeroed();
		// N.B. we are assuming the _second_ body of the first asset is the unfractured mesh.
		if( DestructibleAsset->Assets.Num() > 0 )
		{
			if( DestructibleAsset->Assets(0)->BodySetup.Num() > 1 )	// Account for dummy bone
			{
				// Add boxes and record start position
				TArrayNoInit<FKBoxElem> & BoxElems = DestructibleAsset->Assets(0)->BodySetup(1)->AggGeom.BoxElems;
				BoxElemStart( AssetIndex ) = DetailedCollision->AggGeom.BoxElems.Num();
				DetailedCollision->AggGeom.BoxElems.Append( BoxElems );
				// Add convexes and record start position
				TArrayNoInit<FKConvexElem> & ConvexElem = DestructibleAsset->Assets(0)->BodySetup(1)->AggGeom.ConvexElems;
				ConvexElemStart( AssetIndex ) = DetailedCollision->AggGeom.ConvexElems.Num();
				DetailedCollision->AggGeom.ConvexElems.Append( ConvexElem );
			}
		}
	}

	return TRUE;
}

UBOOL UPhysXDestructibleComponent::DestroyStaticFragmentCollision( INT FragmentIndex )
{
	if( FragmentIndex < 0 || FragmentIndex >= Fragmented.Num() )
	{
		return FALSE;
	}

	if( Fragmented( FragmentIndex ) )
	{
		return FALSE;
	}

	return TRUE;
}

URB_BodySetup * UPhysXDestructibleComponent::GetRBBodySetup()
{
	return DetailedCollision;
}

void UPhysXDestructibleComponent::InitComponentRBPhys(UBOOL bFixed)
{
	if( BodyInstance )
	{
		return;
	}

	Super::InitComponentRBPhys( bFixed );

	// Now we must associate NxShapes with the fragments
}

/*
 *	UPhysicsLODVerticalDestructible
 */
IMPLEMENT_CLASS(UPhysicsLODVerticalDestructible);

#if WITH_NOVODEX

/*
 *	FPhysXDestructibleManager
 *
 *	Global parameters and management for the destructibes.
 */

FPhysXDestructibleManager::FPhysXDestructibleManager()
{
	// Defaults
	DynamicChunkFIFOMax = 1000;
	DynamicChunkFIFONum = 0;
	DebrisLifetime = 60.0f;
	FIFOHead = INDEX_NONE;
	FIFOTail = INDEX_NONE;
	Flags = 0;
}

FPhysXDestructibleManager::~FPhysXDestructibleManager()
{
}

void FPhysXDestructibleManager::SetMaxDynamicChunkCount( INT MaxChunkCount )
{
	DynamicChunkFIFOMax = Max( MaxChunkCount, 1 );

	CapDynamicChunkCount( DynamicChunkFIFOMax );
}

void FPhysXDestructibleManager::SetDebrisLifetime( FLOAT Lifetime )
{
	DebrisLifetime = Max( Lifetime, 0.0f );
}

void FPhysXDestructibleManager::TickManager( FLOAT DeltaTime )
{
	for( INT StructureKillIndex = 0; StructureKillIndex < StructureKillList.Num(); ++StructureKillIndex )
	{
		UPhysXDestructibleStructure * Structure = StructureKillList( StructureKillIndex );
		if( Structure )
		{
			INT StructureIndex;
			if( Structures.FindItem( Structure, StructureIndex ) )
			{
				Structures.Remove( StructureIndex );
			}
			Structure->Manager = NULL;
			debugf( TEXT("Destructible: Structure %s is being removed."), *Structure->GetName() );
			for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
			{
				APhysXDestructibleActor * & Actor = Structure->Actors( ActorIndex );
				if( Actor )
				{
					debugf( TEXT("Destructible: Warning!  Removed structure %s had actor %s"), *Structure->GetName(), *Actor->GetName() );
					Actor->Structure = NULL;
					Actor = NULL;
				}
			}
		}
	}
	StructureKillList.Empty();

	for( INT StructureIndex = 0; StructureIndex < Structures.Num(); ++StructureIndex )
	{
		UPhysXDestructibleStructure * Structure = Structures( StructureIndex );
		if( Structure )
		{
			Structure->TickStructure( DeltaTime );
		}
	}

	// Read values from vertical
	AWorldInfo * Wi = GWorld->GetWorldInfo();
	if(Wi)
	{
		UPhysicsLODVerticalDestructible * VeDe = Wi->DestructibleVertical;
		check(VeDe);
	
		SetMaxDynamicChunkCount( VeDe->MaxDynamicChunkCount );
		SetDebrisLifetime( VeDe->DebrisLifetime );
	}
}

void FPhysXDestructibleManager::SetFlag( EPhysXDestructibleManagerFlag Flag, UBOOL bValue )
{
	INT Mask = 1<<Flag;
	if( bValue )
	{
		Flags |= Mask;
	}
	else
	{
		Flags &= ~Mask;
	}
}

UBOOL FPhysXDestructibleManager::GetFlag( EPhysXDestructibleManagerFlag Flag )
{
	INT Mask = 1<<Flag;
	return (Flags&Mask) != 0;
}

UBOOL FPhysXDestructibleManager::RegisterActor( APhysXDestructibleActor * Actor )
{
	if( Actor->Structure )
	{
		return FALSE;
	}

	check( StructureKillList.Num() == 0 );
	check( FIFOHead == INDEX_NONE && FIFOTail == INDEX_NONE );

	TArray<APhysXDestructibleActor *> Actors;
	Actors.AddItem( Actor );

	Actor->Neighbors.Empty();

	UBOOL bFixed = ( Actor->Physics != PHYS_RigidBody );
	if( bFixed )
	{	// Static actor
		const FLOAT Padding = 1.0f;
		FBox Box = Actor->GetComponentsBoundingBox().ExpandBy( Padding );

		// Find structures that this actor touches
		for( INT StructureIndex = 0; StructureIndex < Structures.Num(); )
		{
			UPhysXDestructibleStructure * Structure = Structures( StructureIndex );
			UBOOL bOverlaps = FALSE;
			TArray<UBOOL> bOverlapArray;
			bOverlapArray.AddZeroed( Structure->Actors.Num() );
			for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
			{
				APhysXDestructibleActor * ExistingActor = Structure->Actors( ActorIndex );
				FBox ExistingActorBox = ExistingActor->GetComponentsBoundingBox().ExpandBy( Padding );
				if( Box.Intersect( ExistingActorBox ) )
				{
					if( ActorConvexOverlap( *Actor, *ExistingActor, Padding ) )
					{	// Record individual actor touches for neighbor list
						bOverlaps = TRUE;
						bOverlapArray( ActorIndex ) = TRUE;
					}
				}
			}
			if( bOverlaps )
			{
				// Record the actors, and destroy the structure
				INT ActorIndexOffset = Actors.Num();
				Actors.Append( Structure->Actors );
				Structures.Remove( StructureIndex );
				Structure->Manager = NULL;
				for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
				{
					APhysXDestructibleActor * & ExistingActor = Structure->Actors( ActorIndex );
					if( ExistingActor )
					{
						// Offset ExistingActor's current neighbors based upon its position in new structure
						for( INT NeighborIndex = 0; NeighborIndex < ExistingActor->Neighbors.Num(); ++NeighborIndex )
						{
							ExistingActor->Neighbors( NeighborIndex ) += ActorIndexOffset;
						}
						if( bOverlapArray( ActorIndex ) )
						{
							Actor->Neighbors.AddItem( ActorIndex + ActorIndexOffset );
							ExistingActor->Neighbors.AddItem( 0 );
						}
						// Prepare to be added to new structure
						ExistingActor->Structure = NULL;
						ExistingActor = NULL;
					}
				}
			}
			else
			{
				++StructureIndex;
			}
		}
	}

	// Create the new structure.  If valid, add it and return
	UPhysXDestructibleStructure * NewStructure = CreateStructure( Actors );
	if( NewStructure == NULL )
	{
		return FALSE;
	}

	Structures.AddItem( NewStructure );
	return TRUE;
}

UPhysXDestructibleStructure * FPhysXDestructibleManager::CreateStructure( const TArray<APhysXDestructibleActor *> & Actors )
{
	if( Actors.Num() == 0 )
	{
		return NULL;
	}

	UPhysXDestructibleStructure * Structure = ConstructObject<UPhysXDestructibleStructure>( UPhysXDestructibleStructure::StaticClass(), INVALID_OBJECT, NAME_None, RF_Standalone );
	Structure->Manager = this;
	Structure->Actors = Actors;

	for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
	{
		APhysXDestructibleActor * Actor = Structure->Actors( ActorIndex );

		UBOOL bFixed = ( Actor->Physics != PHYS_RigidBody );
		if( !bFixed )
		{
			check( Actors.Num() == 1 );	// All dynamic destructibles must be in their own structure
			Actor->PerFrameProcessBudget = 0;
		}

		Structure->PerFrameProcessBudget += Max( Actor->PerFrameProcessBudget, 0 );
		Structure->SupportDepth = Max( Structure->SupportDepth, Actor->SupportDepth );
		Actor->Structure = Structure;
		Actor->PartFirstChunkIndices.Empty();
		Actor->PartFirstChunkIndices.AddZeroed( Actor->PhysXDestructible->DestructibleAssets.Num() + 1 );

		FMatrix ActorTM = FRotationTranslationMatrix( Actor->Rotation, Actor->Location );
		FVector ActorScale = Actor->DrawScale*Actor->DrawScale3D;

		// Add all the assets
		for( INT FragmentIndex = 0; FragmentIndex < Actor->PhysXDestructible->DestructibleAssets.Num(); ++FragmentIndex )
		{
			UPhysXDestructibleAsset * Asset = Actor->PhysXDestructible->DestructibleAssets( FragmentIndex );

			INT FirstChunk = Structure->Chunks.Num();
			INT NumChunks = Asset->ChunkTree.Num();

			Actor->PartFirstChunkIndices( FragmentIndex ) = FirstChunk;

			Structure->Chunks.Add( NumChunks );

			for( INT i = 0; i < NumChunks; ++i )
			{
				FPhysXDestructibleChunk & Target = Structure->Chunks( FirstChunk + i );
				const FPhysXDestructibleAssetChunk & Source = Asset->ChunkTree( i );

				Target.ActorIndex			= ActorIndex;
				Target.Index				= FirstChunk + i;
				Target.FragmentIndex		= Source.FragmentIndex;
				Target.MeshIndex			= Source.MeshIndex;
				Target.BoneIndex			= Source.BoneIndex;
				Target.BoneName				= Source.BoneName;
				Target.BodyIndex			= Source.BodyIndex;
				Target.WorldMatrixValid		= FALSE;
				Target.WorldCentroidValid	= FALSE;
				Target.ParentIndex			= (Source.ParentIndex != INDEX_NONE) ? FirstChunk + Source.ParentIndex : INDEX_NONE;
				Target.FirstChildIndex		= (Source.FirstChildIndex != INDEX_NONE) ? FirstChunk + Source.FirstChildIndex : INDEX_NONE;
				Target.NumChildren			= Source.NumChildren;
				Target.Depth				= Source.Depth;
				Target.Age					= 0;
				Target.bCrumble				= FALSE;
				Target.Size					= Source.Size;
				Target.Structure			= Structure;
				Target.FIFOIndex			= INDEX_NONE;
				Target.Damage				= 0;
				Target.IsEnvironmentSupported = FALSE;
				Target.ShortestRoute		= -1;
				Target.NumSupporters		= 0;
				Target.IsRouting			= FALSE;
				Target.IsRouteValid			= FALSE;
				Target.IsRouteBlocker		= FALSE;
				Target.FirstOverlapIndex	= INDEX_NONE;
				Target.NumOverlaps			= 0;
				Target.NumChildrenDup		= Source.NumChildren;

				UPhysXDestructibleAsset * DestAsset = Actor->PhysXDestructible->DestructibleAssets( Target.FragmentIndex );
				UPhysicsAsset * Asset = DestAsset->Assets( Target.MeshIndex );
				FKAggregateGeom& Geom = Asset->BodySetup( Target.BodyIndex )->AggGeom;
				FBox Box = Geom.CalcAABB( ActorTM, ActorScale );
				Target.WorldCentroid = Box.GetCenter();	// For initial debug visualization
				Target.Radius = Box.GetExtent().Size();
				Target.RelativeCentroid = ActorTM.InverseTransformFVector( Target.WorldCentroid );
				if( !bFixed )
				{
					Target.WorldCentroidValid = FALSE;
					Target.CurrentState = Target.ParentIndex == INDEX_NONE ? DCS_DynamicRoot : DCS_DynamicChild;
					Target.IsRouteBlocker = TRUE;
				}
				else
				{
					Target.WorldCentroidValid = TRUE;
					Target.CurrentState = Target.ParentIndex == INDEX_NONE ? DCS_StaticRoot : DCS_StaticChild;
				}

				RegisterChunk( &Target );
			}
		}

		// Append "stop" value
		Actor->PartFirstChunkIndices( Actor->PhysXDestructible->DestructibleAssets.Num() ) = Structure->Chunks.Num();
	}

	// Skip adding support if there is no process budget
	if( Structure->PerFrameProcessBudget > 0 )
	{
		// Generate support structure
		const FLOAT Padding = 1.0f;

		TArray<INT> SupportChunks;
		TArray<FBox> SupportChunkAABBs;
		TArray<FMatrix> ActorTMs( Structure->Actors.Num() );

		for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
		{
			APhysXDestructibleActor * Actor = Structure->Actors( ActorIndex );
			ActorTMs( ActorIndex ) = FRotationTranslationMatrix( Actor->Rotation, Actor->Location );
		}

		for( INT ChunkIndex = 0; ChunkIndex < Structure->Chunks.Num(); ChunkIndex++ )
		{
			FPhysXDestructibleChunk & Chunk = Structure->Chunks( ChunkIndex );
			if( Chunk.Depth == Structure->SupportDepth )
			{
				APhysXDestructibleActor * Actor = Structure->Actors( Chunk.ActorIndex );
				UPhysXDestructibleAsset * DestAsset = Actor->PhysXDestructible->DestructibleAssets( Chunk.FragmentIndex );
				UPhysicsAsset * Asset = DestAsset->Assets( Chunk.MeshIndex );
				FKAggregateGeom & Geom = Asset->BodySetup( Chunk.BodyIndex )->AggGeom;
				FBox Box = Geom.CalcAABB( ActorTMs( Chunk.ActorIndex ), Actor->DrawScale*Actor->DrawScale3D ).ExpandBy( Padding );
				SupportChunks.AddItem( ChunkIndex );
				SupportChunkAABBs.AddItem( Box );
				UBOOL bWorldSupported = FALSE;
				if( Actor->bSupportChunksTouchWorld )
				{
					check( GWorld );
					FCheckResult Hit;
					bWorldSupported = GWorld->EncroachingWorldGeometry( Hit, Box.GetCenter(), Box.GetExtent(), TRUE );
				}
				UBOOL bFragmentSupported = FALSE;
				if( Actor->bSupportChunksInSupportFragment )
				{
					bFragmentSupported = Actor->FracturedStaticMeshComponent->IsRootFragment( Chunk.FragmentIndex );
				}
				Chunk.IsEnvironmentSupported = Actor->bSupportChunksTouchWorld && Actor->bSupportChunksInSupportFragment ?
					( bWorldSupported && bFragmentSupported ) : ( bWorldSupported || bFragmentSupported );
			}
		}

		// Find AABB overlaps
		TArray<FIntPair> AABBOverlaps;
		OverlapAABBs( SupportChunkAABBs, AABBOverlaps );

		// Now do detailed overlap test
		TArray<FIntPair> Overlaps;
		for( INT OverlapIndex = 0; OverlapIndex < AABBOverlaps.Num(); ++OverlapIndex )
		{
			FIntPair & Pair = AABBOverlaps( OverlapIndex );
			INT IndexA = SupportChunks( Pair.A );
			INT IndexB = SupportChunks( Pair.B );
			FPhysXDestructibleChunk & ChunkA = Structure->Chunks( IndexA );
			APhysXDestructibleActor * ActorA = Structure->Actors( ChunkA.ActorIndex );
			UPhysXDestructibleAsset * DestAssetA = ActorA->PhysXDestructible->DestructibleAssets( ChunkA.FragmentIndex );
			UPhysicsAsset * AssetA = DestAssetA->Assets( ChunkA.MeshIndex );
			FKAggregateGeom & GeomA = AssetA->BodySetup( ChunkA.BodyIndex )->AggGeom;
			FPhysXDestructibleChunk & ChunkB = Structure->Chunks( IndexB );
			APhysXDestructibleActor * ActorB = Structure->Actors( ChunkB.ActorIndex );
			UPhysXDestructibleAsset * DestAssetB = ActorB->PhysXDestructible->DestructibleAssets( ChunkB.FragmentIndex );
			UPhysicsAsset * AssetB = DestAssetB->Assets( ChunkB.MeshIndex );
			FKAggregateGeom & GeomB = AssetB->BodySetup( ChunkB.BodyIndex )->AggGeom;
			if( AggregateConvexOverlap( GeomA, ActorTMs( ChunkA.ActorIndex ), ActorA->DrawScale*ActorA->DrawScale3D,
										GeomB, ActorTMs( ChunkB.ActorIndex ), ActorB->DrawScale*ActorB->DrawScale3D, 2*Padding ) )
			{
				INT NextIndex = Overlaps.Add( 2 );
				FIntPair NewPair = { IndexA, IndexB };
				Overlaps( NextIndex ) = NewPair;
				// Make 'Overlaps' symmetric - i.e. if (A,B) is in the array, then so is (B,A).
				NewPair.A = IndexB;
				NewPair.B = IndexA;
				Overlaps( NextIndex+1 ) = NewPair;
			}
		}

		// Now sort Overlaps by index A and index B (symmetric, so it doesn't matter which we sort by first)
		appQsort( &Overlaps(0), Overlaps.Num(), sizeof(FIntPair), (QSORT_COMPARE)IntPairCmp );

		INT LastIndexA = INDEX_NONE;
		for( INT OverlapIndex = 0; OverlapIndex < Overlaps.Num(); ++OverlapIndex )
		{
			FIntPair & Pair = Overlaps( OverlapIndex );
			FPhysXDestructibleChunk & ChunkA = Structure->Chunks( Pair.A );
			FPhysXDestructibleChunk & ChunkB = Structure->Chunks( Pair.B );
			if( Pair.A != LastIndexA )
			{
				LastIndexA = Pair.A;
				ChunkA.FirstOverlapIndex = Structure->Overlaps.Num();
				ChunkA.NumOverlaps = 0;
			}
			FPhysXDestructibleOverlap Overlap;
			Overlap.Adjacent = 0;
			Overlap.ChunkIndex0 = Pair.A;
			Overlap.ChunkIndex1 = Pair.B;
			Structure->Overlaps.AddItem( Overlap );
			ChunkA.NumOverlaps++;
		}

		// Connect the siblings - can this be made more efficient?
		for (INT OverlapIndex = 0; OverlapIndex < Structure->Overlaps.Num(); OverlapIndex++)
		{
			FPhysXDestructibleOverlap& Overlap = Structure->Overlaps(OverlapIndex);
			INT OtherOverlapIndex;
			for (OtherOverlapIndex = 0; OtherOverlapIndex < OverlapIndex; OtherOverlapIndex++)
			{
				FPhysXDestructibleOverlap& OtherOverlap = Structure->Overlaps(OtherOverlapIndex);
				if (OtherOverlap.ChunkIndex0 == Overlap.ChunkIndex1 && 
					OtherOverlap.ChunkIndex1 == Overlap.ChunkIndex0)
				{
					OtherOverlap.Adjacent = OverlapIndex;
					Overlap.Adjacent = OtherOverlapIndex;
				}
			}
		}

		//Init IsEnvironmentSupported related chunk member.
		TArray<INT> RouteFrontierChunks;
		INT RouteDepth = 0;
		for (INT SupportIndex = 0; SupportIndex < SupportChunks.Num(); SupportIndex++)
		{
			INT ChunkIndex = SupportChunks(SupportIndex);
			FPhysXDestructibleChunk& Chunk = Structure->Chunks(ChunkIndex);
			if (Chunk.IsEnvironmentSupported)
			{
					Chunk.ShortestRoute = 0;
					Chunk.IsRouteValid = TRUE;
					RouteFrontierChunks.AddItem(ChunkIndex);
			}
		}

		// prepare Fifo capacity and so on according to SupportDepth
		Structure->PseudoSupporterFifo.Empty(SupportChunks.Num());
		Structure->FractureOriginFifo.Empty(SupportChunks.Num());
		Structure->PerFrameProcessBudget = Min(Structure->PerFrameProcessBudget, SupportChunks.Num());

		while (RouteFrontierChunks.Num())
		{
			TArray<INT>	NewFrontierChunks;
			NewFrontierChunks.Empty();
			RouteDepth++;

			for (INT i = 0; i < RouteFrontierChunks.Num(); i++)
			{
				INT ChunkIndex = RouteFrontierChunks(i);
				FPhysXDestructibleChunk& Chunk = Structure->Chunks(ChunkIndex);
				for (INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; j++)
				{
					FPhysXDestructibleOverlap& overlap = Structure->Overlaps(j);
					FPhysXDestructibleChunk& OtherChunk = Structure->Chunks(overlap.ChunkIndex1);
					if (!OtherChunk.IsRouteValid)
					{
						OtherChunk.ShortestRoute = RouteDepth;
						OtherChunk.IsRouteValid = TRUE;
						NewFrontierChunks.AddItem(overlap.ChunkIndex1);
						OtherChunk.NumSupporters++;
					}
					else if (OtherChunk.ShortestRoute == RouteDepth)
					{
						OtherChunk.NumSupporters++;
					}
					else
					{
						// Sanity check
						check(OtherChunk.ShortestRoute == RouteDepth-1 || OtherChunk.ShortestRoute == RouteDepth-2);
					}
				}

			}

			RouteFrontierChunks = NewFrontierChunks;
		}

		// all chunks are not supported yet all are taken as fixed in the environment.
		for (INT SupportIndex = 0; SupportIndex < SupportChunks.Num(); SupportIndex++)
		{
			INT ChunkIndex = SupportChunks( SupportIndex );
			FPhysXDestructibleChunk& Chunk = Structure->Chunks( ChunkIndex );
			if (!Chunk.IsRouteValid)
			{
				Chunk.IsEnvironmentSupported = TRUE;
				Chunk.ShortestRoute = 0;
				Chunk.IsRouteValid = TRUE;
				check(Chunk.NumSupporters == 0);
				check(Chunk.IsRouting == FALSE);
				check(Chunk.IsRouteBlocker == FALSE);
			}
		}

		// Test
#if 0
		for (INT SupportIndex = 0; SupportIndex < SupportChunks.Num(); SupportIndex++)
		{
			FPhysXDestructibleChunk& Chunk = Structure->Chunks(SupportIndex);

			check(Chunk.CurrentState == DCS_StaticRoot || Chunk.CurrentState == DCS_StaticChild);
				check(Chunk.IsRouteValid);
			int testCount = 0;
			for (INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; j++)
			{
				FPhysXDestructibleOverlap& overlap = Structure->Overlaps(j);
				FPhysXDestructibleChunk& OtherChunk = Structure->Chunks(overlap.ChunkIndex1);

				check(OtherChunk.CurrentState == DCS_StaticRoot || OtherChunk.CurrentState == DCS_StaticChild);
				check(OtherChunk.IsRouteValid);
				if (OtherChunk.ShortestRoute + 1 == Chunk.ShortestRoute)
				{
					testCount++;
				}
			}
			check(Chunk.NumSupporters == testCount);
		}
#endif
	}

	return Structure;
}

UBOOL FPhysXDestructibleManager::RemoveStructure( UPhysXDestructibleStructure * Structure )
{
	if( Structure )
	{
		StructureKillList.AddUniqueItem( Structure );
		return TRUE;
	}

	return FALSE;
}

UBOOL FPhysXDestructibleManager::RegisterChunk( FPhysXDestructibleChunk * Chunk )
{
	check( Chunk );

	if( Chunk->FIFOIndex >= 0 )
	{
		appErrorf( TEXT("FPhysXDestructibleManager::RegisterChunk attempting to re-register Chunk.") );
		return FALSE;
	}

	Chunk->FIFOIndex = FIFOEntries.Add();

	FIFOEntry & Entry = FIFOEntries( Chunk->FIFOIndex );
	Entry.Structure = Chunk->Structure;
	Entry.ChunkIndex = Chunk->Index;
	Entry.NextEntry = INDEX_NONE;
	Entry.PrevEntry = INDEX_NONE;

	return TRUE;
}

void FPhysXDestructibleManager::AddChunk( FPhysXDestructibleChunk * Chunk )
{
	check( Chunk && Chunk->FIFOIndex >= 0 );

	FIFOEntry & Entry = FIFOEntries( Chunk->FIFOIndex );

	if( Entry.NextEntry >= 0 || Entry.PrevEntry >= 0 )
	{
		appErrorf( TEXT("Chunk already added") );
		return;
	}

	CapDynamicChunkCount( DynamicChunkFIFOMax-1 );

	if( FIFOTail >= 0 )
	{
		check( FIFOEntries( FIFOTail ).NextEntry < 0 );
		FIFOEntries( FIFOTail ).NextEntry = Chunk->FIFOIndex;
		Entry.PrevEntry = FIFOTail;
	}
	else
	{
		check( FIFOHead < 0 );
		FIFOHead = Chunk->FIFOIndex;
	}
	FIFOTail = Chunk->FIFOIndex;

	++DynamicChunkFIFONum;
}

FPhysXDestructibleChunk * FPhysXDestructibleManager::RemoveFirstChunk()
{
	if( FIFOHead < 0 )
	{
		check( FIFOTail < 0 );
		check( DynamicChunkFIFONum == 0 );
		return NULL;
	}

	FIFOEntry & Entry = FIFOEntries( FIFOHead );

	FIFOHead = Entry.NextEntry;
	if( FIFOHead < 0 )
	{
		FIFOTail = INDEX_NONE;
	}
	else
	{
		FIFOEntries( FIFOHead ).PrevEntry = INDEX_NONE;
	}

	Entry.NextEntry = Entry.PrevEntry = INDEX_NONE;
	--DynamicChunkFIFONum;

	return &Entry.Structure->Chunks( Entry.ChunkIndex );
}

FPhysXDestructibleChunk * FPhysXDestructibleManager::RemoveChunk( FPhysXDestructibleChunk * Chunk )
{
	if( FIFOHead < 0 )
	{
		check( FIFOTail < 0 );
		check( DynamicChunkFIFONum == 0 );
		return NULL;
	}

	if( Chunk->FIFOIndex == FIFOHead )
	{
		return RemoveFirstChunk();
	}

	FIFOEntry & Entry = FIFOEntries( Chunk->FIFOIndex );

	if( Chunk->FIFOIndex == FIFOTail )
	{
		FIFOTail = FIFOEntries( FIFOTail ).PrevEntry;
		check( FIFOTail >= 0 );	// The single-chunk case should have been handled by Chunk == HeadChunk, above
		FIFOEntries( FIFOTail ).NextEntry = INDEX_NONE;
	}
	else
	{
		FIFOEntries( Entry.NextEntry ).PrevEntry = Entry.PrevEntry;
		FIFOEntries( Entry.PrevEntry ).NextEntry = Entry.NextEntry;
	}

	Entry.NextEntry = Entry.PrevEntry = INDEX_NONE;
	--DynamicChunkFIFONum;

	return Chunk;
}

void FPhysXDestructibleManager::CapDynamicChunkCount( INT ChunkCountCap )
{
	check( ChunkCountCap >= 0 );
	while( ChunkCountCap < DynamicChunkFIFONum )
	{
		FPhysXDestructibleChunk * RemovedChunk = RemoveFirstChunk();
		if( RemovedChunk )
		{
			RemovedChunk->Structure->CrumbleChunk( RemovedChunk->Index );
		}
	}
}
#endif // WITH_NOVODEX

/*
 *	UPhysXDestructible
 *
 *	Contains a list of UPhysXDestructibleAsset.  It is referenced by a APhysXDestructibleActor.
 */
IMPLEMENT_CLASS(UPhysXDestructible);

UBOOL
UPhysXDestructible::ApplyCookingScalesToAssets()
{
	for( INT AssetIndex = 0; AssetIndex < DestructibleAssets.Num(); ++AssetIndex )
	{
		UPhysXDestructibleAsset * DestructibleAsset = DestructibleAssets( AssetIndex );
		for( INT MeshIndex = 0; MeshIndex < DestructibleAsset->Assets.Num(); ++MeshIndex )
		{
			UPhysicsAsset * PhysicsAsset = DestructibleAsset->Assets( MeshIndex );
			UBOOL bChanged = FALSE;
			for( INT BodyIndex = 0; BodyIndex < PhysicsAsset->BodySetup.Num(); ++BodyIndex )
			{
				URB_BodySetup * RB_BodySetup = PhysicsAsset->BodySetup( BodyIndex );
				// Compare scales individually to see if we need to change the cache array
				if( RB_BodySetup->PreCachedPhysScale.Num() == CookingScales.Num() )
				{
					INT i;
					for( i = 0; i < CookingScales.Num(); ++i )
					{
						FVector & V1 = CookingScales( i );
						INT j;
						for( j = 0; j < RB_BodySetup->PreCachedPhysScale.Num(); ++j )
						{
							FVector & V2 = RB_BodySetup->PreCachedPhysScale( j );
							if( !(V1-V2).IsNearlyZero() )
							{
								break;
							}
						}
						if( j < RB_BodySetup->PreCachedPhysScale.Num() )
						{
							break;
						}
					}
					if( i == CookingScales.Num() )
					{
						continue;
					}
				}
				bChanged = TRUE;
				RB_BodySetup->PreCachedPhysScale = CookingScales;
			}
			if( bChanged )
			{
				PhysicsAsset->MarkPackageDirty();
			}
		}
	}

	return TRUE;
}


/*
 *	UPhysXDestructibleAsset
 *
 *	Contains the classes needed to instantiate one APhysXDestructiblePart.
 */
IMPLEMENT_CLASS(UPhysXDestructibleAsset);

void UPhysXDestructibleAsset::ComputeChunkSurfaceAreaAndVolume( INT ChunkIndex, FLOAT & Area, FLOAT & Volume ) const
{
	Area = 0.0f;
	Volume = 0.0f;

	if( ChunkIndex >= 0 && ChunkIndex < ChunkTree.Num() )
	{
		const FPhysXDestructibleAssetChunk & Chunk = ChunkTree( ChunkIndex );
		if( Chunk.MeshIndex >= 0 && Chunk.MeshIndex < Assets.Num() )
		{
			if( Chunk.BodyIndex >= 0 && Chunk.BodyIndex < Assets( Chunk.MeshIndex )->BodySetup.Num() )
			{
				URB_BodySetup* BodySetup = Assets( Chunk.MeshIndex )->BodySetup( Chunk.BodyIndex );
				for( INT i = 0; i < BodySetup->AggGeom.ConvexElems.Num(); ++i )
				{
					FLOAT ElemArea;
					FLOAT ElemVolume;
					BodySetup->AggGeom.ConvexElems(i).CalcSurfaceAreaAndVolume( ElemArea, ElemVolume );
					Area += ElemArea;
					Volume += ElemVolume;
				}
			}
		}
	}
}

/*
 *	APhysXDestructibleActor
 *
 *	Derived from AFracturedStaticMeshActor.  Instead of fracturing into AFracturedStaticMeshParts,
 *	it fractures into APhysXDestructibleParts.
 */
IMPLEMENT_CLASS(APhysXDestructibleActor);

void APhysXDestructibleActor::Init()
{
#if WITH_NOVODEX
	Parts.Empty();
	Parts.AddZeroed( PhysXDestructible->FracturedStaticMesh->GetNumFragments() );

	INT MaxDepth = 0;
	for( INT i = 0; i < PhysXDestructible->DestructibleAssets.Num(); ++i )
	{
		MaxDepth = Max( MaxDepth, PhysXDestructible->DestructibleAssets(i)->MaxDepth );
	}
	SupportDepth = Clamp( SupportDepth, 0, MaxDepth );

	GWorld->RBPhysScene->PhysXDestructibleManager->RegisterActor( this );

	LinearSize = (DrawScale3D*PhysXDestructible->FracturedStaticMesh->Bounds.BoxExtent).GetMax();

	NumPartsRemaining = PhysXDestructible->FracturedStaticMesh->GetNumFragments();

	if( DestructibleParameters.ForceToDamage > 0 && DestructibleParameters.DepthParameters( 0 ).bTakeImpactDamage )
	{
		// Dividing by MaxPhysicsDeltaTime because our force calculation uses the same factor to normalize for framerate
		FracturedStaticMeshComponent->BodyInstance->SetContactReportForceThreshold(
			U2PScale * DestructibleParameters.DamageThreshold / (DestructibleParameters.ForceToDamage*MaxForceNormalizationTime) );
	}

	if( DestructibleParameters.DamageThreshold > 0.0f )
	{
		DestructibleParameters.ScaledDamageToRadius = LinearSize*DestructibleParameters.DamageToRadius/DestructibleParameters.DamageThreshold;
	}

	// This is done to handle old formats, and ensure we have a sufficient # of DepthParameters
	while( DestructibleParameters.DepthParameters.Num() < MaxDepth+2 )
	{
		FPhysXDestructibleDepthParameters Params;
		appMemzero( &Params, sizeof( Params ) );
		if( DestructibleParameters.DepthParameters.Num() == 0 )
		{	// Set level 0 parameters to the more expensive settings
			Params.bDoNotTimeOut = TRUE;
			Params.bPlayParticleEffect = TRUE;
			Params.bPlaySoundEffect = TRUE;
			Params.bTakeImpactDamage = TRUE;
		}
		Params.bNoKillDummy = TRUE;
		DestructibleParameters.DepthParameters.AddItem( Params );
	}

	if( DestructibleParameters.CrumbleParticleSize < 1.0f )
	{
		debugf(TEXT("WARNING!  PhysXDestructibleActor %s has a CrumbleParticleSize less than 1.  This is very small in Unreal Units, now in use.  Assuming in PhysX units, converting to Unreal Units."), *GetName() );
		DestructibleParameters.CrumbleParticleSize *= P2UScale;
	}
#endif
}

void APhysXDestructibleActor::Term()
{
#if WITH_NOVODEX
	if( Structure )
	{
		INT RemainingActors = 0;
		for( INT ActorIndex = 0; ActorIndex < Structure->Actors.Num(); ++ActorIndex )
		{
			APhysXDestructibleActor * & Actor = Structure->Actors( ActorIndex );
			if( Actor == this )
			{
				Actor = NULL;
			}
			RemainingActors += (INT)( Actor != NULL );
		}
		if( RemainingActors == 0 )
		{
			Structure->Manager->RemoveStructure( Structure );
		}
	}
#endif
}

void APhysXDestructibleActor::TakeRadiusDamage( AController* InstigatedBy, FLOAT BaseDamage, FLOAT DamageRadius, UClass* DamageType, FLOAT Momentum, FVector HurtOrigin, UBOOL bFullDamage, AActor* DamageCauser )
{
	UBOOL bHackSpawned = FALSE;	// For now

	if( DestructibleParameters.DamageCap > 0.0f )
	{
		BaseDamage = Min( BaseDamage, DestructibleParameters.DamageCap );
	}

	// Search over intact fragments and see which ones might take damage
	for( INT FragmentIndex = 0; FragmentIndex < FracturedStaticMeshComponent->GetNumFragments(); ++FragmentIndex )
	{
		if( FracturedStaticMeshComponent->IsFragmentVisible( FragmentIndex ) )
		{
			FBox AABB = FracturedStaticMeshComponent->GetFragmentBox( FragmentIndex );
			FVector	Disp = AABB.GetCenter() - HurtOrigin;
			FLOAT Radius = AABB.GetExtent().Size();
			if( Disp.SizeSquared() < Square( DamageRadius+Radius ) )
			{
				INT FirstChunkIndex = PartFirstChunkIndices( FragmentIndex );
				INT NumChunks = PartFirstChunkIndices( FragmentIndex+1 ) - FirstChunkIndex;
				UBOOL bFractured = Structure->ApplyDamage(
					FirstChunkIndex, NumChunks, BaseDamage, DamageRadius, Momentum, HurtOrigin, TRUE, bFullDamage );
				bHackSpawned |= bFractured;
			}
		}
	}

	// Must spawn them all for now
	if( bHackSpawned )
	{
		const UBOOL bFixed = (Physics != PHYS_RigidBody) || bStatic;

		for( INT FragmentIndex = 0; FragmentIndex < FracturedStaticMeshComponent->GetNumFragments(); ++FragmentIndex )
		{
			if( FracturedStaticMeshComponent->IsFragmentVisible( FragmentIndex ) )
			{
				SpawnPart( FragmentIndex, bFixed );
			}
		}
	}
}

void APhysXDestructibleActor::NativeTakeDamage( INT Damage, AController * EventInstigator, FVector HitLocation, FVector Momentum, UClass * DamageType, FTraceHitInfo HitInfo, AActor * DamageCauser )
{
	// Somehow hit an invisible fragment
	if( !FracturedStaticMeshComponent->IsFragmentVisible( HitInfo.Item ) )
	{
		return;
	}

	// Use TakeRadiusDamage in order to propagate damage into the actor
	FLOAT BaseDamage = (FLOAT)Damage;
	if( DestructibleParameters.DamageCap > 0.0f )
	{
		BaseDamage = Min( BaseDamage, DestructibleParameters.DamageCap );
	}
	FLOAT DamageRadius = BaseDamage*DestructibleParameters.ScaledDamageToRadius;
	TakeRadiusDamage( EventInstigator, BaseDamage, DamageRadius, DamageType, Momentum.Size(), HitLocation, FALSE, DamageCauser );

	// Take care of neighbors too
	for( INT NeighborIndex = 0; NeighborIndex < Neighbors.Num(); ++NeighborIndex )
	{
		INT ActorIndex = Neighbors( NeighborIndex );
		APhysXDestructibleActor * Neighbor = Structure->Actors( ActorIndex );
		if( Neighbor )
		{
			// Approximate damage propagation radius
			FLOAT EffectiveRadius = 0.5f*(DamageRadius + BaseDamage*Neighbor->DestructibleParameters.ScaledDamageToRadius);
			Neighbor->TakeRadiusDamage( EventInstigator, BaseDamage, EffectiveRadius, DamageType, Momentum.Size(), HitLocation, FALSE, DamageCauser );
		}
	}
}

void APhysXDestructibleActor::NativeSpawnEffects()
{
	// Spawn particles for the crumbled pieces
	// This is meant to use FillVolume.  Therefore it will not need the chunks' positions,
	// and one emitter may be used for all chunks
	if( VolumeFill != NULL )
	{
		APhysXEmitterSpawnable * Effect = (APhysXEmitterSpawnable*)GWorld->SpawnActor
			( APhysXEmitterSpawnable::StaticClass(), NAME_None, Location, Rotation, NULL, FALSE, FALSE, this );
		Effect->SetTemplate(DestructibleParameters.CrumbleParticleSystem, true);
		Effect->VolumeFill = VolumeFill;
		// Get the max lifetime so we can set a lifespan for the emitter
		FLOAT MaxLife = 0.0f;
		for( INT i = 0; i < Effect->ParticleSystemComponent->EmitterInstances.Num(); ++i )
		{
			FParticleEmitterInstance * EmitterInstance = Effect->ParticleSystemComponent->EmitterInstances(i);
			UParticleSpriteEmitter * SpriteTemplate = EmitterInstance->SpriteTemplate;
			UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel( EmitterInstance );
			for( INT ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ++ModuleIndex )
			{
				UParticleModule* HighModule	= LODLevel->SpawnModules( ModuleIndex );
				UParticleModuleLifetime * LifetimeModule = Cast<UParticleModuleLifetime>( HighModule );
				if( LifetimeModule && LifetimeModule->bEnabled )
				{
					MaxLife = Max( MaxLife, LifetimeModule->GetMaxLifetime() );
				}
			}
		}
		Effect->LifeSpan = MaxLife + 1.0f;	// +1 just in case
		VolumeFill = NULL;	// APhysXEmitterSpawnable will delete this
	}

	if( GWorld->GetWorldInfo()->NetMode != NM_DedicatedServer )
	{
		if( bPlayFractureSound && DestructibleParameters.FractureSound )
		{
			PlaySound( DestructibleParameters.FractureSound, TRUE );
		}
	}

	bPlayFractureSound = false;

	eventSpawnEffects();
}

void APhysXDestructibleActor::SpawnPart( INT FragmentIndex, UBOOL bFixed )
{
#if WITH_NOVODEX
	if( !FracturedStaticMeshComponent->IsFragmentVisible( FragmentIndex ) )
	{	// Already spawned
		return;
	}
	check( Parts( FragmentIndex ) == NULL );

	TArray<BYTE> FragmentVis = FracturedStaticMeshComponent->GetVisibleFragments();
	FragmentVis( FragmentIndex ) = 0;

	// Save off COM, vel & ang. vel
	NxActor * nActor = FracturedStaticMeshComponent->GetNxActor();
	FVector COM = N2UVectorCopy( nActor->getCMassGlobalPosition() );
	FVector LinVel = N2UVectorCopy( nActor->getLinearVelocity() );
	FVector AngVel = N2UVectorCopy( nActor->getAngularVelocity() );

	/* Hack: For now, we must spawn all parts here! */
	UBOOL bCanDestroyRBBPhys = TRUE;
	for( INT Index = 0; Index < FracturedStaticMeshComponent->GetNumFragments(); ++Index )
	{
		if( FragmentVis( Index ) )
		{
			bCanDestroyRBBPhys = FALSE;
			break;
		}
	}
	// Swap physics representations
	if( bCanDestroyRBBPhys )
	{
		Physics = PHYS_None;
		FracturedStaticMeshComponent->TermComponentRBPhys(NULL);
	}
//	DestructibleComponent->InitComponentRBPhys( TRUE );
	SetCollisionType( COLLIDE_NoCollision );
	/* End hack */

	FracturedStaticMeshComponent->SetVisibleFragments( FragmentVis );

	APhysXDestructiblePart * Part = CastChecked<APhysXDestructiblePart>( GWorld->SpawnActor( APhysXDestructiblePart::StaticClass(), NAME_None, Location, Rotation ) );
	Parts( FragmentIndex ) = Part;
	check( Part );

	Part->DestructibleAsset = PhysXDestructible->DestructibleAssets( FragmentIndex );
	Part->DestructibleActor = this;
	Part->Structure = Structure;
	Part->FirstChunk = PartFirstChunkIndices( FragmentIndex );
	Part->NumChunks = Part->DestructibleAsset->ChunkTree.Num();
	Part->NumChunksRemaining.Empty( Part->DestructibleAsset->Meshes.Num() );
	Part->NumMeshesRemaining = Part->DestructibleAsset->Meshes.Num();
	check( Part->NumMeshesRemaining );

	for( INT Index = 0; Index < Part->DestructibleAsset->Meshes.Num(); ++Index )
	{
		USkeletalMeshComponent * SkelMeshComp = ConstructObject<USkeletalMeshComponent>(
				USkeletalMeshComponent::StaticClass(), 
				Part,
				MakeUniqueObjectName(Part, USkeletalMeshComponent::StaticClass()));

		Part->SkeletalMeshComponents.AddItem( SkelMeshComp );
		
		// USkeletalMeshComponent fields
		SkelMeshComp->SkeletalMesh						= Part->DestructibleAsset->Meshes( Index );
		SkelMeshComp->PhysicsAsset						= Part->DestructibleAsset->Assets( Index );
		SkelMeshComp->bHasPhysicsAssetInstance			= TRUE;
		SkelMeshComp->bUpdateKinematicBonesFromAnimation= FALSE;		
		SkelMeshComp->PhysicsWeight						= 1.0f;
		SkelMeshComp->LightEnvironment					= Part->LightEnvironment;

		if( CollisionComponent )
		{
			SkelMeshComp->bFluidDrain				= CollisionComponent->bFluidDrain;
			SkelMeshComp->bFluidTwoWay				= CollisionComponent->bFluidTwoWay;
			SkelMeshComp->bIgnoreForceField			= CollisionComponent->bIgnoreForceField;
			SkelMeshComp->bIgnoreRadialForce		= CollisionComponent->bIgnoreRadialForce;
			SkelMeshComp->bIgnoreRadialImpulse		= CollisionComponent->bIgnoreRadialImpulse;
			SkelMeshComp->bUseCompartment			= CollisionComponent->bUseCompartment;
			SkelMeshComp->bCastDynamicShadow		= CollisionComponent->bCastDynamicShadow;
			SkelMeshComp->RBChannel					= CollisionComponent->RBChannel;
			SkelMeshComp->RBCollideWithChannels		= CollisionComponent->RBCollideWithChannels;
			SkelMeshComp->RBDominanceGroup			= CollisionComponent->RBDominanceGroup;
			SkelMeshComp->PhysMaterialOverride		= CollisionComponent->PhysMaterialOverride;
			SkelMeshComp->bNotifyRigidBodyCollision = CollisionComponent->bNotifyRigidBodyCollision;
		}

		Part->NumChunksRemaining( Index ) = SkelMeshComp->SkeletalMesh->RefSkeleton.Num() - 1;

		Part->AttachComponent( SkelMeshComp );

		HideMesh( SkelMeshComp );
	}

	// Sanity check
	INT ChunkCount = 0;
	for( INT Index = 0; Index < Part->DestructibleAsset->Meshes.Num(); ++Index )
	{
		ChunkCount += Part->NumChunksRemaining( Index );
	}
	check( ChunkCount == Part->NumChunks );

	// UE3 will call this later (without harm), and may allow some amortization
	Part->InitRBPhys();

	Part->BlockRigidBody = TRUE;
	Part->SetDrawScale( DrawScale );
	Part->SetDrawScale3D( DrawScale3D );

	FMatrix InvParentTM;
	InvParentTM.SetIdentity();

	FVector Scale = DrawScale*DrawScale3D;

	for( INT i = Part->FirstChunk; i < Part->FirstChunk + Part->NumChunks; ++i )
	{
		FPhysXDestructibleChunk & Chunk = Structure->Chunks( i );
		USkeletalMeshComponent * MeshComp = Structure->GetChunkMesh( i );
		URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );

		// Update transformations
		FMatrix BoneTM = MeshComp->GetBoneMatrix( Chunk.BoneIndex );
		BoneTM.RemoveScaling();
		Chunk.WorldMatrix = BoneTM;
		Chunk.WorldMatrixValid = bFixed;
//		Chunk.RelativeMatrix = Chunk.WorldMatrix * InvParentTM;

		FBox ChunkAABB = MeshComp->PhysicsAsset->BodySetup( Chunk.BodyIndex )->AggGeom.CalcAABB( BoneTM, Scale );
		Chunk.WorldCentroid = ChunkAABB.GetCenter();
		Chunk.Radius = ChunkAABB.GetExtent().Size();
		Chunk.RelativeCentroid = BoneTM.InverseSafe().TransformFVector( Chunk.WorldCentroid );
		Chunk.WorldCentroidValid = bFixed;

		InvParentTM = Chunk.WorldMatrix.Inverse();

		if ( Chunk.ParentIndex == INDEX_NONE )
		{
			Structure->ShowChunk( i, bFixed, &BoneTM );
			if( bFixed )
			{
				Chunk.CurrentState = DCS_StaticRoot;
			}
			else
			{
				Chunk.CurrentState = DCS_DynamicRoot;
				Structure->Manager->AddChunk( &Chunk );
				Structure->Active.AddItem( Chunk.Index );
				NxActor * nBodyActor = Body->GetNxActor();
				nBodyActor->wakeUp();
				nBodyActor->setAngularVelocity( U2NVectorCopy( AngVel ) );
				nBodyActor->setLinearVelocity( U2NVectorCopy( LinVel ) +
					U2NVectorCopy( AngVel ).cross( nBodyActor->getCMassGlobalPosition() - U2NVectorCopy( COM ) ) );
			}
		}
		else
		{
			MeshComp->HideBone( Chunk.BoneIndex, FALSE );
			URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
			Body->TermBody( GWorld->RBPhysScene );
			Chunk.CurrentState = bFixed ? DCS_StaticChild : DCS_DynamicChild;
		}
	}
#endif
}

void APhysXDestructibleActor::QueueEffects( FPhysXDestructibleChunk & Chunk, INT DepthOffset )
{
	const INT Depth = Clamp( Chunk.Depth+DepthOffset, 0, DestructibleParameters.DepthParameters.Num()-1 );
	FPhysXDestructibleDepthParameters & DepthParameters = DestructibleParameters.DepthParameters( Depth );

	bPlayFractureSound = bPlayFractureSound || DepthParameters.bPlaySoundEffect;

	if( DepthParameters.bPlayParticleEffect )
	{
		FSpawnBasis SpawnBasis;
		SpawnBasis.Location = Chunk.WorldCentroid;
		SpawnBasis.Rotation = FracturedStaticMeshComponent->GetFragmentAverageExteriorNormal( Chunk.FragmentIndex ).Rotation();
		SpawnBasis.Scale = Min( 1.0f, Chunk.Radius/FracturedStaticMeshComponent->GetFragmentBox( Chunk.FragmentIndex ).GetExtent().Size() );
		EffectBases.AddItem( SpawnBasis );
	}
}

void APhysXDestructibleActor::OnRigidBodyCollision( const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData )
{
#if WITH_NOVODEX
	FVector Position;
	FLOAT Damage;
	INT ThisIndex;

	UBOOL bImpact = CalculateDamageFromImpact( this, Info0, Info1, RigidCollisionData, DestructibleParameters, Damage, Position, ThisIndex );

	if( bImpact && Damage > DestructibleParameters.DamageThreshold )
	{
		NativeTakeDamage( (INT)(Damage+0.5f), NULL, Position, FVector(0,0,0), NULL );
	}
#endif
}

void APhysXDestructibleActor::PostLoad()
{
	Super::PostLoad();

	if( Physics == PHYS_RigidBody )
	{
		TickGroup = TG_PostAsyncWork;
		bStatic = FALSE;
		bNoDelete = FALSE;
		bCollideActors = TRUE;
		bBlockActors = TRUE;
//		bWorldGeometry = FALSE;
		bCollideWorld = FALSE;
		RemoteRole = ROLE_SimulatedProxy;
		bNetInitialRotation = TRUE;
		bSkipActorPropertyReplication = FALSE;
		bReplicateRigidBodyLocation = TRUE;
//		bProjTarget = TRUE;
		bAlwaysRelevant = TRUE;
		bUpdateSimulatedPosition = TRUE;
		bReplicateMovement = TRUE;
		bBlocksTeleport = TRUE;
		bBlocksNavigation = TRUE;
		bMovable = TRUE;
		UDynamicLightEnvironmentComponent * DynamicLightEnvironment = ConstructObject<UDynamicLightEnvironmentComponent>( UDynamicLightEnvironmentComponent::StaticClass(), this );
		Components.AddItem( DynamicLightEnvironment );
		LightEnvironment = DynamicLightEnvironment;
		LightEnvironment->SetEnabled( FALSE );
		FracturedStaticMeshComponent->SetLightEnvironment( LightEnvironment );
		FracturedStaticMeshComponent->bForceDirectLightMap = FALSE;
		FracturedStaticMeshComponent->LightingChannels.Dynamic = TRUE;
	}
}

/*
 *	APhysXDestructiblePart
 *
 *	Basic unit of hierarchical destruction.
 */
IMPLEMENT_CLASS(APhysXDestructiblePart);

FBox APhysXDestructiblePart::GetSkeletalMeshComponentsBoundingBox()
{
	FBox Box(0);

	for( INT Index = 0; Index < SkeletalMeshComponents.Num(); ++Index )
	{
		USkeletalMeshComponent * SkelMeshComp = SkeletalMeshComponents(Index);
		if( SkelMeshComp && SkelMeshComp->IsAttached() )
		{
			Box += SkelMeshComp->Bounds.GetBox();
		}
	}

	return Box;
}

void APhysXDestructiblePart::InitRBPhys()
{
	// We define our own initialization to avoid confusion.
	// We simply want to call InitComponentRBPhys on all SkeletalMeshComponents.
	// AActor::InitRBPhys has special handling for PHYS_RigidBody which we don't want.

	if(bDeleteMe)
	{
		return;
	}

#if WITH_NOVODEX
	if (!GWorld->RBPhysScene)
	{
		return;
	}
#endif // WITH_NOVODEX

	// Physics for object are considered 'fixed' unless Physics mode is PHYS_RigidBody
	UBOOL bFixed = TRUE;
	if( Physics == PHYS_RigidBody )
	{
		bFixed = FALSE;
	}

	for( INT Index = 0; Index < SkeletalMeshComponents.Num(); ++Index )
	{
		USkeletalMeshComponent * SkelMeshComp = SkeletalMeshComponents( Index );
		if( SkelMeshComp )
		{
			// Initialize any physics for this component.
			SkelMeshComp->InitComponentRBPhys( bFixed );
		}
	}
}

void APhysXDestructiblePart::TermRBPhys( FRBPhysScene* Scene )
{
	Super::TermRBPhys( Scene );
}

void APhysXDestructiblePart::SyncActorToRBPhysics()
{
	/* This is not needed for a dynamic PhysXDestructiblePart */
}

void APhysXDestructiblePart::OnRigidBodyCollision( const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData )
{
#if WITH_NOVODEX
	check( DestructibleActor );

	FVector Position;
	FLOAT Damage;
	INT ThisIndex;

	UBOOL bImpact = CalculateDamageFromImpact( this, Info0, Info1, RigidCollisionData, DestructibleActor->DestructibleParameters, Damage, Position, ThisIndex );

	if( bImpact && Damage > DestructibleActor->DestructibleParameters.DamageThreshold )
	{
		FLOAT DamageRadius = Damage*DestructibleActor->DestructibleParameters.ScaledDamageToRadius;

		const FRigidBodyCollisionInfo * Info = ThisIndex == 0 ? (&Info0) : (&Info1);
		if( Info->BodyIndex >= 0 )
		{
			// A reverse lookup would be great here
			for( INT ChunkIndex = FirstChunk; ChunkIndex < FirstChunk+NumChunks; ++ChunkIndex )
			{
				if( Structure->GetChunkMesh( ChunkIndex ) == Info->Component )
				{
					FPhysXDestructibleChunk & Chunk = Structure->Chunks( ChunkIndex );
					if( Chunk.BodyIndex == Info->BodyIndex )
					{
						if( Chunk.CurrentState == DCS_DynamicRoot )
						{
							// This is a dynamic root, damage just it and exit
							TArray<INT> AffectedChunks;
							Structure->DamageChunk( ChunkIndex, Position, Damage, DamageRadius, FALSE, AffectedChunks );
							for( INT i = 0; i < AffectedChunks.Num(); ++i )
							{
								FVector Impulse(0,0,0);
								Structure->FractureChunk( AffectedChunks(i), Position, Impulse, FALSE );
							}
							return;
						}
					}
				}
			}
		}

		// Otherwise use Apply in order to propagate damage
		Structure->ApplyDamage( FirstChunk, NumChunks, Damage, DamageRadius, 0.0f, Position, FALSE, FALSE );
	}
#endif
}

void APhysXDestructiblePart::TakeRadiusDamage( AController* InstigatedBy, FLOAT BaseDamage, FLOAT DamageRadius, UClass* DamageType, FLOAT Momentum, FVector HurtOrigin, UBOOL bFullDamage, AActor* DamageCauser )
{
	if( DestructibleActor->DestructibleParameters.DamageCap > 0.0f )
	{
		BaseDamage = Min( BaseDamage, DestructibleActor->DestructibleParameters.DamageCap );
	}
	Structure->ApplyDamage( FirstChunk, NumChunks, BaseDamage, DamageRadius, Momentum, HurtOrigin, TRUE, bFullDamage );
}

void APhysXDestructiblePart::TakeDamage( INT Damage, AController * EventInstigator, FVector HitLocation, FVector Momentum, UClass * DamageType, FTraceHitInfo HitInfo, AActor * DamageCauser )
{
	FLOAT BaseDamage = (FLOAT)Damage;
	if( DestructibleActor->DestructibleParameters.DamageCap > 0.0f )
	{
		BaseDamage = Min( BaseDamage, DestructibleActor->DestructibleParameters.DamageCap );
	}
	FLOAT DamageRadius = BaseDamage*DestructibleActor->DestructibleParameters.ScaledDamageToRadius;

	if( HitInfo.Item >= 0 )
	{
		// A reverse lookup would be great here
		for( INT ChunkIndex = FirstChunk; ChunkIndex < FirstChunk+NumChunks; ++ChunkIndex )
		{
			if( Structure->GetChunkMesh( ChunkIndex ) == HitInfo.HitComponent )
			{
				FPhysXDestructibleChunk & Chunk = Structure->Chunks( ChunkIndex );
				if( Chunk.BoneIndex == HitInfo.Item )
				{
					if( Chunk.CurrentState == DCS_DynamicRoot )
					{
						// This is a dynamic root, damage just it and exit
						TArray<INT> AffectedChunks;
						Structure->DamageChunk( ChunkIndex, HitLocation, BaseDamage, DamageRadius, FALSE, AffectedChunks );
						for( INT i = 0; i < AffectedChunks.Num(); ++i )
						{
							INT ChunkIndex = AffectedChunks(i);
							FVector Impulse = (Structure->GetChunkCentroid( ChunkIndex ) - HitLocation).SafeNormal() * Momentum.Size();
							Structure->FractureChunk( ChunkIndex, HitLocation, Impulse, TRUE );
						}
						return;
					}
				}
			}
		}
	}

	// Otherwise use TakeRadiusDamage in order to propagate damage
	TakeRadiusDamage( EventInstigator, BaseDamage, DamageRadius, DamageType, Momentum.Size(), HitLocation, FALSE, DamageCauser );
}


/*
 *	APhysXDestructibleStructure
 *
 *	Contiguous group of destructible actors.
 */
IMPLEMENT_CLASS(UPhysXDestructibleStructure);

void UPhysXDestructibleStructure::TickStructure( FLOAT DeltaTime )
{
#if WITH_NOVODEX
	const FLOAT MAX_RADIUS = 0.1f*WORLD_MAX;

	// Process active chunks
	for( INT i = 0; i < Active.Num(); )
	{
		INT ChunkIndex = Active( i );
		FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
		APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
		if( !Actor )
		{
			++i;
			continue;
		}
		MarkMoved( ChunkIndex );
		Chunk.Age += DeltaTime;
		if( Chunk.CurrentState == DCS_Hidden )
		{
			Active( i ) = Active( Active.Num() - 1 );
			Active.Remove( Active.Num() - 1 );
		}
		else
		if ( (GetChunkCentroid(Active(i)) - Actor->Location).SizeSquared() > MAX_RADIUS*MAX_RADIUS )
		{	// Remove this chunk, it's too far away
			HideChunk( ChunkIndex );
			Manager->RemoveChunk( &Chunk );
			Active( i ) = Active( Active.Num() - 1 );
			Active.Remove( Active.Num() - 1 );
		}
		else
		if( !Actor->DestructibleParameters.DepthParameters( Chunk.Depth+1 ).bDoNotTimeOut
			&& Chunk.Age >= Manager->DebrisLifetime )
		{	// Crumble this chunk, it's expired
			CrumbleChunk( ChunkIndex );
			Manager->RemoveChunk( &Chunk );
			Active( i ) = Active( Active.Num() - 1 );
			Active.Remove( Active.Num() - 1 );
		}
		else
		{	// Leave it alone and go to the next chunk
			++i;
		}
	}

	// Process effects (crumbling, particle systems, sounds)
	if( GWorld->GetWorldInfo() &&
		(GWorld->GetWorldInfo()->NetMode == NM_Standalone || GWorld->GetWorldInfo()->Role != ROLE_Authority) )
	{
		// Only show this if we are running offline or are a client
		for( INT i = 0; i < Actors.Num(); ++i )
		{
			APhysXDestructibleActor * Actor = Actors(i);
			if( Actor )
			{
				Actor->NativeSpawnEffects();

				// Debug drawing
				if( Manager->GetFlag( DMF_VisualizeSupport ) )
				{
					// Build a list of all the lines we want to draw
					TArray<ULineBatchComponent::FLine> DebugLines;
					INT FirstChunk = Actor->PartFirstChunkIndices( 0 );
					INT LastChunk = Actor->PartFirstChunkIndices( Actor->PhysXDestructible->DestructibleAssets.Num() );
					for( INT i = FirstChunk; i < LastChunk; ++i )
					{
						FPhysXDestructibleChunk & Chunk = Actor->Structure->Chunks(i);
						if (Chunk.Depth != Actor->Structure->SupportDepth)
						{
							continue;
						}
						if( Chunk.CurrentState != DCS_StaticRoot && Chunk.CurrentState != DCS_StaticChild )
						{
							continue;
						}
						if( Chunk.IsEnvironmentSupported && Chunk.WorldCentroidValid )
						{
							FColor R(255,0,0), G(0,255,0), B(0,0,255);
							new(DebugLines) ULineBatchComponent::FLine(Chunk.WorldCentroid-FVector(1,0,0), Chunk.WorldCentroid+FVector(1,0,0), R, 0.f, SDPG_Foreground);
							new(DebugLines) ULineBatchComponent::FLine(Chunk.WorldCentroid-FVector(0,1,0), Chunk.WorldCentroid+FVector(0,1,0), G, 0.f, SDPG_Foreground);
							new(DebugLines) ULineBatchComponent::FLine(Chunk.WorldCentroid-FVector(0,0,1), Chunk.WorldCentroid+FVector(0,0,1), B, 0.f, SDPG_Foreground);
						}

						for( INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; ++j )
						{
							FPhysXDestructibleOverlap& Overlap = Actor->Structure->Overlaps(j);
							FPhysXDestructibleChunk& OtherChunk = Actor->Structure->Chunks(Overlap.ChunkIndex1);

							if( OtherChunk.CurrentState != DCS_StaticRoot && OtherChunk.CurrentState != DCS_StaticChild )
							{
								continue;
							}
							// if chunk is OtherChunk's support child 
							if( OtherChunk.ShortestRoute + 1 == Chunk.ShortestRoute )
							{
								if( Chunk.WorldCentroidValid && OtherChunk.WorldCentroidValid )
								{
									FVector MidPoint = (Chunk.WorldCentroid + OtherChunk.WorldCentroid)/2;
									new(DebugLines) ULineBatchComponent::FLine(OtherChunk.WorldCentroid, MidPoint, FColor(255,255,255), 0.f, SDPG_Foreground);
									static const FColor LineColor2 = FColor::MakeRandomColor();
									new(DebugLines) ULineBatchComponent::FLine(MidPoint, Chunk.WorldCentroid, LineColor2, 0.f, SDPG_Foreground);
								}
							}
						}
					}
					// Draw them all in one call.
					GWorld->LineBatcher->DrawLines(DebugLines);
				}
			}
		}
	}

	// Here we update support graphs
	PropagateFracture();

	for( INT ActorKillIndex = 0; ActorKillIndex < ActorKillList.Num(); ++ActorKillIndex )
	{
		APhysXDestructibleActor * Actor = ActorKillList( ActorKillIndex );
		if( Actor )
		{
			INT ActorIndex;
			if( Actors.FindItem( Actor, ActorIndex ) )
			{
				Actors( ActorIndex ) = NULL;
			}
			Actor->Structure = NULL;
			debugf( TEXT("Destructible: Actor %s is being removed."), *Actor->GetName() );
			GWorld->DestroyActor( Actor, TRUE );
		}
	}
	ActorKillList.Empty();
#endif
}

UBOOL UPhysXDestructibleStructure::ApplyDamage( INT FirstChunkIndex, INT NumChunks, FLOAT BaseDamage, FLOAT DamageRadius, FLOAT Momentum, FVector HurtOrigin, UBOOL bInheritRootVel, UBOOL bFullDamage )
{
	TArray<INT> AffectedChunks;
	for( INT ChunkIndex = FirstChunkIndex; ChunkIndex < FirstChunkIndex+NumChunks; ++ChunkIndex )
	{
		FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
		if( Chunk.CurrentState == DCS_StaticRoot || Chunk.CurrentState == DCS_DynamicRoot )
		{
			DamageChunk( ChunkIndex, HurtOrigin, BaseDamage, DamageRadius, bFullDamage, AffectedChunks );
		}
	}
	for( INT i = 0; i < AffectedChunks.Num(); ++i )
	{
		INT ChunkIndex = AffectedChunks(i);
		FVector Impulse = (GetChunkCentroid( ChunkIndex ) - HurtOrigin).SafeNormal() * Momentum;
		FractureChunk( ChunkIndex, HurtOrigin, Impulse, bInheritRootVel );
	}
	return AffectedChunks.Num() > 0;
}

void UPhysXDestructibleStructure::PropagateFracture()
{
	if( !PerFrameProcessBudget )
	{
		FractureOriginChunks.Empty();
		return;
	}

	for (INT i = 0; i < FractureOriginChunks.Num(); i++)
	{
		AppendFractureOriginFifo(FractureOriginChunks(i));
	}
	FractureOriginChunks.Empty();

	RouteUpdateArea.Empty(PerFrameProcessBudget);

	ExtendRerouteAreaFromPseudoSupporter(RouteUpdateArea, PerFrameProcessBudget);
	ExtendRerouteAreaFromFractureOrigin(RouteUpdateArea, PerFrameProcessBudget);
	RerouteArea(RouteUpdateArea);
}

void UPhysXDestructibleStructure::SupportDepthPassiveFracture(INT ChunkIndex)
{
	FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);
	check(Chunk.Depth >= SupportDepth);
	if (Chunk.CurrentState == DCS_Hidden)
	{
		for (INT i = Chunk.FirstChildIndex; i < Chunk.FirstChildIndex  + Chunk.NumChildren; i++)
		{
			SupportDepthPassiveFracture(i);
		}
	}
	else if (Chunk.CurrentState == DCS_StaticRoot)
	{
		FractureChunk( ChunkIndex, FVector(0,0,0), FVector(0,0,0), FALSE );
	}
}

void UPhysXDestructibleStructure::AppendFractureOriginFifo(INT ChunkIndex)
{
	FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);
	if (Chunk.Depth < SupportDepth)
	{
		for (INT i = Chunk.FirstChildIndex; i < Chunk.FirstChildIndex  + Chunk.NumChildren; i++)
		{
			AppendFractureOriginFifo(i);
		}
	}
	else if (Chunk.Depth == SupportDepth)
	{
		if (!Chunk.IsRouteBlocker)
		{
			FractureOriginFifo.AddItem(ChunkIndex);
			Chunk.IsRouteBlocker = TRUE;
		}
	}
	else if (Chunk.Depth > SupportDepth)
	{
		for (INT i = Chunk.Depth - SupportDepth; i > 0; i--)
		{
			ChunkIndex = Chunks(ChunkIndex).ParentIndex;
		}
		if (!Chunks(ChunkIndex).IsRouteBlocker)
		{
			FractureOriginFifo.AddItem(ChunkIndex);
			Chunks(ChunkIndex).IsRouteBlocker = TRUE;
			SupportDepthPassiveFracture(ChunkIndex);
		}
	}
}

void UPhysXDestructibleStructure::ExtendRerouteAreaFromPseudoSupporter(TArray<INT>& Area, INT Limit)
{
	while (true)
	{
		if (PseudoSupporterFifoStart == PseudoSupporterFifo.Num())
		{
			// if no Pseudo Supporter left to process, it is a chance to clear the array for later use. This ensure PseudoSupporterFifo.Num() will never exceed SupportChunks.Num().
			if (PseudoSupporterFifoStart)
			{
				// reset PseudoSupporterFifo
				PseudoSupporterFifo.Empty(PseudoSupporterFifo.GetSlack() + PseudoSupporterFifo.Num());
				PseudoSupporterFifoStart = 0;
			}
			break;
		}

		if (Area.Num() >= Limit)
		{
			break;
		}

		// pop PseudoSupporterFifo
		check(PseudoSupporterFifoStart < PseudoSupporterFifo.Num());
		INT ChunkIndex = PseudoSupporterFifo(PseudoSupporterFifoStart++);
		FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);
		check(Chunk.IsRouteValid);

		for (INT i = Chunk.FirstOverlapIndex; i < Chunk.FirstOverlapIndex+ Chunk.NumOverlaps; i++)
		{
			FPhysXDestructibleOverlap& overlap = Overlaps(i);
			FPhysXDestructibleChunk& OtherChunk = Chunks(overlap.ChunkIndex1);

			if (OtherChunk.IsRouteValid)
			{
				// if OtherChunk is taken as Chunk's support child
				if (OtherChunk.ShortestRoute == Chunk.ShortestRoute + 1)
				{
					OtherChunk.NumSupporters--;
					if (OtherChunk.NumSupporters == 0)
					{
						// push PseudoSupporterFifo
						PseudoSupporterFifo.AddItem(overlap.ChunkIndex1);
					}
				}

			}
		}

		//
		Chunk.IsRouteValid = FALSE;
		Area.AddItem(ChunkIndex);
	}
}

void UPhysXDestructibleStructure::ExtendRerouteAreaFromFractureOrigin(TArray<INT>& Area, INT Limit)
{
	while (FractureOriginFifoStart < FractureOriginFifo.Num() && Area.Num() < Limit)
	{
		// pop FractureOriginFifo
		FPhysXDestructibleChunk& Chunk = Chunks(FractureOriginFifo(FractureOriginFifoStart++));

		if (Chunk.IsRouteValid)
		{
			for (INT i = Chunk.FirstOverlapIndex; i < Chunk.FirstOverlapIndex+ Chunk.NumOverlaps; i++)
			{
				FPhysXDestructibleOverlap& overlap = Overlaps(i);
				FPhysXDestructibleChunk& OtherChunk = Chunks(overlap.ChunkIndex1);

				if (OtherChunk.IsRouteValid)
				{
					// if OtherChunk is taken as Chunk's support child
					if (OtherChunk.ShortestRoute == Chunk.ShortestRoute + 1)
					{
						OtherChunk.NumSupporters--;
						if (OtherChunk.NumSupporters == 0)
						{
							// push PseudoSupporterFifo
							PseudoSupporterFifo.AddItem(overlap.ChunkIndex1);
						}
					}
				}
			}
			Chunk.IsRouteValid = FALSE;
			Chunk.NumSupporters = 0;
		}

		ExtendRerouteAreaFromPseudoSupporter(Area, Limit);
	}
}

void UPhysXDestructibleStructure::RerouteArea(TArray<INT>& Area)
{
	if (Area.Num() == 0)
	{
		// just a quick return path.
		return;
	}

	INT AntiSelfSupportDepth;
	if (PseudoSupporterFifoStart < PseudoSupporterFifo.Num())
	{
		// get the first of PseudoSupporterFifo, that has the min of all Pseudo Supporters' ShortestRoute.
		AntiSelfSupportDepth = Chunks(PseudoSupporterFifo(PseudoSupporterFifoStart)).ShortestRoute;
	}
	else
	{
		AntiSelfSupportDepth = 0x7fffffff;
	}

	RouteUpdateFifo.Empty(PerFrameProcessBudget);
	RouteUpdateFifoStart = 0;
	for (INT i = 0; i < Area.Num(); i++)
	{
		INT ChunkIndex = Area(i);
		FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);

		check(!Chunk.IsRouteValid);
		check(Chunk.NumSupporters == 0);

		if (Chunk.IsRouteBlocker)
		{
			continue;
		}

		for (INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; j++)
		{
			FPhysXDestructibleOverlap& overlap = Overlaps(j);
			FPhysXDestructibleChunk& OtherChunk = Chunks(overlap.ChunkIndex1);

			if (OtherChunk.IsRouteValid && OtherChunk.ShortestRoute < AntiSelfSupportDepth)
			{
				INT OtherShortestRoutePlus = OtherChunk.ShortestRoute + 1;
				if (!Chunk.IsRouting)
				{
					Chunk.IsRouting = TRUE;
					RouteUpdateFifo.AddItem(ChunkIndex);
					Chunk.ShortestRoute = OtherShortestRoutePlus;
				}
				else if (Chunk.ShortestRoute > OtherShortestRoutePlus)
				{
					Chunk.ShortestRoute = OtherShortestRoutePlus;
				}
			}
		}
	}

	while (RouteUpdateFifoStart < RouteUpdateFifo.Num())
	{
		// pop RouteUpdateFifo
		FPhysXDestructibleChunk& Chunk = Chunks(RouteUpdateFifo(RouteUpdateFifoStart++));

		if (!Chunk.IsRouteValid)
		{
			Chunk.IsRouteValid = TRUE;
			for (INT i = RouteUpdateFifoStart; i < RouteUpdateFifo.Num(); i++) 
			{
				Chunks(RouteUpdateFifo(i)).IsRouteValid = TRUE;
			}
		}

		Chunk.IsRouting = FALSE;
		INT ShortestRoutePlus = Chunk.ShortestRoute + 1;

		for (INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; j++)
		{
			FPhysXDestructibleOverlap& overlap = Overlaps(j);
			FPhysXDestructibleChunk& OtherChunk = Chunks(overlap.ChunkIndex1);

			if (OtherChunk.IsRouteValid)
			{
				if (!OtherChunk.IsRouting)
				{
					if (Chunk.ShortestRoute == OtherChunk.ShortestRoute + 1)
					{
						Chunk.NumSupporters++;
					}
					else if (OtherChunk.ShortestRoute == ShortestRoutePlus)
					{
						// if OtherChunk.NumSupporters == 0 && OtherChunk.ShortestRoute != 0, it is pseudo supported and not allowed to inc NumSupporters.
						if (OtherChunk.NumSupporters)
						{
							OtherChunk.NumSupporters++;
						}
					}
				}
			}
			else if (!OtherChunk.IsRouteBlocker)
			{
				if (!OtherChunk.IsRouting)
				{
					OtherChunk.IsRouting = TRUE;
					// push RouteUpdateFifo
					RouteUpdateFifo.AddItem(overlap.ChunkIndex1);
					OtherChunk.ShortestRoute = ShortestRoutePlus;
				}
				else if (OtherChunk.ShortestRoute > ShortestRoutePlus)
				{
					OtherChunk.ShortestRoute = ShortestRoutePlus;
				}
			}
		}
	}

	PassiveFractureChunks.Empty(PerFrameProcessBudget*2);
	for (INT i = 0; i < Area.Num(); i++)
	{
		INT ChunkIndex = Area(i);
		FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);

		if (!Chunk.IsRouteValid)
		{
			// find all chunks that are no longer supported from any possible route.
			if (!Chunk.IsRouteBlocker)
			{
				check(Chunk.CurrentState == DCS_StaticRoot || Chunk.CurrentState == DCS_StaticChild);
				Chunk.IsRouteBlocker = TRUE;

				PassiveFractureChunks.AddItem(ChunkIndex);
				INT parentChunkIndex = Chunk.ParentIndex;
				while (parentChunkIndex != INDEX_NONE)
				{
					FPhysXDestructibleChunk& parentChunk = Chunks(parentChunkIndex);
					if (--parentChunk.NumChildrenDup)
					{
						break;
					}
					parentChunk.IsRouteBlocker = TRUE;
					PassiveFractureChunks.AddItem(parentChunkIndex);
					parentChunkIndex = parentChunk.ParentIndex;
				}
			}
		}
	}

	for (INT i = 0; i < PassiveFractureChunks.Num(); i++)
	{
		INT ChunkIndex = PassiveFractureChunks(i);
		FPhysXDestructibleChunk& Chunk = Chunks(ChunkIndex);
		INT parentChunkIndex = Chunk.ParentIndex;
		// Call FractureChunk() if the chunk is the biggest possible
		if (parentChunkIndex != INDEX_NONE)
		{
			if (Chunks(parentChunkIndex).NumChildrenDup == 0)
			{
				continue;
			}
			Chunks(parentChunkIndex).NumChildrenDup++;
		}
		check(Chunk.IsRouteBlocker == TRUE);
		FractureChunk( ChunkIndex, FVector(0,0,0), FVector(0,0,0), FALSE );
	}

#if 0	// Testing
	for (INT i = 0; i < Chunks.Num(); i++)
	{
		FPhysXDestructibleChunk& Chunk = Chunks(i);
		if (Chunk.Depth != SupportDepth)
		{
			continue;
		}

		if (Chunk.IsRouteValid)
		{
			int testCount = 0;
			for (INT j = Chunk.FirstOverlapIndex; j < Chunk.FirstOverlapIndex + Chunk.NumOverlaps; j++)
			{
				FPhysXDestructibleOverlap& overlap = Overlaps(j);
				FPhysXDestructibleChunk& OtherChunk = Chunks(overlap.ChunkIndex1);

				if (OtherChunk.IsRouteValid)
				{
					if (OtherChunk.ShortestRoute + 1 == Chunk.ShortestRoute)
					{
						testCount++;
					}
				}
			}
			check(Chunk.NumSupporters == testCount || (Chunk.NumSupporters == 0 && Chunk.ShortestRoute));
		}
		else
		{
			check(Chunk.CurrentState != DCS_StaticRoot && Chunk.CurrentState != DCS_StaticChild);
			check(Chunk.NumSupporters == 0);
		}
	}
#endif

}

#if WITH_NOVODEX
NxActor* UPhysXDestructibleStructure::GetChunkNxActor( INT ChunkIndex )
{
	FPhysXDestructibleChunk& Chunk = Chunks( ChunkIndex );
	USkeletalMeshComponent* MeshComp = GetChunkMesh( ChunkIndex );
	URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
	return Body->GetNxActor();
}
#endif

USkeletalMeshComponent* UPhysXDestructibleStructure::GetChunkMesh( INT ChunkIndex )
{
	FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
	APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
	APhysXDestructiblePart * Part = Actor->Parts( Chunk.FragmentIndex );
	return Part ? Part->SkeletalMeshComponents( Chunk.MeshIndex ) : NULL;
}

void UPhysXDestructibleStructure::ShowChunk( INT ChunkIndex, BOOL bFixed, FMatrix * InitTM )
{
	FPhysXDestructibleChunk& Chunk = Chunks( ChunkIndex );
	USkeletalMeshComponent* MeshComp = GetChunkMesh( ChunkIndex );
	ShowMesh( MeshComp );
	FMatrix BoneTM;
	if( !InitTM )
	{
		BoneTM = GetChunkMatrix( ChunkIndex );
		InitTM = &BoneTM;
	}
	MeshComp->UnHideBone( Chunk.BoneIndex );
	URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
	APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
	FPhysXDestructibleParameters & DestructibleParameters = Actor->DestructibleParameters;
	if( DestructibleParameters.ForceToDamage > 0 &&
		DestructibleParameters.DepthParameters( Chunk.Depth+1 ).bTakeImpactDamage )
	{
		// Dividing by MaxPhysicsDeltaTime because our force calculation uses the same factor to normalize for framerate
		Body->ContactReportForceThreshold = U2PScale * DestructibleParameters.DamageThreshold / (DestructibleParameters.ForceToDamage*MaxForceNormalizationTime);
	}
	Body->InitBody( 
		MeshComp->PhysicsAsset->BodySetup( Chunk.BodyIndex ),
		*InitTM,
		MeshComp->GetOwner()->DrawScale*MeshComp->GetOwner()->DrawScale3D,
		bFixed,
		MeshComp,
		GWorld->RBPhysScene);
}

void UPhysXDestructibleStructure::HideChunk( INT ChunkIndex, UBOOL bRecurse )
{
	FPhysXDestructibleChunk& Chunk = Chunks( ChunkIndex );
	USkeletalMeshComponent* MeshComp = GetChunkMesh( ChunkIndex );
	URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
	Body->TermBody( GWorld->RBPhysScene );
	MeshComp->HideBone( Chunk.BoneIndex, FALSE );
	Chunk.CurrentState = DCS_Hidden;

	INT NumHidden = 1;
	if( bRecurse )
	{
		NumHidden += SwitchToHidden( this, ChunkIndex );
	}

	APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
	Actor->QueueEffects( Chunk, 1 );
	APhysXDestructiblePart * Part = Actor->Parts( Chunk.FragmentIndex );
	check( Part );
	BYTE & NumRemaining = Part->NumChunksRemaining( Chunk.MeshIndex );
	// Do it this way to prevent underflow.  Theoretically, this won't happen, but this way the code is more robust.
	if( NumHidden >= Part->NumChunksRemaining( Chunk.MeshIndex ) )
	{
		NumRemaining = 0;
		Actor->DetachComponent( MeshComp );
//		debugf( TEXT("Destructible: Actor %s, Part %d, Mesh %d detached."), *Actor->GetName(), Chunk.FragmentIndex, Chunk.MeshIndex );
		if( --Part->NumMeshesRemaining == 0 )
		{
			GWorld->DestroyActor( Part, TRUE );
			Actor->Parts( Chunk.FragmentIndex ) = NULL;
//			debugf( TEXT("Destructible: Actor %s, Part %d destroyed."), *Actor->GetName(), Chunk.FragmentIndex );
			if( --Actor->NumPartsRemaining == 0 )
			{
//				debugf( TEXT("Destructible: Actor %s destroyed."), *Actor->GetName() );
				RemoveActor( Actor );
			}
		}
	}
	else
	{
		NumRemaining -= NumHidden;
	}
}

void UPhysXDestructibleStructure::MarkMoved( INT ChunkIndex )
{		
	FPhysXDestructibleChunk& Parent = Chunks( ChunkIndex );
	Parent.WorldMatrixValid = FALSE;
	Parent.WorldCentroidValid = FALSE;
	for( INT ChildIndex = Parent.FirstChildIndex; ChildIndex < Parent.FirstChildIndex + Parent.NumChildren; ++ChildIndex )
	{
		MarkMoved( ChildIndex );
	}
}

void UPhysXDestructibleStructure::SwitchToDynamic( INT ChunkIndex )
{		
	FPhysXDestructibleChunk& Parent = Chunks( ChunkIndex );
	for( INT ChildIndex = Parent.FirstChildIndex; ChildIndex < Parent.FirstChildIndex + Parent.NumChildren; ++ChildIndex )
	{
		if( Chunks( ChildIndex ).CurrentState == DCS_StaticChild )
		{
			Chunks( ChildIndex ).CurrentState  = DCS_DynamicChild;
			SwitchToDynamic( ChildIndex );
		}
	}
}

UBOOL UPhysXDestructibleStructure::DamageChunk( INT ChunkIndex, FVector Point, FLOAT BaseDamage, FLOAT Radius, UBOOL bFullDamage, TArray<INT>& Output )
{
	FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
	if( Chunk.CurrentState == DCS_Hidden )
	{
		return FALSE;
	}

	const INT StartingFracturedCount = Output.Num();

	APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
	FPhysXDestructibleParameters & DestructibleParameters = Actor->DestructibleParameters;

	FLOAT Dist = (Point - GetChunkCentroid(ChunkIndex)).Size();
	if( Chunk.Depth == Actor->PhysXDestructible->DestructibleAssets( Chunk.FragmentIndex )->MaxDepth )
	{
		Dist -= Chunk.Radius;	// Test against bounding sphere for smallest chunks
	}
	if( Dist < Radius )
	{
		FLOAT DamageFraction = 1.0f;
		if( !bFullDamage )
		{
			DamageFraction -= Max( Dist/Radius, 0.0f );
		}

		FLOAT OldChunkDamage = Chunk.Damage;
		Chunk.Damage += DamageFraction*BaseDamage;
		if( Chunk.Damage >= DestructibleParameters.DamageThreshold )
		{
			Chunk.Damage = Min( Chunk.Damage, DestructibleParameters.DamageThreshold );
			if( Chunk.CurrentState != DCS_DynamicRoot )
			{	// Fracture
				BaseDamage -= Chunk.Damage - OldChunkDamage;
				Output.AddItem( ChunkIndex );
				if( Chunk.NumChildren == 0 && DamageFraction*BaseDamage >= DestructibleParameters.DamageThreshold )
				{	// Crumble
					Chunk.bCrumble = TRUE;
				}
			}
			else
			if( Chunk.NumChildren == 0 )
			{	// Crumble
				Output.AddItem( ChunkIndex );
				Chunk.bCrumble = TRUE;
			}
		}
		if( !DestructibleParameters.bAccumulateDamage )
		{
			Chunk.Damage = 0.0f;
		}
	}

	if( !Chunk.bCrumble )
	{
		// Recurse
		for( INT ChildIndex = Chunk.FirstChildIndex; ChildIndex < Chunk.FirstChildIndex + Chunk.NumChildren; ++ChildIndex )
		{
			DamageChunk( ChildIndex, Point, BaseDamage, Radius, bFullDamage, Output );
		}
	}

	return Output.Num() > StartingFracturedCount;
}

void UPhysXDestructibleStructure::FractureChunk( INT ChunkIndex, FVector Point, FVector Impulse, UBOOL bInheritRootVel )
{
#if WITH_NOVODEX
	if (ChunkIndex == INDEX_NONE) return;

	FPhysXDestructibleChunk * Chunk = &Chunks( ChunkIndex );

	NxVec3 RootLinVel(0,0,0);
	NxVec3 RootAngVel(0,0,0);
	NxVec3 RootCOM(0,0,0);

	APhysXDestructibleActor * Actor = Actors( Chunk->ActorIndex );
	if( !Actor )
	{
		return;
	}
	if( !Actor->Parts( Chunk->FragmentIndex ) )
	{
		const UBOOL bFixed = (Actor->Physics != PHYS_RigidBody) || Actor->bStatic;
		Actor->SpawnPart( Chunk->FragmentIndex, bFixed );
		if( !Actor->Parts( Chunk->FragmentIndex ) )
		{
//			debugf( TEXT("UPhysXDestructibleStructure::FractureChunk could not spawn FragmentIndex %d in Actor %s"), Chunk->FragmentIndex, *Actor->GetName() );
			return;
		}
		if( bInheritRootVel && !bFixed )
		{
			NxActor * nActor = Actor->FracturedStaticMeshComponent->GetNxActor();
			if( nActor )
			{
				RootCOM = nActor->getCMassGlobalPosition();
				RootLinVel = nActor->getLinearVelocity();
				RootAngVel = nActor->getAngularVelocity();
			}
		}
	}

	if(!Chunk->IsRouteBlocker)
	{
		FractureOriginChunks.AddItem( ChunkIndex );
	}

	if( Chunk->bCrumble && (Chunk->CurrentState == DCS_StaticRoot || Chunk->CurrentState == DCS_DynamicRoot) )
	{
		NxActor * nActor = GetChunkNxActor( ChunkIndex );
		if( nActor && !bInheritRootVel )
		{
			nActor->setLinearVelocity( NxVec3(0,0,0) );
			nActor->setAngularVelocity( NxVec3(0,0,0) );
		}
		CrumbleChunk( ChunkIndex );
		if( Chunk->CurrentState == DCS_DynamicRoot )
		{
			Manager->RemoveChunk( Chunk );
		}
		return;
	}

	if( Chunk->CurrentState == DCS_DynamicRoot )
	{
		if( Impulse.SizeSquared() > 0 )
		{
			USkeletalMeshComponent* MeshComp = GetChunkMesh( ChunkIndex );
			MeshComp->AddImpulse( Impulse, Point, Chunk->BoneName );
		}
		return;
	}

	if( Chunk->CurrentState == DCS_Hidden )
	{
		return;
	}
	
	TArray<INT> NewDynamicRoots;
	NewDynamicRoots.Empty();	// Should cause no harm, and we may want to make this array static

	Chunk->CurrentState = DCS_DynamicRoot;
	NewDynamicRoots.AddItem( ChunkIndex );

	INT ParentIndex = Chunk->ParentIndex;
	INT ChildIndex = ChunkIndex;

	while ( ParentIndex != INDEX_NONE )
	{
		FPhysXDestructibleChunk& Parent = Chunks( ParentIndex );
		for (INT i = Parent.FirstChildIndex; i<Parent.FirstChildIndex + Parent.NumChildren; i++)
		{
			FPhysXDestructibleChunk& Child = Chunks( i );
			if ( Child.CurrentState == DCS_DynamicChild )
			{
				Child.CurrentState = DCS_DynamicRoot;
				NewDynamicRoots.AddItem( i );
			}
			else if ( Child.CurrentState == DCS_StaticChild )
			{
				ShowChunk( i, TRUE );
				Child.CurrentState = DCS_StaticRoot;
			}
		}
		if (Parent.CurrentState == DCS_DynamicRoot)
		{
			NxActor * nActor = GetChunkNxActor( ParentIndex );
			if( nActor )
			{
				RootLinVel = nActor->getLinearVelocity();
				RootAngVel = nActor->getAngularVelocity();
				RootCOM = nActor->getCMassGlobalPosition();
			}
			HideChunk( ParentIndex );
			Manager->RemoveChunk( &Parent );			
			break;
		}
		else if (Parent.CurrentState == DCS_StaticRoot)
		{
			HideChunk( ParentIndex );
			break;
		}
		else if ( Parent.CurrentState == DCS_Hidden )
		{
			break;
		}
		else
		{
			HideChunk( ParentIndex, FALSE );
		}
		ChildIndex = ParentIndex;
		ParentIndex = Parent.ParentIndex;
	}

	// Start off assuming we will add the whole list
	INT FirstNewDynamicChunk = 0;
	if( Manager && NewDynamicRoots.Num() > Manager->DynamicChunkFIFOMax )
	{
		FirstNewDynamicChunk = NewDynamicRoots.Num() - Manager->DynamicChunkFIFOMax;
	}

	for( INT i = 0; i < NewDynamicRoots.Num(); ++i )
	{
		INT Index = NewDynamicRoots(i);
		FPhysXDestructibleChunk& NewDynamicRootChunk = Chunks( Index );
		USkeletalMeshComponent* MeshComp = GetChunkMesh( Index );
		URB_BodyInstance* Body = MeshComp->PhysicsAssetInstance->Bodies( NewDynamicRootChunk.BodyIndex );
		ShowChunk( Index, TRUE );
		Body->SetFixed( FALSE );
		SwitchToDynamic( Index );
		Active.AddItem( Index );
		APhysXDestructibleActor * Actor = Actors( NewDynamicRootChunk.ActorIndex );
		NxActor * nActor = Body->GetNxActor();
		if( nActor  )
		{
			nActor->wakeUp();
			if( bInheritRootVel )
			{
				nActor->setLinearVelocity( RootLinVel + RootAngVel.cross(nActor->getCMassGlobalPosition()-RootCOM) );
				nActor->setAngularVelocity( RootAngVel );
			}
		}
		if( i == 0 && Impulse.SizeSquared() > 0 )
		{
			MeshComp->AddImpulse( Impulse, Point, NewDynamicRootChunk.BoneName );
		}
		if( NewDynamicRootChunk.bCrumble || i < FirstNewDynamicChunk )
		{
			CrumbleChunk( Index );
		}
		else
		if( Manager )
		{
			Manager->AddChunk( &NewDynamicRootChunk );
		}
	}
#endif
}

void UPhysXDestructibleStructure::CrumbleChunk( INT ChunkIndex )
{
#if WITH_NOVODEX
	if (ChunkIndex == INDEX_NONE) return;

	FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
	APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
	if( !Actor ) return;
	APhysXDestructiblePart * Part = Actor->Parts( Chunk.FragmentIndex );
	if( !Part ) return;
	FPhysXDestructibleParameters & DestructibleParameters = Actor->DestructibleParameters;
	if( DestructibleParameters.CrumbleParticleSystem )
	{
		USkeletalMeshComponent * SkelMesh = Part->SkeletalMeshComponents( Chunk.MeshIndex );
		check( SkelMesh && SkelMesh->PhysicsAssetInstance );
		URB_BodyInstance * Body = SkelMesh->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
		if( Body )
		{
			NxActor * nActor = Body->GetNxActor();
			if( nActor )
			{
				if( Actor->VolumeFill == NULL )
				{
					Actor->VolumeFill = new FRBVolumeFill();
				}
				FIndexedRBState IRBS;
				IRBS.Index = Actor->VolumeFill->Positions.Num();
				IRBS.CenterOfMass = N2UVectorCopy( nActor->getCMassGlobalPosition() );
				IRBS.LinearVelocity = N2UVectorCopy( nActor->getLinearVelocity() );
				IRBS.AngularVelocity = N2UVectorCopy( nActor->getAngularVelocity() );
				Actor->VolumeFill->RBStates.AddItem( IRBS );
				UPhysXDestructibleAsset * DestAsset = Actor->PhysXDestructible->DestructibleAssets( Chunk.FragmentIndex );
				UPhysicsAsset * Asset = DestAsset->Assets( Chunk.MeshIndex );
				FKAggregateGeom & Geom = Asset->BodySetup( Chunk.BodyIndex )->AggGeom;
				ScanGeom( Geom, GetChunkMatrix( ChunkIndex ), Actor->DrawScale*Actor->DrawScale3D,
					DestructibleParameters.CrumbleParticleSize, 4000, Actor->VolumeFill->Positions );
			}
		}
	}
	HideChunk( Chunk.Index );
#endif
}

FMatrix UPhysXDestructibleStructure::GetChunkMatrix(INT ChunkIndex)
{
#if WITH_NOVODEX
	if ( ChunkIndex != INDEX_NONE )
	{
		FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
		if (!Chunk.WorldMatrixValid )
		{
			APhysXDestructibleActor * Actor = Actors( Chunk.ActorIndex );
			if( Actor->Parts( Chunk.FragmentIndex ) == NULL )
			{
				check( Chunk.CurrentState == DCS_DynamicRoot || Chunk.CurrentState == DCS_DynamicChild );
				FMatrix BoneTM = FRotationTranslationMatrix( Actor->Rotation, Actor->Location );
				BoneTM.RemoveScaling();
				Chunk.WorldMatrix = BoneTM;
				Chunk.WorldMatrixValid = true;
			}
			else
			if ( Chunk.CurrentState == DCS_StaticRoot || Chunk.CurrentState == DCS_DynamicRoot )
			{
				USkeletalMeshComponent* Mesh = GetChunkMesh( ChunkIndex );
				URB_BodyInstance* Body = Mesh->PhysicsAssetInstance->Bodies( Chunk.BodyIndex );
				FMatrix BoneTM;
				if ( Body->GetNxActor() != 0 )
				{
					BoneTM = Body->GetUnrealWorldTM();
				}
				else
				{
					BoneTM = Mesh->GetBoneMatrix( Chunk.BoneIndex );
				}
				BoneTM.RemoveScaling();
				Chunk.WorldMatrix = BoneTM;
				Chunk.WorldMatrixValid = true;
			}
			else if ( Chunk.CurrentState == DCS_StaticChild || Chunk.CurrentState == DCS_DynamicChild )
			{
				Chunk.WorldMatrix = /*Chunk.RelativeMatrix **/ GetChunkMatrix( Chunk.ParentIndex );
				Chunk.WorldMatrixValid = true;
			}
			else
			{
				// nobody cares here
			}
		}
		return Chunk.WorldMatrix;
	}
	return FMatrix();
#else
	return FMatrix::Identity;
#endif
}

FVector UPhysXDestructibleStructure::GetChunkCentroid(INT ChunkIndex)
{
	if ( ChunkIndex != INDEX_NONE )
	{
		FPhysXDestructibleChunk & Chunk = Chunks( ChunkIndex );
		if (!Chunk.WorldCentroidValid )
		{
			FMatrix BoneTM = GetChunkMatrix( ChunkIndex );
			Chunk.WorldCentroid = BoneTM.TransformFVector( Chunk.RelativeCentroid );
			Chunk.WorldCentroidValid = TRUE;
		}
		return Chunk.WorldCentroid;
	}
	return FVector(0);
}

UBOOL UPhysXDestructibleStructure::RemoveActor( APhysXDestructibleActor * Actor )
{
	if( Actor )
	{
		ActorKillList.AddUniqueItem( Actor );
		return TRUE;
	}

	return FALSE;
}
