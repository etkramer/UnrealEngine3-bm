/*=============================================================================
	UnNavigationPoint.cpp:

  NavigationPoint and subclass functions

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnPath.h"

void ANavigationPoint::SetNetworkID(INT InNetworkID)
{
	NetworkID = InNetworkID;
}


void MergeNetworkAIntoB(INT A, INT B)
{
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		if(Nav && Nav->NetworkID == A)
		{
			Nav->SetNetworkID(B);
		}
	}
}

void ANavigationPoint::BuildNetworkIDs()
{
	// Clear all IDs
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		Nav->NetworkID = -1;
	}	


	INT NewNetworkID = -1;
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		if(Nav->NetworkID == -1)
		{
			//debugf(TEXT("%s was not on a network, adding new network.. (%i)"),*Nav->GetName(),NewNetworkID);
			Nav->SetNetworkID(++NewNetworkID);
		}

		for(INT Idx=0;Idx<Nav->PathList.Num();Idx++)
		{
			UReachSpec* Spec = Nav->PathList(Idx);
			if(Spec != NULL && Spec->End.Nav() != NULL)
			{
				ANavigationPoint* CurEnd = Spec->End.Nav();
				if(CurEnd->NetworkID == -1)
				{
					CurEnd->SetNetworkID(Nav->NetworkID);
				}
				else if(CurEnd->NetworkID != Nav->NetworkID)
				{
					//debugf(TEXT("%s encountered neighbor (%s) which already had a network, merging %i and %i"),*Nav->GetName(),*CurEnd->GetName(),Nav->NetworkID,CurEnd->NetworkID);
					MergeNetworkAIntoB(Nav->NetworkID,CurEnd->NetworkID);
				}
			}
		}
	}

	// print report :D
	//for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	//{
	//	debugf(TEXT("%s NETWORK %i"),*Nav->GetName(), Nav->NetworkID);
	//}
}

void ANavigationPoint::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}


/** 
 *	Detect when path building is going to move a pathnode around
 *	This may be undesirable for LDs (ie b/c cover links define slots by offsets)
 */
void ANavigationPoint::Validate()
{
	AScout *Scout = FPathBuilder::GetScout();
	if( Scout && ShouldBeBased() && (GoodSprite || BadSprite) )
	{
		FVector OrigLocation = Location;

		FCheckResult Hit(1.f);
		FVector HumanSize = Scout->GetSize(FName(TEXT("Human"), FNAME_Find));
		FVector Slice(HumanSize.X, HumanSize.X, 1.f);
		if( CylinderComponent->CollisionRadius < HumanSize.X )
		{
			Slice.X = CylinderComponent->CollisionRadius;
			Slice.Y = CylinderComponent->CollisionRadius;
		}

		UBOOL bResult = TRUE;

		// Check for adjustment
		GWorld->SingleLineCheck( Hit, this, Location - FVector(0,0, 4.f * CylinderComponent->CollisionHeight), Location, TRACE_AllBlocking, Slice );
		if( Hit.Actor )
		{
			FVector Dest = Hit.Location + FVector(0.f,0.f,CylinderComponent->CollisionHeight-2.f);

			// Move actor (TEST ONLY) to see if navigation point moves
			GWorld->FarMoveActor( this, Dest, FALSE, TRUE, TRUE );

			// If only adjustment was down towards the floor
			if( Location.X == OrigLocation.X &&  
				Location.Y == OrigLocation.Y && 
				Location.Z <= OrigLocation.Z )
			{
				// Valid placement
				bResult = TRUE;
			}
			else
			{
				// Otherwise, pathnode is moved unexpectedly
				bResult = FALSE;
			}

			// Move actor back to original position
			GWorld->FarMoveActor( this, OrigLocation, FALSE, TRUE, TRUE );
		}	
		
		// Update sprites by result
		if( GoodSprite )
		{
			GoodSprite->HiddenEditor = !bResult;
		}
		if( BadSprite )
		{
			BadSprite->HiddenEditor = bResult;
		}
	}
	FPathBuilder::DestroyScout();

	// Force update of icon
	ForceUpdateComponents(FALSE,FALSE);
}

void ANavigationPoint::AddToNavigationOctree()
{
	// Don't add nav point to octree if it already has an octree node (already in the octree)
	if( CylinderComponent != NULL &&
		NavOctreeObject.OctreeNode == NULL )
	{
		// add ourselves to the octree
		NavOctreeObject.SetOwner(this);
		FVector Extent(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, CylinderComponent->CollisionHeight);
		NavOctreeObject.SetBox(FBox(Location - Extent, Location + Extent));
		GWorld->NavigationOctree->AddObject(&NavOctreeObject);
	}
	// add ReachSpecs to the octree
	for (INT i = 0; i < PathList.Num(); i++)
	{
		if (PathList(i) != NULL)
		{
			PathList(i)->AddToNavigationOctree();
		}
	}
}

void ANavigationPoint::RemoveFromNavigationOctree()
{
	GWorld->NavigationOctree->RemoveObject(&NavOctreeObject);
	for (INT Idx = 0; Idx < PathList.Num(); Idx++)
	{
		UReachSpec *Spec = PathList(Idx);
		if (Spec != NULL)
		{
			Spec->RemoveFromNavigationOctree();
		}
	}
}

void ANavigationPoint::ClearCrossLevelReferences()
{
	Super::ClearCrossLevelReferences();

	for( INT PathIdx = 0; PathIdx < PathList.Num(); PathIdx++ )
	{
		UReachSpec *Spec = PathList(PathIdx);
		if( Spec == NULL ||
			Spec->Start == NULL ||
			(*Spec->End == NULL && !Spec->End.Guid.IsValid()) ||
			Spec->Start != this )
		{
			PathList.Remove(PathIdx--,1);
			continue;
		}
		if( *Spec->End != NULL && Spec->Start->GetOutermost() != Spec->End->GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			Spec->End.Guid = *Spec->End->GetGuid();
		}
	}

	for( INT VolIdx = 0; VolIdx < Volumes.Num(); VolIdx++ )
	{
		FActorReference& VolRef = Volumes(VolIdx);
		if( *VolRef == NULL && !VolRef.Guid.IsValid() )
		{
			Volumes.Remove( VolIdx--, 1 );
			continue;
		}

		if( *VolRef != NULL && GetOutermost() != VolRef->GetOutermost() )
		{
			bHasCrossLevelPaths = TRUE;
			VolRef.Guid = *VolRef->GetGuid();
		}
	}
}

void ANavigationPoint::GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel)
{
	Super::GetActorReferences(ActorRefs,bIsRemovingLevel);
	if( bHasCrossLevelPaths )
	{
		// look at each path,
		for (INT PathIdx = 0; PathIdx < PathList.Num(); PathIdx++)
		{
			// if it crosses a level and isn't already valid,
			UReachSpec *Spec = PathList(PathIdx);
			if (Spec->End.Guid.IsValid())
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if ((bIsRemovingLevel && *Spec->End != NULL) ||
					(!bIsRemovingLevel && *Spec->End == NULL))
				{
					ActorRefs.AddItem(&Spec->End);
				}
			}
		}
		for( INT VolIdx = 0; VolIdx < Volumes.Num(); VolIdx++ )
		{
			FActorReference& VolRef = Volumes(VolIdx);
			if( VolRef.Guid.IsValid() )
			{
				if( ( bIsRemovingLevel && *VolRef != NULL) ||
					(!bIsRemovingLevel && *VolRef == NULL) )
				{
					ActorRefs.AddItem(&VolRef);
				}
			}
		}

		// handle the forced/proscribed lists as well
		if (GIsEditor)
		{
			for (INT PathIdx = 0; PathIdx < EditorForcedPaths.Num(); PathIdx++)
			{
				FActorReference &ActorRef = EditorForcedPaths(PathIdx);
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
			for (INT PathIdx = 0; PathIdx < EditorProscribedPaths.Num(); PathIdx++)
			{
				FActorReference &ActorRef = EditorProscribedPaths(PathIdx);
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
		}
	}
}

void ANavigationPoint::PostScriptDestroyed()
{
	Super::PostScriptDestroyed();

	UReachSpec* OutSpec = NULL;
	UReachSpec* InSpec = NULL;

	// if this is a dynamic path node, and we're being destroyed during gameplay clean up specs that point back to us
	if(	!bStatic )
	{
		// remove specs which point back to me
		for(INT Idx=0;Idx<PathList.Num();Idx++)
		{
			OutSpec = PathList(Idx);
			if(OutSpec && OutSpec->End.Nav())
			{
				ANavigationPoint* End = OutSpec->End.Nav();
				for(INT Jdx=0;Jdx<End->PathList.Num();Jdx++)
				{
					InSpec = End->PathList(Jdx);
					if(InSpec != NULL && InSpec->End.Actor == this)
					{
						InSpec->RemoveFromNavigationOctree();
						End->PathList.RemoveItem(InSpec);
						break;
					}
				}
			}
		}
	}

	GetLevel()->RemoveFromNavList( this, TRUE );
	RemoveFromNavigationOctree();
}

/**
 * Works through the component arrays marking entries as pending kill so references to them
 * will be NULL'ed.
 *
 * @param	bAllowComponentOverride		Whether to allow component to override marking the setting
 */
void ANavigationPoint::MarkComponentsAsPendingKill(UBOOL bAllowComponentOverride)
{
	Super::MarkComponentsAsPendingKill(bAllowComponentOverride);

	if (!bAllowComponentOverride)
	{
		// also mark ReachSpecs as pending kill so that any lingering references to them don't force this level to stay in memory
		for (INT i = 0; i < PathList.Num(); i++)
		{
			if (PathList(i) != NULL)
			{
				PathList(i)->MarkPendingKill();
			}
		}
	}
}

void ANavigationPoint::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	//@fixme FIXME: What about ReachSpecs using this NavigationPoint? Should they be updated? Should we not add them to the octree in the first place?

	UBOOL bUpdateInOctree = CylinderComponent != NULL && (CylinderComponent->NeedsReattach() || CylinderComponent->NeedsUpdateTransform()) && (!bCollisionUpdate || CylinderComponent == CollisionComponent);
	
	Super::UpdateComponentsInternal(bCollisionUpdate);

	if (bUpdateInOctree)
	{
		FVector Extent(CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius, CylinderComponent->CollisionHeight);
		NavOctreeObject.SetBox(FBox(Location - Extent, Location + Extent));
	}
}

/** When a NavigationPoint is added to a Prefab, clear all the pathing information held in it. */
void ANavigationPoint::OnAddToPrefab()
{
	ClearPaths();
}

//
// Get height/radius of big cylinder around this actors colliding components.
//
void ANavigationPoint::GetBoundingCylinder(FLOAT& CollisionRadius, FLOAT& CollisionHeight) const
{
	if ( CylinderComponent )
	{
		CollisionRadius = CylinderComponent->CollisionRadius;
		CollisionHeight = CylinderComponent->CollisionHeight;
	}
	else
	{
		Super::GetBoundingCylinder(CollisionRadius, CollisionHeight);
	}
}

void ANavigationPoint::SetVolumes(const TArray<class AVolume*>& Volumes)
{
	Super::SetVolumes( Volumes );

	if ( PhysicsVolume )
		bMayCausePain = (PhysicsVolume->DamagePerSec != 0);
}

void ANavigationPoint::SetVolumes()
{
	Super::SetVolumes();

	if ( PhysicsVolume )
		bMayCausePain = (PhysicsVolume->DamagePerSec != 0);

}

UBOOL ANavigationPoint::CanReach(ANavigationPoint *Dest, FLOAT Dist, UBOOL bUseFlag, UBOOL bAllowFlying)
{
	if (Dist < 1.f)
	{
		return FALSE;
	}
	if ( (bUseFlag && bCanReach) || (this == Dest) )
	{
		bCanReach = TRUE;
		return TRUE;
	}

	INT NewWeight = appTrunc(Dist);
	if ( visitedWeight >= NewWeight)
	{
		return FALSE;
	}
	visitedWeight = NewWeight;
	
	for (INT i = 0; i < PathList.Num(); i++)
	{
		if ( !PathList(i)->IsProscribed() && (bAllowFlying || !(PathList(i)->reachFlags & R_FLY)))
		{
			if (PathList(i)->Distance > KINDA_SMALL_NUMBER && ~PathList(i)->End != NULL && PathList(i)->End.Nav()->CanReach(Dest, Dist - PathList(i)->Distance, FALSE, bAllowFlying))
			{
				bCanReach = TRUE;
				return TRUE;
			}
		}
	}

	return FALSE;
}

void ANavigationPoint::ReviewPath(APawn* Scout)
{
	// check for invalid path distances
	for (INT i = 0; i < PathList.Num(); i++)
	{
		if (PathList(i)->Distance <= KINDA_SMALL_NUMBER && !PathList(i)->IsProscribed())
		{
			GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("negative or zero distance to %s!"), *PathList(i)->End->GetName()), MCACTION_NONE, TEXT("NegativeOrZeroDistance"));
		}
	}

	if ( bMustBeReachable )
	{
		for ( ANavigationPoint* M=GWorld->GetFirstNavigationPoint(); M!=NULL; M=M->nextNavigationPoint )
			M->bCanReach = false;

		// check that all other paths can reach me
		INT NumFailed = 0;
		for ( ANavigationPoint* N=GWorld->GetFirstNavigationPoint(); N!=NULL; N=N->nextNavigationPoint )
		{
			if ( !N->bDestinationOnly )
			{
				for ( ANavigationPoint* M=GWorld->GetFirstNavigationPoint(); M!=NULL; M=M->nextNavigationPoint )
				{
					M->visitedWeight = 0;
				}
				if (!N->CanReach(this, UCONST_INFINITE_PATH_COST, TRUE, TRUE))
				{
					GWarn->MapCheck_Add( MCTYPE_ERROR, N, *FString::Printf(TEXT("Cannot reach %s from this node!"), *GetName()), MCACTION_NONE, TEXT("NotReachableFromAll"));
					NumFailed++;
					if ( NumFailed > 8 )
						break;
				}
			}
		}
	}
}

/** Check whether there is an acceptably short connection (relative to the distance between them) between this NavigationPoint and Other
*/
UBOOL ANavigationPoint::CheckSatisfactoryConnection(ANavigationPoint* Other)
{
	for ( INT i=0; i<PathList.Num(); i++ )
		if ( PathList(i)->End == Other )
			return true;

	// check for short enough alternate path to warrant no symmetry
	FLOAT Dist = (Location - Other->Location).Size();
	for ( ANavigationPoint* N=GWorld->GetFirstNavigationPoint(); N!=NULL; N=N->nextNavigationPoint )
	{
		N->bCanReach = false;
		N->visitedWeight = 0;
	}
	return CanReach(Other, MAXTESTMOVESIZE + Dist * PATHPRUNING, FALSE, bFlyingPreferred || Other->bFlyingPreferred);
}

UReachSpec* ANavigationPoint::GetReachSpecTo( ANavigationPoint *Nav, UClass* SpecClass )
{
	for( INT i = 0; i < PathList.Num(); i++ )
	{
		UReachSpec* Spec = PathList(i);
		if(  Spec && 
			 (SpecClass == NULL || SpecClass == Spec->GetClass()) &&
			 (!Spec->bDisabled || SpecClass != NULL) &&
			 Spec->End == Nav )
		{
			return Spec;
		}
	}
	return NULL;
}

AActor* AActor::AssociatedLevelGeometry()
{
	if ( bWorldGeometry )
		return this;

	return NULL;
}

UBOOL AActor::HasAssociatedLevelGeometry(AActor *Other)
{
	return ( bWorldGeometry && (Other == this) );
}

/* if navigationpoint is moved, paths are invalid
*/
void ANavigationPoint::PostEditMove( UBOOL bFinished )
{
	// Update all of the components of paths we connect to.  So they can update their 
	// path lines to point to our new location.
	for(INT ReachIdx=0; ReachIdx < PathList.Num(); ReachIdx++)
	{
		UReachSpec* Reach = PathList(ReachIdx);
		if( Reach )
		{
			ANavigationPoint* Nav = (ANavigationPoint*)(~Reach->End);
			if( Nav )
			{
				Nav->ForceUpdateComponents(FALSE,FALSE);
			}
		}
	}

	if( bFinished )
	{
		if ( GWorld->GetWorldInfo()->bPathsRebuilt )
		{
			debugf(TEXT("PostEditMove Clear paths rebuilt"));
		}
		GWorld->GetWorldInfo()->bPathsRebuilt = FALSE;
		bPathsChanged = TRUE;

		// Validate collision
		Validate();
	}

	Super::PostEditMove( bFinished );
}

/* if navigationpoint is spawned, paths are invalid
*/
void ANavigationPoint::Spawned()
{
	Super::Spawned();

	// Only desired update of paths for static nodes (ie NOT dynamic anchors)
	if( bStatic || bNoDelete )
	{
		if ( GWorld->GetWorldInfo()->bPathsRebuilt )
		{
			debugf(TEXT("Spawned Clear paths rebuilt"));
		}
		GWorld->GetWorldInfo()->bPathsRebuilt = FALSE;
		bPathsChanged = true;
	}

	if (GWorld->HasBegunPlay())
	{
		// this navpoint was dynamically spawned, make sure it's in the proper lists
		ULevel* const Level = GetLevel();
		Level->AddToNavList(this);
		Level->CrossLevelActors.AddItem( this );
		bHasCrossLevelPaths = TRUE;
	}

}

void ANavigationPoint::CleanUpPruned()
{
	for( INT i = PathList.Num() - 1; i >= 0; i-- )
	{
		if( PathList(i) && PathList(i)->bPruned )
		{
			PathList.Remove(i);
		}
	}

	PathList.Shrink();
}

UBOOL ANavigationPoint::CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance)
{
	if ( (bOneWayPath && (((Nav->Location - Location) | Rotation.Vector()) <= 0))
		|| (bCheckDistance && (Location - Nav->Location).SizeSquared() > MAXPATHDISTSQ) )
	{
		return false;
	}
	else
	{
		return (!Nav->bDeleteMe &&	!Nav->bNoAutoConnect &&	!Nav->bSourceOnly && !Nav->bMakeSourceOnly && Nav != this);
	}
}

UBOOL ALadder::CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance)
{
	// don't allow normal connection to other Ladder actors on same ladder
	ALadder *L = Cast<ALadder>(Nav);
	if ( L && (MyLadder == L->MyLadder) )
	{
		return false;
	}
	else
	{
		return Super::CanConnectTo(Nav, bCheckDistance);
	}
}

UBOOL ANavigationPoint::ShouldBeBased()
{
	return ((PhysicsVolume == NULL || !PhysicsVolume->bWaterVolume) && !bNotBased && CylinderComponent);
}

void ANavigationPoint::AddForcedSpecs( AScout *Scout )
{
}

/**
 * Builds a forced reachspec from this navigation point to the
 * specified navigation point.
 */
UReachSpec* ANavigationPoint::ForcePathTo(ANavigationPoint *Nav, AScout *Scout, UClass* ReachSpecClass )
{
	// if specified a valid nav point
	if (Nav != NULL &&
		Nav != this)
	{
		// search for the scout if not specified
		if (Scout == NULL)
		{
			Scout = FPathBuilder::GetScout();
		}
		if (Scout != NULL)
		{
			if( !ReachSpecClass )
			{
				ReachSpecClass = UForcedReachSpec::StaticClass();
			}

			// create the forced spec
			UReachSpec *newSpec = ConstructObject<UReachSpec>(ReachSpecClass,GetOuter(),NAME_None);
			FVector ForcedSize = newSpec->GetForcedPathSize( this, Nav, Scout );
			newSpec->CollisionRadius = appTrunc(ForcedSize.X);
			newSpec->CollisionHeight = appTrunc(ForcedSize.Y);
			newSpec->Start = this;
			newSpec->End = Nav;
			newSpec->Distance = appTrunc((Location - Nav->Location).Size());
			// and add the spec to the path list
			PathList.AddItem(newSpec);

			return newSpec;
		}
	}

	return NULL;
}

/**
 * If path from this NavigationPoint to Nav should be proscribed,
 * Builds a proscribed reachspec fromt his navigation point to the
 * specified navigation point.
 */
UBOOL ANavigationPoint::ProscribePathTo(ANavigationPoint *Nav, AScout *Scout)
{
	// if specified a valid nav point
	if ( Nav == NULL || Nav == this )
	{
		return TRUE;
	}

	// see if destination is in list of proscribed paths
	UBOOL bHasPath = FALSE;
	for (INT PathIdx = 0; PathIdx < EditorProscribedPaths.Num(); PathIdx++)
	{
		if (EditorProscribedPaths(PathIdx).Actor == Nav)
		{
			bHasPath = TRUE;
			break;
		}
	}
	if (!bHasPath)
	{
		return FALSE;
	}

	// create the forced spec
	UReachSpec *newSpec = ConstructObject<UReachSpec>(UProscribedReachSpec::StaticClass(),GetOuter(),NAME_None);
	// no path allowed because LD marked it - mark it with a reachspec so LDs will know there is a proscribed path here
	newSpec->Start = this;
	newSpec->End = Nav;
	newSpec->Distance = appTrunc((Location - Nav->Location).Size());
	PathList.AddItem(newSpec);
	return TRUE;
}

/* addReachSpecs()
Virtual function - adds reachspecs to path for every path reachable from it.
*/
void ANavigationPoint::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	// warn if no base
	if (Base == NULL &&
		ShouldBeBased() &&
		GetClass()->ClassFlags & CLASS_Placeable)
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("Navigation point not on valid base, or too close to steep slope"), MCACTION_NONE, TEXT("NavPointBadBase"));
	}
	// warn if bad base
	if( Base && Base->bPathColliding )
	{
		if( !Base->bStatic && bStatic )
		{
			GWarn->MapCheck_Add( MCTYPE_ERROR, this, *FString::Printf(TEXT("This type of NavigationPoint cannot be based on %s"), *Base->GetName()), MCACTION_NONE, TEXT("NavPointLocationInvalid") );
		}
		else
		if( Base->bStatic && !bStatic )
		{
			GWarn->MapCheck_Add( MCTYPE_ERROR, this, *FString::Printf(TEXT("No need for dynamic NavigationPoint when base is non-static %s"), *Base->GetName()), MCACTION_NONE, TEXT("NavPointLocationInvalid") );
		}
	}


	// try to build a spec to every other pathnode in the level
	FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		if((!bOnlyChanged || bPathsChanged || Nav->bPathsChanged) && Nav != this)
		{
			if( !ProscribePathTo(Nav, Scout) )
			{
				// check if paths are too close together
				if( ((Nav->Location - Location).SizeSquared() < 2.f * HumanSize.X) && 
					(Nav->GetClass()->ClassFlags & CLASS_Placeable) )
				{
					GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("May be too close to other navigation points"), MCACTION_NONE, TEXT("NavPointTooClose"));
				}

				// check if forced path
				UBOOL bForcedPath = FALSE;
				for (INT PathIdx = 0; PathIdx < EditorForcedPaths.Num(); PathIdx++)
				{
					if (EditorForcedPaths(PathIdx).Actor == Nav)
					{
						// If this node is not one way OR
						// connection is in the direction we respect
						if( !bOneWayPath ||
							((Nav->Location - Location) | Rotation.Vector()) >= 0 )
						{
							// Force the path
							ForcePathTo(Nav,Scout);
							bForcedPath = TRUE;
						}
						break;
					}
				}
				if( !bForcedPath && !bDestinationOnly && CanConnectTo( Nav, TRUE ) )
				{
					UClass*		ReachSpecClass  = GetReachSpecClass( Nav, Scout->GetDefaultReachSpecClass() );
					UReachSpec *newSpec			= ConstructObject<UReachSpec>(ReachSpecClass,GetOuter(),NAME_None);
					if( newSpec->defineFor( this, Nav, Scout ) )
					{
						// debugf(TEXT("***********added new spec from %s to %s"),*GetName(),*Nav->GetName());
						PathList.AddItem(newSpec);

						// look for paths coming the opposite direction and use the smallest of the collision found
						UReachSpec* ReturnSpec = newSpec->End.Nav()->GetReachSpecTo(this);
						if(ReturnSpec != NULL && !ReturnSpec->IsForced())
						{
							ReturnSpec->CollisionHeight = Min<INT>(ReturnSpec->CollisionHeight,newSpec->CollisionHeight);
							newSpec->CollisionHeight = ReturnSpec->CollisionHeight;
							
							ReturnSpec->CollisionRadius = Min<INT>(ReturnSpec->CollisionRadius,newSpec->CollisionRadius);
							newSpec->CollisionRadius = ReturnSpec->CollisionRadius;

							Scout->SetPathColor(ReturnSpec);
							Scout->SetPathColor(newSpec);
						}

					}
				}
			}
		}
	}
}

UBOOL ANavigationPoint::GetAllNavInRadius(class AActor* chkActor,FVector ChkPoint,FLOAT Radius,TArray<class ANavigationPoint*>& out_NavList,UBOOL bSkipBlocked,INT inNetworkID,FCylinder MinSize)
{
	TArray<FNavigationOctreeObject*> NavObjects;
	GWorld->NavigationOctree->RadiusCheck(ChkPoint,Radius,NavObjects);
	for (INT Idx = 0; Idx < NavObjects.Num(); Idx++)
	{
		ANavigationPoint *Nav = NavObjects(Idx)->GetOwner<ANavigationPoint>();
		if (Nav != NULL)
		{
			if (inNetworkID >= 0 && Nav->NetworkID != inNetworkID)
			{
				continue;
			}

			if (bSkipBlocked && Nav->bBlocked)
			{
				continue;
			}

			if( (MinSize.Height > 0 && MinSize.Height > Nav->MaxPathSize.Height) ||
				(MinSize.Radius > 0 && MinSize.Radius > Nav->MaxPathSize.Radius) )
			{
				continue;
			}

			FLOAT DistSq = (Nav->Location - ChkPoint).SizeSquared();
			UBOOL bInserted = FALSE;
			for (INT ListIdx = 0; ListIdx < out_NavList.Num(); ListIdx++)
			{
				if ((out_NavList(ListIdx)->Location - ChkPoint).SizeSquared() >= DistSq)
				{
					bInserted = TRUE;
					out_NavList.InsertItem(Nav,ListIdx);
					break;
				}
			}
			if (!bInserted)
			{
				out_NavList.AddItem(Nav);
			}
		}
	}
	return (out_NavList.Num() > 0);
}

