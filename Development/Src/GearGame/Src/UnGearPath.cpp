//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================
#include "GearGame.h"
#include "UnPath.h"

IMPLEMENT_CLASS(AGearWallPathNode);
IMPLEMENT_CLASS(AGearAirPathNode);
IMPLEMENT_CLASS(AJumpPoint);
IMPLEMENT_CLASS(ACoverLink_Dynamic);
IMPLEMENT_CLASS(ACoverSlotMarker_Dynamic);

INT ACoverLink_Dynamic::AddMyMarker(AActor *S)
{
	for (INT Idx = 0; Idx < Slots.Num(); Idx++)
	{
		FCoverSlot &Slot = Slots(Idx);
		AScout* Scout  = Cast<AScout>(S);

		// create a marker at a location pushed out from cover surface
		FRotator SpawnRot = GetSlotRotation(Idx);
		FVector  SpawnLoc = GetSlotLocation(Idx);

		Slot.SlotMarker = Cast<ACoverSlotMarker>(GWorld->SpawnActor(ACoverSlotMarker_Dynamic::StaticClass(),NAME_None,SpawnLoc,SpawnRot,NULL,TRUE,FALSE,this));
		if (Slot.SlotMarker != NULL)
		{
			// update the marker to point to us
			Slot.SlotMarker->OwningSlot.Link = this;
			Slot.SlotMarker->OwningSlot.SlotIdx = Idx;
		}
	}
	return Slots.Num();
}

void ACoverLink_Dynamic::PostBeginPlay()
{
	Super::PostBeginPlay();
	// if we are based on an actor
	if (Base != NULL)
	{
		// force each slot marker to share the same base
		for (INT Idx = 0; Idx < Slots.Num(); Idx++)
		{
			if (Slots(Idx).SlotMarker != NULL)
			{
				ACoverSlotMarker *Marker = Slots(Idx).SlotMarker;
				if ( Marker->Base != Base)
				{
					GWorld->FarMoveActor(Slots(Idx).SlotMarker,GetSlotLocation(Idx),FALSE,TRUE,FALSE);
					//debugf(TEXT("forcing %s base to %s"),*Marker->GetName(),*Base->GetName());
					Marker->SetBase(Base);
					Marker->setPhysics(PHYS_Interpolating);
					Marker->ForceUpdateComponents(FALSE);
				}
			}
		}
	}
}

void ACoverLink_Dynamic::UpdateCoverLink()
{
	/*
	TArray<ANavigationPoint*> NavList;
	NavList.AddItem(this);
	// remove from octree so it knows we moved and
	RemoveFromNavigationOctree();

	if( bClearPaths )
	{
		PathList.Empty();
		for (INT Idx = 0; Idx < Slots.Num(); Idx++)
		{
			if (Slots(Idx).SlotMarker != NULL)
			{
				Slots(Idx).SlotMarker->RemoveFromNavigationOctree();
				Slots(Idx).SlotMarker->PathList.Empty();
				NavList.AddItem(Slots(Idx).SlotMarker);
			}
		}

		// remove any paths to this cover
		for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
		{
			for (INT PathIdx = 0; PathIdx < Nav->PathList.Num(); PathIdx++)
			{
				UReachSpec *Spec = Nav->PathList(PathIdx);
				if (Spec != NULL)
				{
					for (INT NavIdx = 0; NavIdx < NavList.Num(); NavIdx++)
					{
						if (Spec->End == NavList(NavIdx))
						{
							Nav->PathList.Remove(PathIdx--,1);
							break;
						}
					}
				}
			}
		}
	}
	*/

	// save the last update location
	LastStableLocation = Location;
	// move the slot markers
	for (INT Idx = 0; Idx < Slots.Num(); Idx++)
	{
		if (Slots(Idx).SlotMarker != NULL)
		{
			ACoverSlotMarker *Marker = Slots(Idx).SlotMarker;
			if (Base != NULL && Marker->Base != Base)
			{
				// first move it to the correct location
				GWorld->FarMoveActor(Slots(Idx).SlotMarker,GetSlotLocation(Idx),FALSE,TRUE,FALSE);
				//debugf(TEXT("forcing %s base to %s"),*Marker->GetName(),*Base->GetName());
				Marker->SetBase(Base);
				Marker->setPhysics(PHYS_Interpolating);
				Slots(Idx).SlotMarker->ForceUpdateComponents(FALSE);
			}
			else
			// if there is no base then just update the current position
			if (Base == NULL)
			{
				GWorld->FarMoveActor(Slots(Idx).SlotMarker,GetSlotLocation(Idx),FALSE,TRUE,FALSE);
			}
		}
	}

	/*
	// look for nearby nodes to link to
	if (bAddToPathNetwork)
	{
		AScout *Scout = FPathBuilder::GetScout();
		if (Scout != NULL)
		{
			UReachSpec *NewSpec = NULL;
			for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
			{
				if (!Nav->bBlocked)
				{
					for (INT NavIdx = 0; NavIdx < NavList.Num(); NavIdx++)
					{
						ANavigationPoint *EndNav = NavList(NavIdx);
						if (EndNav != Nav && EndNav->CanConnectTo(Nav,TRUE))
						{
							if (NewSpec == NULL)
							{
								NewSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GWorld,NAME_None);
							}
							if (NewSpec->defineFor(EndNav,Nav,Scout))
							{
								//debugf(TEXT("- %s defined new path to %s"),*GetName(),*Nav->GetName());
								EndNav->PathList.AddItem(NewSpec);
								// add a path to the other end as well
								NewSpec = ConstructObject<UReachSpec>(Scout->GetDefaultReachSpecClass(),GWorld,NAME_None);
								if (NewSpec->defineFor(Nav,EndNav,Scout))
								{
									Nav->PathList.AddItem(NewSpec);
									NewSpec = NULL;
								}
							}
						}
					}
				}
			}
			// rebuild all the fire links
			BuildFireLinks( Scout );
			// clean up the scout
			FPathBuilder::DestroyScout();
		}
	}

	// re-add to the nav octree so it knows we moved
	
	RemoveFromNavigationOctree();
	AddToNavigationOctree();
	for (INT Idx = 0; Idx < Slots.Num(); Idx++)
	{
		if (Slots(Idx).SlotMarker != NULL)
		{
			Slots(Idx).SlotMarker->RemoveFromNavigationOctree();
			Slots(Idx).SlotMarker->AddToNavigationOctree();
		}
	}
	*/
}