/** Returns if this navigation point is on a different network than the given */
UBOOL ANavigationPoint::IsOnDifferentNetwork( ANavigationPoint* Nav )
{
	if( Nav != NULL )
	{
		if( Nav->NetworkID != -1 &&
			NetworkID	   != -1 && 
			NetworkID	   != Nav->NetworkID )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** sorts the PathList by distance, shortest first */
void ANavigationPoint::SortPathList()
{
	UReachSpec* TempSpec = NULL;
	for (INT i = 0; i < PathList.Num(); i++)
	{
		for (INT j = 0; j < PathList.Num() - 1; j++)
		{
			if (PathList(j)->Distance > PathList(j + 1)->Distance)
			{
				TempSpec = PathList(j+1);
				PathList(j+1) = PathList(j);
				PathList(j) = TempSpec;
			}
		}
	}
}

/** builds long range paths (> MAXPATHDIST) between this node and all other reachable nodes
 * for which a straight path would be significantly shorter or the only way to reach that node
 * done in a separate pass at the end because it's expensive and so we want to early out in the maximum number
 * of cases (e.g. if suitable short range paths already get there)
 */
void ANavigationPoint::AddLongPaths(AScout* Scout, UBOOL bOnlyChanged)
{
	if (bBuildLongPaths && !bDestinationOnly)
	{
		UReachSpec* NewSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(), GetOuter(), NAME_None);
		for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
		{
			FCheckResult Hit(1.0f);
			if ( (!bOnlyChanged || bPathsChanged || Nav->bPathsChanged) && Nav->bBuildLongPaths && CanConnectTo(Nav, false) && (Nav->Location - Location).SizeSquared() > MAXPATHDISTSQ &&
				GetReachSpecTo(Nav) == NULL && GWorld->SingleLineCheck(Hit, this, Nav->Location, Location, TRACE_World | TRACE_StopAtAnyHit) && !CheckSatisfactoryConnection(Nav) &&
				NewSpec->defineFor(this, Nav, Scout) )
			{
				//debugf(TEXT("***********added long range spec from %s to %s"), *GetName(), *Nav->GetName());
				PathList.AddItem(NewSpec);
				NewSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(), GetOuter(), NAME_None);
			}
		}
	}
}

void ALadder::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	UReachSpec *newSpec = ConstructObject<UReachSpec>(ULadderReachSpec::StaticClass(),GetOuter(),NAME_None);

	//debugf("Add Reachspecs for Ladder at (%f, %f, %f)", Location.X,Location.Y,Location.Z);
	bPathsChanged = bPathsChanged || !bOnlyChanged;

	// connect to all ladders in same LadderVolume
	if ( MyLadder )
	{
		for( FActorIterator It; It; ++ It )
		{
			ALadder *Nav = Cast<ALadder>(*It);
			if ( Nav && (Nav != this) && (Nav->MyLadder == MyLadder) && (bPathsChanged || Nav->bPathsChanged) && Nav->GetOutermost() == GetOutermost() )
			{
				// add reachspec from this to other Ladder
				// FIXME - support single direction ladders (e.g. zipline)
				FVector CommonSize = Scout->GetSize(FName(TEXT("Common"),FNAME_Find));
				newSpec->CollisionRadius = appTrunc(CommonSize.X);
				newSpec->CollisionHeight = appTrunc(CommonSize.Y);
				newSpec->Start = this;
				newSpec->End = Nav;
				newSpec->Distance = appTrunc((Location - Nav->Location).Size());
				PathList.AddItem(newSpec);
				newSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GetOuter(),NAME_None);
			}
		}
	}
	ANavigationPoint::addReachSpecs(Scout,bOnlyChanged);

	// Prune paths that require jumping
	for ( INT i=0; i<PathList.Num(); i++ )
		if ( PathList(i) && (PathList(i)->reachFlags & R_JUMP)
			&& (PathList(i)->End->Location.Z < PathList(i)->Start->Location.Z - PathList(i)->Start->CylinderComponent->CollisionHeight) )
			PathList(i)->bPruned = true;

}

void ATeleporter::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	//debugf("Add Reachspecs for node at (%f, %f, %f)", Location.X,Location.Y,Location.Z);
	bPathsChanged = bPathsChanged || !bOnlyChanged;

	for( FActorIterator It; It; ++ It )
	{
		ATeleporter *Nav = Cast<ATeleporter>(*It);
		if (Nav != NULL && Nav != this && Nav->Tag != NAME_None && URL == Nav->Tag.ToString() && (bPathsChanged || Nav->bPathsChanged))
		{
			UReachSpec* NewSpec = ConstructObject<UReachSpec>(UTeleportReachSpec::StaticClass(),GetOuter(),NAME_None);
			FVector MaxSize = Scout->GetMaxSize();
			NewSpec->CollisionRadius = appTrunc(MaxSize.X);
			NewSpec->CollisionHeight = appTrunc(MaxSize.Y);
			NewSpec->Start = this;
			NewSpec->End = Nav;
			NewSpec->Distance = 100;
			PathList.AddItem(NewSpec);
			break;
		}
	}

	ANavigationPoint::addReachSpecs(Scout, bOnlyChanged);
}

UBOOL ATeleporter::CanTeleport(AActor* A)
{
	return (A != NULL && A->bCanTeleport && (bCanTeleportVehicles || Cast<AVehicle>(A) == NULL));
}

void APlayerStart::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	ANavigationPoint::addReachSpecs(Scout, bOnlyChanged);

	// check that playerstart is useable
	FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));
	Scout->SetCollisionSize(HumanSize.X, HumanSize.Y);
	if ( !GWorld->FarMoveActor(Scout,Location,1) )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("PlayerStart is not useable"), MCACTION_NONE, TEXT("PlayerStartInvalid"));
	}

}

void ALadder::InitForPathFinding()
{
	// find associated LadderVolume
	MyLadder = NULL;
	for( FActorIterator It; It; ++ It )
	{
		ALadderVolume *V = Cast<ALadderVolume>(*It);
		if ( V && (V->Encompasses(Location) || V->Encompasses(Location - FVector(0.f, 0.f, CylinderComponent->CollisionHeight))) )
		{
			MyLadder = V;
			break;
		}
	}
	if ( !MyLadder )
	{
		// Warn if there is no ladder volume
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("Ladder is not in a LadderVolume"), MCACTION_NONE, TEXT("NoLadderVolume"));
		return;
	}

	LadderList = MyLadder->LadderList;
	MyLadder->LadderList = this;
}

/* ClearForPathFinding()
clear transient path finding properties right before a navigation network search
*/
void ANavigationPoint::ClearForPathFinding()
{
	visitedWeight	= UCONST_INFINITE_PATH_COST;
	nextOrdered		= NULL;
	prevOrdered		= NULL;
	previousPath	= NULL;
	bEndPoint		= bTransientEndPoint;
	bTransientEndPoint = FALSE;

	// Figure out total cost of movement to this node
	Cost =	ExtraCost + 
			TransientCost + 
			FearCost;

	CostArray.Empty();
	DEBUGREGISTERCOST( this, TEXT("Extra"), ExtraCost );
	DEBUGREGISTERCOST( this, TEXT("Transient"), TransientCost );
	DEBUGREGISTERCOST( this, TEXT("Fear"), FearCost );
	
	TransientCost = 0;
	bAlreadyVisited = FALSE;

	// check to see if we should delete our anchored pawn
	if (AnchoredPawn != NULL &&
		!AnchoredPawn->ActorIsPendingKill())
	{
		if (AnchoredPawn->Controller == NULL ||
			AnchoredPawn->Health <= 0)
		{
			AnchoredPawn = NULL;
		}
	}
}

/* ClearPaths()
remove all path information from a navigation point. (typically before generating a new version of this
information
*/
void ANavigationPoint::ClearPaths()
{
	nextNavigationPoint = NULL;
	nextOrdered = NULL;
	prevOrdered = NULL;
	previousPath = NULL;
	PathList.Empty();
}

void ALadder::ClearPaths()
{
	Super::ClearPaths();

	if ( MyLadder )
		MyLadder->LadderList = NULL;
	LadderList = NULL;
	MyLadder = NULL;
}

void ANavigationPoint::FindBase()
{
	if ( GWorld->HasBegunPlay() )
	{
		return;
	}

	SetZone(1,1);
	if( ShouldBeBased() )
	{
		// not using find base, because don't want to fail if LD has navigationpoint slightly interpenetrating floor
		FCheckResult Hit(1.f);
		AScout *Scout = FPathBuilder::GetScout();
		check(Scout != NULL && "Failed to find scout for point placement");
		// get the dimensions for the average human player
		FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));
		FVector CollisionSlice(HumanSize.X, HumanSize.X, 1.f);
		// and use this node's smaller collision radius if possible
		if (CylinderComponent->CollisionRadius < HumanSize.X)
		{
			CollisionSlice.X = CollisionSlice.Y = CylinderComponent->CollisionRadius;
		}
		// check for placement
		GWorld->SingleLineCheck( Hit, Scout, Location - FVector(0,0, 4.f * CylinderComponent->CollisionHeight), Location, TRACE_AllBlocking, CollisionSlice );
		if (Hit.Actor != NULL)
		{
			if (Hit.Normal.Z > Scout->WalkableFloorZ)
			{
				GWorld->FarMoveActor(this, Hit.Location + FVector(0.f,0.f,CylinderComponent->CollisionHeight-2.f),0,1,0);
			}
			else
			{
				Hit.Actor = NULL;
			}
		}
		SetBase(Hit.Actor, Hit.Normal);
		if (GoodSprite != NULL)
		{
			GoodSprite->HiddenEditor = FALSE;
		}
		if (BadSprite != NULL)
		{
			BadSprite->HiddenEditor = TRUE;
		}
	}
}

#define DEBUG_PRUNEPATHS (0)

/* Prune paths when an acceptable route using an intermediate path exists
*/
INT ANavigationPoint::PrunePaths()
{
	INT pruned = 0;

#if DEBUG_PRUNEPATHS
	debugf(TEXT("Prune paths from %s"),*GetName());
#endif

	for ( INT i=0; i<PathList.Num(); i++ )
	{
		UReachSpec* iSpec = PathList(i);
		if (CanPrunePath(i) && !iSpec->IsProscribed() && !iSpec->IsForced())
		{
#if DEBUG_PRUNEPATHS
			debugf(TEXT("Try to prune %s to %s (%s)"), *iSpec->Start->GetName(), *iSpec->End->GetName(), *iSpec->GetName() );
#endif
			for (ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
			{
				Nav->visitedWeight = UCONST_INFINITE_PATH_COST;
			}

			for ( INT j=0; j<PathList.Num(); j++ )
			{
				UReachSpec* jSpec = PathList(j);
				if( i != j && 
					jSpec->ShouldPruneAgainst( iSpec ) )
				{
					if( jSpec->End.Nav()->FindAlternatePath( iSpec, jSpec->Distance ) )
					{
						iSpec->bPruned = TRUE;
#if DEBUG_PRUNEPATHS
						debugf(TEXT("!!!!! Pruned path to %s (%s) because of path through %s (%s)"), *PathList(i)->End->GetName(), *PathList(i)->GetName(), *PathList(j)->End->GetName(), *PathList(j)->GetName() );
#endif

						j = PathList.Num();
						pruned++;
					}
				}
#if DEBUG_PRUNEPATHS
				else
				{

					debugf(TEXT("Reject spec %s to %s (%s) ... %d %d ShouldPrune %s - %s %s %s %s"), 
						*jSpec->Start->GetName(),
						*jSpec->End->GetName(), 
						*jSpec->GetName(),
						i, j,
						jSpec->ShouldPruneAgainst( iSpec )?TEXT("TRUE"):TEXT("FALSE"),
						jSpec->bPruned?TEXT("TRUE"):TEXT("FALSE"),
						jSpec->bSkipPrune?TEXT("TRUE"):TEXT("FALSE"),
						(*jSpec<=*iSpec)?TEXT("TRUE"):TEXT("FALSE"),
						(*jSpec->End!=NULL)?TEXT("TRUE"):TEXT("FALSE") );
				}
#endif
			}
		}
	}

	CleanUpPruned();

	// make sure ExtraCost is not negative
	ExtraCost = ::Max<INT>(ExtraCost, 0);

	UpdateMaxPathSize();
	
	return pruned;
}


#define MAX_CHECK_DIST 512.f
UBOOL NodeSupportsCollisionSize(ANavigationPoint* Node, UReachSpec* SupportsCollisionofThis)
{
	if(Node == NULL || SupportsCollisionofThis == NULL || SupportsCollisionofThis->Start == NULL || SupportsCollisionofThis->End.Nav() == NULL)
	{
		return FALSE;
	}

	// if the incomign node is smaller than both the endpoints of our spec
	if((SupportsCollisionofThis->Start->MaxPathSize.Height > Node->MaxPathSize.Height ||
		SupportsCollisionofThis->Start->MaxPathSize.Radius > Node->MaxPathSize.Radius) &&
		(SupportsCollisionofThis->End.Nav()->MaxPathSize.Height > Node->MaxPathSize.Height ||
		SupportsCollisionofThis->End.Nav()->MaxPathSize.Radius > Node->MaxPathSize.Radius)
		)
	{
		return FALSE;
	}

	return TRUE;
}
/*
 * NodeAHasShortishAlteranteRouteToNodeB
 * - recursive function which searches for an alternate route to NodeB from NodeA which does not use ProscribedSpec, and people that could use ProscribedSpec could also use (e.g. the alternate route is wide enough),
 *   and is no more than MAX_CHECK_DIST longer of a detour to get to NodeB than ProscribedSpec is
 */
UBOOL NodeAHasShortishAlteranteRouteToNodeBWorker(ANavigationPoint* NodeA, ANavigationPoint* NodeB, UReachSpec* ProscribedSpec, INT TraveledDistance, FLOAT CheckDelta)
{
	// if we've traversed too far, bail
	//debugf(TEXT("-> %s->%s"),*NodeA->GetName(),*NodeB->GetName());
	if(TraveledDistance > ProscribedSpec->Distance + CheckDelta)
	{
		//debugf(TEXT("not progressing because traveleddistance is too big %i vs %i"),TraveledDistance,ProscribedSpec->Distance+CheckDelta);
		return FALSE;
	}
	else if(NodeB == NodeA) // then we found the node we're looking for
	{
		//debugf(TEXT("Found alternate route, googgogo!"));
		return TRUE;
	}

	
	if(NodeA->visitedWeight <= TraveledDistance)
	{
		return FALSE;
	}
	else
	{
		NodeA->visitedWeight = TraveledDistance;
	}

	UReachSpec* CurSpec = NULL;
	for(INT Idx=0; Idx < NodeA->PathList.Num(); Idx++)
	{
		CurSpec = NodeA->PathList(Idx);
		// if this is the spec we're checking against, or it doesn't support the size of the spec we're checking against.., or the endpoints don't support the proscribed spec's size skip
		if(CurSpec != ProscribedSpec &&
			CurSpec->ShouldPruneAgainst(ProscribedSpec)			
		  )
		{
			if(NodeAHasShortishAlteranteRouteToNodeBWorker(CurSpec->End.Nav(),NodeB,ProscribedSpec,TraveledDistance+CurSpec->Distance,CheckDelta))
			{
				return TRUE;
			}		
		}
		else if( CurSpec != ProscribedSpec)
		{
		/*	debugf(TEXT("ShouldPruneAgainst forbade %s->%s Pruned? %i SkipPrune? %i EndNULL? %i Rad/height: %i/%i vs %i/%i Flags: %i vs %i LandingVel: %i vs %i <=? %i"),
				*CurSpec->Start->GetName(),*CurSpec->End.Actor->GetName(),
				CurSpec->bPruned,
				CurSpec->bSkipPrune,
				(INT)CurSpec->End.Actor,
				CurSpec->CollisionRadius,
				CurSpec->CollisionHeight,
				ProscribedSpec->CollisionRadius,
				ProscribedSpec->CollisionHeight,
				CurSpec->reachFlags,
				ProscribedSpec->reachFlags,
				CurSpec->MaxLandingVelocity,
				ProscribedSpec->MaxLandingVelocity,
				*CurSpec<=*ProscribedSpec);*/
		}
	}
	return FALSE;
}

UBOOL NodeAHasShortishAlteranteRouteToNodeB(ANavigationPoint* NodeA, ANavigationPoint* NodeB, UReachSpec* ProscribedSpec, INT TraveledDistance, FLOAT CheckDelta)
{
	//debugf(TEXT("NodeAHasShortishAlteranteRouteToNodeB -----------------------------"));
	for (ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		Nav->visitedWeight = UCONST_INFINITE_PATH_COST;
	}
	return NodeAHasShortishAlteranteRouteToNodeBWorker(NodeA,NodeB,ProscribedSpec,TraveledDistance,CheckDelta);
}

UBOOL PruneLongerPath(UReachSpec* iSpec, UReachSpec *jSpec, INT& pruned, FLOAT CheckDelta)
{
	
	// we need to check to make sure the opposite spec can be pruned before we determine based on distance..
	// this is so that we will prune a spec even if it's shorter when the longer spec isn't eligible for pruning
	UBOOL bOtherSpecCanBePruned = NodeAHasShortishAlteranteRouteToNodeB(jSpec->Start,jSpec->End.Nav(),jSpec,0,CheckDelta);
	if(bOtherSpecCanBePruned)
	{
		// if there is a spec going in the opposite direction prune that at the same time
		UReachSpec* OtherDirSpec = jSpec->End.Nav()->GetReachSpecTo(jSpec->Start);
		// see if we prune iSpec, if the nodes that spec connects will still be connected to each other
		bOtherSpecCanBePruned = (OtherDirSpec == NULL || NodeAHasShortishAlteranteRouteToNodeB(OtherDirSpec->Start,OtherDirSpec->End.Nav(),OtherDirSpec,0,CheckDelta));
	}

	// pick the longer edge and prune it
	if(iSpec->Distance > jSpec->Distance || !bOtherSpecCanBePruned)
	{

		// see if we prune iSpec, if the nodes that spec connects will still be connected to each other

		if(NodeAHasShortishAlteranteRouteToNodeB(iSpec->Start,iSpec->End.Nav(),iSpec,0,CheckDelta))
		{
			// if there is a spec going in the opposite direction prune that at the same time
			UReachSpec* OtherDirSpec = iSpec->End.Nav()->GetReachSpecTo(iSpec->Start);

			// see if we prune iSpec, if the nodes that spec connects will still be connected to each other
			if(OtherDirSpec == NULL || NodeAHasShortishAlteranteRouteToNodeB(OtherDirSpec->Start,OtherDirSpec->End.Nav(),OtherDirSpec,0,CheckDelta))
			{
				/*debugf(TEXT("Pruning %s->%s in favor of %s->%s %i/%i"),
					*iSpec->Start->GetName(),
					*iSpec->End->GetName(),
					*jSpec->Start->GetName(),
					*jSpec->End->GetName(),
					iSpec->Distance,
					jSpec->Distance);*/
				
				// there was another path to get to that node
				iSpec->bPruned=TRUE;

				if(OtherDirSpec)
				{
					OtherDirSpec->bPruned = TRUE;
					pruned++;
				}
				pruned++;
				return TRUE;
			}
		}
	}

	return FALSE;
}

IMPLEMENT_COMPARE_POINTER( UReachSpec, Distance, { return (B->Distance - A->Distance); } )

INT ANavigationPoint::AggressivePrunePaths()
{
	INT pruned = 0;
	AScout* Scout = FPathBuilder::GetScout();	
	for ( INT i=0; i<PathList.Num(); i++ )
	{
		UReachSpec* iSpec = PathList(i);
		TArray<UReachSpec*> IntersectingSpecs;
		if (!iSpec->bPruned && CanPrunePath(i) && !iSpec->IsProscribed() && !iSpec->IsForced())
		{
			TArray<FNavigationOctreeObject*> NavObjects;  
			GWorld->NavigationOctree->RadiusCheck(Location, MAXPATHDIST, NavObjects);
			for(INT Idx=0; Idx < NavObjects.Num(); Idx++)
			{
				ANavigationPoint* Nav = NavObjects(Idx)->GetOwner<ANavigationPoint>();
				if(Nav == NULL || Nav == this || *iSpec->End == Nav)
				{
					continue;
				}

				FVector Pt1, Pt2;
				for ( INT j=0; j<Nav->PathList.Num(); j++ )
				{	
					UReachSpec* jSpec = Nav->PathList(j);	
					// if this edge has been pruned, or it shares an endpoint with iSpec, or shouldpruneagainst fails, skip it
					if(jSpec->bPruned ||
						jSpec->End == iSpec->End || jSpec->End == iSpec->Start || iSpec->End == jSpec->Start || iSpec->Start == jSpec->Start ||
						!jSpec->ShouldPruneAgainst(iSpec) ||
						!Nav->CanPrunePath(j)) // if this is a spec which can't be pruned, don't care if it intersects other specs
					{			
						continue;
					}

					//debugf(TEXT("Checking to see if spec for %s->%s intersects %s->%s! --" ),*iSpec->Start->GetName(),*iSpec->End->GetName(),*jSpec->Start->GetName(),*jSpec->End->GetName());
					SegmentDistToSegment(iSpec->Start->Location,iSpec->End->Location,jSpec->Start->Location,jSpec->End->Location,Pt1,Pt2);

			
					if((Pt1-Pt2).Size2D() < 25.f && Abs<FLOAT>(Pt1.Z - Pt2.Z) < Scout->GetDefaultCollisionSize().Z )
					{			
						IntersectingSpecs.AddItem(jSpec);
					}	
				}

				if(iSpec->bPruned)
				{
					break;
				}
			}

			// we add all the intersecting specs to a list, and then sort the list based on length of the edges, so that 
			// we prune the longest edges first 
			Sort<USE_COMPARE_POINTER(UReachSpec,Distance)>(&IntersectingSpecs(0),IntersectingSpecs.Num());
			for( INT j=0;j<IntersectingSpecs.Num();j++)
			{
				UReachSpec* jSpec = IntersectingSpecs(j);
				PruneLongerPath(jSpec,iSpec,pruned,Max<FLOAT>(iSpec->Distance * 0.6f,MAX_CHECK_DIST));
			}
		}
	}

	CleanUpPruned();

	// make sure ExtraCost is not negative
	ExtraCost = ::Max<INT>(ExtraCost, 0);

	UpdateMaxPathSize();

	return pruned;
}

INT ANavigationPoint::SecondPassAggressivePrunePaths()
{
	INT pruned = 0;
	// second pass, prune out paths close to the same angle
	for ( INT i=0; i<PathList.Num(); i++ )
	{
		UReachSpec* iSpec = PathList(i);
		if (!iSpec->bPruned && CanPrunePath(i) && !iSpec->IsProscribed() && !iSpec->IsForced())
		{
			for ( INT j=0; j<PathList.Num(); j++ )
			{	
				UReachSpec* jSpec = PathList(j);	
				// if this edge has been pruned, or it shares an endpoint with iSpec, or shouldpruneagainst fails, skip it
				if(jSpec->bPruned ||
					jSpec == iSpec ||
					!jSpec->ShouldPruneAgainst(iSpec) ||
					!CanPrunePath(j) || 
					(iSpec->Direction | jSpec->Direction) < 0.807f ) // within 35 degrees of each other
				{			
					continue;
				}

				if(PruneLongerPath(iSpec,jSpec,pruned,PATHPRUNING * iSpec->Distance))
				{
					break;
				}
			}
		}
	}

	CleanUpPruned();

	// make sure ExtraCost is not negative
	ExtraCost = ::Max<INT>(ExtraCost, 0);

	UpdateMaxPathSize();

	return pruned;
}

void ANavigationPoint::UpdateMaxPathSize()
{
	// set MaxPathSize based on remaining paths
	MaxPathSize.Radius = MaxPathSize.Height = 0.f;
	for (INT i = 0; i < PathList.Num(); i++)
	{
		UReachSpec* Spec = PathList(i);
		if( !Spec->bDisabled )
		{
			MaxPathSize.Radius = ::Max<FLOAT>(MaxPathSize.Radius, PathList(i)->CollisionRadius);
			MaxPathSize.Height = ::Max<FLOAT>(MaxPathSize.Height, PathList(i)->CollisionHeight);
		}
	}
}

/** recursively determines if there is an acceptable alternate path to the passed in one
 * used by path pruning
 * this routine uses each NavigationPoint's visitedWeight to avoid unnecessarily checking nodes
 * that have already been checked with a better distance, so you need to initialize them by
 * setting it to UCONST_INFINITE_PATH_COST on all NavigationPoints before calling this function
 */
UBOOL ANavigationPoint::FindAlternatePath(UReachSpec* StraightPath, INT AccumulatedDistance)
{
	if ( bBlocked || bBlockable )
	{
		return FALSE;
	}
	if (StraightPath->Start == NULL || *StraightPath->End == NULL)
	{
		return FALSE;
	}

	// if we have already visited this node with a lower distance, there is no point in trying it again
	if (visitedWeight <= AccumulatedDistance)
	{
		return FALSE;
	}
	visitedWeight = AccumulatedDistance;

#if DEBUG_PRUNEPATHS
	debugf(TEXT("%s FindAlternatePath Straight %s to %s (%s) AccDist %d"), *GetName(), *StraightPath->Start->GetName(), *StraightPath->End->GetName(), *StraightPath->GetName(), AccumulatedDistance );
#endif

	FVector StraightDir = (StraightPath->End->Location - StraightPath->Start->Location).SafeNormal();
	FLOAT DistThresh = PATHPRUNING * StraightPath->Distance;

#if DEBUG_PRUNEPATHS
	debugf(TEXT("Dir %s Thresh %f"), *StraightDir.ToString(), DistThresh );
#endif

	// check if the endpoint is directly reachable
	for (INT i = 0; i < PathList.Num(); i++)
	{
		UReachSpec* Spec = PathList(i);
		if( !Spec->bPruned && 
			 Spec->End.Nav() == StraightPath->End.Nav() )
		{
			FLOAT DotP = (StraightDir | (StraightPath->End->Location - Location).SafeNormal());

#if DEBUG_PRUNEPATHS
			debugf(TEXT("Test direct reach %s to %s (%s) DotP %f"), *Spec->Start->GetName(), *Spec->End->GetName(), *Spec->GetName(), DotP );
#endif

			if( DotP >= 0.f )
			{
				FLOAT Dist = AccumulatedDistance + Spec->Distance;

#if DEBUG_PRUNEPATHS
				debugf( TEXT("Dist check %f thresh %f ShouldPrune %d"), Dist, DistThresh, Spec->ShouldPruneAgainst( StraightPath ) );
				if( !Spec->ShouldPruneAgainst(StraightPath) )
				{
					debugf(TEXT("Str8 %s (%s) bPruned %s bSkipPrune %s End %s PruneList %d %d operator %s %s %s Rad %d %d Height %d %d"), 
						*StraightPath->GetName(), 
						*Spec->GetName(), 
						Spec->bPruned?TEXT("TRUE"):TEXT("FALSE"),
						Spec->bSkipPrune?TEXT("TRUE"):TEXT("FALSE"),
						*Spec->End->GetName(),
						Spec->PruneSpecList.FindItemIndex(StraightPath->GetClass()),
						StraightPath->PruneSpecList.FindItemIndex(Spec->GetClass()),
						(*Spec <= *StraightPath) ? TEXT("TRUE") : TEXT("FALSE"),
						Spec->IsProscribed()?TEXT("TRUE"):TEXT("FALSE"),
						Spec->IsForced()?TEXT("TRUE"):TEXT("FALSE"),
						Spec->CollisionRadius, StraightPath->CollisionRadius,
						Spec->CollisionHeight, StraightPath->CollisionHeight );
				}
#endif
				if( (Dist < DistThresh) && Spec->ShouldPruneAgainst( StraightPath ) )
				{
#if DEBUG_PRUNEPATHS
					debugf(TEXT(">>>> Direct path from %s to %s (%s)"),*GetName(), *Spec->End->GetName(), *Spec->GetName() );
#endif
					return TRUE;
				}
				else
				{
#if DEBUG_PRUNEPATHS
					debugf( TEXT(">>>> No direct path from %s to %s (%s)"), *GetName(), *Spec->End->GetName(), *Spec->GetName() );
#endif

					return FALSE;
				}
			}
		}
	}

	// now continue looking for path
	for ( INT i=0; i<PathList.Num(); i++ )
	{
		UReachSpec* Spec = PathList(i);

#if DEBUG_PRUNEPATHS
		debugf(TEXT("try to recurse with %s to %s (%s)... ShouldPrune %s SpecDist %d AccDist %d Thresh %d"), 
			*Spec->Start->GetName(), 
			*Spec->End->GetName(), 
			*Spec->GetName(),
			Spec->ShouldPruneAgainst(StraightPath)?TEXT("TRUE"):TEXT("FALSE"),
			Spec->Distance,
			AccumulatedDistance + Spec->Distance, 
			appTrunc(PATHPRUNING * StraightPath->Distance) );
#endif

		if ( Spec->ShouldPruneAgainst( StraightPath )
			&& (Spec->Distance > 0)
			&& (AccumulatedDistance + Spec->Distance < appTrunc(PATHPRUNING * StraightPath->Distance))
			&& (Spec->End != StraightPath->Start)
			&& ((StraightDir | (Spec->End->Location - Location).SafeNormal()) > 0.f)
			)
		{
			if( Spec->End.Nav()->FindAlternatePath(StraightPath, AccumulatedDistance + Spec->Distance) )
			{
#if DEBUG_PRUNEPATHS
				debugf(TEXT("Partial path from %s to %s"),*GetName(), *Spec->End->GetName());
#endif

				return TRUE;
			}
		}
	}

	return FALSE;
}

IMPLEMENT_CLASS(ACoverLink);
IMPLEMENT_CLASS(ACoverGroup);
IMPLEMENT_CLASS(ACoverSlotMarker);
IMPLEMENT_CLASS(AMantleMarker);

FVector ACoverSlotMarker::GetSlotLocation()
{
	if( OwningSlot.Link != NULL )
	{
		return OwningSlot.Link->GetSlotLocation(OwningSlot.SlotIdx);
	}
	return FVector(0,0,0);
}

FRotator ACoverSlotMarker::GetSlotRotation()
{
	if( OwningSlot.Link != NULL )
	{
		return OwningSlot.Link->GetSlotRotation(OwningSlot.SlotIdx);
	}

	return FRotator(0,0,0);
}

void ACoverSlotMarker::SetSlotEnabled( UBOOL bEnable )
{
	if( OwningSlot.Link != NULL )
	{
		OwningSlot.Link->eventSetSlotEnabled( OwningSlot.SlotIdx, bEnable );
	}
}

UBOOL ACoverSlotMarker::PlaceScout(AScout *Scout)
{
	// Nothing bigger than a boomer
	if( !bIgnoreSizeLimits && Scout->CylinderComponent->CollisionRadius >= 60 )
	{
		return FALSE;
	}

	if( !GWorld->FarMoveActor(Scout,Location,FALSE,TRUE) )
	{
		debugf(TEXT("[%s] failed to place scout at location"),*GetName());
	}
	return TRUE;
}

void ACoverSlotMarker::addReachSpecs(AScout* Scout, UBOOL bOnlyChanged)
{
	FCoverSlot* Slot = ACoverLink::CoverInfoToSlotPtr( OwningSlot );

	// if we have a valid slot and it's referencing this
	if( Slot &&
		Slot->SlotMarker == this )
	{
		OwningSlot.Link->FindBase();

		// add reachspecs to this marker
		Super::addReachSpecs(Scout,FALSE);
	}
	else
	{
		// otherwise delete ourselves
		GWorld->DestroyActor(this,FALSE);
	}
}

void ACoverSlotMarker::AddForcedSpecs( AScout *Scout )
{
	FCoverSlot* Slot = ACoverLink::CoverInfoToSlotPtr( OwningSlot );
	if( !Slot )
	{
		return;
	}
	
	// Clean out all special reachspecs
	for( INT Idx = 0; Idx < PathList.Num(); Idx++ )
	{
		UReachSpec* Spec = PathList(Idx);
		if( Cast<UMantleReachSpec>(Spec) )
		{
			PathList.Remove( Idx-- );
		}
	}

	// MANTLE
	// If mantling OVER to another slot
	if( Slot->MantleTarget.SlotIdx >= 0 )
	{
		FCoverSlot* TargSlot = ACoverLink::CoverRefToSlotPtr( Slot->MantleTarget );
		if( TargSlot && TargSlot->SlotMarker )
		{
			// Remove previous normal paths
			for( INT Idx = 0; Idx < PathList.Num(); Idx++ )
			{
				UReachSpec* Spec = PathList(Idx);
				if( Spec &&
					Spec->End == TargSlot->SlotMarker )
				{
					PathList.Remove( Idx-- );
				}
			}

			ForcePathTo( TargSlot->SlotMarker, Scout, UMantleReachSpec::StaticClass()  );
		}
	}
	else
	{
		// Otherwise, try to setup climb up specs
		AMantleMarker* Marker = Cast<AMantleMarker>(Slot->MantleTarget.Nav());
		if( Marker != NULL && !Marker->bDeleteMe )
		{
			UMantleReachSpec* MantleSpec = Cast<UMantleReachSpec>(ForcePathTo( Marker, Scout, UMantleReachSpec::StaticClass() ));
			if( MantleSpec != NULL )
			{
				MantleSpec->bClimbUp = TRUE;
			}
			MantleSpec = Cast<UMantleReachSpec>(Marker->ForcePathTo( this, Scout, UMantleReachSpec::StaticClass() ));
			if( MantleSpec != NULL )
			{
				MantleSpec->bClimbUp = TRUE;
			}

			Marker->ForceUpdateComponents(FALSE,FALSE);
		}
	}

	for( INT SlipIdx = 0; SlipIdx < Slot->SlipTarget.Num(); SlipIdx++ )
	{
		UCoverSlipReachSpec* Spec = Cast<UCoverSlipReachSpec>(ForcePathTo(Slot->SlipTarget(SlipIdx).Nav(), Scout, UCoverSlipReachSpec::StaticClass() ));
		if( Spec != NULL )
		{
			Spec->SpecDirection = (Slot->SlipTarget(SlipIdx).Direction > 0) ? CD_Right : CD_Left;
		}
	}
}

UBOOL ACoverSlotMarker::CanConnectTo( ANavigationPoint* Nav, UBOOL bCheckDistance )
{
	ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(Nav);
	if( Marker )
	{
		// Don't allow connection between markers from same cover link
		// unless they are adjacent slots
		if( OwningSlot.Link == Marker->OwningSlot.Link )
		{
			if( OwningSlot.Link->bCircular )
			{
				return FALSE;
			}

			INT NumBetween = Abs(OwningSlot.SlotIdx - Marker->OwningSlot.SlotIdx);
			if( NumBetween != 1 )
			{
				INT NumSlots = OwningSlot.Link->Slots.Num();
				if( !OwningSlot.Link->bLooped || 
					((NumBetween != NumSlots - 1) && (OwningSlot.SlotIdx != 0 || Marker->OwningSlot.SlotIdx != 0)) )
				{
					return FALSE;
				}
			}
		}		
	}

	return Super::CanConnectTo( Nav, bCheckDistance );
}

UBOOL AMantleMarker::CanConnectTo( ANavigationPoint* Nav, UBOOL bCheckDistance )
{
	ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(Nav);
	if( Marker != NULL )
	{
		if( Marker->OwningSlot.Link	   == OwningSlot.Link	 &&
			Marker->OwningSlot.SlotIdx != OwningSlot.SlotIdx )
		{
			return FALSE;
		}
	}

	return Super::CanConnectTo( Nav, bCheckDistance );
}

UClass* ACoverSlotMarker::GetReachSpecClass( ANavigationPoint* Nav, UClass* ReachSpecClass )
{
	ACoverSlotMarker* OtherMarker = Cast<ACoverSlotMarker>(Nav);
	if( OtherMarker != NULL )
	{
		if( OtherMarker->OwningSlot.Link == OwningSlot.Link )
		{
			return USlotToSlotReachSpec::StaticClass();
		}

		FCoverSlot* Slot = ACoverLink::CoverInfoToSlotPtr( OwningSlot );
		for( INT Idx = 0; Idx < Slot->TurnTarget.Num(); Idx++ )
		{
			FCoverSlot* TargSlot = ACoverLink::CoverRefToSlotPtr( Slot->TurnTarget(Idx) );
			if( TargSlot && TargSlot->SlotMarker == Nav )
			{
				return USwatTurnReachSpec::StaticClass();
			}
		}		
	}
	return ReachSpecClass;
}

UBOOL ACoverSlotMarker::CanPrunePath( INT Idx )
{
	if( PathList(Idx)->IsA(USlotToSlotReachSpec::StaticClass()) ||
		PathList(Idx)->IsA(USwatTurnReachSpec::StaticClass()) )
	{
		return FALSE;
	}
	return TRUE;
}

/**
 * Creates a new slot at the specified location/rotation and either inserts it at the specified index or
 * appends it to the end.  Also rebuilds the slot information if not currently in game.
 */
INT ACoverLink::AddCoverSlot(FVector SlotLocation, FRotator SlotRotation, INT SlotIdx, UBOOL bForceSlotUpdate)
{
	debugf(TEXT("** adding slot **"));
	// create a new slot based on default
	FCoverSlot NewSlot = GetArchetype<ACoverLink>()->Slots(0);
	NewSlot.LocationOffset = FRotationMatrix(Rotation).InverseTransformFVectorNoScale(SlotLocation - Location);
	NewSlot.RotationOffset = SlotRotation - Rotation;
	if (SlotIdx == -1)
	{
		SlotIdx = Slots.AddItem(NewSlot);
	}
	else
	{
		SlotIdx = Slots.InsertItem(NewSlot,SlotIdx);
	}
	if (!GIsGame)
	{
		// update the slot info
		AutoAdjustSlot(SlotIdx,FALSE);
		AutoAdjustSlot(SlotIdx,TRUE);
		BuildSlotInfo(SlotIdx);
	}
	else
	if (bForceSlotUpdate)
	{
		BuildSlotInfo(SlotIdx);
	}
	return SlotIdx;
}

/**
 * Adds the given slot to this link's list.  Returns the index.
 */
INT ACoverLink::AddCoverSlot(FVector& SlotLocation, FRotator& SlotRotation, FCoverSlot Slot, INT SlotIdx)
{
	debugf(TEXT("** adding slot **"));
	// adjust the location/rotation of the slot for it's new owner
	Slot.LocationOffset = FRotationMatrix(Rotation).InverseTransformFVectorNoScale(SlotLocation - Location);
	Slot.RotationOffset = SlotRotation - Rotation;
	if (SlotIdx == -1)
	{
		SlotIdx = Slots.AddItem(Slot);
	}
	else
	{
		SlotIdx = Slots.InsertItem(Slot,SlotIdx);
	}
	return SlotIdx;
}

UBOOL ACoverLink::FindCoverEdges(const FVector& StartLoc, FVector AxisX, FVector AxisY, FVector AxisZ)
{
	FLOAT StandingOffset = Cast<ACoverLink>(ACoverLink::StaticClass()->GetDefaultObject())->MidHeight;
	Slots.Empty();
	FCheckResult Hit;
	UBOOL bFoundEdge = FALSE;
	FVector Start = StartLoc;
	FVector LastHitNormal = AxisX * -1.f;
	INT IterationCount = 0;
	while (!bFoundEdge && IterationCount++ < 150)
	{
		// remember the initial start for this iteration in case we find an edge
		FVector InitialStart = Start;
		// trace to the left, first looking for a wall
		FVector End = Start + (AxisY * AlignDist * -2.f);
//		DrawDebugLine(Start,End,0,128,0,1);
		if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
		{
			// hit nothing, so trace down to make sure we have a valid base
			Start = End;
			End = Start + FVector(0,0,AlignDist * -4.f);
//			DrawDebugLine(Start,End,0,128,0,1);
			if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
			{
				// hit nothing, found a floor gap so trace back to get the correct edge
				debugf(TEXT("-+ found floor gap"));
				bFoundEdge = TRUE;
				Start = End;
				End = Start + AxisY * AlignDist * 4.f;
//				DrawDebugLine(Start,End,0,128,0,1);
				FVector SlotLocation = InitialStart;
				if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
				{
					// found the edge, set the slot location here
					SlotLocation += -AxisY * ((AlignDist * 2.f) - (Hit.Location - Start).Size() - AlignDist);
				}
				AddCoverSlot(SlotLocation,AxisX.Rotation());
			}
			else
			{
				// trace forward to look for a gap
				End = Start + AxisX * AlignDist * 2.f;
//				DrawDebugLine(Start,End,0,128,0,1);
				if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
				{
					debugf(TEXT("-+ found gap"));
					// hit nothing, found gap
					bFoundEdge = TRUE;
					// trace back to find the edge
					Start = Start + AxisX * AlignDist * 1.5f;
					End = Start + AxisY * AlignDist * 4.f;
					FVector SlotLocation = InitialStart;
//					DrawDebugLine(Start,End,0,128,0,1);
					if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
					{
						// found the edge, set the slot location here
						SlotLocation += -AxisY * ((AlignDist * 2.f) - (Hit.Location - Start).Size() - AlignDist);
					}
					AddCoverSlot(SlotLocation,(LastHitNormal * -1).Rotation());
				}
				else
				{
					// hit something, adjust the axis and continue
					debugf(TEXT("-+ hit wall, walking"));
//					DrawDebugLine(Hit.Location,Hit.Location + (Hit.Normal * 256.f),255,0,0,1);
					FRotationMatrix((Hit.Normal * -1).Rotation()).GetAxes(AxisX,AxisY,AxisZ);
					LastHitNormal = Hit.Normal;
					Start = Hit.Location + AxisX * -AlignDist;
				}
			}
		}
		else
		{
			debugf(TEXT("-+ hit adjacent wall"));
			// hit a wall, adjust the axes and keep going
			FRotationMatrix((Hit.Normal * -1).Rotation()).GetAxes(AxisX,AxisY,AxisZ);
			Start = Hit.Location + AxisX * -AlignDist;
		}
	}
	// if we failed to find an edge then abort now
	if (!bFoundEdge)
	{
		return FALSE;
	}
	debugf(TEXT("----- found left edge, working back -----"));
	// if we found a left edge then start filling in to the right edge, looking for transitions along the way
	bFoundEdge = FALSE;
	IterationCount = 0;
	INT LastSlotIdx = 0;
	FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
	Start = GetSlotLocation(LastSlotIdx);
	FVector LastPotentialSlotLocation = Start;
	FVector End;
	while (!bFoundEdge && IterationCount++ < 150)
	{
		FCoverSlot &LastSlot = Slots(LastSlotIdx);
		FVector InitialStart = Start;
		// then walk along until we find a high gap
		End = Start + (AxisY * AlignDist * 2.f);
//		DrawDebugLine(Start,End,0,128,128,1);
		if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
		{
			debugf(TEXT("> side clear"));
			FLOAT SideDist = (End - Start).Size();
			// trace fwd
			Start = End;
			End = Start + (AxisX * AlignDist * 3.f);
//			DrawDebugLine(Start,End,0,128,128,1);
			if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
			{
				debugf(TEXT(">- hit fwd wall"));
				FRotator HitRotation = (Hit.Normal * -1).Rotation();
				// hit something, check from standing height
				Start.Z += StandingOffset;
				End.Z += StandingOffset;
//				DrawDebugLine(Start,End,0,128,128,1);
				if (GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
				{
					debugf(TEXT(">-- fwd tall clear"));
					if (LastSlot.CoverType == CT_Standing)
					{
						// didn't hit, trace back to find the gap
						Start = Start + (AxisX * AlignDist * 1.5f);
						End = Start + (AxisY * AlignDist * -2.f);
//						DrawDebugLine(Start,End,128,128,64,1);
						if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
						{
							debugf(TEXT(">--+ traced back to find edge (standing->mid)"));
							// hit the wall, lay down two slots
							FVector Mid = InitialStart + (AxisY * (SideDist - (Hit.Location - Start).Size()));
							AddCoverSlot(Mid - (AxisY * AlignDist),HitRotation);
							LastSlotIdx = AddCoverSlot(Mid + (AxisY * AlignDist),HitRotation);
							FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
							// setup the new location/rotation for the next placement check
							Start = GetSlotLocation(LastSlotIdx);
							FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
						}
						else
						{
							debugf(TEXT(">--+ FAILED traced back to find edge"));
						}
					}
					else
					{
						// didn't hit something, still mid level cover
						Start = InitialStart + (AxisY * AlignDist * 2.f);
						FRotationMatrix(HitRotation).GetAxes(AxisX,AxisY,AxisZ);
						debugf(TEXT(">--+ retaining mid level"));
					}
				}
				else
				{
					if (LastSlot.CoverType == CT_Standing)
					{
						debugf(TEXT(">-- fwd tall hit, retaining standing"));
						// hit something, keep on trucking
						Start = InitialStart + (AxisY * AlignDist * 2.f); 
						FRotationMatrix((Hit.Normal * -1).Rotation()).GetAxes(AxisX,AxisY,AxisZ);
					}
					else
					{
						// trace back to find the edge and lay down the transition slots
						Start = InitialStart + (AxisX * AlignDist * 1.5f) + FVector(0,0,StandingOffset);
						End = Start + (AxisY * AlignDist * 2.f);
//						DrawDebugLine(Start,End,128,128,64,1);
						if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
						{
							debugf(TEXT(">--+ traced back to find edge (mid->standing)"));
							// hit the wall, place transition slots
							//FVector Mid = InitialStart + (AxisY * (SideDist - (Hit.Location - Start).Size() - AlignDist));
							FVector Mid = InitialStart + (AxisY * (Hit.Location - Start).Size());
							AddCoverSlot(Mid - (AxisY * AlignDist),HitRotation);
							LastSlotIdx = AddCoverSlot(Mid + (AxisY * AlignDist),HitRotation);
							FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
							// setup the new location/rotation for the next placement check
							Start = GetSlotLocation(LastSlotIdx);
							FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
						}
					}
				}
			}
			else
			{
				debugf(TEXT(">- didn't hit fwd, tracing for edge"));
				// didn't hit something, trace back to find the edge
				bFoundEdge = TRUE;
				Start = Start + (AxisX * AlignDist * 1.5f);
				End = Start + (AxisY * AlignDist * -2.f);
				if (!GWorld->SingleLineCheck(Hit,this,End,Start,TRACE_World,FVector(1.f)))
				{
					debugf(TEXT(">-- found edge"));
					// hit the edge, place slot
					FVector NewSlotLocation = InitialStart + (AxisY * (AlignDist * 2.f - (Hit.Location - Start).Size() - AlignDist));
					// make sure there was enough distance to make it worth placing an extra slot
					if ((NewSlotLocation - GetSlotLocation(LastSlotIdx)).Size() > AlignDist)
					{
						// add the new cover slot
						LastSlotIdx = AddCoverSlot(NewSlotLocation,(Hit.Normal * -1).Rotation());
					}
					else
					{
						debugf(TEXT(">--- too close to original slot, skipping"));
						// save this position in case we need the information to auto-center the previous slot
						LastPotentialSlotLocation = NewSlotLocation;
					}
				}
				else
				{
					debugf(TEXT(">-- missed edge, placing back at start position"));
					LastSlotIdx = AddCoverSlot(InitialStart,AxisX.Rotation());
				}
			}
		}
		else
		{
			debugf(TEXT("> hit side wall"));
			// hit a side wall, throw down both slots and adjust
            FVector NewSlotLocation = Hit.Location + (Hit.Normal * AlignDist);
			//@note - the following assumes that AxisX is semi-perpindicular to Hit.Normal, and for better or worse it seems to work well enough
			// add first slot farther up facing original direction
			AddCoverSlot(NewSlotLocation + (Hit.Normal * 16.f),AxisX.Rotation());
			// add second facing new direction
			LastSlotIdx = AddCoverSlot(NewSlotLocation + (AxisX * -16.f),(Hit.Normal * -1).Rotation());
			// setup the new location/rotation for the next placement check
			Start = GetSlotLocation(LastSlotIdx);
			FRotationMatrix(GetSlotRotation(LastSlotIdx)).GetAxes(AxisX,AxisY,AxisZ);
		}
	}
	if (Slots.Num() == 1 && bFoundEdge)
	{
		debugf(TEXT("> correcting single slot position"));
		FVector OldSlotLocation = GetSlotLocation(0);
		Slots(0).LocationOffset = FRotationMatrix(Rotation).InverseTransformFVectorNoScale((OldSlotLocation + (((OldSlotLocation - LastPotentialSlotLocation).Size() * 0.5f) * (LastPotentialSlotLocation - OldSlotLocation).SafeNormal())) - Location);
	}
	return TRUE;
}

void ACoverLink::EditorAutoSetup(FVector Direction, FVector *HitLoc, FVector *HitNorm)
{
	FVector SetupLoc, SetupNorm;
	UBOOL bTrySetup = FALSE;
	// if loc/norm were passed in
	if (HitLoc != NULL && HitNorm != NULL)
	{
		// then just use those values
		SetupLoc = *HitLoc;
		SetupNorm = *HitNorm;
		bTrySetup = TRUE;
	}
	else
	{
		// search for the surface that was clicked on
		FCheckResult Hit;
		if (!GWorld->SingleLineCheck(Hit,this,Location + Direction * 256.f,Location,TRACE_World,FVector(1.f)))
		{
			SetupLoc = Hit.Location;
			SetupNorm = Hit.Normal;
			bTrySetup = TRUE;
		}
	}
	// if we have valid loc/norm
	if (bTrySetup)
	{
		// check to see if the surface is valid
		if (Abs(SetupNorm | FVector(0,0,1)) > 0.3f)
		{
			appMsgf(AMT_OK,TEXT("Invalid surface normal"));
			GWorld->DestroyActor(this);
			return;
		}

		// attempt to move the coverlink into position
		SetRotation((SetupNorm * -1).Rotation());
		SetLocation(SetupLoc + SetupNorm * 128.f);
		FindBase();

		// get base rotation axes
		FRotationMatrix RotMatrix(Rotation);
		FVector AxisX, AxisY, AxisZ;
		RotMatrix.GetAxes(AxisX,AxisY,AxisZ);

		// and attempt to setup
		if (!FindCoverEdges(Location + AxisX * 96.f + AxisZ * 16.f,AxisX,AxisY,AxisZ))
		{
			appMsgf(AMT_OK,TEXT("Failed to place any slots"));
			GWorld->DestroyActor(this);
			return;
		}

		ForceUpdateComponents(FALSE,FALSE);
		debugf(TEXT("all finished"));
        //GCallbackEvent->Send( CALLBACK_RedrawAllViewports );
	}
	else
	{
		// kick up a warning letting the designer know what happened
		appMsgf(AMT_OK,TEXT("Failed to find valid surface"));
		GWorld->DestroyActor(this);
	}
}

/**
 *	Gives a valid FCoverSlot* from a FCoverInfo ref
 */
FCoverSlot* ACoverLink::CoverInfoToSlotPtr( FCoverInfo& Info )
{
	FCoverSlot* Result = NULL;
	if( Info.Link &&
		Info.SlotIdx >= 0 && 
		Info.SlotIdx <  Info.Link->Slots.Num() )
	{
		Result = &Info.Link->Slots(Info.SlotIdx);
	}
	return Result;
}

FCoverSlot* ACoverLink::CoverRefToSlotPtr( FCoverReference& InRef )
{
	FCoverSlot* Result = NULL;
	ACoverLink* Link   = Cast<ACoverLink>(InRef.Nav());
	if( Link &&
		InRef.SlotIdx >= 0 &&
		InRef.SlotIdx <  Link->Slots.Num() )
	{
		Result = &Link->Slots(InRef.SlotIdx);
	}
	return Result;
}

INT ACoverLink::AddMyMarker(AActor *S)
{
	// first pass looks for really long sections to add intermediate slots
	// JF: Commented until it can be fixed to work better, at the LDs' request.
	//const FLOAT MaxSlotDistance = CylinderComponent->CollisionHeight * 6.f;
	//for (INT Idx = 0; Idx < Slots.Num() - 1; Idx++)
	//{
	//	FCoverSlot &SlotA = Slots(Idx), &SlotB = Slots(Idx+1);
	//	// only allow intermediate between slots of the same type
	//	if (SlotA.CoverType == SlotB.CoverType)
	//	{
	//		FVector SlotALocation = GetSlotLocation(Idx), SlotBLocation = GetSlotLocation(Idx+1);
	//		FRotator SlotARotation = GetSlotRotation(Idx);
	//		const FLOAT Distance = (SlotALocation - SlotBLocation).Size();
	//		if (Distance > MaxSlotDistance)
	//		{
	//			// figure out how many to place
	//			const INT NumSlotsNeeded = appTrunc(Distance/MaxSlotDistance);
	//			// slot direction for calculating new positions
	//			FVector SlotDir = (SlotBLocation - SlotALocation).SafeNormal();
	//			FLOAT SpacingDistance = Distance/(NumSlotsNeeded + 1);
	//			for (INT SlotIdx = 0; SlotIdx < NumSlotsNeeded; SlotIdx++)
	//			{
	//				AddCoverSlot(SlotALocation + (SlotDir * SpacingDistance * (SlotIdx + 1)),SlotARotation,Idx+1);
	//				// skip over this added slot
	//				Idx++;
	//			}
	//		}
	//	}
	//}

	INT AdditionalMarkers = 0;
	for (INT Idx = 0; Idx < Slots.Num(); Idx++)
	{
		FCoverSlot &Slot = Slots(Idx);
		AScout* Scout  = Cast<AScout>(S);
				
		// create a marker at a location pushed out from cover surface
		FRotator SpawnRot = GetSlotRotation(Idx);
		FVector  SpawnLoc = GetSlotLocation(Idx);
			
		Slot.SlotMarker = Cast<ACoverSlotMarker>(GWorld->SpawnActor(ACoverSlotMarker::StaticClass(),NAME_None,SpawnLoc,SpawnRot,NULL,TRUE,FALSE,this));
		if (Slot.SlotMarker != NULL)
		{
			// update the marker to point to us
			Slot.SlotMarker->OwningSlot.Link = this;
			Slot.SlotMarker->OwningSlot.SlotIdx = Idx;
			// pass ExtraCost to the marker
			Slot.SlotMarker->ExtraCost = Slot.ExtraCost;

			Slot.SlotMarker->bBlocked = bBlocked;
			Slot.SlotMarker->SetCollisionType( CollisionType );
		}

		// Mantle Up/Down Markers
		if( Slot.bAllowClimbUp && Slot.bAllowPopup )
		{
			// Calc spawn location of mantle marker
			SpawnLoc	+= FRotationMatrix(SpawnRot).TransformNormal(FVector(64.f,0.f,0.f));
			SpawnLoc.Z	+= 64.f;

			// If spot is clear
			FVector Extent = Scout->GetSize(TEXT("Human"));
			Extent.Z = Extent.Y;
			Extent.Y = Extent.X;
			if( Scout->FindSpot( Extent , SpawnLoc ) )
			{
				// Create marker and set owning slot
				AMantleMarker* Marker		= Cast<AMantleMarker>(GWorld->SpawnActor(AMantleMarker::StaticClass(),NAME_None,SpawnLoc,SpawnRot,NULL,TRUE,FALSE,this));
				Marker->OwningSlot.Link		= this;
				Marker->OwningSlot.SlotIdx	= Idx;

				Marker->bBlocked			= bBlocked;
				Marker->SetCollisionType( CollisionType );

				AdditionalMarkers++;
			}
		}
	}


	return (Slots.Num() + AdditionalMarkers);
}

void ACoverLink::PreBeginPlay()
{
	Super::PreBeginPlay();
	if( GIsGame )
	{
		/*		
		// copy forced links
		//disable forcefirelinks
		for (INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++)
		{
			FCoverSlot &Slot = Slots(SlotIdx);
			for (INT Idx = 0; Idx < Slot.ForcedFireLinks.Num(); Idx++)
			{
				Slot.FireLinks.AddItem(Slot.ForcedFireLinks(Idx));
			}
		}*/
	}
}

void ACoverLink::GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel)
{
	Super::GetActorReferences(ActorRefs,bIsRemovingLevel);
	for (INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++)
	{
		FCoverSlot &Slot = Slots(SlotIdx);
		if (Slot.MantleTarget.Guid.IsValid())
		{
			ActorRefs.AddItem(&Slot.MantleTarget);
		}
		for (INT Idx = 0; Idx < Slot.TurnTarget.Num(); Idx++)
		{
			FActorReference &ActorRef = Slot.TurnTarget(Idx);
			if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL))
			{
				ActorRefs.AddItem(&ActorRef);
			}
		}
		for (INT Idx = 0; Idx < Slot.SlipTarget.Num(); Idx++)
		{
			FActorReference &ActorRef = Slot.SlipTarget(Idx);
			if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL))
			{
				ActorRefs.AddItem(&ActorRef);
			}
		}
		for (INT Idx = 0; Idx < Slot.FireLinks.Num(); Idx++)
		{
			FActorReference &ActorRef = Slot.FireLinks(Idx).TargetMarker;
			if (ActorRef.Guid.IsValid())
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
		}
		for( INT Idx = 0; Idx < Slot.ExposedFireLinks.Num(); Idx++ )
		{
			FActorReference &ActorRef = Slot.ExposedFireLinks(Idx).TargetMarker;
			if( ActorRef.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
		}
		for( INT Idx = 0; Idx < Slot.OverlapClaims.Num(); Idx++ )
		{
			FActorReference &ActorRef = Slot.OverlapClaims(Idx);
			if( ActorRef.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
		}
		for( INT Idx = 0; Idx < Slot.DangerLinks.Num(); Idx++ )
		{
			FActorReference& ActorRef = Slot.DangerLinks(Idx).DangerNav;
			if( ActorRef.Guid.IsValid() )
			{
				// if removing a level, only valid if not null,
				// if not removing, only if null
				if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
					(!bIsRemovingLevel && ActorRef.Actor == NULL))
				{
					ActorRefs.AddItem(&ActorRef);
				}
			}
		}
	}
}