#define PATH_UPDATE_DIST MAXPATHDIST
#define MAX_DYNAMIC_PATHS 3
#define MAX_RAYCASTS 10

ANavigationPoint* GetLongestSpecDist(ANavigationPoint* Nav, FLOAT& WorstSq)
{
	FLOAT CurrentSq = -1.f;
	WorstSq = -1.f;
	INT WorstIdx = -1;
	check(Nav);
	for(INT Idx=0;Idx<Nav->PathList.Num(); Idx++)
	{
		if(Nav->PathList(Idx)->IsA(UMantleReachSpec::StaticClass()))
		{
			continue;
		}

		CurrentSq = (Nav->Location - Nav->PathList(Idx)->End->Location).SizeSquared();

		if(CurrentSq > WorstSq || WorstSq < 0.f)
		{
			WorstSq = CurrentSq;
			WorstIdx = Idx;
		}
	}

	return (WorstIdx>=0) ? Nav->PathList(WorstIdx)->End.Nav() : NULL;
}

void RemoveSpecBetweenAandB(ANavigationPoint* A, ANavigationPoint* B)
{
	if(A != NULL && B != NULL)
	{
		// first find a spec from A to B
		for(INT AIdx=0;AIdx<A->PathList.Num();AIdx++)
		{
			UReachSpec* FwdSpec = NULL;
			if(A->PathList(AIdx)->End.Nav() == B)
			{
				FwdSpec = A->PathList(AIdx);
				//check(!FwdSpec->IsA(UMantleReachSpec::StaticClass()));

				if(FwdSpec->IsA(UMantleReachSpec::StaticClass()))
				{
#if !FINAL_RELEASE
					warnf(TEXT("RemoveSpecBetweenAandB tried to delete a mantlespec?!?! (%s->%s) %s"),*A->GetName(),*B->GetName(),*FwdSpec->GetName());
#endif
					continue;
				}

				FwdSpec->RemoveFromNavigationOctree();
				A->PathList.Remove(AIdx);
			}
		}

		// now remove spec from B to A
		for(INT BIdx=0;BIdx<B->PathList.Num();BIdx++)
		{
			UReachSpec* BackSpec = NULL;
			if(B->PathList(BIdx)->End.Nav() == A)
			{
				BackSpec = B->PathList(BIdx);
				//check(!BackSpec->IsA(UMantleReachSpec::StaticClass()));
				if(BackSpec->IsA(UMantleReachSpec::StaticClass()))
				{
#if !FINAL_RELEASE
					warnf(TEXT("RemoveSpecBetweenAandB tried to delete a mantlespec?!?! (%s->%s) %s"),*A->GetName(),*B->GetName(),*BackSpec->GetName());
#endif
					continue;
				}
				BackSpec->RemoveFromNavigationOctree();
				B->PathList.Remove(BIdx);
			}
		}
	}
}