void ACoverLink::CheckForErrors()
{
	Super::CheckForErrors();

	for( INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++ )
	{
		FCoverSlot* Slot = &Slots(SlotIdx);
		if( Slot->CoverType == CT_None )
		{
			GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s Slot %d has not CoverType"), *GetName(), SlotIdx ) );
		}
		ACoverLink *MantleLink = Cast<ACoverLink>(~Slot->MantleTarget);
		if( MantleLink != NULL && 
				(Slot->MantleTarget.SlotIdx < 0 ||
				 Slot->MantleTarget.SlotIdx >= MantleLink->Slots.Num()) )
		{
			GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s Slot %d has invalid mantle target!"), *GetName(), SlotIdx ) );
		}
		if( Slot->bFailedToFindSurface )
		{
			GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s Slot %d failed to align to surface!"), *GetName(), SlotIdx ) );
		}
	}
}

UBOOL ACoverLink::FindSlots(FVector CheckLocation, FLOAT MaxDistance, INT& LeftSlotIdx, INT& RightSlotIdx)
{
	LeftSlotIdx  = -1;
	RightSlotIdx = -1;
	// run through each slot
	FRotationMatrix RotMatrix(Rotation);
	for (INT SlotIdx = 0; SlotIdx < Slots.Num() - 1; SlotIdx++)
	{
		FVector LeftSlotLocation = Location + RotMatrix.TransformFVector(Slots(SlotIdx).LocationOffset);
		FVector RightSlotLocation = Location + RotMatrix.TransformFVector(Slots(SlotIdx+1).LocationOffset);
		// adjust for edge slots
		if (SlotIdx == 0)
		{
			LeftSlotLocation += (LeftSlotLocation - RightSlotLocation).SafeNormal() * AlignDist;
		}
		if (SlotIdx == Slots.Num() - 2)
		{
			RightSlotLocation += (RightSlotLocation - LeftSlotLocation).SafeNormal() * AlignDist;
		}

		// Get the axis between the two slots
		FVector Axis = (RightSlotLocation-LeftSlotLocation).SafeNormal();

		// Only allow in, if half way to an enabled slot
		FLOAT Pct = Clamp<FLOAT>(((CheckLocation - LeftSlotLocation) | Axis) / (RightSlotLocation - LeftSlotLocation).Size(), 0.f, 1.f);
		if( (Pct < 0.5f && Slots(SlotIdx).bEnabled)		||
			(Pct >= 0.5f && Slots(SlotIdx+1).bEnabled)	)
		{
			// check to see if we are between the two locations
			if( ((LeftSlotLocation  - CheckLocation).SafeNormal() | -Axis) > 0.f &&
				((RightSlotLocation - CheckLocation).SafeNormal() |  Axis) > 0.f )
			{
				// find the distance from the link plane
				FVector ProjectedLocation = LeftSlotLocation + (Axis * ((CheckLocation-LeftSlotLocation) | Axis));
				FLOAT	Dist = (ProjectedLocation - CheckLocation).Size();
				if( Dist <= MaxDistance )
				{
					// mark the slots and break out
					LeftSlotIdx		= SlotIdx;
					RightSlotIdx	= SlotIdx + 1;
					break;
				}
			}
		}
	}
	return (LeftSlotIdx != -1 && RightSlotIdx != -1);
}


/**
 * Returns the height for the specified slot.
 */
FLOAT ACoverLink::GetSlotHeight(INT SlotIdx)
{
	if( SlotIdx >= 0 && SlotIdx < Slots.Num() )
	{
		switch (Slots(SlotIdx).CoverType)
		{
			case CT_MidLevel:
				return MidHeight;
			case CT_Standing:
			default:
				return StandHeight;
		}
	}
	return 0.f;
}

ACoverSlotMarker* ACoverLink::GetSlotMarker( INT SlotIdx )
{
	ACoverSlotMarker* Marker = NULL;
	if( SlotIdx >= 0 && SlotIdx < Slots.Num() )
	{
		Marker = Slots(SlotIdx).SlotMarker;
	}
	return Marker;
}

FVector ACoverLink::GetSlotLocation(INT SlotIdx, UBOOL bForceUseOffset)
{
	if (SlotIdx >= 0 && SlotIdx < Slots.Num())
	{
		// if in-game and has a valid slot marker
		if( GIsGame && !bForceUseOffset && Slots(SlotIdx).SlotMarker != NULL )
		{
			// then just use the current marker location
			return Slots(SlotIdx).SlotMarker->Location;
		}
		return Location + FRotationMatrix(Rotation).TransformFVector(Slots(SlotIdx).LocationOffset);
	}
	return Location;
}

FRotator ACoverLink::GetSlotRotation(INT SlotIdx, UBOOL bForceUseOffset)
{
	if (SlotIdx >= 0 && SlotIdx < Slots.Num())
	{
		if (GIsGame && !bForceUseOffset && Slots(SlotIdx).SlotMarker != NULL)
		{
			return Slots(SlotIdx).SlotMarker->Rotation;
		}
		return FRotator(Rotation.Quaternion() * Slots(SlotIdx).RotationOffset.Quaternion());
	}
	return Rotation;
}

FVector ACoverLink::GetSlotViewPoint( INT SlotIdx, BYTE Type, BYTE Action )
{
	if( SlotIdx >= 0 && SlotIdx < Slots.Num() )
	{
		FVector Offset;
		FVector ViewPt = GetSlotLocation(SlotIdx);

		if( Type == CT_None )
		{
			Type = Slots(SlotIdx).CoverType;
		}

		switch( Type )
		{
			case CT_Standing:
				Offset	  = StandingLeanOffset;
				break;
			case CT_MidLevel:
			default:
				Offset	  = CrouchLeanOffset;
				break;
		}

		if( Action != CA_Default )
		{
			FVector X, Y, Z;
			FRotationMatrix(GetSlotRotation(SlotIdx)).GetAxes(X,Y,Z);

			if( Action == CA_LeanLeft	|| 
				Action == CA_BlindLeft	|| 
				Action == CA_PeekLeft	)
			{
				ViewPt += (Offset.X *  X + 
						   Offset.Y * -Y + 
						   Offset.Z *  Z);
			}
			else
			if( Action == CA_LeanRight	|| 
				Action == CA_BlindRight || 
				Action == CA_PeekRight	)
			{
				ViewPt += (Offset.X *  X + 
						   Offset.Y *  Y + 
						   Offset.Z *  Z);
			}
			else if( Type == CT_MidLevel )
			{
				if( Action == CA_PopUp		|| 
					Action == CA_BlindUp	|| 
					Action == CA_PeekUp		)
				{
					ViewPt += (PopupOffset.X *  X + 
							   PopupOffset.Y *  Y + 
							   PopupOffset.Z *  Z);
				}
			}
		}
		else
		{
			ViewPt.Z += Offset.Z;
		}

		return ViewPt;
	}
	return Location;
}

BYTE ACoverLink::GetLocationDescription(INT SlotIdx)
{
	if( SlotIdx >= 0 && SlotIdx < Slots.Num() )
	{
		BYTE Desc = Slots(SlotIdx).LocationDescription;
		if (Desc == CoverDesc_None)
		{
			Desc = LocationDescription;
		}

		return Desc;
	}

	return CoverDesc_None;
}

UBOOL ACoverLink::CanFireLinkHit( const FVector &ViewPt, const FVector &TargetLoc, UBOOL bDebugLines)
{
	//debugf(TEXT("- can fire at %s (%d)?"),*Link->GetName(),SlotIdx);
	FCheckResult Hit(1.f);
	FVector TraceExtent( 0.f );

	GWorld->SingleLineCheck( Hit, this, TargetLoc, ViewPt, TRACE_World|TRACE_StopAtAnyHit|TRACE_ComplexCollision, TraceExtent );
	if( Hit.Actor == NULL )
	{
		return 1;
	}

	//debug
	if( bDebugLines ) { DrawDebugLine( ViewPt, TargetLoc, 0, 0, 255, 1 ); }

	// no luck
	return 0;
}

/**
************>>> ACoverLink::SortSlots() helper functions follow
*/
/** statics for sort comparator **/
#define SORT_DEBUG 0 && !FINAL_RELEASE && !PS3

static INT ClockWiseAngleDist(INT AYaw, INT BYaw)
{
	if(AYaw < 0)
	{
		AYaw = 65535 + AYaw;
	}
	if(BYaw < 0)
	{
		BYaw = 65535 + BYaw;
	}
	INT Delta = BYaw - AYaw;
	if(Delta < 0)
	{
		Delta += 65535;
		return Delta;
	}

	return Delta;
}
FLOAT GTraceDistance = 128.0f;
FLOAT GGapIncrement = 10.0f;
#define GAP_INCREMENT GGapIncrement
#define TRACE_DISTANCE GTraceDistance
#define GAP_THRESHOLD 150.f
#define SORT_TRACE_FLAGS TRACE_World|TRACE_StopAtAnyHit
#define SORT_TRACE_EXTENT FVector(1.f)
UBOOL HasGapBetween(ACoverLink* CoverLink, INT SlotA, INT SlotB)
{
	FVector SlotALoc = CoverLink->GetSlotLocation(SlotA);
	FVector SlotBLoc = CoverLink->GetSlotLocation(SlotB);

	FCheckResult Hit(1.f);
	// if there is something inbetween us and the slot, forget it
	if(!GWorld->SingleLineCheck(Hit,NULL,SlotALoc,SlotBLoc,SORT_TRACE_FLAGS,SORT_TRACE_EXTENT))
	{
#if SORT_DEBUG
		CoverLink->DrawDebugLine(SlotALoc,SlotBLoc,52,255,0,TRUE);
#endif
		return TRUE;
	}

	FVector TraceDir = ((CoverLink->GetSlotRotation(SlotA).Vector() + CoverLink->GetSlotRotation(SlotB).Vector())/2.f).SafeNormal();

	FVector TestDir = (SlotBLoc - SlotALoc).SafeNormal();
	
	FVector TestPos = SlotALoc;
	UBOOL LastTestWasGap = FALSE;
	FLOAT GapDist = 0.f;
	while((TestPos - SlotBLoc | TestDir) < 0.f)
	{
		//DEBUG
		#if SORT_DEBUG
			CoverLink->DrawDebugLine(TestPos,TestPos+(TraceDir*TRACE_DISTANCE),255,255,255,TRUE);
		#endif

		if(GWorld->SingleLineCheck(Hit,NULL,TestPos+(TraceDir*TRACE_DISTANCE),TestPos,SORT_TRACE_FLAGS,SORT_TRACE_EXTENT) && GWorld->SingleLineCheck(Hit,NULL,TestPos+(TraceDir*TRACE_DISTANCE),TestPos,SORT_TRACE_FLAGS))
		{
		#if SORT_DEBUG
			CoverLink->DrawDebugLine(TestPos,TestPos+(TraceDir*TRACE_DISTANCE),200,200,200,TRUE);
		#endif

			// if we pass through increment the gap size counter
			if(LastTestWasGap == TRUE)
			{
				GapDist += GAP_INCREMENT;
			}
			else
			{
				LastTestWasGap = TRUE;
			}

			if(GapDist >= GAP_THRESHOLD)
			{
				#if SORT_DEBUG
					CoverLink->DrawDebugLine(TestPos,TestPos+(TraceDir*TRACE_DISTANCE),255,0,0,TRUE);
					CoverLink->DrawDebugLine(SlotALoc,SlotBLoc,255,128,0,TRUE);
				#endif

				return TRUE;
			}			
		}
		else
		{
			LastTestWasGap = FALSE;
			GapDist = 0.f;
		}
		
		TestPos += TestDir * GAP_INCREMENT;
	}

	return FALSE;
}

#define RIGHT_DIR 1
#define LEFT_DIR -1
INT GetRatingFromAToB(ACoverLink* Link, INT SlotIdxA, INT SlotIdxB, INT Dir, INT YawConversionFactor=50)
{
	FRotationMatrix SlotRot = Link->GetSlotRotation(SlotIdxA);
	FVector ASlotLoc = Link->GetSlotLocation(SlotIdxA);

	INT AYaw = SlotRot.Rotator().Yaw;
	FVector AToB = Link->GetSlotLocation(SlotIdxB) - ASlotLoc;

	INT BYaw = AToB.Rotation().Yaw;
	INT YawDiff = Abs<INT>(ClockWiseAngleDist(AYaw,BYaw));

	if(Dir == LEFT_DIR && YawDiff > 0)
	{
		YawDiff = 65535 - YawDiff;
	}

	INT Dist = YawDiff / YawConversionFactor;

	// add in actual distance
	Dist += appTrunc(AToB.Size());

	return Dist;
}

INT FindBestMatchForSlot(ACoverLink* Link, INT SlotIdx, INT Dir, TDoubleLinkedList<INT>& ResortedList, UBOOL bCheckGap, INT YawConversionFactor=50)
{
	INT BestIdx = -1;
	FLOAT BestDist = BIG_NUMBER;
	for(INT Idx=0; Idx < Link->Slots.Num(); Idx++)
	{
		if(Idx == SlotIdx)
		{
			continue;
		}

		INT ADist = GetRatingFromAToB(Link,SlotIdx,Idx,Dir,YawConversionFactor);
		
		// if this is better, and valid (not already added)
		if(ADist < BestDist && ResortedList.FindNode(Idx) == NULL && (!bCheckGap || !HasGapBetween(Link,SlotIdx,Idx)))
		{
			BestDist = ADist;
			BestIdx = Idx;
		}
	}

	return BestIdx;
}

UBOOL LinkToBestCandidate(ACoverLink* Link, TDoubleLinkedList<INT>::TIterator& Itt, TDoubleLinkedList<INT>& ResortedList,INT Direction, UBOOL bCheckGap=TRUE)
{
	INT MatchIdx = FindBestMatchForSlot(Link,*Itt,Direction,ResortedList,bCheckGap);		
	if(MatchIdx != -1 && ResortedList.FindNode(MatchIdx)==NULL)
	{				
		if(Direction == LEFT_DIR)
		{
			ResortedList.InsertNode(MatchIdx,Itt.GetNode());
		}
		else if(Itt.GetNode() != ResortedList.GetTail())
		{
			ResortedList.InsertNode(MatchIdx,Itt.GetNode()->GetNextNode());
		}
		else
		{
			ResortedList.AddTail(MatchIdx);
		}

		return TRUE; 
	}

	
	return FALSE;
}

void InsertAtBestPoint(ACoverLink* Link, INT IdxToInsert, TDoubleLinkedList<INT>& ResortedList, INT Direction)
{
	INT BestRating = 65535;
	UBOOL bLeft = FALSE;
	TDoubleLinkedList<INT>::TIterator BestRated(NULL);
	for(TDoubleLinkedList<INT>::TIterator Itt(ResortedList.GetHead());Itt;++Itt)
	{
		INT CurRating = GetRatingFromAToB(Link,IdxToInsert,*Itt,RIGHT_DIR);

		if(CurRating < BestRating)
		{
			bLeft = FALSE;
			BestRating = CurRating;
			BestRated = Itt;
		}

	}

	for(TDoubleLinkedList<INT>::TIterator Itt(ResortedList.GetTail());Itt;--Itt)
	{
		INT CurRating = GetRatingFromAToB(Link,IdxToInsert,*Itt,LEFT_DIR);

		if(CurRating < BestRating)
		{
			bLeft = TRUE;
			BestRating = CurRating;
			BestRated = Itt;
		}
	}


	if(BestRated)
	{
		if(bLeft)
		{
			if(BestRated.GetNode()->GetNextNode() == NULL)
			{
				ResortedList.AddTail(IdxToInsert);
			}
			else
			{
				ResortedList.InsertNode(IdxToInsert,BestRated.GetNode()->GetNextNode());
			}
		}
		else
		{
			ResortedList.InsertNode(IdxToInsert,BestRated.GetNode());
		}

		
	}
}

/** ******>>> End SortSlots Helper functions			**/
void ACoverLink::SortSlots()
{
	if( bAutoSort && !bCircular && Slots.Num() > 0)
	{
	
#if SORT_DEBUG
		FlushPersistentDebugLines();
#endif
		
		//** Find the node with the worst rating to its neighbor to the left, chances are this is the leftmost node
		TDoubleLinkedList<INT> ResortedList;		
		INT BestDistSq = -1;
		INT LeftMostIdx = 0; // init to 0 in case no leftmost is found
		for(INT SlotIdx=0;SlotIdx<Slots.Num();SlotIdx++)
		{
			INT LeftOfMeIdx = FindBestMatchForSlot(this,SlotIdx,LEFT_DIR,ResortedList,TRUE,40);
#if SORT_DEBUG && 0
			FVector SlotLoc = GetSlotLocation(SlotIdx) + FVector(0.f,0.f,10.f);
			if(LeftOfMeIdx < 0)
			{
				DrawDebugLine(SlotLoc,SlotLoc + FVector(0.f,0.f,50.f),255,0,0,TRUE);
			}
			else
			{
				FVector LeftSlotLoc = GetSlotLocation(LeftOfMeIdx) + FVector(0.f,0.f,10.f);;
				DrawDebugLine(SlotLoc,SlotLoc + ((LeftSlotLoc-SlotLoc)*0.9f),255,255,0,TRUE);
			}			
#endif
			if(LeftOfMeIdx != -1)
			{
				INT CurDistSq = GetRatingFromAToB(this,SlotIdx,LeftOfMeIdx,LEFT_DIR,40);
				if( CurDistSq > BestDistSq)
				{
					LeftMostIdx = SlotIdx;
					BestDistSq = CurDistSq;
				}
			}
		}

#if SORT_DEBUG
		DrawDebugCoordinateSystem(GetSlotLocation(LeftMostIdx),GetSlotRotation(LeftMostIdx),50.f,TRUE);
#endif
		
		// add the leftmost at the head of the list
		ResortedList.AddHead(LeftMostIdx);

		// add in nodes to the right
		for(TDoubleLinkedList<INT>::TIterator Itt(ResortedList.GetHead());Itt;++Itt)
		{
			LinkToBestCandidate(this,Itt,ResortedList,RIGHT_DIR);
		}

		// if we're at the end, and we don't have everything sorted out yet start over from the front and add in nodes to the left
		if(ResortedList.Num() < Slots.Num())
		{
			for(TDoubleLinkedList<INT>::TIterator Itt(ResortedList.GetHead());Itt;--Itt)
			{
				LinkToBestCandidate(this,Itt,ResortedList,LEFT_DIR);
			}
		}	


		// if there are orphans, go back and place them in the list at the best point, and skip gap checks
		if(ResortedList.Num() < Slots.Num())
		{
			// add in nodes that didn't get sorted so we don't drop slots
			for(INT Idx=0;Idx<Slots.Num();Idx++)
			{
				if(ResortedList.FindNode(Idx) == NULL)
				{
#if SORT_DEBUG
					DrawDebugLine(GetSlotLocation(Idx),GetSlotLocation(Idx)+FVector(0.f,0.f,100.f),255,0,0,TRUE);
#endif
					//ResortedList.AddTail(Idx);
					InsertAtBestPoint(this,Idx,ResortedList,RIGHT_DIR);
				}
			}
		}

		if(ResortedList.Num() < Slots.Num())
		{
			debugf(TEXT("Could not fully sort slots for %s!"),*GetName());
		}

		INT IdxCounter = 0;
		TArray<FCoverSlot> Temp;

		for(TDoubleLinkedList<INT>::TIterator Itt(ResortedList.GetHead());Itt;++Itt)
		{
			INT SlotIdx = Temp.AddItem(Slots(*Itt));

			if (Temp(SlotIdx).SlotMarker != NULL)
			{
				Temp(SlotIdx).SlotMarker->OwningSlot.SlotIdx = SlotIdx;
			}
			SlotIdx++;			
		}

		Slots = Temp;
	}
}

#define DESIREDMINFIRELINKANGLE  0.65
#define MINFIRELINKANGLE 0.45

static UBOOL IsValidMantleTarget(ACoverLink *SrcLink, INT SrcSlotIdx, ACoverLink *DestLink, INT DestSlotIdx, FLOAT& DotP, FLOAT& Dist)
{
	if( DestLink == NULL )
	{
		return FALSE;
	}

	DotP = 0.f;
	Dist = 0.f;
	FVector SrcLocation  = SrcLink->GetSlotLocation(SrcSlotIdx);
	FVector DestLocation = DestLink->GetSlotLocation(DestSlotIdx);
	FVector SrcToDest	 = DestLocation - SrcLocation;

	//debugf(TEXT("%s %d vs %s %d %2.3f"),*SrcLink->GetName(),SrcSlotIdx,*DestLink->GetName(),DestSlotIdx,SrcToDest.Size());
	if( SrcToDest.Size() < 256.f )
	{
		FVector SrcSlotVect  = FRotationMatrix(SrcLink->GetSlotRotation(SrcSlotIdx)).GetAxis(0);
		FVector DestSlotVect = FRotationMatrix(DestLink->GetSlotRotation(DestSlotIdx)).GetAxis(0);

		// If dest slot is in front of src slot
		FLOAT LocDotP = (SrcToDest.SafeNormal() | SrcSlotVect);

//		debugf(TEXT("- front %2.5f"),LocDotP);
		if( LocDotP > 0.95f )
		{
			DotP =  SrcSlotVect | DestSlotVect;

//			debugf(TEXT("- dot %2.5f"),DotP);
			if( DotP < -0.8f )
			{
				Dist = PointDistToLine(SrcLocation,FRotationMatrix(DestLink->GetSlotRotation(DestSlotIdx)).GetAxis(1),DestLocation);
				//debugf(TEXT("- dist %2.3f"),Dist);
				if (Dist > 32.f && Dist < 180.f)
				{
					FCheckResult Hit;
					FVector Extent(SrcLink->AlignDist);
					FVector Start = SrcLocation + FVector(0,0,1) * (SrcLink->AlignDist + SrcLink->MidHeight);
					FVector	End	  = FVector(DestLocation.X, DestLocation.Y, Start.Z);

					//debug
//					SrcLink->FlushPersistentDebugLines();
//					SrcLink->DrawDebugBox( Start,  Extent, 0, 255, 0, TRUE );
//					SrcLink->DrawDebugBox( End, Extent, 255, 0, 0, TRUE );

					if( GWorld->SingleLineCheck( Hit, SrcLink, Start, End, TRACE_World|TRACE_StopAtAnyHit, Extent ) && !Hit.bStartPenetrating)
					{
						return TRUE;
					}
#if !FINAL_RELEASE
					else
					{
						// Draw nasty debug cylinder
						// REMOVE AFTER LDs SOLVE ISSUES
						SrcLink->DrawDebugBox(SrcLocation, FVector(34.f,34.f,80.f), 255, 0, 0, TRUE);
						return FALSE;
					}
#endif
				}
			}
		}
	}
	return FALSE;
}