void ACoverLink_Dynamic::UpdateCoverSlot( INT SlotIdx, UBOOL bUpdateOctree, AScout* Scout )
{
	check(Slots(SlotIdx).SlotMarker != NULL);

	ACoverSlotMarker* Marker = Slots(SlotIdx).SlotMarker;
	if(bUpdateOctree)
	{
		Marker->RemoveFromNavigationOctree();
		Marker->AddToNavigationOctree();
	}

	// remove non-mantle specs
	UReachSpec* Spec = NULL;
	for(INT Idx=Marker->PathList.Num()-1; Idx >= 0; Idx--)
	{
		Spec = Marker->PathList(Idx);
		UMantleReachSpec* MantleSpec = Cast<UMantleReachSpec>(Spec);
		if(MantleSpec != NULL )
		{
			MantleSpec->ReInitialize();
		}		
		else 
		{
			RemoveSpecBetweenAandB(Marker,Spec->End.Nav());		
		}
			
	}
	
	if(Role==ROLE_Authority)
	{
		// Find all nav objects within PATH_UPDATE_DIST, and check to see if we should connect to them
		UBOOL bCreatedScout = FALSE;
		if(Scout == NULL)
		{
			bCreatedScout=TRUE;
			Scout = FPathBuilder::GetScout();
		}
		TArray<FNavigationOctreeObject*> NavObjects;  
		GWorld->NavigationOctree->RadiusCheck(GetSlotLocation(SlotIdx), PATH_UPDATE_DIST, NavObjects);

		FLOAT CurSizeSq = 0.f;
		FLOAT WorstSizeSq = -1.f;
		ANavigationPoint* WorstNav = NULL;

		INT NumRaycastsDone = 0;
		ANavigationPoint* CurNav = NULL;
		for(INT Idx = 0; Idx < NavObjects.Num(); Idx++)
		{
			CurNav = NavObjects(Idx)->GetOwner<ANavigationPoint>();
			if(CurNav != NULL && CurNav != this && CurNav != Marker && 
				//don't link to other dynamic cover
				!CurNav->IsA(ACoverSlotMarker_Dynamic::StaticClass())
				)
			{
				if(CurNav->CanConnectTo(Marker,TRUE))
				{
					CurSizeSq = (CurNav->Location - Marker->Location).SizeSquared();	

					// if we're at the cap, then only proceed if this spec is shorter than our current worst	
					if( Marker->PathList.Num() < MAX_DYNAMIC_PATHS || CurSizeSq < WorstSizeSq)
					{
						FVector TryExtent = Scout->GetSize(FName(TEXT("Common")));
						TryExtent.Z = TryExtent.Y;
						TryExtent.Y = TryExtent.X;

						TryExtent.Z *= 0.5f;
						FCheckResult Hit(1.f);
						if(GWorld->SingleLineCheck(Hit,Scout,Marker->Location,CurNav->Location,TRACE_StopAtAnyHit|(TRACE_AllBlocking&~TRACE_Pawns),TryExtent))
						{
							// make sure there is no pit!
							FVector Dir, Pos, End;
							Pos = CurNav->Location;
							End = Marker->Location;
							Dir = (End - Pos).SafeNormal();
							UBOOL bFoundPit = FALSE;
							// pit tests are for sissies
							//while(((End-Pos) | Dir) > 0.f)
							//{
							//	//CurNav->DrawDebugLine(Pos-FVector(0.f,0.f,TryExtent.Z*2.f),Pos,255,255,0,TRUE);
							//	if(GWorld->SingleLineCheck(Hit,Scout,Pos-FVector(0.f,0.f,TryExtent.Z*2.f),Pos,TRACE_StopAtAnyHit|TRACE_World,TryExtent*0.25f))
							//	{
							//		bFoundPit=TRUE;
							//		break;
							//	}
							//	Pos += Dir * 600.f;
							//}
							if(!bFoundPit)
							{
								if(ConditionalLinkSlotMarkerTo(Scout,Marker,CurNav))
								{
									// if our current size is bigger than our worst, we have no paths yet or we're not at the cap
									if(CurSizeSq > WorstSizeSq)
									{
										WorstSizeSq = CurSizeSq;
										WorstNav = CurNav;
									}
									else
									{
										// otherwise we are adding this in place of another one in the list.. remove the worst and calc the new worst
										RemoveSpecBetweenAandB(Marker,WorstNav);
										WorstNav = GetLongestSpecDist(Marker,WorstSizeSq);
										check(WorstNav);
									}
								}
							}

						}						
						if(++NumRaycastsDone > MAX_RAYCASTS)
						{
							break;
						}
					}

				}
			}
		}

		if(bCreatedScout)
		{
			FPathBuilder::DestroyScout();
		}
	}

}