static UBOOL CanMantle( AScout* Scout, ACoverLink* SrcLink, INT SrcSlotIdx )
{
	UBOOL bResult = FALSE;
	FCoverSlot &Slot = SrcLink->Slots(SrcSlotIdx);
	FVector		SlotLocation = SrcLink->GetSlotLocation(SrcSlotIdx);
    
	//@fixme - can't always rely on coverlist being valid, but it would be a great optimization
	//AWorldInfo *Info = GWorld->GetWorldInfo();
	//for( ACoverLink *Link = Info->CoverList; Link != NULL; Link = Link->NextCoverLink )
	for (FActorIterator ActorIt; ActorIt; ++ActorIt)
	{
		ACoverLink *Link = Cast<ACoverLink>(*ActorIt);
		if( Link != NULL && Link != SrcLink )
		{
			// check each slot for distance
			for( INT Idx = 0; Idx < Link->Slots.Num(); Idx++ )
			{
				FLOAT Dist, DotP;
				if( IsValidMantleTarget( SrcLink, SrcSlotIdx, Link, Idx, DotP, Dist ) )
				{
					// If there is no target yet OR
					// This one is closer than the current target
					ACoverLink *MantleLink = Cast<ACoverLink>(Slot.MantleTarget.Actor);
					if( MantleLink == NULL ||
						((MantleLink->GetSlotLocation(Slot.MantleTarget.SlotIdx)-SlotLocation).Size() > 
							(Link->GetSlotLocation(Idx)-SlotLocation).Size()) )
					{
						Slot.MantleTarget.Actor		=  Link;
						Slot.MantleTarget.Guid		= *Link->GetGuid();
						Slot.MantleTarget.SlotIdx	=  Idx;
						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

static UBOOL CanClimbUp( AScout* Scout, ACoverLink* SrcLink, INT SrcSlotIdx )
{
	UBOOL bResult = FALSE;
	FCoverSlot& Slot = SrcLink->Slots(SrcSlotIdx);

	// Search for all cover markers
	for( FActorIterator ActorIt; ActorIt; ++ ActorIt )
	{
		// If marker is valid and not deleted
		AMantleMarker* Marker = Cast<AMantleMarker>(*ActorIt);
		if( Marker != NULL && !Marker->bDeleteMe )
		{
			// If marker is owned by this slot
			if( Marker->OwningSlot.Link == SrcLink && 
				Marker->OwningSlot.SlotIdx == SrcSlotIdx )
			{
				// Set mantle target and exit
				Slot.MantleTarget.Actor		=  Marker;
				Slot.MantleTarget.Guid		= *Marker->GetGuid();
				Slot.MantleTarget.SlotIdx	= -1;				
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}

static UBOOL CanCoverSlip( AScout* Scout, ACoverLink* Link, INT SlotIdx, FVector& SlotLocation, FRotationMatrix& R, INT Direction )
{
	UBOOL	bResult = TRUE;

	ACoverSlotMarker* Marker = Link->GetSlotMarker( SlotIdx );
	FCoverSlot& Slot = Link->Slots(SlotIdx);
	
	Slot.SlipTarget.Empty();

	// Make sure scout has physics set up
	Scout->InitForPathing( NULL, NULL );
	
	// Get nav point collision
	FLOAT	NavRadius = 0.f;
	FLOAT	NavHeight = 0.f;
	Link->GetBoundingCylinder( NavRadius, NavHeight );
	NavRadius = Link->AlignDist;
	
	// Get scout collision
	FVector Size = Scout->GetSize( FName(TEXT("Human"),FNAME_Find) );
	FVector Extent = FVector(Size.X,Size.X,Size.Y);

	FLOAT	Adjust	= Extent.X / 3.f;
	FVector InitLoc = SlotLocation + 
						(-R.GetAxis(0) * ((Extent.X - NavRadius) + Adjust)) +
						( R.GetAxis(2) * (Extent.Z - NavHeight));

	// Move scout to start of cover slip
	if( !GWorld->FarMoveActor( Scout, InitLoc, FALSE, GIsGame ) )
	{
		// If failed to move scout, no slip allowed
		bResult = FALSE;
	}

	// Trace to make sure side is clear
	FVector EndTrace(0,0,0);
	//INT r,g,b;
	if( bResult )
	{
		EndTrace = Scout->Location + (Direction * R.GetAxis(1) * (Extent.X * 3.0f));
		// Can slip if line is clear and we can move scout to that location
		bResult = Scout->pointReachable( EndTrace ) &&
					GWorld->FarMoveActor( Scout, EndTrace, FALSE, GIsGame );

/*
		if(bResult)
		{
			r=b=0;
			g=255;
		}
		else
		{
			g=b=0;
			r=255;
		}	

		Scout->DrawDebugBox(Scout->Location,Extent,r,g,b,TRUE);
		Scout->DrawDebugBox(EndTrace,Extent,r,g,b,TRUE);
		Scout->DrawDebugLine(Scout->Location,EndTrace,r,g,b,TRUE);
*/
	}
	
	// Trace forward to make sure we can coverslip the full distance
	if( bResult )
	{
		EndTrace = Scout->Location + (R.GetAxis(0) * Link->SlipDist);
		// bump upwards slightly if there is collision at the destination
		FCheckResult Hit;
		if (!GWorld->SingleLineCheck(Hit,Scout,EndTrace,Scout->Location,TRACE_AllColliding))
		{
			EndTrace.Z += NavHeight;
		}

		// Can slip if line is clear and we can move scout to that location
		bResult = Scout->pointReachable( EndTrace );

/*
		if(bResult)
		{
			r=b=0;
			g=255;
		}
		else
		{
			g=b=0;
			r=255;
		}	

		Scout->DrawDebugBox(Scout->Location,Extent,r,g,b,TRUE);
		Scout->DrawDebugBox(EndTrace,Extent,r,g,b,TRUE);
		Scout->DrawDebugLine(Scout->Location,EndTrace,r,g,b,TRUE);
*/
	}

	if( bResult )
	{
		INT bCheckedFall = 0;
		INT bMustJump = 0;

		// Add some fudge room for numerical imprecision
		FLOAT SlipDist = Link->SlipDist * 0.9f;

		// Make sure there are no ledges in front of us (use longer value to give some fudge room)
		FVector Delta = Scout->CheckForLedges( R.GetAxis(0), R.GetAxis(0) * Link->SlipDist, FVector(0,0,-1), bCheckedFall, bMustJump );
		if( Delta.SizeSquared() < (SlipDist * SlipDist) )
		{
			bResult = FALSE;
		}
	}

	if( bResult && Marker != NULL )
	{
		EndTrace -= R.GetAxis(0) * (Link->SlipDist * 0.5f);

		ANavigationPoint* BestSlipTarget = NULL;
		FLOAT BestDist = 1000000.f;

		const FLOAT SearchRadius = 64.f;

		//debug
//		Marker->DrawDebugBox( EndTrace, FVector(5,5,5), 255, 0, 0, TRUE );
//		Marker->DrawDebugSphere( EndTrace, SearchRadius, 10, 255, 0, 0, TRUE );

		// For each nav near the end of the slip
		for( ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav; Nav = Nav->nextNavigationPoint )
		{
			if( Cast<ACoverSlotMarker>(Nav) == NULL )
			{
				continue;
			}

			// If farther away than our current best nav - SKIP
			FLOAT DistSq = (Nav->Location - EndTrace).SizeSquared();
			if( DistSq > (SearchRadius*SearchRadius) || DistSq >= BestDist )
			{
				continue;
			}

			// If nav is behind our slip start location - SKIP
			FVector MarkerToNav = (Nav->Location - SlotLocation).SafeNormal();
			FLOAT DotP = (MarkerToNav | R.GetAxis(0));
			if( DotP <= 0.f )
			{
				continue;
			}

			// If geometry blocks us from getting to the nav - SKIP
			FCheckResult Hit;
			if( !GWorld->SingleLineCheck( Hit, Marker, Nav->Location, EndTrace, TRACE_World|TRACE_StopAtAnyHit ) )
			{
				continue;
			}
			
			// We found a better slip target! WOOT!
			BestSlipTarget = Nav;
			BestDist = DistSq;
		}

		// If we have a target
		if( BestSlipTarget != NULL )
		{
			// Save it in the slip list
			INT SlipIdx = Slot.SlipTarget.AddZeroed();
			Slot.SlipTarget(SlipIdx).Actor		= BestSlipTarget;
			Slot.SlipTarget(SlipIdx).Direction	= Direction;
		}
	}

	return bResult;
}

static UBOOL CanSwatTurn( AScout* Scout, ACoverLink* SrcLink, INT SrcSlotIdx, FRotationMatrix& R, INT Direction )
{
	UBOOL bResult = FALSE;

	// INIT
	FCoverSlot& SrcSlot			= SrcLink->Slots(SrcSlotIdx);
	FVector		SrcSlotLocation = SrcLink->GetSlotLocation( SrcSlotIdx );
	FLOAT		BestTargetDist	= 9999.f;
	ACoverLink* BestTargetLink	= NULL;
	INT			BestSlotIdx		= -1;

	// Make sure scout has physics set up
	Scout->InitForPathing( NULL, NULL );
	
	// Get nav point collision
	FLOAT	NavRadius = 0.f;
	FLOAT	NavHeight = 0.f;
	SrcLink->GetBoundingCylinder( NavRadius, NavHeight );
	NavRadius = SrcLink->AlignDist;
	
	// Get scout collision
	FVector Size = Scout->GetSize( FName(TEXT("Human"),FNAME_Find) );
	FVector Extent = FVector(Size.X*1.25f,Size.X*1.25f,Size.Y);

	FVector InitLoc = SrcSlotLocation + 
						(-R.GetAxis(0) * (Extent.X*1.25f - NavRadius)) +
						( R.GetAxis(2) * (Extent.Z - NavHeight));

	FLOAT CoverDirDot	= 0.80f;
	FLOAT LateralDirDot	= 0.90f;

	AWorldInfo *Info = GWorld->GetWorldInfo();
	for( ACoverLink *Link = Info->CoverList; Link != NULL; Link = Link->NextCoverLink )
	{
		if(  Link != SrcLink &&
			!Link->bCircular &&
			 GWorld->FarMoveActor( Scout, InitLoc, FALSE, GIsGame ) ) // Need to reset scout position after each slot check
		{
			for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
			{
				FCoverSlot& Slot = Link->Slots(SlotIdx);
				// If checking for right turn and target is not a left turn OR If checking for left turn and target is not a right turn
				if( (Direction ==  1 && !Slot.bLeanLeft)  ||
					(Direction == -1 && !Slot.bLeanRight) )
				{
					continue;
				}

				// If distance is too far - SKIP
				FVector SlotLocation = Link->GetSlotLocation( SlotIdx );
				FLOAT Dist = (SlotLocation-SrcSlotLocation).Size();
				if( Dist > SrcLink->TurnDist )
				{
					continue;
				}

				// If not pointing in the same direction - SKIP
				FRotator SlotRotation = Link->GetSlotRotation( SlotIdx );
				FLOAT DotP = SlotRotation.Vector() | R.GetAxis(0);
				if( DotP < CoverDirDot )
				{
					continue;
				}

				// If not in the direction we are looking - SKIP
				FVector SrcToTarg = (SlotLocation - SrcSlotLocation).SafeNormal();
				DotP = SrcToTarg | (R.GetAxis(1) * Direction);
				if( DotP < LateralDirDot )
				{
					continue;
				}

				FRotationMatrix Rot(Link->GetSlotRotation(SlotIdx));
				// Adjust final location off the wall a bit
				FVector FinalLoc = SlotLocation + 
									(-Rot.GetAxis(0) * (Extent.X - NavRadius)) +
									( Rot.GetAxis(2) * (Extent.Z - NavHeight)) + 
									(-Rot.GetAxis(0) *  Extent.X);


				// Can turn if line is clear and we can move scout to that location
				UBOOL bSuccess = Scout->pointReachable( FinalLoc ) &&
						GWorld->FarMoveActor( Scout, FinalLoc, GIsGame );

				// If scout could move there and we are close enough
				if( bSuccess && 
					Dist < BestTargetDist )
				{
					// Store best info
					BestTargetDist = Dist;
					BestTargetLink = Link;
					BestSlotIdx	   = SlotIdx;
				}
			}
		}
	}

	// If successfully found a link
	if( BestTargetLink )
	{
		// Set item info
		FCoverReference CoverInfo;
		CoverInfo.Actor			= BestTargetLink;
		CoverInfo.Guid			= *BestTargetLink->GetGuid();
		CoverInfo.SlotIdx		= BestSlotIdx;
		CoverInfo.Direction		= Direction;

		// Add new item
		SrcLink->Slots(SrcSlotIdx).TurnTarget.AddItem( CoverInfo );

		// Report success
		bResult = TRUE;
	}

	return bResult;
}

static void FindOverlapSlots( AScout* Scout, ACoverLink* SrcLink, INT SrcSlotIdx )
{
	if( !Scout || !SrcLink )
	{
		return;
	}

	FCoverSlot& SrcSlot = SrcLink->Slots(SrcSlotIdx);
	FVector SrcSlotLocation = SrcLink->GetSlotLocation( SrcSlotIdx );
	FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));
	FLOAT	ChkRadiusSum = HumanSize.X + HumanSize.X;

	FLOAT SrcTopZ = SrcSlotLocation.Z + SrcLink->GetSlotHeight(SrcSlotIdx) * 0.5f;
	FLOAT SrcBotZ = SrcTopZ - SrcLink->GetSlotHeight(SrcSlotIdx);
	for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
	{
		ACoverLink* Link = Cast<ACoverLink>(Nav);
		if( Link )
		{
			for( INT SlotIdx = 0; SlotIdx < Link->Slots.Num(); SlotIdx++ )
			{
				// Skip overlaps with myself
				if( Link == SrcLink && SlotIdx == SrcSlotIdx )
				{
					continue;
				}

				FVector SlotLocation = Link->GetSlotLocation( SlotIdx );
				FLOAT TopZ = SlotLocation.Z + Link->GetSlotHeight(SlotIdx) * 0.5f;
				FLOAT BotZ = TopZ - Link->GetSlotHeight(SlotIdx);

				// Check lateral overlap
				FLOAT DistSq2D = (SlotLocation-SrcSlotLocation).SizeSquared2D();
				if( DistSq2D < ChkRadiusSum*ChkRadiusSum )
				{
					// Check vertical overlap
					if( (TopZ <= SrcTopZ && BotZ >= SrcBotZ) ||
						(TopZ >= SrcTopZ && BotZ <= SrcBotZ) ||
						(TopZ <= SrcTopZ && TopZ >= SrcBotZ) ||
						(BotZ <= SrcTopZ && BotZ >= SrcBotZ) )
					{
						INT Idx = SrcSlot.OverlapClaims.AddZeroed();
						SrcSlot.OverlapClaims(Idx).Actor	=  Link;
						SrcSlot.OverlapClaims(Idx).Guid		= *Link->GetGuid();
						SrcSlot.OverlapClaims(Idx).SlotIdx	=  SlotIdx;
					}
				}
			}
		}
	}
}

static UBOOL CanPopUp(ACoverLink *Link, INT SlotIdx)
{
	FCoverSlot &Slot = Link->Slots(SlotIdx);

	// early out if LD has disabled popup for this slot
	if( !Slot.bAllowPopup )
	{
		return FALSE;
	}

	FCheckResult CheckResult;
	if( Slot.CoverType == CT_MidLevel )
	{
		// trace up to see if our body fits
		FVector CheckLocation = Link->GetSlotLocation( SlotIdx );
		FVector ViewPt = Link->GetSlotViewPoint( SlotIdx, Slot.CoverType, CA_PopUp );
		UBOOL bBodyFits = GWorld->SingleLineCheck( CheckResult, 
												Link,
												ViewPt,
												CheckLocation,
												TRACE_World,
												FVector(1.f));

		// trace forward to see if we would be looking at a wall
		FVector Forward = Link->GetSlotRotation(SlotIdx).Vector(); 		

		UBOOL bForwardClear = GWorld->SingleLineCheck( CheckResult, 
												Link,
												ViewPt + (Forward * 48.0f),
												ViewPt,
												TRACE_AllBlocking | TRACE_ComplexCollision );

		return (bBodyFits && bForwardClear);
	}

	return FALSE;
}

void ACoverLink::BuildSlotInfo(INT SlotIdx)
{
	check(SlotIdx >= 0 && SlotIdx < Slots.Num());
	FCoverSlot &Slot = Slots(SlotIdx);

	// Pop up
	Slot.bCanPopUp = Slot.bForceCanPopUp || CanPopUp(this, SlotIdx);

	// Mantle over mid or std cover.
	Slot.bCanMantle = Slot.bCanPopUp && Slot.bAllowMantle;

	// Mantle up mid or std cover
	Slot.bCanClimbUp = Slot.bCanPopUp && Slot.bAllowClimbUp;

	// Set swat turn flag if can lean left/right
	Slot.bCanSwatTurn_Left	= !bCircular && Slot.bAllowSwatTurn && Slot.bLeanLeft  && (SlotIdx==0);
	Slot.bCanSwatTurn_Right = !bCircular && Slot.bAllowSwatTurn && Slot.bLeanRight && (SlotIdx==Slots.Num()-1);

	AScout *Scout = FPathBuilder::GetScout();

	FVector	 SlotLocation = GetSlotLocation( SlotIdx );
	FRotator SlotRotation = GetSlotRotation( SlotIdx );
	FRotationMatrix R(SlotRotation);

	// only need to check this in the editor
	if (!GIsGame)
	{
		//// BEGIN OVERLAP SLOTS ////
		Slot.OverlapClaims.Empty();
		FindOverlapSlots( Scout, this, SlotIdx );
		//// END OVERLAP SLOTS ////
	}

	//// BEGIN MANTLE ////
	if( Slot.bCanClimbUp )
	{
		Slot.bCanClimbUp = CanClimbUp( Scout, this, SlotIdx );
		if( Slot.bCanClimbUp )
		{
			Slot.bCanMantle = FALSE;
		}
	}
	if( Slot.bCanMantle )
	{
		Slot.MantleTarget.Actor		= NULL;
		Slot.MantleTarget.Guid		= FGuid(0,0,0,0);
		Slot.MantleTarget.SlotIdx	= -1;

		Slot.bCanMantle = CanMantle(Scout, this, SlotIdx);
	}
	
	if( !Slot.bCanClimbUp && !Slot.bCanMantle )
	{
		Slot.MantleTarget.Actor		= NULL;
		Slot.MantleTarget.Guid		= FGuid(0,0,0,0);
		Slot.MantleTarget.SlotIdx	= -1;
	}

	//// END MANTLE ////

	//// BEGIN COVERSLIP ////
	// Set cover slip flag if standing cover and can lean left or right
	Slot.bCanCoverSlip_Left	 = Slot.bAllowCoverSlip &&
		Slot.bLeanLeft &&
		IsLeftEdgeSlot(SlotIdx,TRUE) &&
		(Slot.CoverType == CT_Standing || (Slot.CoverType == CT_MidLevel));

	Slot.bCanCoverSlip_Right = Slot.bAllowCoverSlip &&
		Slot.bLeanRight &&
		IsRightEdgeSlot(SlotIdx,TRUE) &&
		(Slot.CoverType == CT_Standing || (Slot.CoverType == CT_MidLevel));

	// Empty list of targets
	Slot.SlipTarget.Empty();
	// Verify cover slip left
	if( Slot.bCanCoverSlip_Left )
	{
		Slot.bCanCoverSlip_Left = Slot.bForceCanCoverSlip_Left || CanCoverSlip( Scout, this, SlotIdx, SlotLocation, R, -1 );
	}
	// Verify cover slip right
	if( Slot.bCanCoverSlip_Right )
	{
		Slot.bCanCoverSlip_Right = Slot.bForceCanCoverSlip_Right || CanCoverSlip( Scout, this, SlotIdx, SlotLocation, R, 1 );
	}
	//// END COVERSLIP ////

	//// BEGIN SWAT TURN ////
	// Empty list of targets
	Slot.TurnTarget.Empty();
	// Verify swat turn left
	if( Slot.bCanSwatTurn_Left )
	{
		Slot.bCanSwatTurn_Left = CanSwatTurn( Scout, this, SlotIdx, R, -1 );
	}
	// Verify swat turn right
	if( Slot.bCanSwatTurn_Right )
	{
		Slot.bCanSwatTurn_Right = CanSwatTurn( Scout, this, SlotIdx, R, 1 );
	}
	//// END SWAT TURN ////

	// Update paths for markers
	ACoverSlotMarker* Marker = Slots(SlotIdx).SlotMarker;
	if( Marker != NULL && Scout != NULL )
	{
		Marker->AddForcedSpecs( Scout );
		Marker->ForceUpdateComponents(FALSE,FALSE);
	}

	// if in game be sure to delete the scout
	if (GIsGame && Scout != NULL)
	{
		Scout = NULL;
		FPathBuilder::DestroyScout();
	}
}


void ACoverLink::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	if( appStricmp( *PropertyThatChanged->GetName(), TEXT("ForceCoverType") ) == 0 )
	{
		for( INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++ )
		{
			AutoAdjustSlot( SlotIdx, TRUE );
		}
	}

	if( appStricmp( *PropertyThatChanged->GetName(), TEXT("bBlocked") )		 == 0 || 
		appStricmp( *PropertyThatChanged->GetName(), TEXT("CollisionType") ) == 0 )
	{
		GWorld->GetWorldInfo()->bPathsRebuilt = FALSE;
		bPathsChanged = TRUE;
	}
}


/**
 * Attempts to orient the specified slot to the nearest wall, and determine the height of the slot
 * based upon the facing wall.
 */
UBOOL ACoverLink::AutoAdjustSlot(INT SlotIdx, UBOOL bOnlyCheckLeans)
{
	if( SlotIdx < 0 || SlotIdx >= Slots.Num() )
	{
		return FALSE;
	}

	UBOOL bResult = FALSE;
	FCoverSlot &Slot = Slots(SlotIdx);
	FVector SlotLocation = GetSlotLocation(SlotIdx);
	BYTE OldCoverType = Slot.CoverType;
	FCheckResult CheckResult;
	FRotationMatrix RotMatrix(Rotation);
	FVector CylExtent = GetCylinderExtent();

	// determine what the height of this node is
	// by checking the normal height of the node
	if( !GIsGame || !bOnlyCheckLeans )
	{
		// first move this slot down to ground level
		// @laurent - disabled for second pass, because for height, we need to take into account width of cylinder (for slopes). But that is unsafe for first pass.
		// So we let positioning being entirely done during first pass.
		if( !Slot.bForceNoGroundAdjust && !bOnlyCheckLeans )
		{
			if( !GWorld->SingleLineCheck(CheckResult, this, SlotLocation-FVector(0.f,0.f,4.f*AlignDist), SlotLocation, TRACE_World, FVector(1.f,1.f,1.f)))
			{
				//DrawDebugCoordinateSystem(CheckResult.Location,GetSlotRotation(SlotIdx),50.f,TRUE);
				Slot.LocationOffset = RotMatrix.InverseTransformFVectorNoScale(CheckResult.Location + FVector(0.f,0.f,CylExtent.Z) - Location);
				SlotLocation = GetSlotLocation(SlotIdx);
			}
		}

		if( Slot.ForceCoverType == CT_None )
		{
			// Check for mid to standing
			FVector CheckDir = (Rotation + Slot.RotationOffset).Vector();
			FVector CheckLoc = SlotLocation;
			CheckLoc.Z += -CylExtent.Z + MidHeight;
			FLOAT CheckDist = StandHeight - MidHeight;
			UBOOL bHit = TRUE;
			while (CheckDist > 0 &&
				bHit)
			{
				bHit = !GWorld->SingleLineCheck(CheckResult,
					this,
					CheckLoc + CheckDir * (AlignDist*4.f),
					CheckLoc,
					TRACE_AllBlocking | TRACE_ComplexCollision);
				CheckDist -= 16.f;
				CheckLoc.Z += 16.f;
			}
			// if we found a gap, assume mid level cover
			if (!bHit)
			{
				// if this is in game
				if (GIsGame)
				{
					// check for the cover being removed entirely
					if ( GWorld->SingleLineCheck(CheckResult,
						this,
						SlotLocation + CheckDir * (AlignDist*4.f),
						SlotLocation,
						TRACE_World,
						FVector(1.f)))
					{
						// no cover in front of this slot, so disable
						Slot.bEnabled = FALSE;
					}
				}
				Slot.CoverType = CT_MidLevel;
			}
			else
			{
				// otherwise it's full standing cover
				//debugf(TEXT("link %s slot %d hit %s for standing"),*GetName(),SlotIdx,*CheckResult.Actor->GetName());
				Slot.CoverType = CT_Standing;
			}
		}
		else
		{
			Slot.CoverType = Slot.ForceCoverType;
		}

		// if we changed the cover type indicate that in the return value
		bResult = (OldCoverType != Slot.CoverType);
	}

	if( bAutoAdjust )
	{
		if( !bOnlyCheckLeans )
		{
			// orient to the nearest feature by tracing
			// in various directions
			{
				FRotator CheckRotation = Rotation + Slot.RotationOffset;
				FCheckResult BestCheckResult;
				FLOAT Angle = 0.f;
				FVector CheckLocation = GetSlotLocation(SlotIdx);
				CheckLocation.Z -= CylExtent.Z * 0.5f;
				FLOAT CheckDist = AlignDist * 4.f;
				const INT AngleCheckCount = 128;
				Slot.bFailedToFindSurface = FALSE;	
				//FlushPersistentDebugLines();
				for (INT Idx = 0; Idx < AngleCheckCount; Idx++)
				{
					Angle += 65536.f/AngleCheckCount * Idx;
					CheckRotation.Yaw += appTrunc(Angle);
					FVector EndLocation = CheckLocation + (CheckRotation.Vector() * CheckDist);
					if (!GWorld->SingleLineCheck(CheckResult,
												 this,
												 EndLocation,
												 CheckLocation,
												 TRACE_World,
												 FVector(1.f)))
					{
						//DrawDebugLine(CheckLocation,CheckResult.Location,0,255,0, TRUE);
						//DrawDebugLine(CheckResult.Location,CheckResult.Location + CheckResult.Normal * 8.f,0,255,0, TRUE);
						FLOAT Rating = 1.f - ((CheckResult.Location - SlotLocation).Size2D()/CheckDist);
						// scale by the current rotation to allow ld's a bit of control
						Rating += -0.5f * (CheckResult.Normal | (GetSlotRotation(SlotIdx).Vector()));
						// favor blocking volume hits more than regular geometery
						if (CheckResult.Actor != NULL &&
							CheckResult.Actor->IsA(ABlockingVolume::StaticClass()))
						{
							Rating *= 1.25f;
						}
						// compare against our best Check, if not set or
						// this Check resulted in a closer hit
						if (Rating > 0.f &&
							(BestCheckResult.Actor == NULL ||
							BestCheckResult.Time < Rating))
						{
							BestCheckResult = CheckResult;
							BestCheckResult.Time = Rating;
						}
					}
				}
				if (BestCheckResult.Actor != NULL)
				{
					// set the rotation based on the hit normal
					FRotator NewRotation = Rotation + Slot.RotationOffset;
					NewRotation.Yaw = (BestCheckResult.Normal * -1).Rotation().Yaw;
					FVector X, Y, Z;
					// attempt to do 2 parallel traces along the new rotation to generate an average surface normal
					FRotationMatrix(NewRotation).GetAxes(X,Y,Z);
					FCheckResult CheckResultA, CheckResultB;
					if (!GWorld->SingleLineCheck(CheckResultA,
												 this,
												 SlotLocation + (X * (AlignDist*4.f) + Y * 31.f),
												 SlotLocation + Y * 31.f,
												 TRACE_World,
												 FVector(1.f)) &&
						!GWorld->SingleLineCheck(CheckResultB,
												 this,
												 SlotLocation + (X * (AlignDist*4.f) - Y * 31.f),
												 SlotLocation - Y * 31.f,
												 TRACE_World,
												 FVector(1.f)))
					{
						/*
						DrawDebugLine(SlotLocation + (X * (AlignDist*4.f) - Y * 31.f),SlotLocation - Y * 31.f,0,0,255,1);
						DrawDebugLine(CheckResultA.Location,CheckResultA.Location + CheckResultA.Normal * 256.f,0,128,128,1);
						DrawDebugLine(SlotLocation + (X * (AlignDist*4.f) + Y * 31.f),SlotLocation + Y * 31.f,0,0,255,1);
						DrawDebugLine(CheckResultB.Location,CheckResultB.Location + CheckResultB.Normal * 256.f,0,128,128,1);
						*/
						//NewRotation.Yaw = (((CheckResultA.Normal + CheckResultB.Normal)/2.f) * -1).Rotation().Yaw;
						NewRotation.Yaw = ((CheckResultA.Normal + CheckResultB.Normal) * -1).Rotation().Yaw;
					}

					Slot.RotationOffset = NewRotation - Rotation;
					FVector NewDirection = NewRotation.Vector();

					// Use a line trace to get as close to the wall as we can get					
					if( !GWorld->SingleLineCheck(CheckResult,
												 this,
												 SlotLocation + (NewDirection * (AlignDist*4.f)),
												 SlotLocation,
												 TRACE_World,
												 FVector(1.f)))
					{
						Slot.LocationOffset = RotMatrix.InverseTransformFVectorNoScale((CheckResult.Location + NewDirection * -AlignDist) - Location);
						SlotLocation = GetSlotLocation(SlotIdx);
					}

					// Place a fake scout at the location and move the scout out until it fits
					FVector ScoutExtent(AlignDist, AlignDist, CylinderComponent->CollisionHeight/2.f);
					FVector ScoutCheckLocation = SlotLocation + FVector(0,0,8);
					INT AdjustCounter = 16;
					UBOOL bFits = FALSE;
					while( !bFits && AdjustCounter-- > 0 )
					{
						FVector OriginalScoutCheckLocation = ScoutCheckLocation;

						//DrawDebugBox(ScoutCheckLocation, ScoutExtent, 0, 255, 0, TRUE);

						// Try to fit scout in check spot
						bFits = GWorld->FindSpot(ScoutExtent, ScoutCheckLocation, FALSE);

						//DrawDebugBox(ScoutCheckLocation, ScoutExtent, 255, 0, 0, TRUE);

						// If couldn't fit
						if( !bFits )
						{
							// Move check spot away from the wall
							ScoutCheckLocation = OriginalScoutCheckLocation - NewDirection * 4.f;
						}
					}

					// Try to trace back to original location.
					// The reason for this is when the slot rotation is not axis aligned, FindSpot() will push it away, 
					// but not away along cover normal, but along world X or Y. So that effectively makes slots slide along cover.
					// So we push the original location away from the wall from the same distance as the newly found location, and we try to move the slot back there by tracing.
					if( !GWorld->SingleLineCheck(CheckResult,
												 this,
												 ScoutCheckLocation + (NewDirection * (AlignDist*4.f)),
												 ScoutCheckLocation,
												 TRACE_World,
												 FVector(1.f)) )
					{
						FLOAT DistanceToCoverWall = (ScoutCheckLocation - CheckResult.Location).Size();
						FVector AdjustedSourceLocation = SlotLocation + NewDirection * (AlignDist - DistanceToCoverWall);


						if( !GWorld->SingleLineCheck(CheckResult,
													 this,
													 AdjustedSourceLocation,
													 ScoutCheckLocation,
													 TRACE_World,
													 FVector(1.f)) )
						{
							ScoutCheckLocation = CheckResult.Location; // MT->WTF?
						}
						else
						{
							ScoutCheckLocation = AdjustedSourceLocation;
						}
					}

					// Trace Down to Place Scout on the floor, taking into account its width.
					// Positions slot correctly against cover when on slopes. If we use just a single trace, then slot will sink in floor and be too low or high against the wall.
					// Downside is that collision cylinder is AABB, so it pushes slot a bit further from cover if not axis aligned.
					if( !GWorld->SingleLineCheck(CheckResult, this, ScoutCheckLocation-FVector(0.f,0.f,4.f*AlignDist), ScoutCheckLocation, TRACE_World, FVector(AlignDist, AlignDist, 1.f)) )
					{
						// Catch cases where we couldn't fit the Scout
						if( !bFits || ScoutCheckLocation.Z == CheckResult.Location.Z )
						{
							debugf(TEXT("FAILED TO PLACE SCOUT! AdjustCounter:%d"), AdjustCounter);

							// Resort to using a single line trace to put the slot on the ground.
							if( !GWorld->SingleLineCheck(CheckResult, this, ScoutCheckLocation-FVector(0.f,0.f,4.f*AlignDist), ScoutCheckLocation, TRACE_World, FVector(1.f)) )
							{
								ScoutCheckLocation.Z = CheckResult.Location.Z + CylinderComponent->CollisionHeight;
							}
						}
						else
						{
							ScoutCheckLocation.Z = CheckResult.Location.Z + CylinderComponent->CollisionHeight;
						}
					}

					Slot.LocationOffset = RotMatrix.InverseTransformFVectorNoScale(ScoutCheckLocation - Location);
					SlotLocation = GetSlotLocation(SlotIdx);

					// Finally place slot against cover at AlignDist distance with no consideration for AABB.
					// So slot remains at a fixed distance from wall.
					if( !GWorld->SingleLineCheck(	CheckResult, 
													this, 
													SlotLocation + (NewDirection * (AlignDist*4.f)),
													SlotLocation,
													TRACE_World,
													FVector(1.f)))
					{
						SlotLocation = CheckResult.Location - NewDirection * AlignDist;
						Slot.LocationOffset = RotMatrix.InverseTransformFVectorNoScale(SlotLocation - Location);
						SlotLocation = GetSlotLocation(SlotIdx);
					}
				}
				else
				{
					Slot.bFailedToFindSurface = TRUE;	
				}
				// limit the pitch/roll
				Slot.RotationOffset.Pitch = 0;
				Slot.RotationOffset.Roll = 0;
			}
		}

		// if dealing with circular cover
		if (bCircular &&
			Slots.Num() >= 2)
		{
			// calculate origin/radius
			FVector A = GetSlotLocation(0), B = GetSlotLocation(1);
			CircularOrigin = (A + B)/2.f;
			CircularRadius = (A - B).Size()/2.f;
			// force rotation to the origin
			Slot.RotationOffset = (CircularOrigin - GetSlotLocation(SlotIdx)).Rotation() - Rotation;
			// and enable leans for both directions
			Slot.bLeanLeft  = TRUE;
			Slot.bLeanRight = TRUE;
		}
		else
		{
			// update the lean left/right flags
			if (Slot.LeanTraceDist <= 0.f)
			{
				Slot.LeanTraceDist = 64.f;
			}
			Slot.bLeanLeft  = FALSE;
			Slot.bLeanRight = FALSE;
			UBOOL bHit;

			// Get cover axes
			FVector X, Y, Z;
			FRotationMatrix(GetSlotRotation(SlotIdx)).GetAxes( X, Y, Z );

			// Get start location for slot traces
			FVector CheckLocation = GetSlotLocation(SlotIdx);
			CheckLocation.Z += GetSlotHeight(SlotIdx) * 0.375f;
			
			if (IsLeftEdgeSlot(SlotIdx,FALSE))
			{
				// verify that there is no wall to the left
				FVector ViewPt = GetSlotViewPoint( SlotIdx, CT_None, CA_LeanLeft );
				bHit = !GWorld->SingleLineCheck(CheckResult,
												this,
												ViewPt,
												CheckLocation,
												TRACE_World,
												FVector(1.f));
				if (!bHit)
				{
					// verify that we are able to fire forward from the lean location
					bHit = !GWorld->SingleLineCheck( CheckResult,
													 this,
													 ViewPt + X * Slot.LeanTraceDist,
													 ViewPt,
													 TRACE_AllBlocking | TRACE_ComplexCollision );
					if( !bHit )
					{
						Slot.bLeanLeft = TRUE;
					}
				}
			}
			if (IsRightEdgeSlot(SlotIdx,FALSE))
			{
				FVector ViewPt = GetSlotViewPoint( SlotIdx, CT_None, CA_LeanRight );
				bHit = !GWorld->SingleLineCheck(CheckResult,
												this,
												ViewPt,
												CheckLocation,
												TRACE_World,
												FVector(1.f));
				if (!bHit)
				{
					// verify that we are able to fire forward from the lean location
					bHit = !GWorld->SingleLineCheck( CheckResult,
													 this,
													 ViewPt + X * Slot.LeanTraceDist,
													 ViewPt,
													 TRACE_AllBlocking | TRACE_ComplexCollision);
					if( !bHit )
					{
						Slot.bLeanRight = TRUE;
					}
				}
			}
		}

		// if in game, and the slot is enabled
		if (GIsGame && Slot.bEnabled)
		{
			// figure out the slip/mantle/swat stuff as well
			BuildSlotInfo(SlotIdx);
		}
	}

	if (GIsGame)
	{
		FPathBuilder::DestroyScout();
	}

	return bResult;
}

UBOOL ACoverLink::IsEnabled()
{
	if( bDisabled )
	{
		return FALSE;
	}

	for( INT Idx = 0; Idx < Slots.Num(); Idx++ )
	{
		if( Slots(Idx).bEnabled )
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL ACoverLink::IsEdgeSlot(INT SlotIdx, UBOOL bIgnoreLeans)
{
	// if not circular cover, and
	// if start of list, or left slot is disabled, or end of list, or right slot is disabled
	return (!bLooped && !bCircular && (IsLeftEdgeSlot(SlotIdx,bIgnoreLeans) || IsRightEdgeSlot(SlotIdx,bIgnoreLeans)));
}

UBOOL ACoverLink::IsLeftEdgeSlot(INT SlotIdx, UBOOL bIgnoreLeans)
{
	// if not circular, and
	// is start of list or the left slot is disabled
	return (!bLooped && !bCircular && 
		SlotIdx < Slots.Num() && 
		(SlotIdx <= 0 || !Slots(SlotIdx-1).bEnabled || (!bIgnoreLeans && Slots(SlotIdx-1).CoverType > Slots(SlotIdx).CoverType)));
}

UBOOL ACoverLink::IsRightEdgeSlot(INT SlotIdx, UBOOL bIgnoreLeans)
{
	// if not circular, and
	// is end of list, or right slot is disabled
	return (!bLooped && !bCircular && 
		(SlotIdx == Slots.Num()-1 || SlotIdx >= Slots.Num() || !Slots(SlotIdx+1).bEnabled || (!bIgnoreLeans && Slots(SlotIdx+1).CoverType > Slots(SlotIdx).CoverType)));
}

INT ACoverLink::GetSlotIdxToLeft( INT SlotIdx, INT Cnt )
{
	INT NextSlotIdx = SlotIdx - Cnt;
	if( bLooped )
	{
		while( NextSlotIdx < 0 )
		{
			NextSlotIdx += Slots.Num();
		}		
	}
	return (NextSlotIdx >= 0 && NextSlotIdx < Slots.Num()) ? NextSlotIdx : -1;
}

INT ACoverLink::GetSlotIdxToRight( INT SlotIdx, INT Cnt )
{
	INT NextSlotIdx = SlotIdx + Cnt;
	if( bLooped )
	{
		while( NextSlotIdx >= Slots.Num() )
		{
			NextSlotIdx -= Slots.Num();
		}		
	}
	return (NextSlotIdx >= 0 && NextSlotIdx < Slots.Num()) ? NextSlotIdx : -1;
}

void ACoverLink::ClearExposedFireLinks()
{
	// clear out exposed fire links
	for( INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++ )
	{
		FCoverSlot &Slot = Slots(SlotIdx);	
		Slot.ExposedFireLinks.Empty();
	}
}

void ACoverLink::BuildFireLinks( AScout* Scout )
{
	// For every slot
	for( INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++ )
	{
		FCoverSlot &Slot = Slots(SlotIdx);		

		// Clear previous links
		Slot.FireLinks.Empty();
		Slot.DangerLinks.Empty();
		Slot.Actions.Empty();

		ACoverSlotMarker* Marker = GetSlotMarker( SlotIdx );
		if( Marker == NULL )
		{
			continue;
		}

		// If we can't perform any attacks from this slot
		FFireLinkInfo Info( this, SlotIdx );
		if( Info.Actions.Num() == 0 )
		{
			// Skip it!
			continue;
		}

		// For every other slot marker
		for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
		{
			ACoverSlotMarker* TestMarker = Cast<ACoverSlotMarker>(Nav);
			if( TestMarker == NULL || TestMarker == Marker || TestMarker->OwningSlot.Link == NULL)
			{
				continue;
			}

			GetFireActions( Info, TestMarker );
		}
	}
}

UBOOL ACoverLink::GetFireActions( FFireLinkInfo& SrcInfo, ACoverSlotMarker* TestMarker, UBOOL bFill )
{
	if( TestMarker == NULL )
	{
		return FALSE;
	}

	//debug
	UBOOL bDebugLines = FALSE;
/*	if( SrcInfo.Slot->bSelected ) 
	{
		FlushPersistentDebugLines();
		bDebugLines = TRUE; 
	} */

	// Setup info for test marker
	FFireLinkInfo DestInfo( TestMarker->OwningSlot.Link, TestMarker->OwningSlot.SlotIdx );
	DestInfo.Actions.AddItem( CA_Default ); // Add firing at enemy w/o a cover action

	// If test marker is too far away
	FLOAT SrcToDestDist = (DestInfo.SlotLocation-SrcInfo.SlotLocation).Size();
	if( SrcToDestDist > MaxFireLinkDist )
	{
		// Can't fire at it
		return FALSE;
	}

	TArray<FFireLinkItem>	ValidItems;
	for( INT SrcTypeIdx = 0; SrcTypeIdx < SrcInfo.Types.Num(); SrcTypeIdx++ )
	{
		BYTE SrcType = SrcInfo.Types(SrcTypeIdx);
		for( INT SrcActionIdx = 0; SrcActionIdx < SrcInfo.Actions.Num(); SrcActionIdx++ )
		{
			BYTE SrcAction = SrcInfo.Actions(SrcActionIdx);
			SrcInfo.Slot->Actions.AddUniqueItem( SrcAction );

			for( INT DestTypeIdx = 0; DestTypeIdx < DestInfo.Types.Num(); DestTypeIdx++ )
			{
				BYTE DestType = DestInfo.Types(DestTypeIdx);
				for( INT DestActionIdx = 0; DestActionIdx < DestInfo.Actions.Num(); DestActionIdx++ )
				{
					BYTE DestAction = DestInfo.Actions(DestActionIdx);
					
					FVector SrcViewPt  = SrcInfo.Link->GetSlotViewPoint(  SrcInfo.SlotIdx,  SrcType, SrcAction );
					FVector DestViewPt = DestInfo.Link->GetSlotViewPoint( DestInfo.SlotIdx, DestType, DestAction );
					if( CanFireLinkHit( SrcViewPt, DestViewPt, bDebugLines ) )
					{
						INT ValidIdx = ValidItems.AddZeroed();
						FFireLinkItem& Item = ValidItems(ValidIdx);
						Item.SrcType	= SrcType;
						Item.SrcAction	= SrcAction;
						Item.DestType	= DestType;
						Item.DestAction = DestAction;
					}					
				}
			}
		}
	}

	if( ValidItems.Num() == 0 )
	{
		return FALSE;
	}

	UBOOL bResult = FALSE;

	FVector SrcToDest		= DestInfo.SlotLocation - SrcInfo.SlotLocation;
	FVector SrcToDestDir	= SrcToDest.SafeNormal();
	FLOAT	FireAngleDot	= SrcInfo.SlotRotation.Vector() | SrcToDestDir;
	FLOAT	FireAngleDist	= SrcInfo.SlotRotation.Vector() | SrcToDest;
	
	if( FireAngleDot  >= MINFIRELINKANGLE && 
		FireAngleDist >= 128.f )
	{
		bResult = TRUE;

		if( bFill )
		{
			INT LinkIdx	  = SrcInfo.Slot->FireLinks.AddZeroed();
			FFireLink& FL = SrcInfo.Slot->FireLinks(LinkIdx);
			FL.TargetMarker				= TestMarker;
			FL.LastTargetMarkerLocation	= TestMarker->Location;
			FL.LastSrcMarkerLocation	= SrcInfo.SlotLocation;
			FL.bFallbackLink			= (SrcInfo.SlotRotation.Vector() | SrcToDestDir) < DESIREDMINFIRELINKANGLE;
			for( INT ValidIdx = 0; ValidIdx < ValidItems.Num(); ValidIdx++ )
			{
				FL.Items.AddItem( ValidItems(ValidIdx) );
			}
			if( SrcInfo.out_FireLinkIdx != NULL )
			{
				*SrcInfo.out_FireLinkIdx = LinkIdx;
			}			

			// tell the node we can fire on, that it's exposed to us
			if(TestMarker != NULL && TestMarker->OwningSlot.Link != NULL)
			{
				FCoverSlot& Slot = TestMarker->OwningSlot.Link->Slots(TestMarker->OwningSlot.SlotIdx);					
				FLOAT ExposedScale = 0.f;
				ACoverSlotMarker* SrcMarker = SrcInfo.Slot->SlotMarker;
				if(GetExposedInfo(SrcMarker,TestMarker,ExposedScale) && ExposedScale > 0.f)
				{
					INT ExpIdx = Slot.ExposedFireLinks.AddZeroed();
					BYTE Scale = (BYTE)(ExposedScale/1.0f * 255);
					Slot.ExposedFireLinks(ExpIdx).TargetMarker.Actor = SrcMarker;
					Slot.ExposedFireLinks(ExpIdx).TargetMarker.Guid = *SrcMarker->GetGuid();
					Slot.ExposedFireLinks(ExpIdx).ExposedScale=Scale;
				}

			}
		}
	}
	
	return bResult;
}

void ACoverLink::BuildOtherLinks( AScout* Scout )
{
	// For every slot
	for( INT SlotIdx = 0; SlotIdx < Slots.Num(); SlotIdx++ )
	{
		FCoverSlot &Slot = Slots(SlotIdx);

		ACoverSlotMarker* Marker = GetSlotMarker(SlotIdx);
		if( Marker == NULL )
		{
			continue;
		}
		FVector  SlotLocation = GetSlotLocation( SlotIdx );
		FRotator SlotRotation = GetSlotRotation( SlotIdx );

		for( ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint )
		{
			if( Nav == Marker )
				continue;

			FVector SlotToNav	= Nav->Location - SlotLocation;
			FLOAT	Dist		= SlotToNav.Size();
			FLOAT	DotP		= SlotToNav.SafeNormal() | SlotRotation.Vector();

			// If nav is in front of the cover
			if( DotP >= 0 && 
				Dist <= UCONST_COVERLINK_DangerDist  )
			{
				UBOOL bHit = FALSE;
				for( INT ActionIdx = 0; ActionIdx < Slot.Actions.Num() && !bHit; ActionIdx++ )
				{
					FVector ViewPt = GetSlotViewPoint( SlotIdx, CT_None, Slot.Actions(ActionIdx) );
					if( CanFireLinkHit( ViewPt, Nav->Location + FVector(0,0,20), FALSE ) )
					{
						FLOAT	ScaleByDist		= (Dist / UCONST_COVERLINK_DangerDist);
						INT		DangerCost		= appTrunc( UCONST_COVERLINK_DangerDist * (1.f - ScaleByDist) );

						INT DangerIdx = Slot.DangerLinks.AddZeroed();
						Slot.DangerLinks(DangerIdx).DangerNav.Actor =  Nav;
						Slot.DangerLinks(DangerIdx).DangerNav.Guid  = *Nav->GetGuid();
						Slot.DangerLinks(DangerIdx).DangerCost	    =  DangerCost;
						bHit = TRUE;
					}
				}
			}
		}
	}
}

UBOOL ACoverLink::GetExposedInfo( ACoverSlotMarker* SrcMarker, ACoverSlotMarker* DestMarker, FLOAT& out_ExposedScale )
{
	if( SrcMarker == NULL || DestMarker == NULL )
		return FALSE;
		
	// Check to see if are exposed by check cover
	FVector VectToChk = (SrcMarker->GetSlotLocation() - DestMarker->GetSlotLocation() );
	FLOAT	DistSq = VectToChk.SizeSquared();
	VectToChk.Normalize();

	if( DistSq > MaxFireLinkDist * MaxFireLinkDist )
	{
		return FALSE;
	}

	FRotationMatrix RotMatrix(DestMarker->GetSlotRotation());
	FVector X, Y, Z;
	RotMatrix.GetAxes( X, Y, Z );

	// Determine the angle we want to use
	// If slot is too far to the side of an edge slot, increase the valid exposure angle
	FLOAT		TestDot	  = UCONST_COVERLINK_ExposureDot;
	FLOAT		YDot = (Y | VectToChk);
	FCoverSlot& DestSlot  = DestMarker->OwningSlot.Link->Slots(DestMarker->OwningSlot.SlotIdx);
	if( (DestSlot.bLeanLeft  && YDot < -UCONST_COVERLINK_EdgeCheckDot) ||
		(DestSlot.bLeanRight && YDot >  UCONST_COVERLINK_EdgeCheckDot) )
	{
		TestDot = UCONST_COVERLINK_EdgeExposureDot;
	}

	FLOAT XDot = (X | VectToChk);
	if( XDot <= TestDot )
	{
		// If threat is still in front of the marker
		if( XDot > 0.f )
		{
			// Scale exposure danger
			out_ExposedScale = 1.f - (XDot / TestDot);
		}
		else
		{
			// Otherwise, threat has a great flank = max exposure
			out_ExposedScale = 1.f;
		}
		
		// If threat is farther than half the max firelink distance
		FLOAT Dist	  = (DestMarker->GetSlotLocation() - SrcMarker->GetSlotLocation()).Size();
		FLOAT HalfMax = MaxFireLinkDist/2.f;
		if( Dist > HalfMax )
		{
			// Scale exposure down by distance
			out_ExposedScale *= 1.f - ((Dist - HalfMax) / HalfMax);
		}

		return TRUE;
	}

	return FALSE;
}


UBOOL ACoverLink::IsFireLinkValid( INT SlotIdx, FFireLink* FireLink ) 
{
	if( FireLink == NULL )
	{
		return FALSE;
	}

	ACoverSlotMarker* TargetMarker = Cast<ACoverSlotMarker>(*FireLink->TargetMarker);
	if( TargetMarker == NULL )
	{
		return FALSE;
	}

	UBOOL bTargetDynamic = TargetMarker->OwningSlot.Link->bDynamicCover;
	// If both src link and target link are static cover - link is always valid
	if( !bDynamicCover && !bTargetDynamic )
	{
		return TRUE;
	}

	const FLOAT Thresh = InvalidateDistance * InvalidateDistance;
	
	// If target is dynamic cover
	if( bTargetDynamic )
	{
		// Get distance from last valid target marker location
		FLOAT DistSq = (FireLink->LastTargetMarkerLocation-TargetMarker->Location).SizeSquared();
		// Invalid if marker is outside acceptable range
		if( DistSq > Thresh )
		{
			return FALSE;
		}
	}

	// If source is dynamic cover
	if( bDynamicCover )
	{
		ACoverSlotMarker* SrcMarker = GetSlotMarker( SlotIdx );
		if( SrcMarker != NULL )
		{
			// Get distance from  last valid src marker location
			FLOAT DistSq = (FireLink->LastSrcMarkerLocation - SrcMarker->Location).SizeSquared();
			// Invalid if marker is outside acceptable range
			if( DistSq > Thresh )
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}

	// Everything is close enough to remain valid
	return TRUE;
}


/**
 * Searches for a fire link to the specified cover/slot and returns the cover actions.
 */
UBOOL ACoverLink::GetFireLinkTo( INT SlotIdx, FCoverInfo& ChkCover, BYTE ChkAction, BYTE ChkType, INT& out_FireLinkIdx, TArray<INT>& out_Items )
{
	UBOOL bFound  = FALSE;
	UBOOL bResult = FALSE;
	FCoverSlot& Slot = Slots(SlotIdx);
	FVector SrcSlotLocation = GetSlotLocation(SlotIdx);

	ACoverSlotMarker* TestMarker = ChkCover.Link->GetSlotMarker(ChkCover.SlotIdx);

	// If slot has rejected links
	if( Slot.RejectedFireLinks.Num() )
	{
		// Fail if already rejected
		for( INT Idx = 0; Idx < Slot.RejectedFireLinks.Num(); Idx++ )
		{
			FFireLink* RejectLink = &Slot.RejectedFireLinks(Idx);
			if( RejectLink->TargetMarker == TestMarker )
			{
				if( IsFireLinkValid( SlotIdx, RejectLink ) )
				{
					// Early exit
					return FALSE;
				}
				else
				{
#if !FINAL_RELEASE && !PS3
					//debug
					if( bDebug )
					{
						debugf(TEXT("%s ACoverLink_Dynamic::GetFireLinkTo Invalidate FireLinks"), *GetName() );
					}
#endif

					Slot.FireLinks.Empty();
					Slot.RejectedFireLinks.Empty();
				}
			}
		}
	}

	for( INT FireLinkIdx = 0; FireLinkIdx < Slot.FireLinks.Num(); FireLinkIdx++ )
	{
		FFireLink* FireLink = &Slot.FireLinks(FireLinkIdx);

		// If the fire link matches the link, slot, and type
		if( FireLink->TargetMarker == TestMarker )
		{
			bFound = TRUE;
			UBOOL bValidLink = ChkCover.Link->IsFireLinkValid( SlotIdx, FireLink );

			// If fire link info is valid
			// (Always valid for stationary targets... checks validity of dynamic targets)
			if( bValidLink )
			{
				out_FireLinkIdx = FireLinkIdx;
				for( INT ItemIdx = 0; ItemIdx < FireLink->Items.Num(); ItemIdx++ )
				{
					FFireLinkItem& Item = FireLink->Items(ItemIdx);
					if( (ChkAction == CA_Default || Item.DestAction == ChkAction) &&
						(ChkType   == CT_None	 || Item.DestType	== ChkType))
					{
						out_Items.AddItem(ItemIdx);
					}
				}
				// Success if actions exist
                bResult = (out_Items.Num() > 0);
			}
			// Otherwise, if not valid
			else
			{
				// Update LastLinkLocation
				FireLink->LastTargetMarkerLocation = TestMarker->Location;
				FireLink->LastSrcMarkerLocation	   = SrcSlotLocation;

				// Remove all actions from the fire link
				FireLink->Items.Empty();

				// Try to find a link to that slot
				FFireLinkInfo Info( this, SlotIdx );
				ACoverSlotMarker* TestMarker = ChkCover.Link->GetSlotMarker(ChkCover.SlotIdx);
				if( GetFireActions( Info, TestMarker ) )
				{
					// Remove the old fire link
					Slot.FireLinks.Remove( FireLinkIdx--, 1 );

					// Get created fire link
					FFireLink* NewLink = &Slot.FireLinks(Slot.FireLinks.Num()-1);

					out_FireLinkIdx = Slot.FireLinks.Num() - 1;
					for( INT ItemIdx = 0; ItemIdx < NewLink->Items.Num(); ItemIdx++ )
					{
						FFireLinkItem& Item = NewLink->Items(ItemIdx);
						if( (ChkAction == CA_Default || Item.DestAction == ChkAction) &&
							(ChkType   == CT_None	 || Item.DestType	== ChkType))
						{
							out_Items.AddItem(ItemIdx);
						}
					}					
				}
				
				// Success if new actions found
				bResult = (out_Items.Num()>0);
			}

			break;
		}
	}

	// If a link was not found and 
	// this link is stationary while the target is dynamic
	if( !bFound )
	{
		FFireLinkInfo Info( this, SlotIdx );

		if( !bDynamicCover &&
			ChkCover.Link->bDynamicCover )
		{
			// Handles the case where a dynamic link could potentially move into view of a stationary link

			// Try to find a link to the slot
			bResult = GetFireActions( Info, TestMarker );

			// If found a link
			if( bResult )
			{
				FFireLink* NewLink = &Slot.FireLinks(Slot.FireLinks.Num()-1);

				out_FireLinkIdx = Slot.FireLinks.Num() - 1;
				for( INT ItemIdx = 0; ItemIdx < NewLink->Items.Num(); ItemIdx++ )
				{
					FFireLinkItem& Item = NewLink->Items(ItemIdx);
					if( (ChkAction == CA_Default || Item.DestAction == ChkAction) &&
						(ChkType   == CT_None	 || Item.DestType	== ChkType))
					{
						out_Items.AddItem(ItemIdx);
					}
				}	
				// Success if new actions found
				bResult = (out_Items.Num()>0);
			}
			// If no link found
			else
			{
				// Add fire link w/ no actions
				INT LinkIdx = Slot.FireLinks.AddZeroed();
				Slot.FireLinks(LinkIdx).TargetMarker				= TestMarker;
				Slot.FireLinks(LinkIdx).LastTargetMarkerLocation	= TestMarker->Location;
				Slot.FireLinks(LinkIdx).LastSrcMarkerLocation		= SrcSlotLocation;
			}
		}
		else
		// Otherwise, if this is dynamic cover
		// Try to reacquire the target link (it's NOT yet in rejected list or we'd never get here)
		if( bDynamicCover )
		{
			// Try to find a link to that slot
			bResult = GetFireActions( Info, TestMarker );
			// If succeeded
			if( bResult )
			{
				FFireLink* NewLink = &Slot.FireLinks(Slot.FireLinks.Num()-1);
				out_FireLinkIdx = Slot.FireLinks.Num() - 1;
				for( INT ItemIdx = 0; ItemIdx < NewLink->Items.Num(); ItemIdx++ )
				{
					FFireLinkItem& Item = NewLink->Items(ItemIdx);
					if( (ChkAction == CA_Default || Item.DestAction == ChkAction) &&
						(ChkType   == CT_None	 || Item.DestType	== ChkType))
					{
						out_Items.AddItem(ItemIdx);
					}
				}	
				// Success if new actions found
				bResult = (out_Items.Num()>0);
			}
			// Otherwise, if failed
			else
			{
				// Add to the rejected list
				INT Idx = Slot.RejectedFireLinks.AddZeroed();
				FFireLink& Reject = Slot.RejectedFireLinks(Idx);
				Reject.TargetMarker = TestMarker;
				Reject.LastTargetMarkerLocation = TestMarker->Location;
				Reject.LastSrcMarkerLocation = SrcSlotLocation;
			}
		}
	}

	return bResult;
}

void ACoverLink::execGetFireLinkTo(FFrame &Stack,RESULT_DECL)
{
	P_GET_INT(SlotIdx);
	P_GET_STRUCT(struct FCoverInfo,ChkCover);
	P_GET_BYTE(ChkAction);
	P_GET_BYTE(ChkType);
	P_GET_INT_REF(out_FireLinkIdx);
	P_GET_TARRAY_REF(INT,out_Items);
	P_FINISH;
	*(UBOOL*)Result = GetFireLinkTo( SlotIdx, ChkCover, ChkAction, ChkType, out_FireLinkIdx, out_Items );
}

/**
 * Searches for a valid fire link to the specified cover/slot.
 */
UBOOL ACoverLink::HasFireLinkTo( INT SlotIdx, FCoverInfo &ChkCover, UBOOL bAllowFallbackLinks )
{
	FCoverSlot &Slot = Slots(SlotIdx);
	ACoverSlotMarker* ChkMarker = ChkCover.Link->GetSlotMarker( ChkCover.SlotIdx );
	for (INT FireLinkIdx = 0; FireLinkIdx < Slot.FireLinks.Num(); FireLinkIdx++)
	{
		FFireLink &FireLink = Slot.FireLinks(FireLinkIdx);
		if( FireLink.TargetMarker == ChkMarker && 
			(bAllowFallbackLinks || !FireLink.bFallbackLink) )
		{
			return (FireLink.Items.Num() > 0);
		}
	}
	return FALSE;
}

void ACoverLink::execHasFireLinkTo(FFrame &Stack,RESULT_DECL)
{
	P_GET_INT(SlotIdx);
	P_GET_STRUCT(FCoverInfo,ChkCover);
	P_GET_UBOOL_OPTX(bAllowFallbackLinks,FALSE);
	P_FINISH;
	*(UBOOL*)Result = HasFireLinkTo( SlotIdx, ChkCover, bAllowFallbackLinks );
}

UBOOL ACoverLink::IsExposedTo( INT SlotIdx, FCoverInfo ChkCover, FLOAT& out_ExposedScale )
{
	FCoverSlot* ChkSlot = CoverInfoToSlotPtr(ChkCover);
	if( ChkSlot )
	{
		FCoverSlot& Slot = Slots(SlotIdx);
		for( INT Idx = 0; Idx < Slot.ExposedFireLinks.Num(); Idx++ )
		{
			ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(*Slot.ExposedFireLinks(Idx).TargetMarker);
			if( Marker != NULL &&
				Marker->OwningSlot.Link == ChkCover.Link &&
				Marker->OwningSlot.SlotIdx == ChkCover.SlotIdx )
			{
				out_ExposedScale *= (Slot.ExposedFireLinks(Idx).ExposedScale/255.f);
				return TRUE;
			}
		}
	}

	return FALSE;
}

void ACoverLink::GetSlotActions(INT SlotIdx, TArray<BYTE> &Actions)
{
	if (SlotIdx >= 0 && SlotIdx < Slots.Num())
	{
		FCoverSlot &Slot = Slots(SlotIdx);
		if (Slot.bLeanRight)
		{
			Actions.AddItem(CA_PeekRight);
		}
		if (Slot.bLeanLeft)
		{
			Actions.AddItem(CA_PeekLeft);
		}
		if (Slot.CoverType == CT_MidLevel && Slot.bAllowPopup)
		{
			Actions.AddItem(CA_PeekUp);
		}
	}
}

static INT OverlapCount = 0;
struct FOverlapCounter
{
	FOverlapCounter()  { OverlapCount++; }
	~FOverlapCounter() { OverlapCount--; }
};

UBOOL ACoverLink::IsOverlapSlotClaimed( APawn *ChkClaim, INT SlotIdx, UBOOL bSkipTeamCheck )
{
	FOverlapCounter OverlapCounter;

	FCoverSlot& Slot = Slots(SlotIdx);
	for( INT Idx = 0; Idx < Slot.OverlapClaims.Num(); Idx++ )
	{
		ACoverLink* OverLink = Cast<ACoverLink>(Slot.OverlapClaims(Idx).Nav());
		INT	OverSlotIdx = Slot.OverlapClaims(Idx).SlotIdx;
		if(  OverLink && 
			!OverLink->IsValidClaim( ChkClaim, OverSlotIdx, bSkipTeamCheck, TRUE ) )
		{
			return TRUE;
		}			
	}

	return FALSE;
}

UBOOL ACoverLink::IsValidClaim( APawn *ChkClaim, INT SlotIdx, UBOOL bSkipTeamCheck, UBOOL bSkipOverlapCheck )
{
	ACoverSlotMarker* Marker = Slots(SlotIdx).SlotMarker;
	return (Marker != NULL && Marker->IsValidClaim( ChkClaim, bSkipTeamCheck, bSkipOverlapCheck ));
}

UBOOL ACoverSlotMarker::IsValidClaim( APawn *ChkClaim, UBOOL bSkipTeamCheck, UBOOL bSkipOverlapCheck )
{
	ACoverLink* Link	= OwningSlot.Link;
	INT			SlotIdx = OwningSlot.SlotIdx;

	// early out if it's an invalid slot or the slot/link is disabled
	if( !Link || !Link->IsEnabled() || ChkClaim == NULL || SlotIdx < 0 || SlotIdx >= Link->Slots.Num() || !Link->Slots(SlotIdx).bEnabled )
	{
		// If inside overlap check - don't invalidate claim when overlapping a disabled slot
		return (OverlapCount > 0);
	}

	// If the slot is already held by the controller or it is empty (and we accept empty slots) - valid claim
	APawn* SlotOwner = Link->Slots(SlotIdx).SlotOwner;
	UBOOL bResult = (SlotOwner == ChkClaim || SlotOwner == NULL || SlotOwner->bDeleteMe || (ChkClaim->IsHumanControlled() && !SlotOwner->IsHumanControlled())) && 
						(ChkClaim->IsHumanControlled() || !bBlocked) && 
						(GWorld->GetTimeSeconds() >= Link->Slots(SlotIdx).SlotValidAfterTime);

	// If we have a valid claim so far and the controller has a pawn
	if( bResult && ChkClaim != NULL )
	{
		// If we need to make sure ALL cover slots are valid by team
		if( !bSkipTeamCheck )
		{
			// Go through the claims list
			for( INT Idx = 0; Idx < Link->Claims.Num() && bResult; Idx++ )
			{
				APawn* C = Link->Claims(Idx);
				if( C == NULL)
				{
					Link->Claims.Remove(Idx--,1);
				}
				else
				// And make sure all the other claims are on the same team
				if( C != NULL && !C->bDeleteMe &&
					(ChkClaim->PlayerReplicationInfo != NULL && C->PlayerReplicationInfo != NULL && C->PlayerReplicationInfo->Team != ChkClaim->PlayerReplicationInfo->Team))
				{
					// If not on same team - invalid claim
					bResult = FALSE;
					break;
				}
			}
		}
	}

#if 0	// going to handle this in the AI with a cheaper method
	// check to make sure we're not going to trap a player that's standing next to the cover
	if( bResult && ChkClaim != NULL )
	{
		for (AController *Controller = GWorld->GetWorldInfo()->ControllerList; Controller != NULL; Controller = Controller->NextController)
		{
			static const FLOAT MaxProximity= 40.0f*40.0f;
			
			if (Controller != NULL && 
				Controller != ChkClaim->Controller &&
				Controller->Pawn != NULL &&
				Controller->Pawn->Health > 0)
			{
				// if this controller is a player within MaxProximity,
				// we'll consider this cover invalid
				if (Controller->GetAPlayerController() != NULL &&
					(Controller->Pawn->Location - Location).SizeSquared() < MaxProximity)
				{
					bResult = FALSE;
					break;
				}
			}
		}
	}
#endif

	if( bResult && !bSkipOverlapCheck )
	{
		if( Link->IsOverlapSlotClaimed( ChkClaim, SlotIdx, bSkipTeamCheck ) )
		{
			bResult = FALSE;
		}
	}

	return bResult;
}

void ACoverGroup::CheckForErrors()
{
	Super::CheckForErrors();
	TArray<INT> ContainedNetworkIDs;
	for (INT Idx = 0; Idx < CoverLinkRefs.Num(); Idx++)
	{
		ACoverLink *Link = Cast<ACoverLink>(~CoverLinkRefs(Idx));
		if (Link != NULL)
		{
			ContainedNetworkIDs.AddUniqueItem(Link->NetworkID);
		}
	}
	if (ContainedNetworkIDs.Num() > 1)
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Contains references to links in different navigation networks!")));
	}
}

void ACoverGroup::EnableGroup()
{
	for( INT Idx = 0; Idx < CoverLinkRefs.Num(); Idx++ )
	{
		ACoverLink* Link = Cast<ACoverLink>(CoverLinkRefs(Idx).Nav());
		if( Link )
		{
			Link->eventSetDisabled(FALSE);
		}
		else
		{
			CoverLinkRefs.Remove( Idx--, 1 );
		}
	}
}

void ACoverGroup::DisableGroup()
{
	for( INT Idx = 0; Idx < CoverLinkRefs.Num(); Idx++ )
	{
		ACoverLink* Link = Cast<ACoverLink>(CoverLinkRefs(Idx).Nav());
		if( Link )
		{
			Link->eventSetDisabled(TRUE);
		}
		else
		{
			CoverLinkRefs.Remove( Idx--, 1 );
		}
	}
}

void ACoverGroup::ToggleGroup()
{
	for( INT Idx = 0; Idx < CoverLinkRefs.Num(); Idx++ )
	{
		ACoverLink* Link = Cast<ACoverLink>(CoverLinkRefs(Idx).Nav());
		if( Link )
		{
			Link->eventSetDisabled(!Link->bDisabled);
		}
		else
		{
			CoverLinkRefs.Remove( Idx--, 1 );
		}
	}
}

void ACoverGroup::PostLoad()
{
	Super::PostLoad();
}

void ACoverGroup::GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel)
{
	Super::GetActorReferences(ActorRefs,bIsRemovingLevel);
	for (INT Idx = 0; Idx < CoverLinkRefs.Num(); Idx++)
	{
		FActorReference &ActorRef = CoverLinkRefs(Idx);
		if (ActorRef.Guid.IsValid())
		{
			if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL))
			{
				ActorRefs.AddItem(&ActorRef);
			}
		}
	}
}

//----------------------------------------------------------------
// Lift navigation support

void ALiftExit::ReviewPath(APawn* Scout)
{
	if ( !MyLiftCenter )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("No LiftCenter associated with this LiftExit"), MCACTION_NONE, TEXT("NoLiftCenter"));
	}
}

void ALiftCenter::FindBase()
{
	if ( GWorld->HasBegunPlay() )
	{
		return;
	}

	SetZone(1,1);
	// FIRST, turn on collision temporarily for InterpActors
	for( FActorIterator It; It; ++It )
	{
		AInterpActor *Actor = Cast<AInterpActor>(*It);
		if( Actor && !Actor->bDeleteMe && Actor->bPathTemp )
		{
			Actor->SetCollision( TRUE, Actor->bBlockActors, Actor->bIgnoreEncroachers );
		}
	}

	// not using find base, because don't want to fail if LD has navigationpoint slightly interpenetrating floor
	FCheckResult Hit(1.f);
	AScout *Scout = FPathBuilder::GetScout();
	check(Scout != NULL && "Failed to find scout for point placement");
	// get the dimensions for the average human player
	FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));
	FVector CollisionSlice(HumanSize.X, HumanSize.X, 1.f);
	// and use this node's smaller collision radius if possible
	if (CylinderComponent->CollisionRadius < HumanSize.X)
	{
		CollisionSlice.X = CollisionSlice.Y = CylinderComponent->CollisionRadius;
	}
	GWorld->SingleLineCheck( Hit, Scout, Location - FVector(0,0, 2.f * CylinderComponent->CollisionHeight), Location, TRACE_AllBlocking );

	// check for placement
	GWorld->SingleLineCheck( Hit, Scout, Location - FVector(0,0, 2.f * CylinderComponent->CollisionHeight), Location, TRACE_AllBlocking, CollisionSlice );
	if (Hit.Actor != NULL)
	{
		if (Hit.Normal.Z > Scout->WalkableFloorZ)
		{
			GWorld->FarMoveActor(this, Hit.Location + FVector(0.f,0.f,CylinderComponent->CollisionHeight-1.f),0,1,0);
		}
		else
		{
			Hit.Actor = NULL;
		}
	}

	SetBase(Hit.Actor, Hit.Normal);

	// Turn off collision for InterpActors
	for( FActorIterator It; It; ++It )
	{
		AInterpActor *Actor = Cast<AInterpActor>(*It);
		if( Actor && !Actor->bDeleteMe && Actor->bPathTemp )
		{
			Actor->SetCollision( FALSE, Actor->bBlockActors, Actor->bIgnoreEncroachers );
		}
	}
}

void ALiftCenter::ReviewPath(APawn* Scout)
{
	if ( !MyLift || (MyLift != Base) )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("LiftCenter not based on an InterpActor"), MCACTION_NONE, TEXT("NeedInterpActorBase"));
	}
	if ( PathList.Num() == 0 )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("No LiftExits associated with this LiftCenter"), MCACTION_NONE, TEXT("NoLiftExit"));
	}
}

UBOOL ALiftCenter::ShouldBeBased()
{
	return true;
}

/* addReachSpecs()
Virtual function - adds reachspecs to LiftCenter for every associated LiftExit.
*/
void ALiftCenter::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	bPathsChanged = bPathsChanged || !bOnlyChanged;

	// find associated mover
	FindBase();
	MyLift = Cast<AInterpActor>(Base);
	if (  MyLift && (MyLift->GetOutermost() != GetOutermost()) )
		MyLift = NULL;

	// Warn if there is no lift
	if ( !MyLift )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("LiftCenter not based on an InterpActor"), MCACTION_NONE, TEXT("NeedInterpActorBase"));
	}
	else
	{
		MyLift->MyMarker = this;
	}

	UReachSpec *newSpec = ConstructObject<UReachSpec>(UAdvancedReachSpec::StaticClass(),GetOuter(),NAME_None);
	//debugf("Add Reachspecs for LiftCenter at (%f, %f, %f)", Location.X,Location.Y,Location.Z);
	INT NumExits = 0;
	FVector MaxCommonSize = Scout->GetSize(FName(TEXT("Max"),FNAME_Find));

	for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		ALiftExit *LE = Cast<ALiftExit>(Nav); 
		if ( LE && !LE->bDeleteMe && (LE->MyLiftCenter == this) && (!bOnlyChanged || bPathsChanged || LE->bPathsChanged) && (LE->GetOutermost() == GetOutermost()) ) 
		{
			NumExits++;

			// add reachspec from LiftCenter to LiftExit
			newSpec->CollisionRadius = appTrunc(MaxCommonSize.X);
			newSpec->CollisionHeight = appTrunc(MaxCommonSize.Y);
			newSpec->Start = this;
			newSpec->End = LE;
			newSpec->Distance = 500;
			PathList.AddItem(newSpec);
			newSpec = ConstructObject<UReachSpec>(UAdvancedReachSpec::StaticClass(),GetOuter(),NAME_None);

			// add reachspec from LiftExit to LiftCenter
			if ( !LE->bExitOnly )
			{
				newSpec->CollisionRadius = appTrunc(MaxCommonSize.X);
				newSpec->CollisionHeight = appTrunc(MaxCommonSize.Y);
				newSpec->Start = LE;
				newSpec->End = this;
				newSpec->Distance = 500;
				LE->PathList.AddItem(newSpec);
				newSpec = ConstructObject<UReachSpec>(UAdvancedReachSpec::StaticClass(),GetOuter(),NAME_None);
			}
		}
	}
	
	// Warn if no lift exits
	if ( NumExits == 0 )
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("No LiftExits associated with this LiftCenter"), MCACTION_NONE, TEXT("NoLiftExit"));
}

UBOOL ALadder::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	// look at difference along ladder direction
	return (P && P->OnLadder && (Abs((Dest - TestPosition) | P->OnLadder->ClimbDir) < P->CylinderComponent->CollisionHeight) );
}

/** returns the position the AI should move toward to reach this actor */
FVector ANavigationPoint::GetDestination(AController* C)
{
	FVector Dest = Super::GetDestination(C);

	if ((!bCollideActors || !bMustTouchToReach) && C != NULL && C->CurrentPath != NULL && C->Pawn != NULL && !(C->CurrentPath->reachFlags & R_JUMP))
	{
		if (C->bUsingPathLanes)
		{
			// move to the right side of the path as far as possible
			Dest -= (C->CurrentPathDir ^ FVector(0.f, 0.f, 1.f)) * C->LaneOffset;
		}
		// if the Controller is on a normal path (not requiring jumping or special movement and not forced)
		else if ( !bSpecialMove && C->ShouldOffsetCorners() && C->NextRoutePath != NULL && C->NextRoutePath->Start != NULL && *C->NextRoutePath->End != NULL &&
				C->Pawn->Physics != PHYS_RigidBody && C->CurrentPath->bCanCutCorners &&
				C->NextRoutePath->bCanCutCorners )
		{
			// offset destination in the direction of the next path (cut corners)
			FLOAT ExtraRadius = FLOAT(C->CurrentPath->CollisionRadius) - C->Pawn->CylinderComponent->CollisionRadius;
			if (ExtraRadius > 0.f)
			{
				Dest += (C->NextRoutePath->End->Location - C->NextRoutePath->Start->Location).SafeNormal2D() * ExtraRadius;
			}
		}
	}

	return Dest;
}		

UBOOL ANavigationPoint::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	if ( TouchReachSucceeded(P, TestPosition) )
	{
		return TRUE;
	}

	// if touch reach failed and bMustTouchToReach=true, fail
	if ( bCollideActors && P->bCollideActors )
	{
		// rigid bodies don't get based on their floor, so if we're both blocking, it's impossible for TouchReachSucceeded() to ever return true, so ignore bMustTouchToReach
		if (P->Physics != PHYS_RigidBody || !bBlockActors || !P->bBlockActors || (CollisionComponent != NULL && CollisionComponent->CollideActors && !CollisionComponent->BlockActors))
		{
			if ( bBlockActors && !bMustTouchToReach )
			{
				if( P->bBlockActors && CollisionComponent )
				{
					FCheckResult Hit(1.f);

					if ( !CollisionComponent->LineCheck(Hit, TestPosition + 30.f*(Location - TestPosition).SafeNormal(), TestPosition, P->GetCylinderExtent(), 0) )
						return true;
				}

			}
			else if ( !GIsEditor && bMustTouchToReach )
			{
				return FALSE;
			}
		}
	}

	// allow success if vehicle P is based on is covering this node
	APawn *V = P->GetVehicleBase();
	if ( V && (Abs(V->Location.Z - Dest.Z) < V->CylinderComponent->CollisionHeight) )
	{
		FVector VDir = V->Location - Dest;
		VDir.Z = 0.f;
		if ( VDir.SizeSquared() < 1.21f * V->CylinderComponent->CollisionRadius * V->CylinderComponent->CollisionRadius )
			return true;
	}

	if ( P->Controller && P->Controller->ForceReached(this, TestPosition) )
	{
		return TRUE;
	}

	// get the pawn's normal height (might be crouching or a Scout, so use the max of current/default)
	FLOAT PawnHeight = Max<FLOAT>(P->CylinderComponent->CollisionHeight, ((APawn *)(P->GetClass()->GetDefaultObject()))->CylinderComponent->CollisionHeight);

	return P->ReachThresholdTest(TestPosition, Dest, this, 
		::Max(0.f,CylinderComponent->CollisionHeight - PawnHeight + P->MaxStepHeight + MAXSTEPHEIGHTFUDGE), 
		::Max(0.f,2.f + P->MaxStepHeight - CylinderComponent->CollisionHeight), 
		0.f);	
}