void ForceBiDirLinkFromAToB(ANavigationPoint* A, ANavigationPoint* B, AScout* Scout)
{
	UReachSpec* NewSpec = A->ForcePathTo(B,Scout,Scout->GetDefaultReachSpecClass());
	NewSpec->bAddToNavigationOctree = TRUE;
	NewSpec->AddToNavigationOctree();
	NewSpec = B->ForcePathTo(A,Scout,Scout->GetDefaultReachSpecClass());
	NewSpec->bAddToNavigationOctree = TRUE;
	NewSpec->AddToNavigationOctree();
}

UBOOL ACoverLink_Dynamic::ConditionalLinkSlotMarkerTo(AScout* Scout, ACoverSlotMarker* Marker, ANavigationPoint* PtToLink)
{
	// first figure out if marker is already linked to another node which is also linked to PtToLink, but closer
	// for each spec in the node we're considering,
	for(INT LinkIdx=0; LinkIdx < PtToLink->PathList.Num(); LinkIdx++)
	{
		ANavigationPoint* IntermediateNode = PtToLink->PathList(LinkIdx)->End.Nav();
		if (IntermediateNode == NULL)
		{
			continue;
		}
		// see if the node on the other end of this spec is already linked to Marker
		UReachSpec* SpecBackToMarker = IntermediateNode->GetReachSpecTo(Marker);

		if(SpecBackToMarker == NULL ||
			SpecBackToMarker->BlockedBy != NULL ||
			SpecBackToMarker->Start->bBlocked ||
			SpecBackToMarker->End.Nav()->bBlocked
			)
		{
			continue;
		}

		// don't mess with mantle specs!
		if(SpecBackToMarker->IsA(UMantleReachSpec::StaticClass()))
		{
			return FALSE;
		}

		FLOAT DistPctCompare = SpecBackToMarker->Distance / (Marker->Location - PtToLink->Location).Size();
		// if the existing spec is less than 80% of the distance than it would be to go to the new node, bail		
		if(DistPctCompare < 0.8f)
		{
			return FALSE;
		}

		// if the old connection is more than 20% longer than the new one, remove it (so that specs that are close to the same distance will both be kept)
		if(DistPctCompare > 1.2f)
		{
			RemoveSpecBetweenAandB(Marker,SpecBackToMarker->Start);
		}
		// otherwise, add a link!
		ForceBiDirLinkFromAToB(Marker,PtToLink, Scout);
		return TRUE;
	}

	// if we got this far, then there were no redundant links found.. so add a new link
	ForceBiDirLinkFromAToB(Marker,PtToLink,Scout);
	return TRUE;
}

UBOOL ACoverLink_Dynamic::HasFireLinkTo( INT SlotIdx, FCoverInfo &ChkCover, UBOOL bAllowFallbackLinks )
{
	TArray<INT> Items;
	INT FireLinkIdx = 0;
	UBOOL bResult = Super::GetFireLinkTo( SlotIdx, ChkCover, 0, 0, FireLinkIdx, Items );

	return bResult;
}

void ACoverSlotMarker_Dynamic::InitializeDynamicMantleSpec(UClass* SpecClass)
{
	FCoverSlot* Slot = ACoverLink::CoverInfoToSlotPtr( OwningSlot );
	if( !Slot )
	{
		return;
	}

	// remove all mantle specs in list currently
	for( INT Idx = 0; Idx < PathList.Num(); Idx++ )
	{
		UReachSpec* Spec = PathList(Idx);
		if( Cast<UMantleReachSpec>(Spec) )
		{
			PathList.Remove( Idx-- );
		}
	}

	// if we have a valid mantle target add a mantle reach spec for it
	if( Slot->MantleTarget.SlotIdx >= 0 )
	{
		AScout* Scout = FPathBuilder::GetScout();
		if(Scout == NULL)
		{
			return;
		}

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

			UReachSpec* NewSpec = ForcePathTo( TargSlot->SlotMarker, Scout, SpecClass  );
			NewSpec->bAddToNavigationOctree = TRUE;
			NewSpec->AddToNavigationOctree();
		}
	}
}

void ACoverSlotMarker_Dynamic::SetMaxPathSize(FLOAT Radius, FLOAT Height)
{
	MaxPathSize.Height = Height;
	MaxPathSize.Radius = Radius;
}

void ACoverSlotMarker_Dynamic::RefreshOctreePosition()
{
	RemoveFromNavigationOctree();
	AddToNavigationOctree();
}

void AGearWallPathNode::AddForcedSpecs( AScout *Scout )
{
	FCheckResult Hit;

	for( ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav; Nav = Nav->nextNavigationPoint )
	{
		// If target is a jump point or cover link AND 
		// there is a clear line
		AJumpPoint* JP	 = Cast<AJumpPoint>(Nav);
		ACoverLink* Link = Cast<ACoverLink>(Nav);
		if( (JP || Link) &&
			GWorld->SingleLineCheck( Hit, this, Location, Nav->Location, TRACE_World|TRACE_StopAtAnyHit ) )
		{
			FVector Normal = -Rotation.Vector();
			FLOAT DistSq = (Nav->Location-Location).SizeSquared();

			// Wall node is on a wall
			if( Normal.Z > -Scout->WalkableFloorZ && 
				Normal.Z <  Scout->WalkableFloorZ )
			{
				// Make sure it is close enough to jump to
				if( DistSq < MaxJumpDist * MaxJumpDist )
				{
					// Force path in both directions
					ForcePathTo( Nav, Scout, UWallTransReachSpec::StaticClass() );
					Nav->ForcePathTo( this, Scout, UWallTransReachSpec::StaticClass() );
				}
			}
			// Otherwise, assume wall node is on the ceiling
			// and check to see if nodes are over top eachother
			else if( (Nav->Location-Location).SizeSquared2D() <= Nav->CylinderComponent->CollisionRadius * Nav->CylinderComponent->CollisionRadius )
			{
				if( DistSq < MaxJumpDist * MaxJumpDist )
				{
					// Force path in both directions
					ForcePathTo( Nav, Scout, UFloorToCeilingReachSpec::StaticClass() );
					Nav->ForcePathTo( this, Scout, UFloorToCeilingReachSpec::StaticClass() );
				}
			}
		}
	}
}

void AGearWallPathNode::FindBase()
{
	SetZone( 1, 1 );

	// Not using find base, because don't want to fail if LD has navigationpoint slightly interpenetrating floor
	if( ShouldBeBased() )
	{
		FCheckResult Hit(1.f);

		AScout *Scout = FPathBuilder::GetScout();
		check(Scout != NULL && "Failed to find scout for point placement");

		// Get the dimensions for the average human player - hack to move point closer to wall
		FVector HumanSize = Scout->GetSize(FName(TEXT("Human"),FNAME_Find)) * 0.15f;
		FVector CollisionSlice( HumanSize.X, HumanSize.X, 1.f );

		// Use this node's smaller collision radius if possible
		if( CylinderComponent->CollisionRadius < HumanSize.X )
		{
			CollisionSlice.X = CollisionSlice.Y = CylinderComponent->CollisionRadius;
		}

		// Check for placement along rotation of the node
		GWorld->SingleLineCheck( Hit, this, Location + Rotation.Vector() * (4.f * CylinderComponent->CollisionHeight), Location, TRACE_AllBlocking, CollisionSlice );
		if( Hit.Actor != NULL )
		{
			// If hit... move out from wall and set rotation of node to be into wall
			GWorld->FarMoveActor( this, Hit.Location + Hit.Normal * (CylinderComponent->CollisionHeight - 2.f), FALSE, TRUE, FALSE );
			SetRotation( (-Hit.Normal).Rotation() );
		}

		SetBase( Hit.Actor, Hit.Normal );
	}
}

UBOOL AGearWallPathNode::IsUsableAnchorFor(APawn* P)
{
	return (P->Physics == PHYS_Spider && Super::IsUsableAnchorFor( P ));
}

UBOOL AGearWallPathNode::CanConnectTo(ANavigationPoint* Dest, UBOOL bCheckDistance)
{
	return (Cast<AGearWallPathNode>(Dest) &&
			 Super::CanConnectTo( Dest, bCheckDistance ));
}

UBOOL AGearAirPathNode::CanConnectTo(ANavigationPoint* Dest, UBOOL bCheckDistance)
{
	return (Cast<AGearAirPathNode>(Dest) &&
			Super::CanConnectTo( Dest, bCheckDistance ));
}