UBOOL ANavigationPoint::TouchReachSucceeded(APawn* P, const FVector& TestPosition)
{
	return (bCanWalkOnToReach && TestPosition == P->Location && P->Base == this && !GIsEditor) ? TRUE : Super::TouchReachSucceeded(P, TestPosition);
}

UBOOL APickupFactory::ReachedBy(APawn* P, const FVector& TestPosition, const FVector& Dest)
{
	UBOOL bResult;

	if (bMustTouchToReach)
	{
		// only actually need to touch if pawn can pick me up
		bMustTouchToReach = P->bCanPickupInventory;
		bResult = Super::ReachedBy(P, TestPosition, Dest);
		bMustTouchToReach = TRUE;
	}
	else
	{
		bResult = Super::ReachedBy(P, TestPosition, Dest);
	}

	return bResult;
}

ANavigationPoint* APickupFactory::SpecifyEndAnchor(APawn* RouteFinder)
{
	APickupFactory* Result = this;
	while (Result->OriginalFactory != NULL)
	{
		Result = Result->OriginalFactory;
	}

	return Result;
}

/** returns whether this NavigationPoint is valid to be considered as an Anchor (start or end) for pathfinding by the given Pawn
 * @param P the Pawn doing pathfinding
 * @return whether or not we can be an anchor
 */
UBOOL ANavigationPoint::IsUsableAnchorFor(APawn* P)
{
	return ( !bBlocked && (!bFlyingPreferred || P->bCanFly) && (!bBlockedForVehicles || !P->IsA(AVehicle::StaticClass())) &&
		MaxPathSize.Radius >= P->CylinderComponent->CollisionRadius && MaxPathSize.Height >= P->CylinderComponent->CollisionHeight && P->IsValidAnchor(this) );
}

void AVolumePathNode::InitForPathFinding()
{
	// calculate flightradius
	// assume starting with reasonable estimate
	CylinderComponent->CollisionHeight = StartingHeight;
	CylinderComponent->CollisionRadius = StartingRadius;

	// look for floor
	FCheckResult Hit(1.f);
	GWorld->SingleLineCheck(Hit, this, Location - FVector(0.f,0.f,CylinderComponent->CollisionHeight), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionHeight *= Hit.Time;
	GWorld->SingleLineCheck(Hit, this, Location + FVector(0.f,0.f,CylinderComponent->CollisionHeight), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionHeight *= Hit.Time;
	FLOAT MaxHeight = CylinderComponent->CollisionHeight;

	GWorld->SingleLineCheck(Hit, this, Location - FVector(CylinderComponent->CollisionRadius,0.f,0.f), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionRadius *= Hit.Time;
	GWorld->SingleLineCheck(Hit, this, Location + FVector(CylinderComponent->CollisionRadius,0.f,0.f), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionRadius *= Hit.Time;

	GWorld->SingleLineCheck(Hit, this, Location - FVector(0.f,CylinderComponent->CollisionRadius,0.f), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionRadius *= Hit.Time;
	GWorld->SingleLineCheck(Hit, this, Location + FVector(0.f,CylinderComponent->CollisionRadius,0.f), Location, TRACE_World);
	if ( Hit.Actor )
		CylinderComponent->CollisionRadius *= Hit.Time;

	// refine radius with non-zero extent point checks
	FVector Extent(CylinderComponent->CollisionRadius,CylinderComponent->CollisionRadius, CylinderComponent->CollisionRadius);
	FVector Unknown = 0.5f * Extent;
	while ( Unknown.X > 2.f )
	{
		if ( GWorld->EncroachingWorldGeometry( Hit, Location, Extent ) )
			Extent -= Unknown;
		else if ( Extent.X >= CylinderComponent->CollisionRadius )
			Unknown.X = 0.f;
		else
			Extent += Unknown;
		Unknown *= 0.5f;
	}
	Extent = Extent - Unknown - FVector(2.f,2.f,2.f);
	if ( Extent.X < 2.f )
	{
		CylinderComponent->CollisionRadius = 2.f;
		CylinderComponent->CollisionHeight = 2.f;
		return;
	}
	CylinderComponent->CollisionRadius = Extent.X;
	CylinderComponent->CollisionHeight = CylinderComponent->CollisionRadius;

	Extent = FVector(CylinderComponent->CollisionRadius,CylinderComponent->CollisionRadius,CylinderComponent->CollisionHeight+4.f);
	if ( !GWorld->EncroachingWorldGeometry( Hit, Location, Extent ) )
	{
		// try to increase height
		Extent.Z = MaxHeight;
		Unknown = 0.5f * Extent;
		Unknown.X = 0.f;
		Unknown.Y = 0.f;
		while ( Unknown.Z > 2.f )
		{
			if ( GWorld->EncroachingWorldGeometry( Hit, Location, Extent ) )
				Extent -= Unknown;
			else if ( Extent.Z >= MaxHeight )
				Unknown.Z = 0.f;
			else
				Extent += Unknown;
			Unknown *= 0.5f;
		}
		CylinderComponent->CollisionHeight = Extent.Z;
	}
	// try to increase radius
	Extent.Z = CylinderComponent->CollisionHeight;
	Extent.X = 4.f * CylinderComponent->CollisionRadius;
	Extent.Y = Extent.X;
	Unknown = 0.5f * Extent;
	Unknown.Z = 0.f;
	while ( Unknown.X > 2.f )
	{
		if ( GWorld->EncroachingWorldGeometry( Hit, Location, Extent ) )
			Extent -= Unknown;
		else if ( Extent.X >= 6.f * CylinderComponent->CollisionRadius )
			Unknown.X = 0.f;
		else
			Extent += Unknown;
		Unknown *= 0.5f;
	}
	CylinderComponent->CollisionRadius = Extent.X;
}

/* addReachSpecs()
Virtual function - adds reachspecs to path for every path reachable from it. 
*/
void AVolumePathNode::addReachSpecs(AScout * Scout, UBOOL bOnlyChanged)
{
	bPathsChanged = bPathsChanged || !bOnlyChanged;
	UReachSpec *newSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GetOuter(),NAME_None);

	FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find));

	// add paths to paths that are within FlightRadius, or intersecting flightradius (intersection radius defines path radius, or dist from edge of radius)
	// Note that none flying nodes need to have a path added from them as well
	for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		if ( !Nav->bDeleteMe && (!Nav->bNoAutoConnect || Cast<AVolumePathNode>(Nav)) && !Nav->bSourceOnly && !Nav->bMakeSourceOnly
				&& (Nav != this) && (bPathsChanged || Nav->bPathsChanged) && (Nav->GetOutermost() == GetOutermost()) )
		{
			if ( !ProscribePathTo(Nav, Scout) )
			{
				// check if paths are too close together
				if ( ((Nav->Location - Location).SizeSquared() < 2.f * HumanSize.X) && (Nav->GetClass()->ClassFlags & CLASS_Placeable) )
				{
					GWarn->MapCheck_Add( MCTYPE_WARNING, this, TEXT("May be too close to other navigation points"), MCACTION_NONE, TEXT("NavPointTooClose"));
				}
			
				// check if forced path
				UBOOL bForced = 0;
				for (INT idx = 0; idx < EditorForcedPaths.Num() && !bForced; idx++)
				{
					if (Nav == EditorForcedPaths(idx).Nav())
					{
						bForced = 1;
					}
				}
				if ( bForced )
				{
					ForcePathTo(Nav,Scout);
				}
				else if ( !bDestinationOnly )
				{
					AVolumePathNode *FlyNav = Cast<AVolumePathNode>(Nav);
					FLOAT SpecRadius = -1.f;
					FLOAT SpecHeight = -1.f;
					FLOAT Dist2D = (Nav->Location - Location).Size2D();
					if ( Dist2D < Nav->CylinderComponent->CollisionRadius + CylinderComponent->CollisionRadius )
					{
						// check if visible
						FCheckResult Hit(1.f);
						GWorld->SingleLineCheck( Hit, this, Nav->Location, Location, TRACE_World|TRACE_StopAtAnyHit );
						if ( !Hit.Actor || (Hit.Actor == Nav) )
						{
							if ( FlyNav )
							{
								SpecHeight = CylinderComponent->CollisionHeight + FlyNav->CylinderComponent->CollisionHeight - Abs(FlyNav->Location.Z - Location.Z);
								if ( SpecHeight >= HumanSize.Y )
								{
									// reachspec radius based on intersection of circles
									if ( Dist2D > 1.f )
									{
										FLOAT R1 = CylinderComponent->CollisionRadius;
										FLOAT R2 = FlyNav->CylinderComponent->CollisionRadius;
										FLOAT Part = 0.5f*Dist2D + (R1*R1 - R2*R2)/(2.f*Dist2D);
										SpecRadius = ::Max<FLOAT>(SpecRadius,appSqrt(R1*R1 - Part*Part));
									}
									else
									{
										SpecRadius = ::Min<FLOAT>(CylinderComponent->CollisionRadius,FlyNav->CylinderComponent->CollisionRadius);
									}
									//debugf(TEXT("Radius from %s to %s is %f"),*GetName(),*FlyNav->GetName(),SpecRadius);
								}
							}
							else if ( Dist2D < CylinderComponent->CollisionRadius )
							{
								// if nav inside my cylinder, definitely add
								if ( Abs(Nav->Location.Z - Location.Z) < CylinderComponent->CollisionHeight )
								{
									SpecRadius = 0.75f * CylinderComponent->CollisionRadius;
									SpecHeight = 0.75f * CylinderComponent->CollisionHeight;
								}
								else if ( Location.Z > Nav->Location.Z )
								{
									// otherwise, try extent trace to cylinder
									FVector Intersect = Nav->Location; 
									Intersect.Z = Location.Z - CylinderComponent->CollisionHeight + 2.f;

									FCheckResult CylinderHit(1.f);
									for ( INT SizeIndex=Scout->PathSizes.Num()-1; SizeIndex >= 0; SizeIndex-- )
									{
                                        GWorld->SingleLineCheck( CylinderHit, this, Intersect, Nav->Location, TRACE_World|TRACE_StopAtAnyHit, FVector(Scout->PathSizes(SizeIndex).Radius,Scout->PathSizes(SizeIndex).Radius,1.f) );
										if ( !CylinderHit.Actor || (CylinderHit.Actor == Nav) )
										{
											SpecHeight = Scout->PathSizes(SizeIndex).Height;
											SpecRadius = Scout->PathSizes(SizeIndex).Radius;
											break;
										}
									}
								}
							}
						}
					}
					if ( SpecRadius > 0.f )
					{
						// we found a good connection
						FLOAT RealDist = (Location - Nav->Location).Size();
						newSpec->CollisionRadius = appTrunc(SpecRadius);
						newSpec->CollisionHeight = appTrunc(SpecHeight);
						INT NewFlags = R_FLY;
						if ( Nav->PhysicsVolume->bWaterVolume && PhysicsVolume->bWaterVolume )
							NewFlags = R_SWIM;
						else if ( Nav->PhysicsVolume->bWaterVolume || PhysicsVolume->bWaterVolume )
							NewFlags = R_SWIM + R_FLY;
						else
						{
							if ( Nav->IsA(AVolumePathNode::StaticClass()) && (RealDist > SHORTTRACETESTDIST) )
							{
								RealDist = ::Max(SHORTTRACETESTDIST, Dist2D - ::Max(CylinderComponent->CollisionRadius, Nav->CylinderComponent->CollisionRadius));
							}
							else
							{
								RealDist = ::Max(1.f, Dist2D);
							}
						}
						newSpec->reachFlags = NewFlags;
						newSpec->Start = this;
						newSpec->End = Nav;
						newSpec->Distance = appTrunc(RealDist);
						PathList.AddItem(newSpec);
						newSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GetOuter(),NAME_None);

						if ( !FlyNav && !Nav->bDestinationOnly )
						{
							newSpec->CollisionRadius = appTrunc(SpecRadius);
							newSpec->CollisionHeight = appTrunc(SpecHeight);
							newSpec->reachFlags = NewFlags;
							newSpec->Start = Nav;
							newSpec->End = this;
							newSpec->Distance = appTrunc(RealDist);
							Nav->PathList.AddItem(newSpec);
							newSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GetOuter(),NAME_None);
						}
						debugfSuppressed(NAME_DevPath, TEXT("***********added new spec from %s to %s size %f %f"),*GetName(),*Nav->GetName(), SpecRadius, SpecHeight);
					}
				}
			}
		}
	}
}

UBOOL AVolumePathNode::CanConnectTo(ANavigationPoint* Nav, UBOOL bCheckDistance)
{
	return Super::CanConnectTo(Nav, false);
}

UBOOL AVolumePathNode::ShouldBeBased()
{
	return false;
}

UBOOL AVolumePathNode::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{ 
	if ( !P->bCanFly && !PhysicsVolume->bWaterVolume )
		return false;
	FVector Dir = TestPosition - Dest;
	if ( Abs(Dir.Z) > CylinderComponent->CollisionHeight )
		return false;
	Dir.Z = 0.f;
	return ( Dir.SizeSquared() < CylinderComponent->CollisionRadius * CylinderComponent->CollisionRadius );
}

void AVolumePathNode::ReviewPath(APawn* Scout)
{
}

UBOOL AVolumePathNode::CanPrunePath(INT index) 
{ 
	return !PathList(index)->End->IsA(AVolumePathNode::StaticClass());
}

AActor* ADoorMarker::AssociatedLevelGeometry()
{
	return MyDoor;
}

UBOOL ADoorMarker::HasAssociatedLevelGeometry(AActor *Other)
{
	return (Other != NULL && Other == MyDoor);
}

void ADoorMarker::PrePath()
{
	// turn off associated mover collision temporarily
	if (MyDoor != NULL)
	{
		MyDoor->MyMarker = this;
		if (MyDoor->bBlockActors && MyDoor->bCollideActors)
		{
			MyDoor->SetCollision( FALSE, MyDoor->bBlockActors, MyDoor->bIgnoreEncroachers );
			bTempDisabledCollision = 1;
		}
	}
}

void ADoorMarker::PostPath()
{
	if (bTempDisabledCollision && MyDoor != NULL)
	{
		MyDoor->SetCollision( TRUE, MyDoor->bBlockActors, MyDoor->bIgnoreEncroachers );
	}
}

void ADoorMarker::FindBase()
{
	if (!GWorld->HasBegunPlay())
	{
		PrePath();
		Super::FindBase();
		PostPath();
	}
}

void ADoorMarker::CheckForErrors()
{
	Super::CheckForErrors();

	if (MyDoor == NULL)
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("DoorMarker with no door"), MCACTION_NONE, TEXT("DoorMarkerNoDoor"));
	}
}

INT APortalTeleporter::AddMyMarker(AActor* S)
{
	AScout* Scout = Cast<AScout>(S);
	if (Scout == NULL)
	{
		return 0;
	}
	else
	{
		if (MyMarker == NULL || MyMarker->bDeleteMe)
		{
			FVector MaxCommonSize = Scout->GetSize(FName(TEXT("Max"),FNAME_Find));
			FLOAT BottomZ = GetComponentsBoundingBox(FALSE).Min.Z;
			// trace downward to find the floor to place the marker on
			FCheckResult Hit(1.0f);
			if (GWorld->SingleLineCheck(Hit, this, Location + FVector(0.f, 0.f, BottomZ), Location, TRACE_AllBlocking, MaxCommonSize))
			{
				Hit.Location = Location + FVector(0.f, 0.f, BottomZ);
			}
			MyMarker = CastChecked<APortalMarker>(GWorld->SpawnActor(APortalMarker::StaticClass(), NAME_None, Hit.Location));
			if (MyMarker != NULL)
			{
				MyMarker->MyPortal = this;
			}
			else
			{
				GWarn->MapCheck_Add(MCTYPE_ERROR, this, TEXT("Failed to add PortalMarker!"), MCACTION_NONE, TEXT("PortalMarkerFailed"));
			}
		}
		return (MyMarker != NULL) ? 1 : 0;
	}
}

void APortalMarker::addReachSpecs(AScout* Scout, UBOOL bOnlyChanged)
{
	// if our portal has collision and a valid destination portal, add a reachspec to the destination portal
	if ( MyPortal != NULL && (MyPortal->bCollideActors || MyPortal->bPathTemp) && MyPortal->SisterPortal != NULL && MyPortal->SisterPortal->MyMarker != NULL &&
		(!bOnlyChanged || bPathsChanged || MyPortal->SisterPortal->MyMarker->bPathsChanged) )
	{
		UReachSpec* NewSpec = ConstructObject<UReachSpec>(UTeleportReachSpec::StaticClass(), GetOuter(), NAME_None);
		FVector MaxSize = Scout->GetMaxSize();
		NewSpec->CollisionRadius = appTrunc(MaxSize.X);
		NewSpec->CollisionHeight = appTrunc(MaxSize.Y);
		NewSpec->Start = this;
		NewSpec->End = MyPortal->SisterPortal->MyMarker;
		NewSpec->Distance = 100;
		PathList.AddItem(NewSpec);
	}

	ANavigationPoint::addReachSpecs(Scout, bOnlyChanged);
}

UBOOL APortalMarker::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	return (P != NULL && (MyPortal == NULL || !MyPortal->bCollideActors || MyPortal->TouchReachSucceeded(P, TestPosition)) && Super::ReachedBy(P, TestPosition, Dest));
}

UBOOL APortalMarker::CanTeleport(AActor* A)
{
	return (MyPortal != NULL && MyPortal->CanTeleport(A));
}

/** PlaceScout()
Place a scout at the location of this NavigationPoint, or as close as possible
*/
UBOOL ANavigationPoint::PlaceScout(AScout * Scout)
{
	// Try placing above and moving down
	FCheckResult Hit(1.f);
	UBOOL bSuccess = FALSE;

	if( Base )
	{
		FVector Up( 0.f, 0.f, 1.f );
		GetUpDir( Up );
		Up *= Scout->CylinderComponent->CollisionHeight - CylinderComponent->CollisionHeight + ::Max(0.f, Scout->CylinderComponent->CollisionRadius - CylinderComponent->CollisionRadius);

		if( GWorld->FarMoveActor( Scout, Location + Up ) )
		{
			bSuccess = TRUE;
			GWorld->MoveActor(Scout, -1.f * Up, Scout->Rotation, 0, Hit);
		}
	}

	if( !bSuccess && !GWorld->FarMoveActor(Scout, Location) )
	{
		return FALSE;
	}

	// If scout is walking, make sure it is on the ground
	if( (Scout->Physics == PHYS_Walking || Scout->Physics == PHYS_Spider) && 
		!Scout->bCrawler &&
		!Scout->PhysicsVolume->bWaterVolume )
	{
		FVector Up(0,0,1);
		GetUpDir( Up );
		GWorld->MoveActor(Scout, -Up * CylinderComponent->CollisionHeight, Scout->Rotation, 0, Hit);
	}
	return TRUE;
}

UBOOL ANavigationPoint::CanTeleport(AActor* A)
{
	return FALSE;
}

void ARoute::PostLoad()
{
	Super::PostLoad();

	if( NavList.Num() > 0 )
	{
		RouteList.Empty();
		for( INT Idx = 0; Idx < NavList.Num(); Idx++ )
		{
			RouteList.AddItem( FActorReference(NavList(Idx).Nav, NavList(Idx).Guid) );
		}
	}
}

void ARoute::GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel)
{
	Super::GetActorReferences(ActorRefs,bIsRemovingLevel);
	for (INT Idx = 0; Idx < RouteList.Num(); Idx++)
	{
		FActorReference &ActorRef = RouteList(Idx);
		if (ActorRef.Guid.IsValid())
		{
			if ((bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL))
			{
				ActorRefs.AddItem(&ActorRef);
			}
		}
	}
}

void ARoute::AutoFillRoute( ERouteFillAction RFA, TArray<ANavigationPoint*>& Points )
{
	// If overwriting or clearing
	if( RFA == RFA_Overwrite || 
		RFA == RFA_Clear )
	{
		// Empty list
		RouteList.Empty();
	}

	// If overwriting or adding selected items
	if( RFA == RFA_Overwrite || RFA == RFA_Add )
	{
		// Go through each selected nav point
		for( INT Idx = 0; Idx < Points.Num(); Idx++ )
		{
			ANavigationPoint* Nav = Points(Idx);
			if( Nav )
			{
				// Add to the list
				FActorReference Item(EC_EventParm);
				Item.Actor = Nav;

				// If nav point is in another map from the route
				if( Item.Actor &&
					GetOutermost() != Item.Actor->GetOutermost() )
				{
					// Use GUID
					Item.Guid = *Item.Actor->GetGuid();
				}

				RouteList.AddItem( Item );
			}
		}
	}
	else
	// Otherwise, if removing selected items
	if( RFA == RFA_Remove )
	{
		// Go through each selected nav point
		for( INT Idx = 0; Idx < Points.Num(); Idx++ )
		{
			// Remove from the list
			for( INT ItemIdx = 0; ItemIdx < RouteList.Num(); ItemIdx++ )
			{
				if( RouteList(ItemIdx).Actor == Points(Idx) )
				{
					RouteList.Remove( ItemIdx-- );
				}
			}
		}
	}

	ForceUpdateComponents(FALSE,FALSE);
}

void ARoute::CheckForErrors()
{
	Super::CheckForErrors();

	for( INT Idx = 0; Idx < RouteList.Num(); Idx++ )
	{
		ANavigationPoint* Nav = ((ANavigationPoint*)~RouteList(Idx));
		if( !Nav )
		{
			GWarn->MapCheck_Add( MCTYPE_ERROR, this, *FString::Printf(TEXT("RouteList Index %d Has Invalid NavigationPoint"), Idx) );
		}
	}
}

void ACoverGroup::AutoFillGroup( ECoverGroupFillAction CGFA, TArray<ACoverLink*>& Links )
{
	// If overwriting or clearing or filling by cylinder
	if( CGFA == CGFA_Overwrite || 
		CGFA == CGFA_Clear || 
		(CGFA != CGFA_Remove && CGFA != CGFA_Add) )
	{
		// Empty list
		CoverLinkRefs.Empty();
	}

	// If overwriting or adding selected items
	if( CGFA == CGFA_Overwrite || CGFA == CGFA_Add )
	{
		for( INT Idx = 0; Idx < Links.Num(); Idx++ )
		{
			// Add to the list
			CoverLinkRefs.AddUniqueItem( FActorReference(Links(Idx),*Links(Idx)->GetGuid()) );
		}
	}
	else
	// Otherwise, if removing selected items
	if( CGFA == CGFA_Remove )
	{
		// Go through each cover link
		for( INT Idx = 0; Idx < Links.Num(); Idx++ )
		{
			// Remove from the list
			for (INT RefIdx = 0; RefIdx < CoverLinkRefs.Num(); RefIdx++)
			{
				if (CoverLinkRefs(RefIdx).Actor ==  Links(Idx) ||
					CoverLinkRefs(RefIdx).Guid  == *Links(Idx)->GetGuid())
				{
					CoverLinkRefs.Remove(RefIdx--,1);
					break;
				}
			}
		}
	}
	else 
	// Otherwise, fill by cylinder
	if( CGFA == CGFA_Cylinder )
	{
		FLOAT RadiusSq = AutoSelectRadius * AutoSelectRadius;
		for( FActorIterator It; It; ++It )
		{
			ANavigationPoint *Nav = Cast<ANavigationPoint>(*It);
			if ( !Nav )
			{
				continue;
			}

			ACoverLink* Link = Cast<ACoverLink>(Nav);
			if( !Link )
			{
				continue;
			}

			FVector LinkToGroup = Link->Location - Location;
			// If link is outside the vertical range of cylinder
			if( AutoSelectHeight > 0 )
			{
				if( Link->Location.Z > Location.Z ||
					LinkToGroup.Z < -AutoSelectHeight )
				{
					continue;
				}
			}
			else
			{
				if( Link->Location.Z < Location.Z ||
					LinkToGroup.Z > -AutoSelectHeight )
				{
					continue;
				}
			}
					
			// If link is outside lateral range of cylinder
			FLOAT DistSq2D = LinkToGroup.SizeSquared2D();
			if( DistSq2D > RadiusSq )
			{
				continue;
			}

			// Link is within cylinder
			// Add to list of links
			CoverLinkRefs.AddUniqueItem( FActorReference(Link,*Link->GetGuid()) );
		}
	}

	ForceUpdateComponents(FALSE,FALSE);
}

/**
 * @return	TRUE if the elements of the specified vector have the same value, within the specified tolerance.
 */
static inline UBOOL AllComponentsEqual(const FVector& Vec, FLOAT Tolerance=KINDA_SMALL_NUMBER)
{
	return Abs( Vec.X - Vec.Y ) < Tolerance && Abs( Vec.X - Vec.Z ) < Tolerance && Abs( Vec.Y - Vec.Z ) < Tolerance;
}

void ACoverGroup::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	if ( bCtrlDown )
	{
		// CTRL+Scaling modifies AutoSelectHeight.  This is for convenience, so that height
		// can be changed without having to use the non-uniform scaling widget (which is
		// inaccessable with spacebar widget cycling).
		AutoSelectHeight += ModifiedScale.X;
		AutoSelectHeight = Max( 0.0f, AutoSelectHeight );
	}
	else
	{
		AutoSelectRadius += ModifiedScale.X;
		AutoSelectRadius = Max( 0.0f, AutoSelectRadius );

		// If non-uniformly scaling, Z scale affects height and Y can affect radius too.
		if ( !AllComponentsEqual(ModifiedScale) )
		{
			AutoSelectHeight += -ModifiedScale.Z;
			AutoSelectHeight = Max( 0.0f, AutoSelectHeight );

			AutoSelectRadius += ModifiedScale.Y;
			AutoSelectRadius = Max( 0.0f, AutoSelectRadius );
		}
	}
}
