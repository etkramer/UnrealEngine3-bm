/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
#include "GearGame.h"
#include "DebugRenderSceneProxy.h"

#include "GearGameWeaponClasses.h"


IMPLEMENT_CLASS(ACombatZone)
IMPLEMENT_CLASS(AGearScout)
IMPLEMENT_CLASS(AFauxPathNode)
IMPLEMENT_CLASS(ALadderMarker)
IMPLEMENT_CLASS(ULeapReachSpec)

IMPLEMENT_CLASS(UCombatZoneRenderingComponent)

IMPLEMENT_CLASS(UPath_WithinCombatZone)
IMPLEMENT_CLASS(UPath_PreferCover)
IMPLEMENT_CLASS(UPath_AvoidFireFromCover)
IMPLEMENT_CLASS(UPath_AvoidanceVolumes)
IMPLEMENT_CLASS(UPath_CoverSlotsOnly)
IMPLEMENT_CLASS(UGoal_AtCover)
IMPLEMENT_CLASS(UGoal_InCombatZone)
IMPLEMENT_CLASS(UGoal_FarthestNavInRange)
IMPLEMENT_CLASS(UGoal_SquadFormation)
IMPLEMENT_CLASS(UGoal_OutsideAvoidanceVolumes)
IMPLEMENT_CLASS(UGoal_SpawnPoints)
IMPLEMENT_CLASS(UGoal_AwayFromPosition)

// Cover constraints
IMPLEMENT_CLASS(UCoverGoalConstraint)
IMPLEMENT_CLASS(UCovGoal_Enemies)
IMPLEMENT_CLASS(UCovGoal_EnemyProximity)
IMPLEMENT_CLASS(UCovGoal_GoalProximity)
IMPLEMENT_CLASS(UCovGoal_MovementDistance)
IMPLEMENT_CLASS(UCovGoal_SquadLeaderProximity)
IMPLEMENT_CLASS(UCovGoal_TeammateProximity)
IMPLEMENT_CLASS(UCovGoal_WithinWeaponRange)
IMPLEMENT_CLASS(UCovGoal_AvoidanceVolumes)
IMPLEMENT_CLASS(UCovGoal_ProtectedByLocation)
IMPLEMENT_CLASS(UCovGoal_WithinAllowedCoverLinks)
IMPLEMENT_CLASS(UCovGoal_WithinCombatZones)

IMPLEMENT_CLASS(AGearNavModificationVolume)


INT AGearScout::PrunePathsForNav(ANavigationPoint* Nav)
{
	return Nav->AggressivePrunePaths();
}

INT AGearScout::SecondPassPrunePathsForNav(ANavigationPoint* Nav)
{
	return Nav->SecondPassAggressivePrunePaths();
}

void AGearScout::InitForPathing( ANavigationPoint* Start, ANavigationPoint* End )
{
	Super::InitForPathing( Start, End );

	AGearPawn* Default = Cast<AGearPawn>(AGearPawn::StaticClass()->GetDefaultActor());
	if( Default )
	{
		bJumpCapable	= Default->bJumpCapable;
		bCanJump		= Default->bCanJump;
		bStopAtLedges	= Default->bStopAtLedges;
		bAvoidLedges	= Default->bAvoidLedges;
		MaxStepHeight	= Default->MaxStepHeight;
	}

	if( Cast<AGearWallPathNode>(Start) && Cast<AGearWallPathNode>(End) )
	{
		Physics = PHYS_Spider;
	}
	else
		if( Cast<AGearAirPathNode>(Start) && Cast<AGearAirPathNode>(End) )
		{
			Physics = PHYS_Flying;
		}
		else
		{
			Physics = PHYS_Walking;
		}
}

UClass* AGearScout::GetDefaultReachSpecClass() 
{ 
	if( Physics == PHYS_Spider )
	{
		return UCeilingReachSpec::StaticClass();
	}
	return DefaultReachSpecClass; 
}

FVector AGearScout::GetSize(FName desc)
{
	if (appStricmp(*desc.ToString(),TEXT("Human")) == 0)
	{
		return Super::GetSize(FName(TEXT("Common")));
	}
	else
	{
		return Super::GetSize(desc);
	}
}

FVector AGearScout::GetDefaultForcedPathSize(UReachSpec* Spec)
{
	if( Spec->IsA(UForcedReachSpec::StaticClass())  ||
		Spec->IsA( ULadderReachSpec::StaticClass()) )
	{
		return GetSize(FName(TEXT("Max"), FNAME_Find));
	}
	else
	{
		return Super::GetDefaultForcedPathSize(Spec);
	}
}

void AGearScout::SetPathColor(UReachSpec* ReachSpec)
{
	INT BestColor = 0;
	for( INT Idx = 0; Idx < PathSizes.Num(); Idx++ )
	{
		if( (ReachSpec->CollisionHeight >= PathSizes(Idx).Height) && 
			(ReachSpec->CollisionRadius >= PathSizes(Idx).Radius) )
		{
			BestColor = PathSizes(Idx).PathColor;
		}
		else
		{
			break;
		}
	}

	ReachSpec->PathColorIndex = BestColor;
}


void AGearScout::Exec( const TCHAR* Str )
{
	if( ParseCommand( &Str, TEXT("ADJUSTCOVER") ) )
	{
		//debug
//		FlushPersistentDebugLines();

		UBOOL bFromDefinePaths = FALSE;

		ParseUBOOL( Str, TEXT("FROMDEFINEPATHS="), bFromDefinePaths );

		AdjustCover( bFromDefinePaths );
	}
	else
	if( ParseCommand( &Str, TEXT("BUILDCOMBATZONES") ) )
	{
		UBOOL bFromDefinePaths = FALSE;

		ParseUBOOL( Str, TEXT("FROMDEFINEPATHS="), bFromDefinePaths );

		BuildCombatZones( bFromDefinePaths );
	}
	else
	if( ParseCommand( &Str, TEXT("ADDLONGREACHSPECS") ) )
	{
		// Skip! - no long paths in gears
	}
	else
	{	
		Super::Exec( Str );
	}
}

void AGearScout::AddSpecialPaths( INT NumPaths, UBOOL bOnlyChanged )
{
	FCheckResult Hit(1.f);

	FLOAT OldJumpZ = JumpZ;
	JumpZ = 1000.f;

	for( ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav; Nav = Nav->nextNavigationPoint )
	{
		AJumpPoint* JumpPoint = Cast<AJumpPoint>(Nav);
		if( JumpPoint == NULL )
		{
			continue;
		}

		if( (!bOnlyChanged || Nav->bPathsChanged) && !Nav->bNoAutoConnect && !Nav->PhysicsVolume->bWaterVolume )
		{
			for( INT DestIdx = 0; DestIdx < JumpPoint->JumpDest.Num(); DestIdx++ )
			{
				ANavigationPoint* JumpDest = JumpPoint->JumpDest(DestIdx);
				if( JumpDest == NULL || JumpDest == JumpPoint )
				{
					JumpPoint->JumpDest.Remove( DestIdx--, 1 );
					continue;
				}

				CreateLeapPath( JumpPoint, JumpDest, Hit, bOnlyChanged );
			}
		}
	}

	JumpZ = OldJumpZ;
}

#define MAXJUMPPATHDIST		1024.f
#define MAXJUMPPATHDISTSQ	(MAXJUMPPATHDIST*MAXJUMPPATHDIST)

UBOOL AGearScout::CreateLeapPath( ANavigationPoint* Nav, ANavigationPoint* DestNav, FCheckResult Hit, UBOOL bOnlyChanged )
{
	if( DestNav->bNoAutoConnect || DestNav->bDestinationOnly || DestNav->bMakeSourceOnly )
	{
		return FALSE;
	}

	FLOAT Dist = (DestNav->Location - Nav->Location).Size();
	if( Dist*Dist > MAXJUMPPATHDISTSQ )
	{
		return FALSE;
	}

	if( Nav->CheckSatisfactoryConnection(DestNav) )
	{
		return FALSE;
	}

	FVector MaxSize = GetSize(TEXT("Max"));
	SetCollisionSize( MaxSize.X, MaxSize.Y );
	
	if( Nav->PlaceScout( this ) )
	{
		FVector JumpVel(0,0,0);
		FVector CurrentPosition = Location;
		if( DestNav->PlaceScout( this ) )
		{
			FVector DesiredPosition = Location;
			if( SuggestJumpVelocity( JumpVel, DesiredPosition, CurrentPosition )	&&
				FindBestJump( DesiredPosition, CurrentPosition ) == TESTMOVE_Moved	&&
				ReachedDestination( CurrentPosition, DesiredPosition, DestNav )	)
			{
				Nav->bPathsChanged = Nav->bPathsChanged || !bOnlyChanged;

				ULeapReachSpec* Spec	= ConstructObject<ULeapReachSpec>( ULeapReachSpec::StaticClass(), Nav->GetOuter(), NAME_None );
				Spec->CollisionRadius	= appTrunc(MaxSize.X);
				Spec->CollisionHeight	= appTrunc(MaxSize.Y);
				Spec->Start				= Nav;
				Spec->End				= DestNav;
				Spec->Distance			= appTrunc(Dist);
				Spec->CachedVelocity	= JumpVel;
				Spec->RequiredJumpZ		= JumpVel.Z;

				Nav->PathList.AddItem( Spec );

				return TRUE;
			}
		}
	}

	return FALSE;
}

FGuid* ACombatZone::GetGuid()
{
	return &ZoneGuid;
}

/** Handles adding all nav references to the list */
void ACombatZone::GetActorReferences(TArray<FActorReference*> &ActorRefs, UBOOL bIsRemovingLevel)
{
	Super::GetActorReferences(ActorRefs,bIsRemovingLevel);

	for( INT Idx = 0; Idx < MyNavRefs.Num(); Idx++ )
	{
		FActorReference &ActorRef = MyNavRefs(Idx);
		if( ActorRef.Guid.IsValid() )
		{
			if( ( bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL) )
			{
				ActorRefs.AddItem( &ActorRef );
			}
		}
	}

	for( INT Idx = 0; Idx < CoverSlotRefs.Num(); Idx++ )
	{
		FActorReference &ActorRef = CoverSlotRefs(Idx);
		if( ActorRef.Guid.IsValid() )
		{
			if( ( bIsRemovingLevel && ActorRef.Actor != NULL) ||
				(!bIsRemovingLevel && ActorRef.Actor == NULL) )
			{
				ActorRefs.AddItem( &ActorRef );
			}
		}
	}

	if( NetworkNode != NULL )
	{
		NetworkNode->GetActorReferences( ActorRefs, bIsRemovingLevel );
	}
}

/** Warn of zones that contain disjointed path networks */
void ACombatZone::CheckForErrors()
{
	Super::CheckForErrors();

	TArray<INT> ContainedNetworkIDs;
	for( INT Idx = 0; Idx < MyNavRefs.Num(); Idx++)
	{
		ANavigationPoint *Nav = Cast<ANavigationPoint>(~MyNavRefs(Idx));
		if( Nav != NULL )
		{
			ContainedNetworkIDs.AddUniqueItem( Nav->NetworkID );
		}
	}
	if( ContainedNetworkIDs.Num() > 1 )
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Contains references to navigation points in different navigation networks!")));
	}
}

// For each navigation point
// Figure out which zone it is in
// Add it to the zone nav list
// If the navigation point is a cover slot
// Add it to the zones cover list

// Create a FauxPathNode for each combat zone
// Check each navigation point in each zone
// If it has a reach spec to a nav point whose zone is not ours
// Create a reach spec from this zone to the other through the FauxNavPoint


/************************************************************************
* BuildCombatZones
*	Setups all combat zone information for the level at compile time
************************************************************************/
void AGearScout::BuildCombatZones( UBOOL bFromDefinePaths )
{
	GWarn->BeginSlowTask( *LocalizeUnrealEd(TEXT("BuildCombatZones")), 1);

	// Store if paths were correctly rebuilt
	UBOOL bPathsRebuilt = GWorld->GetWorldInfo()->bPathsRebuilt;

	TArray<ACombatZone*> Zones;
	TArray<FGuid> ZoneGuids;
	for( FActorIterator It; It; ++It )
	{
		ACombatZone* Zone = Cast<ACombatZone>(*It);
		if( Zone )
		{
			// Build a list of all combat zones
			Zones.AddItem( Zone );
			// Delete faux pathnode
			if( Zone->NetworkNode != NULL )
			{
				GWorld->DestroyActor( Zone->NetworkNode );
			}

			// Empty all ref lists
			Zone->MyNavRefs.Empty();
			Zone->CoverSlotRefs.Empty();
			Zone->ZoneCenter = FVector(0,0,0);

			if( !Zone->GetGuid()->IsValid() || ZoneGuids.ContainsItem(*Zone->GetGuid()) )
			{
				Zone->ZoneGuid = appCreateGuid();
			}
			else
			{
				// save the existing guid to check for duplicates
				ZoneGuids.AddItem(*Zone->GetGuid());
			}
		}
		else
		{
			ANavigationPoint* Nav = Cast<ANavigationPoint>(*It);
			if( Nav )
			{
				// Clear volume list for all nav points
				Nav->Volumes.Empty();
			}
		}
	}

	// For each navigation point in the world
	for( ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav; Nav = Nav->nextNavigationPoint )
	{
		// If point has already been processed by a combat zone - skip
		ACombatZone* Zone = ACombatZone::GetCombatZoneForNavPoint( Nav );
		if( Zone )
			continue;

		// For each combat zone in the world
		for( INT ZoneIdx = 0; ZoneIdx < Zones.Num(); ZoneIdx++ )
		{
			ACombatZone* Zone = Zones(ZoneIdx);

			// If combat zone contains the nav point
			if( Zone->Encompasses( Nav->Location ) )
			{
				// Mark nav point with this zone
				Nav->Volumes.AddItem( FActorReference(Zone, *Zone->GetGuid()) );
				// Store nav point in zone nav list
				Zone->MyNavRefs.AddItem( FActorReference(Nav, *Nav->GetGuid()) );
				Zone->ZoneCenter += Nav->Location;

				// If it is a cover slot
				ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(Nav);
				if( Marker )
				{
					// Store reference to the marker
					Zone->CoverSlotRefs.AddItem( FActorReference(Marker, *Marker->GetGuid()) );
				}

				// Navigation point can only be a member of one combat zone
				break;
			}
		}
	}
	
	for( INT ZoneIdx = 0; ZoneIdx < Zones.Num(); ZoneIdx++ )
	{
		ACombatZone* Zone = Zones(ZoneIdx);
		if( Zone )
		{
			// Average the center
			Zone->ZoneCenter /= Zone->MyNavRefs.Num();

			// Create new node on the network (node is owned by the zone)
			Zone->NetworkNode = Cast<AFauxPathNode>(GWorld->SpawnActor( AFauxPathNode::StaticClass(), NAME_None, Zone->Location, Zone->Rotation, NULL, TRUE, FALSE, Zone ));
			Zone->NetworkNode->NavGuid = appCreateGuid();
		}
	}

	// Go through each zone and create paths to adjacent zones
	for( INT ZoneIdx = 0; ZoneIdx < Zones.Num(); ZoneIdx++ )
	{
		ACombatZone* Zone = Zones(ZoneIdx);
		for( INT NavIdx = 0; NavIdx < Zone->MyNavRefs.Num(); NavIdx++ )
		{
			ANavigationPoint* Nav = Cast<ANavigationPoint>(~Zone->MyNavRefs(NavIdx));
			for( INT PathIdx = 0; PathIdx < Nav->PathList.Num(); PathIdx++ )
			{
				ACombatZone* OtherZone = ACombatZone::GetCombatZoneForNavPoint( Nav->PathList(PathIdx)->End.Nav() );
				if( OtherZone != NULL && Zone != OtherZone && !Zone->IsAdjacentZone( OtherZone ) )
				{
					// Force adjacency path between the zones
					Zone->NetworkNode->ForcePathTo( OtherZone->NetworkNode, this, UForcedReachSpec::StaticClass() );
					Zone->ConditionalForceUpdateComponents(FALSE,TRUE);
				}
			}
		}
	}

	// Set paths rebuilt flag
	GWorld->GetWorldInfo()->bPathsRebuilt = bPathsRebuilt; 

	GWarn->EndSlowTask();
}

/************************************************************************
 * GetCombatZoneForNavPoint
 *	Returns the known combat zone encompassing the nav point (if any)
 ************************************************************************/
ACombatZone* ACombatZone::GetCombatZoneForNavPoint( ANavigationPoint* Nav )
{
	ACombatZone* Zone = NULL;
	if( Nav )
	{
		for( INT Idx = 0; Idx < Nav->Volumes.Num() && !Zone; Idx++ )
		{
			Zone = ExactCast<ACombatZone>(Nav->Volumes(Idx));
		}
	}
	return Zone;
}

/************************************************************************
 * IsNavWithin (static)
 *	Returns if given navigtaion point is within the given combat zone
 ************************************************************************/
UBOOL ACombatZone::IsNavWithin( ANavigationPoint* Nav, ACombatZone* Zone )
{
	if( Nav != NULL && Zone != NULL )
	{
		for( INT VolIdx = 0; VolIdx < Nav->Volumes.Num(); VolIdx++ )
		{
			ACombatZone* CZ = Cast<ACombatZone>(Nav->Volumes(VolIdx));
			if( CZ == Zone )
			{
				return TRUE;
			}
		}			
	}

	return FALSE;
}

/************************************************************************
 * IsAdacentZone
 *	Checks if given zone connected through the combat zone path network
 ************************************************************************/
UBOOL ACombatZone::IsAdjacentZone( ACombatZone* OtherZone )
{
	if( NetworkNode != NULL && OtherZone != NULL )
	{
		for( INT Idx = 0; Idx < NetworkNode->PathList.Num(); Idx++ )
		{
			if( NetworkNode->PathList(Idx)->End == OtherZone->NetworkNode )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/************************************************************************
 * CheckForMovementDelay                                                
 *	Checks if given AI should stop moving until occupancy allows him to move through the zone
 *	Returns TRUE if AI should delay, FALSE otherwise
 *	Also handles bookkeeping of PendingOccupants/Occupants_* changes so AI moves in an orderly fashion
 ************************************************************************/
UBOOL ACombatZone::CheckForMovementDelay( AGearAI* AI )
{
	if(AI == NULL || AI->Pawn == NULL)
	{
		return FALSE;
	}
	
	INT PendingIdx = PendingOccupants.FindItemIndex( AI->Pawn );
	if( bDelayMovesAtMaxOccupancy &&  (DelayMovesForTeam == TEAM_EVERYONE || AI->Pawn->GetTeamNum() == DelayMovesForTeam) )
	{
		if( !IsValidZoneFor( AI, FALSE ) )
		{
			if( PendingIdx < 0 )
			{
				//debug
				//`log( self@GetFuncName()@"Add pending occupant"@AI@AI.Pawn@PendingOccupants.Length, bDebug );

				PendingOccupants.AddItem(AI->Pawn);
			}
			return TRUE;
		}
	}

	if( PendingIdx >= 0 )
	{
		//debug
		//`log( self@GetFuncName()@"Release pending occupant"@AI@AI.Pawn@PendingOccupants.Length, bDebug );

		PendingOccupants.Remove( PendingIdx, 1 );
		eventAddOccupant( AI->Pawn );
	}

	return FALSE;
}

/************************************************************************
* IsValidZoneFor
*	Checks if AI can occupy or reside in this zone
************************************************************************/
UBOOL ACombatZone::IsValidZoneFor( AGearAI* AI, UBOOL bResidenceQuery )
{
	INT Total = 0;

	// Reject if disabled 
	if( !bEnabled )
	{
		return FALSE;
	}

	if( bResidenceQuery )
	{
		if( MaxResidents > 0 )
		{
			Total = Residents_COG.Num() + Residents_Locust.Num();
			if( Total >= MaxResidents && !IsResident( AI->Pawn ) )
			{
				// Don't allow new residents
				return FALSE;
			}
		}		
	}
	else
	{
		// If designer specified max and we are over that max
		if( MaxOccupants > 0 )
		{
			Total = Occupants_COG.Num() + Occupants_Locust.Num();

			// Don't allow new occupants
			if( Total >= MaxOccupants && !IsOccupant( AI->Pawn ) )
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

UBOOL ACombatZone::IsOccupant( APawn* P )
{
	if( P != NULL )
	{
		INT TeamIdx = P->GetTeamNum();
		if( TeamIdx == UCONST_COG_TEAM_INDEX )
		{
			return (Occupants_COG.FindItemIndex( P ) >= 0);
		}
		else
		if( TeamIdx == UCONST_LOCUST_TEAM_INDEX )
		{
			return (Occupants_Locust.FindItemIndex( P ) >= 0);
		}
	}

	return FALSE;
}

UBOOL ACombatZone::IsResident( APawn* P )
{
	if( P != NULL )
	{
		INT TeamIdx = P->GetTeamNum();
		if( TeamIdx == UCONST_COG_TEAM_INDEX )
		{
			return (Residents_COG.FindItemIndex( P ) >= 0);
		}
		else
		if( TeamIdx == UCONST_LOCUST_TEAM_INDEX )
		{
			return (Residents_Locust.FindItemIndex( P ) >= 0);
		}
	}

	return FALSE;
}


void ACombatZone::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove( bFinished );

	if( bFinished  && NetworkNode != NULL )
	{
		for( INT PathIdx = 0; PathIdx < NetworkNode->PathList.Num(); PathIdx++ )
		{
			ACombatZone* OtherZone = Cast<ACombatZone>((~NetworkNode->PathList(PathIdx)->End)->Owner);
			if( OtherZone )
			{
				OtherZone->ConditionalForceUpdateComponents(FALSE,TRUE);
			}
		}
	}
}

/** Represents a CombatZoneRenderingComponent to the scene manager. */
class FCombatZoneRenderingSceneProxy : public FDebugRenderSceneProxy
{
public:

	FCombatZoneRenderingSceneProxy(const UCombatZoneRenderingComponent* InComponent):
	  FDebugRenderSceneProxy(InComponent)
	  {
		  ACombatZone *Zone = Cast<ACombatZone>(InComponent->GetOwner());
		  check(Zone);

		  if( Zone != NULL && Zone->Brush != NULL )
		  {
			  FVector ZoneCenter = Zone->Brush->Bounds.Origin + Zone->Location;
			  for( INT RefIdx = 0; RefIdx < Zone->CoverSlotRefs.Num(); RefIdx++ )
			  {
				  ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(~Zone->CoverSlotRefs(RefIdx));

				  // If link is valid
				  if( Marker && (Marker->IsSelected() || Zone->IsSelected()) )
				  {
					  // Red link if disabled
					  const FColor MarkerColor = FColor(255,0,0);

					  // Draw line to it
					  new(DashedLines) FDashedLine( ZoneCenter, Marker->Location, MarkerColor, 32.f );
				  }
			  }

			  for( INT AmbushIdx = 0; AmbushIdx < Zone->AmbushTargets.Num(); AmbushIdx++ )
			  {
				  if( Zone->AmbushTargets(AmbushIdx) == NULL )
				  {
					  // In the editor, we can't remove these property array elements at render-time,
					  // since a user might be interacting with the property in the GUI
					  if( !GIsEditor )
					  {
						  Zone->AmbushTargets.Remove(AmbushIdx--);
						  continue;
					  }
				  }
				  else
				  {
					  const FColor AmbushColor = FColor(0,0,255);
					  // Draw line to it
					  new(DashedLines) FDashedLine( ZoneCenter, Zone->AmbushTargets(AmbushIdx)->Location, AmbushColor, 32.f );
				  }
			  }				

			  if( Zone->NetworkNode != NULL && Zone->bDebug )
			  {
				  for( INT PathIdx = 0; PathIdx < Zone->NetworkNode->PathList.Num(); PathIdx++ )
				  {
					  ACombatZone* OtherZone = Cast<ACombatZone>(Zone->NetworkNode->PathList(PathIdx)->End->Owner);
					  if( OtherZone != NULL )
					  {
						FVector OtherCenter = OtherZone->Brush->Bounds.Origin + OtherZone->Location;
						new(DashedLines) FDashedLine( ZoneCenter, OtherCenter, FColor(0,0,255), 32.f );
					  }
				  }
			  }
		  }
	  }

	  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	  {
		  FPrimitiveViewRelevance Result;
		  Result.bDynamicRelevance = IsShown(View);
		  Result.SetDPG(SDPG_World,TRUE);
		  return Result;
	  }

	  virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	  virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	  DWORD GetAllocatedSize( void ) const { return( FDebugRenderSceneProxy::GetAllocatedSize() ); }
};

void UCombatZoneRenderingComponent::UpdateBounds()
{
	FBox BoundingBox(0);
	ACombatZone *Zone = Cast<ACombatZone>(Owner);

	if( Zone )
	{
		FBox ComponentBox;
		Zone->GetComponentsBoundingBox( ComponentBox );
		BoundingBox += ComponentBox;
	}
	Bounds = FBoxSphereBounds(BoundingBox.ExpandBy(64.f));
}

FPrimitiveSceneProxy* UCombatZoneRenderingComponent::CreateSceneProxy()
{
	return new FCombatZoneRenderingSceneProxy(this);
}

UBOOL UCombatZoneRenderingComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	// The cover group scene proxy caches a lot of transform dependent info, so it's easier to just recreate it when the transform changes.
	return TRUE;
}

void ATrigger_LadderInteraction::GetTopEntryPoint( FVector& out_Loc, FRotator& out_Rot )
{
	if( LadderSMActor != NULL )
	{
		out_Rot = (LadderSMActor->Rotation + TopRotOffset).GetNormalized();
		out_Loc =  LadderSMActor->Location + FRotationMatrix(out_Rot).TransformNormal( TopLocOffset );
	}
}

void ATrigger_LadderInteraction::GetBottomEntryPoint( FVector& out_Loc, FRotator& out_Rot )
{
	if( LadderSMActor != NULL )
	{
		out_Rot = (LadderSMActor->Rotation + BottomRotOffset).GetNormalized();
		out_Loc =  LadderSMActor->Location + FRotationMatrix(out_Rot).TransformNormal(BottomLocOffset);
	}
}

INT ATrigger_LadderInteraction::AddMyMarker( AActor* S )
{
	AGearScout* Scout = Cast<AGearScout>(S);
	if( Scout != NULL && LadderSMActor != NULL && !bIsTopOfLadder )
	{
		FVector  BottomEntryLoc, TopEntryLoc;
		FRotator BottomEntryRot, TopEntryRot;
		GetBottomEntryPoint( BottomEntryLoc, BottomEntryRot );
		GetTopEntryPoint( TopEntryLoc, TopEntryRot );

		FVector CommonSize = Scout->GetSize(TEXT("Common"));
		if( Scout->FindSpot( FVector(CommonSize.X,CommonSize.X,CommonSize.Y), BottomEntryLoc ) && 
			Scout->FindSpot( FVector(CommonSize.X,CommonSize.X,CommonSize.Y), TopEntryLoc ) )
		{
			BottomMarker = Cast<ALadderMarker>(GWorld->SpawnActor(ALadderMarker::StaticClass(), NAME_None, BottomEntryLoc, BottomEntryRot, NULL, TRUE, FALSE, this));
			BottomMarker->LadderTrigger = this;
			TopMarker = Cast<ALadderMarker>(GWorld->SpawnActor(ALadderMarker::StaticClass(), NAME_None, TopEntryLoc, TopEntryRot, NULL, TRUE, FALSE, this));
			TopMarker->LadderTrigger	= this;
			TopMarker->bIsTopOfLadder	= TRUE;
			
			return 2;
		}
	}

	return 0;
}

UBOOL ALadderMarker::CanConnectTo( ANavigationPoint* Nav, UBOOL bCheckDistance )
{
	// Only allow ladder markers to connect to ladder markers on the same ladder
	ALadderMarker* OtherMarker = Cast<ALadderMarker>(Nav);
	if( OtherMarker != NULL )
	{
		return FALSE;
	}

	return Super::CanConnectTo( Nav, bCheckDistance );
}

void ALadderMarker::AddForcedSpecs( AScout *Scout )
{
	Super::AddForcedSpecs( Scout );
	
	ALadderMarker* OtherMarker = NULL;
	if( LadderTrigger != NULL )
	{
		if( LadderTrigger->TopMarker == this )
		{
			OtherMarker = LadderTrigger->BottomMarker;
		}
		else
		if( LadderTrigger->BottomMarker == this )
		{
			OtherMarker = LadderTrigger->TopMarker;
		}
	}

	if( OtherMarker != NULL )
	{
		ForcePathTo( OtherMarker, Scout, ULadderReachSpec::StaticClass()  );
	}
}

void ALadderMarker::addReachSpecs( AScout* Scout, UBOOL bOnlyChanged )
{
	if( LadderTrigger != NULL &&
		(LadderTrigger->TopMarker == this ||
		 LadderTrigger->BottomMarker == this) )
	{
		return Super::addReachSpecs( Scout, bOnlyChanged );
	}
	else
	{
		GWorld->DestroyActor( this, FALSE );
	}
}

UClass* ALadderMarker::GetReachSpecClass( ANavigationPoint* Nav, UClass* ReachSpecClass )
{
	ALadderMarker* OtherMarker = Cast<ALadderMarker>(Nav);
	if( OtherMarker != NULL )
	{
		return ULadderReachSpec::StaticClass(); 
	}

	return ReachSpecClass;
}

UBOOL ALadderMarker::CanPrunePath( INT Idx )
{
	if( PathList(Idx)->IsA(ULadderReachSpec::StaticClass()) )
	{
		return FALSE;
	}
	return TRUE;
}

INT ULeapReachSpec::CostFor(APawn* P)
{
	if( !P->bCanLeap && !(P->Physics == PHYS_Flying || P->Physics == PHYS_RigidBody) )
	{
		return UCONST_BLOCKEDPATHCOST;
	}
	return Super::CostFor(P);
}


/**********************************
 ********  PATH CONTRAINTS ********
 **********************************/
UBOOL UPath_WithinCombatZone::EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost )
{
	UBOOL bEndWithin	= FALSE;
	UBOOL bStartWithin	= FALSE;

	// check only the specified zone..
	if(SpecificCombatZone != NULL)
	{
		if( ACombatZone::IsNavWithin( Spec->End.Nav(), SpecificCombatZone ) )
		{
			bEndWithin = TRUE;
		}

		if( ACombatZone::IsNavWithin( Spec->Start, SpecificCombatZone ) )
		{
			bStartWithin = TRUE;
		}
	}
	else // otherwise check the list of assigned zones
	{
		for( INT ZoneIdx = 0; ZoneIdx < AI->CombatZoneList.Num(); ZoneIdx++ )
		{
			if( ACombatZone::IsNavWithin( Spec->End.Nav(), AI->CombatZoneList(ZoneIdx) ) )
			{
				bEndWithin = TRUE;
				break;
			}
		}

		for( INT ZoneIdx = 0; ZoneIdx < AI->CombatZoneList.Num(); ZoneIdx++ )
		{
			if( ACombatZone::IsNavWithin( Spec->Start, AI->CombatZoneList(ZoneIdx) ) )
			{
				bStartWithin = TRUE;
				break;
			}
		}
	}

	


	INT AddedCost = 0;
	if( !bEndWithin && bStartWithin )
	{
		if( LeavingCombatZonePenalty >= UCONST_BLOCKEDPATHCOST )
		{
			return FALSE;
		}

		AddedCost += appTrunc(LeavingCombatZonePenalty);
	}

	//debug
	DEBUGREGISTERCOST( *Spec->End, *GetClass()->GetName(), AddedCost );

	out_PathCost += AddedCost;

	return TRUE;
}

UBOOL UPath_PreferCover::EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost )
{
	ANavigationPoint* Nav = Spec->End.Nav();
	ACoverSlotMarker* Marker = Cast<ACoverSlotMarker>(Nav);

	INT AddedCost = 0;
	INT PathNum = Nav->PathList.Num();
	if( Marker == NULL && PathNum > 0 )
	{
		INT Penalty = Spec->Distance / PathNum;
		ANavigationPoint* Nav = Spec->End.Nav();
		for( INT PathIdx = 0; PathIdx < PathNum; PathIdx++ )
		{
			Marker = Cast<ACoverSlotMarker>(Nav->PathList(PathIdx)->End.Nav());
			if( Marker == NULL )
			{
				AddedCost += Penalty;
			}			
		}
	}

	//debug
	DEBUGREGISTERCOST( *Spec->End, *GetClass()->GetName(), AddedCost );

	out_PathCost += AddedCost;

	return TRUE;
}

UBOOL UPath_AvoidFireFromCover::EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost )
{
	INT AddedCost = 0;

	const FLOAT MoodScale =  (AI->CombatMood == AICM_Passive || 
								 AI->CombatMood == AICM_Ambush)	? 2.f  : 
								((AI->CombatMood == AICM_Aggressive) ? 0.5f : 
																  1.f);

	FVector SpecDir = Spec->GetDirection();
	for (INT EnemyIdx = 0; EnemyIdx < EnemyList.Num(); EnemyIdx++)
	{
		const AGearPawn* const EnemyPawn = EnemyList(EnemyIdx).Enemy;
		const FCoverInfo& Info = EnemyList(EnemyIdx).Cover;

		INT StartVal = 0;
		INT	EndVal = 0;

		checkSlow(Info.Link);
		if(!Info.Link || Info.SlotIdx < 0 || Info.SlotIdx >= Info.Link->Slots.Num())
		{
			debugf(TEXT("ERROR! UPath_AvoidFireFromCover::EvaluatePath Link was NULL or SlotIdx was OUT OF BOUNDS!! ZOMG WTF!"));
			return TRUE;
		}
		FCoverSlot& Slot = Info.Link->Slots(Info.SlotIdx);
		for( INT DangerIdx = 0; DangerIdx < Slot.DangerLinks.Num(); DangerIdx++ )
		{
			checkSlow(DangerIdx >= 0 && DangerIdx < Slot.DangerLinks.Num());
			ANavigationPoint* DangerNav = Slot.DangerLinks(DangerIdx).DangerNav.Nav();
			if (DangerNav != NULL)
			{
				if( DangerNav == Spec->Start )
				{
					StartVal = Slot.DangerLinks(DangerIdx).DangerCost;
					if( StartVal > 0 && EndVal > 0 )
					{
						break;
					}
				}
				if( DangerNav == *Spec->End )
				{
					EndVal = Slot.DangerLinks(DangerIdx).DangerCost;
					if( StartVal > 0 && EndVal > 0 )
					{
						break;
					}
				}
			}
		}

		if (StartVal == 0)
		{
			StartVal = EndVal;
		}
		else if (EndVal == 0)
		{
			EndVal = StartVal;
		}

		FLOAT DotP = SpecDir | Info.Link->GetSlotRotation(Info.SlotIdx).Vector();
		AddedCost += appTrunc(((StartVal + EndVal) * (1.f - DotP)) * MoodScale * ::Max<FLOAT>(Info.Link->DangerScale, 0.1f));
	}

	//debug
	DEBUGREGISTERCOST( *Spec->End, *GetClass()->GetName(), AddedCost );

	out_PathCost += AddedCost;

	return TRUE;
}

UBOOL UPath_AvoidanceVolumes::EvaluatePath( UReachSpec* Spec, APawn* Pawn, INT& out_PathCost, INT& out_HeuristicCost )
{
	for(INT Idx=0;Idx<AffectingCylinders.Num();Idx++)
	{
		UAIAvoidanceCylinderComponent* Comp = AffectingCylinders(Idx);
		if(Comp && Comp->DoesSpecIntersect(Spec)==TRUE)
		{
			//Pawn->DrawDebugLine(Spec->Start->Location, Spec->End->Location,255,0,255,TRUE);
			// if this spec intersects with an avoidance volume jack the cost WAY up so it's the last resort, but still available if no other path exists
			out_PathCost += 10000;
		}
	}

	return TRUE;
}

UBOOL UGoal_OutsideAvoidanceVolumes::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	TDoubleLinkedList<UAIAvoidanceCylinderComponent*>& AvoidanceCylinders = UAIAvoidanceCylinderComponent::GetList();
	for(TDoubleLinkedList<UAIAvoidanceCylinderComponent*>::TIterator Itt(AvoidanceCylinders.GetHead());Itt;++Itt)
	{

		UAIAvoidanceCylinderComponent* Comp = *Itt;
		if(Comp && (Comp->ShouldAIAvoidMe(AI)) && Comp->IsNavPointWithin(PossibleGoal)==TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL UGoal_InCombatZone::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	ACombatZone* CZ = ACombatZone::GetCombatZoneForNavPoint(PossibleGoal);
	if( CZ != NULL )
	{
		for( INT ZoneIdx = 0; ZoneIdx < AI->CombatZoneList.Num(); ZoneIdx++ )
		{
			if( CZ == AI->CombatZoneList(ZoneIdx) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


#if 0 && !PS3 && !FINAL_RELEASE
#define SQUADPOSDEBUGLOG(x)		{##x;}
#else
#define SQUADPOSDEBUGLOG(x) 
#endif

// Sort w/ biggest float first
IMPLEMENT_COMPARE_CONSTREF( FLOAT, GearPathBigFirst,   { return (B - A) > 0 ? 1 : -1; } )
IMPLEMENT_COMPARE_CONSTREF( FLOAT, GearPathSmallFirst, { return (B - A) < 0 ? 1 : -1; } )

void UGoal_SquadFormation::ClearPositionList()
{
	PositionList.Empty();
}

UBOOL UGoal_SquadFormation::EvaluateGoal( ANavigationPoint*& PossibleGoal, APawn* Pawn )
{
	if( PossibleGoal != Pawn->Anchor )
	{
		if(Formation->GetNumSquadMembersThatUsePositions() > Formation->Positions.Num())
		{
			debugf(TEXT("UGoal_SquadFormation::EvaluateGoal -> WARNING! Squad %s (%s) has more members than it has squad positions %i dudes will not have a squad position!"),
				*Formation->Squad->GetName(),
				*Formation->Squad->SquadName.ToString(),
				Formation->GetNumSquadMembersThatUsePositions()-Formation->Positions.Num());
		}
		// Initialize position list
		if( PositionList.Num() == 0 )
		{
			for( INT Idx = 0; Idx < Min<INT>(Formation->GetNumSquadMembersThatUsePositions(),Formation->Positions.Num()); Idx++ )
			{
				INT AddIdx = PositionList.AddZeroed();
				PositionList(AddIdx).PosIdx		= Idx;
				PositionList(AddIdx).QueryActor	= Formation->Positions(Idx).AI;
			}
		}

		// For each position in the formation compute the weight of this node
		TArray<FFormationPosition>& Positions = Formation->Positions;
		//for( INT Idx = 0; Idx < PositionList.Num(); Idx++ )
		for( INT Idx = 0; Idx < Min<INT>(Formation->GetNumSquadMembersThatUsePositions(),Formation->Positions.Num()); Idx++ )
		{
			FFormationPosition& Pos = Positions(PositionList(Idx).PosIdx);

// broke running down paths... removed... 
// I think it was supposed to prevent finding paths on other levels from the leader, but that should go away now that we have max traversal evaluation
			// If vertical distance is too great - invalid option
//			if( Abs(Pos.IdealPosition.Z - PossibleGoal->Location.Z) > 32.f )
//				continue;

			// Weight by distance against ideal location of the position
			FLOAT Weight = 1.1f - ((PossibleGoal->Location - Pos.IdealPosition).SizeSquared() / UCONST_MAXPOSITIONDISTSQ);
			if( Weight >= 0 )
			{
				// Add Nav/Weight pair to position list
				PositionList(Idx).ActorList.Set( PossibleGoal, Weight );
			}
		}
	}

	return Super::EvaluateGoal( PossibleGoal, Pawn );
}


// assumes 'ActorList' in each position is sorted according to bDescendingValues
AActor* UGoal_SquadFormation::GetBestPosition( TArray<AActor*> &UsedList, UBOOL bAscendingValues, INT& out_PosIdx )
{
	AActor* Result = NULL;

	// go until we find the best unused actor
	while(Result == NULL)
	{
		
		// select the best actor from the lists
		INT	  BestPosIdx = -1;
		FLOAT BestWeight = (!bAscendingValues) ? -1.f : BIG_NUMBER;
		AActor* BestKey = NULL;
		for( INT PosIdx = 0; PosIdx < PositionList.Num(); PosIdx++ )
		{
			FFormationEvalInfo& PosInfo = PositionList(PosIdx);
			TMap<AActor*,FLOAT>::TIterator It(PosInfo.ActorList);
			
			// change comparison when we're dealing with ascending lists
			if( ( !bAscendingValues && (It && It.Value() > BestWeight) ) || 
				( bAscendingValues && (It && It.Value() < BestWeight) ) )
			{
				BestWeight = It.Value();
				BestPosIdx = PosIdx;
				BestKey = It.Key();
			}			
		}

		//debugf(TEXT("BestKey for position %i is %s"),BestPosIdx,(BestKey)? *BestKey->GetName() : TEXT("NULL"));
		// if there's nothing left to find, bail
		if( BestPosIdx == -1 )
		{
			break;
		}

		// now that we have 'the best' verify that it's not used already
		FFormationEvalInfo& BestPosInfo = PositionList(BestPosIdx);
		if(	BestKey && UsedList.ContainsItem(BestKey) )
		{
			//debugf(TEXT("%s was already used.. removing it from %i's list !"),*BestKey->GetName(),BestPosIdx);
			// since this key is already used elsewhere, pull it out of this list
			PositionList(BestPosIdx).ActorList.Remove(BestKey);
		}
		else
		{
			Result = BestKey;
			out_PosIdx = BestPosInfo.PosIdx;
			UsedList.AddItem(BestKey);
			PositionList.Remove( BestPosIdx, 1 );
		}
	}

	
	return Result;
}

void UGoal_SquadFormation::WeightPositionsBySquadMembers()
{
	// Sort position nav points by AI near them
	AGearSquad* Squad = Formation->Squad;
	PositionList.Empty();
	for( INT Idx = 0; Idx < Min<INT>(Formation->GetNumSquadMembersThatUsePositions(),Formation->Positions.Num()); Idx++ )
	{
		if( Formation->Positions(Idx).Nav == NULL )
			continue;
		INT AddIdx = PositionList.AddZeroed();
		PositionList(AddIdx).PosIdx		= Idx;
		PositionList(AddIdx).QueryActor = Formation->Positions(Idx).Nav;
	}
	// For each position in the formation compute the weight of this node for the AI
	for( INT Idx = 0; Idx < PositionList.Num(); Idx++ )
	{
		FFormationPosition& Pos = Formation->Positions(PositionList(Idx).PosIdx);
		for( INT MemberIdx = 0; MemberIdx < Squad->SquadMembers.Num(); MemberIdx++ )
		{
			AController* SquadMember = Squad->SquadMembers(MemberIdx).Member;
			if( SquadMember == Squad->Leader )
				continue;
			APawn* SquadPawn = SquadMember ? SquadMember->Pawn : NULL;
			if( SquadPawn == NULL )
				continue;
			AGearAI* AI = Cast<AGearAI>(SquadMember);
			if( AI != NULL && AI->bIgnoreSquadPosition )
				continue;

			FLOAT Dist = (Pos.IdealPosition-SquadPawn->Location).SizeSquared();
			PositionList(Idx).ActorList.Set(SquadMember, Dist);
		}
	}
}

UBOOL UGoal_SquadFormation::DetermineFinalGoal( ANavigationPoint*& out_GoalNav )
{
	// Sort all position maps by weight
	for( INT PosIdx = 0; PosIdx < PositionList.Num(); PosIdx++ )
	{
		PositionList(PosIdx).ActorList.ValueSort<COMPARE_CONSTREF_CLASS(FLOAT,GearPathBigFirst)>();

#if !FINAL_RELEASE && !PS3
		UBOOL bPrintDebug = FALSE;
		if( bPrintDebug )
		{
			TArray<AActor*> Keys;
			TArray<FLOAT> Vals;
			PositionList(PosIdx).ActorList.GenerateKeyArray( Keys );
			PositionList(PosIdx).ActorList.GenerateValueArray( Vals );
			debugf(TEXT("PRINT %d %s"), PosIdx, *PositionList(PosIdx).QueryActor->GetName() );
			for( INT i= 0; i < Vals.Num(); i++ )
			{
				debugf(TEXT(".... %s %f"), *Keys(i)->GetName(), Vals(i) );
			}
		}
#endif
	}

	// assign the best nav point to each position, letting the positions closest to their nav points have first pick
	// NOTE: it's descending because distances are converted to weight (0-1) 
	// Keep track of used navigation points
	TArray<AActor*> UsedList;

	// While position list is not empty
	while( PositionList.Num() > 0 )
	{
		INT BestPosIdx = -1;
		ANavigationPoint* BestNav = Cast<ANavigationPoint>(GetBestPosition(UsedList, FALSE, BestPosIdx));
		if( BestNav != NULL )
		{
			FFormationPosition& Pos = Formation->Positions(BestPosIdx);
			Pos.Nav = BestNav;

			//debugf(TEXT("Best Nav for Squad position %d is %s"), BestPosIdx, *Pos.Nav->GetName());
		}
		else
		{
			//debug
			//debugf(TEXT("Failed to find best position while searching for squad formation position %d"), PositionList.Num() );

			break;
		}
	}

//////////////////////////////////////////


	// Sort position nav points by AI near them
	WeightPositionsBySquadMembers();

	// Sort positions by nearness of AI to closest nav point we found above
	for( INT PosIdx = 0; PosIdx < PositionList.Num(); PosIdx++ )
	{
		PositionList(PosIdx).ActorList.ValueSort<COMPARE_CONSTREF_CLASS(FLOAT,GearPathSmallFirst)>();

#if !FINAL_RELEASE && !PS3
		UBOOL bPrintDebug = FALSE;
		if( bPrintDebug )
		{
			TArray<AActor*> Keys;
			TArray<FLOAT> Vals;
			PositionList(PosIdx).ActorList.GenerateKeyArray( Keys );
			PositionList(PosIdx).ActorList.GenerateValueArray( Vals );
			debugf(TEXT("PRINT %d %s"), PosIdx, *PositionList(PosIdx).QueryActor->GetName() );
			for( INT i= 0; i < Vals.Num(); i++ )
			{
				debugf(TEXT(".... %s %f"), *Keys(i)->GetName(), Vals(i) );
			}
		}
#endif
	}

	UsedList.Empty();
	// While position list is not empty
	while( PositionList.Num() > 0 )
	{
		INT BestPosIdx = -1;
		AActor*  BestActor = GetBestPosition(UsedList, FALSE, BestPosIdx);
		AGearAI* BestAI = Cast<AGearAI>(BestActor);
		if( BestAI != NULL )
		{
			FFormationPosition& Pos = Formation->Positions(BestPosIdx);
			Pos.AI = BestAI;

			//debugf(TEXT("Best AI for Nav %s is (%s)%s %2.3f"), *Pos.Nav->GetName(), *BestAI->GetName(),*BestAI->Pawn->GetName(),GWorld->GetTimeSeconds() );
		}
		else if(BestActor == NULL)
		{
			//debug
			//debugf(TEXT("Failed to find best AI for Nav %s %2.3f"), (Pos.Nav) ? *Pos.Nav->GetName() : TEXT("NULL") ,GWorld->GetTimeSeconds());

			break;
		}
	}

/////////////////////////////////////////////////

	// Figure out which position to set as the goal
	INT Idx = PositionIdx;
	if( Idx < 0 )
	{
		for( Idx = 0; Idx < Formation->Positions.Num(); Idx++ )
		{
			if( Formation->Positions(Idx).AI == SeekerAI )
				break;
		}
	}

	if(Idx >= Formation->Positions.Num())
	{
		debugf(TEXT("WARNING! Could not find squad position for %s"),*SeekerAI->GetName());
		out_GoalNav = NULL;
	}
	else
	{
		out_GoalNav = Formation->Positions(Idx).Nav;
	}

	
	return (out_GoalNav != NULL);
}

UBOOL UGoal_AtCover::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	// don't evaluate the Anchor as we do that separately in cases where we want to consider staying put
	if (PossibleGoal != Pawn->Anchor)
	{
		RateSlotMarker(Cast<ACoverSlotMarker>(PossibleGoal), Pawn, PossibleGoal->bestPathWeight);
	}
	
	// abort with our best result if we've already far exceeded our max distance
	if( BestMarker != NULL && NumMarkersTested > MaxToRate)
	{
		PossibleGoal = BestMarker;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void UGoal_AtCover::NotifyExceededMaxPathVisits( ANavigationPoint* BestGuess )
{
#if DO_AI_LOGGING 
	AI->AILog(TEXT("Cover search hit max path visits! MaxPathVisits:%i Cover Considered:%i BestMarker: %s"),
		   MaxPathVisits,
		   NumMarkersTested,
		   (BestMarker)? *BestMarker->GetName() : TEXT("NULL"));
	AI->AILog(TEXT("Cover constraints---------------------------"));
	for(INT Idx=0;Idx<CoverGoalConstraints.Num();Idx++)
	{
		AI->AILog(TEXT("[0]: %s"),*CoverGoalConstraints(Idx)->GetName());
	}
	AI->AILog(TEXT("--------------------------------------------"));
#endif
	GeneratedGoal = BestGuess;
}

#define CHECKABORTRATING(R)	if( R <= BestRating ) { /*return;*/ }

UBOOL UCovGoal_MovementDistance::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* AI, AActor* Goal, FLOAT& Rating)
{
	FLOAT DistMarkerToPawn = (Marker->Location - SeekingPawn->Location).Size();

	if(DistMarkerToPawn < MinCoverDist || DistMarkerToPawn > MaxCoverDist)
	{
		Rating += BestCoverDist;
	}
	else
	if(DistMarkerToPawn < BestCoverDist)
	{
		Rating += 2.0f * (BestCoverDist - DistMarkerToPawn);
	}

	if( bMoveTowardGoal && Goal != NULL )
	{
		FLOAT DistMarkerToGoal = (Marker->Location - Goal->Location).Size();
		FLOAT DistPawnToGoal   = (SeekingPawn->Location - Goal->Location).Size();
		if( (DistPawnToGoal - DistMarkerToGoal) < MinDistTowardGoal )
		{
			return FALSE;
		}
	}
	
		
	return TRUE;
}

UBOOL UCovGoal_SquadLeaderProximity::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* AI, AActor* Goal, FLOAT& Rating)
{
	if (AI->CombatMood == AICM_Normal && AI->Squad->Leader != NULL && AI->Squad->Leader != AI && AI->Squad->Leader->Pawn != NULL)
	{
		FLOAT DistFromLeader = (SquadLeaderLocation - Marker->Location).Size();
		if( DistFromLeader > 512.f )
		{
			Rating += DistFromLeader;
		}
	}

	return TRUE;
}

void UCovGoal_Enemies::Init(UGoal_AtCover* GoalEval)
{
	if(GoalEval == NULL || GoalEval->AI == NULL)
	{
		debugf(NAME_Warning, TEXT("UCovGoal_Enemies::Init() called but, GoalEval was not initialized!"));
		return;
	}

	AGearAI* AI = GoalEval->AI;

	// wipe old cache
	ValidEnemyCache.Empty(ValidEnemyCache.Num());

	// loop through our enemy list and cache off their cover (only for valid enemies)
	for( INT EnemyIdx = 0; EnemyIdx < AI->Squad->EnemyList.Num(); EnemyIdx++ )
	{
		APawn* Enemy = AI->Squad->EnemyList(EnemyIdx).Pawn;
		if( Enemy == NULL || !Enemy->IsValidEnemyTargetFor( AI->PlayerReplicationInfo, FALSE ) )
		{
			continue;
		}

		FValidEnemyCacheDatum NewEnemyCacheDatum;
		NewEnemyCacheDatum.EnemyCover.Link = NULL;
		NewEnemyCacheDatum.EnemyPawn = NULL;
		FLOAT TimeSinceSeen = AI->WorldInfo->TimeSeconds - AI->Squad->EnemyList(EnemyIdx).LastSeenTime;
		if(TimeSinceSeen < 5.0f)
		{
			NewEnemyCacheDatum.EnemyPawn = AI->Squad->EnemyList(EnemyIdx).GearPawn;
		}
	
		if(NewEnemyCacheDatum.EnemyPawn != NULL)
		{
			if(!AI->GetPlayerCover(NewEnemyCacheDatum.EnemyPawn,NewEnemyCacheDatum.EnemyCover,TRUE))
			{
				NewEnemyCacheDatum.EnemyCover.Link = NULL;
			}
			
			ValidEnemyCache.AddItem(NewEnemyCacheDatum);
		}
	}


}

UBOOL UCovGoal_Enemies::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* AI, AActor* Goal, FLOAT& Rating)
{
	FLOAT	ExposedScale = 0.f;
	INT		EnemyCnt	 = 0;
	UBOOL	bTooClose	 = FALSE;
	INT		FireLinkModifier = 0;
	FCoverInfo EnemyCover;
	ACoverLink* Link	= Marker->OwningSlot.Link;
	INT			SlotIdx	= Marker->OwningSlot.SlotIdx;
	// allow slight exposure to enemy if we're aggressive (unless they're actively shooting us)
	const FLOAT EnemyExposureThreshold = (AI->CombatMood == AICM_Aggressive) ? 0.15f : 0.0f;

	for( INT EnemyIdx = 0; EnemyIdx < ValidEnemyCache.Num(); EnemyIdx++ )
	{
		EnemyCnt++;
		FLOAT Exposed	= 0.f;
		APawn* Enemy = ValidEnemyCache(EnemyIdx).EnemyPawn;
		if( AI->IsCoverExposedToAnEnemy( Marker->OwningSlot, Enemy, &Exposed ) )
		{
			// if the guy we're exposed to is our enemy or is actively shooting at us, throw the node out completely
			if ((AI->Enemy == Enemy && Exposed > EnemyExposureThreshold) || (Enemy->Controller != NULL && SeekingPawn->LastHitBy == Enemy->Controller && AI->WorldInfo->TimeSeconds - AI->LastShotAtTime < 5.0f))
			{
				return FALSE;
			}
			ExposedScale += Exposed;
		}

		if( !AI->bIgnoreFireLinks )
		{
			AGearPawn *GP = ValidEnemyCache(EnemyIdx).EnemyPawn;
			
			EnemyCover = ValidEnemyCache(EnemyIdx).EnemyCover;
			if( GP != NULL && EnemyCover.Link != NULL )
			{
				if( Link->HasFireLinkTo( SlotIdx, EnemyCover ) )
				{
					FireLinkModifier += 2; // two for a real fire link
					if( AI->Enemy == Enemy )
					{
						// Additional boost if we can shoot at our current enemy from here
						FireLinkModifier += 2; 
					}
				}
				else if( Link->HasFireLinkTo( SlotIdx, EnemyCover, TRUE ) )
				{
					FireLinkModifier++; // one for a fallback fire link
				}
			}
			// ignore enemies not in cover, since we can't do a fast enough vis check here
			else
			{
				EnemyCnt--;
			}
		}
	}

	Rating += 900.f * ExposedScale;
	if (EnemyCnt > 0)
	{
		if( !AI->bIgnoreFireLinks )
		{
			// if we have no fire links, penalize it heavily
			if( FireLinkModifier < 2 )
			{
				Rating += 5000.f;
			}
			Rating -= 450.0f * FireLinkModifier;
		}
	}

	return TRUE;
}

UBOOL UCovGoal_EnemyProximity::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* AI, AActor* Goal, FLOAT& Rating)
{
	for( INT EnemyIdx = 0; EnemyIdx < AI->Squad->EnemyList.Num(); EnemyIdx++ )
	{
		APawn* Enemy = AI->Squad->EnemyList(EnemyIdx).Pawn;
		if( Enemy == NULL || !Enemy->IsValidEnemyTargetFor( AI->PlayerReplicationInfo, FALSE ) )
		{
			continue;
		}
		FLOAT EnemyDist = (Marker->Location - Enemy->Location).Size();
		if(EnemyDist < AI->EnemyDistance_Short)
		{
			Rating += AI->EnemyDistance_Short - EnemyDist;
		}
	}

	return TRUE;
}

UBOOL UCovGoal_TeammateProximity::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* AI, AActor* Goal, FLOAT& Rating)
{
	FCoverInfo FriendlyCover;
	for( AController *TestController = SeekingPawn->WorldInfo->ControllerList; TestController != NULL; TestController = TestController->NextController )
	{
		AGearPawn *FriendPawn = Cast<AGearPawn>(TestController->Pawn);
		if( TestController != AI &&
			FriendPawn != NULL &&
			(FriendPawn->PlayerReplicationInfo != NULL && FriendPawn->PlayerReplicationInfo->Team == AI->PlayerReplicationInfo->Team) &&
			AI->GetPlayerCover(FriendPawn,FriendlyCover,TRUE) )
		{
			FLOAT DistToFriendly = (FriendlyCover.Link->GetSlotLocation(FriendlyCover.SlotIdx) - Marker->Location).Size();
			if( DistToFriendly <= 256.f )
			{
				Rating += 256.f - DistToFriendly;
			}
		}
	}
	return TRUE;
}

UBOOL UCovGoal_GoalProximity::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{
	if( Goal == NULL)
	{
		return TRUE;
	}

	FLOAT DistToGoal = (Marker->Location - Goal->Location).Size();

	//debug
	//SeekingAI->AILog(TEXT("UCovGoal_GoalProximity::EvaluateCoverMarker Marker: %s DistToGoal:%.2f bHardLimits?%i MaxGoalDist:%.2f"),*Marker->GetName(),DistToGoal,bHardLimits,MaxGoalDist);

	if( DistToGoal > MaxGoalDist || DistToGoal < MinGoalDist)
	{
		Rating += 5000.f;
		return !bHardLimits;
	}
	if(DistToGoal > BestGoalDist)
	{
		Rating += 2.0f * (DistToGoal-BestGoalDist);
	}
	return TRUE;
}

UBOOL UCovGoal_WithinWeaponRange::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{
	if( Goal == NULL)
	{
		return TRUE;
	}

	FLOAT DistToGoal = (Marker->Location - Goal->Location).Size();
	FLOAT BestGoalDist = SeekingAI->EnemyDistance_Medium;
	FLOAT MinGoalDist = SeekingAI->EnemyDistance_Short;
	FLOAT MaxGoalDist = SeekingAI->EnemyDistance_Long;
	if (DistToGoal < MinGoalDist)
	{
		Rating += 5000.f;
		return !bHardConstraint;
	}
	else if (DistToGoal > MaxGoalDist)
	{
		Rating += 5000.f + DistToGoal - MaxGoalDist;
		return !bHardConstraint;
	}

	if(DistToGoal < BestGoalDist)
	{
		Rating += 2.0f * (BestGoalDist-DistToGoal);
	}
	else if (DistToGoal > BestGoalDist)
	{
		// bias slightly towards long range if we're the leader
		if (bIsLeader)
		{
			Rating += 0.5f * (MaxGoalDist - DistToGoal);
		}
		// bias slightly towards medium range unless sniping
		else if (SeekingAI->MyGearPawn->MyGearWeapon == NULL || !SeekingAI->MyGearPawn->MyGearWeapon->bSniping)
		{
			Rating += 0.5f * (DistToGoal - BestGoalDist);
		}
	}
	return TRUE;
}

UBOOL UCovGoal_AvoidanceVolumes::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{
	TDoubleLinkedList<UAIAvoidanceCylinderComponent*>& AvoidanceCylinders = UAIAvoidanceCylinderComponent::GetList();
	for(TDoubleLinkedList<UAIAvoidanceCylinderComponent*>::TIterator Itt(AvoidanceCylinders.GetHead());Itt;++Itt)
	{
		if((*Itt)->IsNavPointWithin(Marker)==TRUE)
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL UCovGoal_ProtectedByLocation::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{
	if( bOnlySlotsWithFireLinks && Marker->OwningSlot.Link->Slots(Marker->OwningSlot.SlotIdx).FireLinks.Num() == 0 )
	{
		return FALSE;
	}

	FVector	 SlotLocation  = Marker->GetSlotLocation();
	FRotator SlotRotation  = Marker->GetSlotRotation();
	FLOAT	 SlotDotAmbush = SlotRotation.Vector() | (ThreatLocation - SlotLocation).SafeNormal();
	if( SlotDotAmbush <= 0.f )
	{
		return FALSE;
	}

	if( bForceMoveTowardGoal )
	{
		FVector PawnToGoal  = (Goal->Location - SeekingPawn->Location).SafeNormal();
		FVector PawnToSlot	= (SlotLocation - SeekingPawn->Location).SafeNormal();
		FLOAT	SlotDotGoal = SlotRotation.Vector() | PawnToGoal;
		FLOAT	SlotDotPawn	= PawnToGoal | PawnToSlot;
		if( SlotDotGoal <= 0.4f || SlotDotPawn < 0.f )
		{
			return FALSE;
		}
	}
	
	Rating += 5000.f * (1.f - SlotDotAmbush);
	
	return TRUE;
}

UBOOL UCovGoal_WithinAllowedCoverLinks::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{

	if(Marker && SeekingAI->AllowedCoverLinks.ContainsItem(Marker->OwningSlot.Link))
	{
		return TRUE;
	}
	return FALSE;
}

UBOOL UCovGoal_WithinCombatZones::EvaluateCoverMarker(ACoverSlotMarker* Marker, APawn* SeekingPawn, AGearAI_Cover* SeekingAI, AActor* Goal, FLOAT& Rating)
{
	// If marker is not within our list of valid zones to be in
	UBOOL bWithinCZ = (SeekingAI->CombatZoneList.Num() == 0);
	for( INT ZoneIdx = 0; ZoneIdx < SeekingAI->CombatZoneList.Num(); ZoneIdx++ )
	{
		if( ACombatZone::IsNavWithin( Marker, SeekingAI->CombatZoneList(ZoneIdx) ) )
		{
			bWithinCZ = TRUE;
			break;
		}
	}
	if( !bWithinCZ )
	{
		return FALSE;
	}

	return TRUE;
}

UBOOL UGoal_AtCover::IsValid( ACoverSlotMarker* Marker, APawn* Pawn )
{
	// validation checks
	// If not a valid claim to end at
	if(  Marker == NULL		|| 
		!AI->IsValidCover(Marker->OwningSlot) || 
		!Marker->IsValidClaim( AI->MyGearPawn ) )
	{
		//AI->AILog(TEXT("UGoal_AtCover::IsValid Throwing out %s because it is not a valid claim or it isn't valid cover!"),*Marker->GetName());
		return FALSE;
	}

	ACoverLink* Link	= Marker->OwningSlot.Link;
	INT			SlotIdx	= Marker->OwningSlot.SlotIdx;

	if( Link->Slots(SlotIdx).FireLinks.Num() == 0 && AI->Squad->EnemyList.Num() > 0 )
	{
		//debug
		//AI->AILog(TEXT("UGoal_AtCover::IsValid Throwing out %s because it has no firelinks and I have enemies!"),*Marker->GetName());
		return FALSE;
	}

	return TRUE;
}

#define DO_COVERCONSTRAINT_PROFILING 0
#if DO_COVERCONSTRAINT_PROFILING
struct FCovConstraintProfileDatum
{
public:
	FCovConstraintProfileDatum()
	{
		CallCount=0;
		MaxTime=-1.f;
		AvgTime=-1.f;
		TotalTime=0.f;
	}

	INT CallCount;
	FLOAT MaxTime;
	FLOAT AvgTime;
	FLOAT TotalTime;
};

TMap<FName,FCovConstraintProfileDatum> ConstraintProfileData;
INT RateSlotMarkerCallCount =0;
FLOAT RateSlotMarkerCallMax = -1.f;
FLOAT RateSlotMarkerCallAvg=-1.f;
FLOAT RateSlotMarkerCallTotal=0.f;

#define SCOPETIMER(CALLCOUNT,CALLAVG,CALLMAX,CALLTOTAL) \
	class ScopeTimer \
	{ \
	public:\
		FLOAT Time; \
		ScopeTimer() \
		{\
			CALLCOUNT ## ++;\
			Time=0.f;\
			CLOCK_CYCLES(Time);\
		}\
		~ScopeTimer()\
		{\
			UNCLOCK_CYCLES(Time);\
			CALLTOTAL+=Time;\
			if(CALLAVG < 0.f)\
			{\
				CALLAVG = Time;\
			}\
			else\
			{\
				CALLAVG += (Time - CALLAVG)/CALLCOUNT;\
			}\
			if(Time > CALLMAX)\
			{\
				CALLMAX = Time;\
			}\
		}\
	};\
	ScopeTimer TheTimer = ScopeTimer();

FCovConstraintProfileDatum* ProfileDatum = NULL;
#else
#define SCOPETIMER(CALLCOUNT,CALLAVG,CALLMAX,CALLTOTAL) {}
#endif

IMPLEMENT_COMPARE_POINTER( UCoverGoalConstraint, CovGoalPriority, { return (A->ConstraintEvaluationPriority - B->ConstraintEvaluationPriority); } )
void UGoal_AtCover::InitNative()
{
#if DO_COVERCONSTRAINT_PROFILING
	ConstraintProfileData.Empty();
	RateSlotMarkerCallCount=0;
	RateSlotMarkerCallMax=-1.0f;
	RateSlotMarkerCallAvg=-1.f;
	RateSlotMarkerCallTotal=0.f;
#endif

	for(INT Idx=CoverGoalConstraints.Num()-1;Idx>=0;Idx--)
	{
		if(CoverGoalConstraints(Idx) == NULL || CoverGoalConstraints(Idx)->IsPendingKill())
		{
#if !FINAL_RELEASE
			debugf(TEXT("UGoal_AtActor::InitNative() cover constraint at Idx(%i) was NULL or PendingKill.. wtf? (owningai:%s)"),Idx,(AI) ? *AI->GetName() : TEXT("NULL"));
#endif
			CoverGoalConstraints.Remove(Idx);
		}
	}
	// sort by eval priority so the expensive stuff gets saved for last
	Sort<USE_COMPARE_POINTER(UCoverGoalConstraint,CovGoalPriority)>(&CoverGoalConstraints(0),CoverGoalConstraints.Num());
}

void UGoal_AtCover::RateSlotMarker(ACoverSlotMarker* Marker, APawn* Pawn, INT BaseRating)
{
#if DO_COVERCONSTRAINT_PROFILING
	SCOPETIMER(RateSlotMarkerCallCount,RateSlotMarkerCallAvg,RateSlotMarkerCallMax,RateSlotMarkerCallTotal);
#endif
	if(!IsValid(Marker,Pawn))
	{
		return;
	}
	// reject if this marker is a last resort and we have a better option already
	if (Marker->bLastChoice && BestMarker != NULL && !BestMarker->bLastChoice)
	{
		return;
	}
	// PASSSED TESTS! RATE IT!

	//AI->AILog(TEXT("Rating %s"), *Marker->GetName());
	NumMarkersTested++;
	FLOAT Rating = FLOAT(BaseRating);
	// double influence ExtraCost so that LDs can affect AI cover priority
	Rating += Marker->ExtraCost;
	// add any extra rating from AI
	if (AI->Squad != NULL)
	{
		Rating += AI->Squad->DecayedCoverMap.FindRef(Marker);
	}

	for(INT Idx=0;Idx<CoverGoalConstraints.Num();Idx++)
	{
		// if one of the constraints returns false that means throw this node out
		if( CoverGoalConstraints(Idx) != NULL)			
		{
#if DO_COVERCONSTRAINT_PROFILING
			FName ClassName = CoverGoalConstraints(Idx)->GetClass()->GetFName();
			ProfileDatum = ConstraintProfileData.Find(ClassName);
			if(ProfileDatum == NULL)
			{
				ProfileDatum = &ConstraintProfileData.Set(ClassName,FCovConstraintProfileDatum());
			}
			SCOPETIMER(ProfileDatum->CallCount,ProfileDatum->AvgTime,ProfileDatum->MaxTime,ProfileDatum->TotalTime)
#endif
			if( CoverGoalConstraints(Idx)->EvaluateCoverMarker(Marker, Pawn, AI, TetherActor, Rating) == FALSE )
			{
				//AI->AILog(TEXT("Rejected by %s"), *CoverGoalConstraints(Idx)->GetName());
				return;
			}
		}
	}

	//AI->AILog(TEXT("RateSlotMarker %s %f"), *Marker->GetName(), Rating );
	
	if (BestMarker == NULL || Rating < BestRating || (BestMarker->bLastChoice && !Marker->bLastChoice))
	{
		BestMarker = Marker;
		BestRating = appTrunc(Rating);
	}
}

void UGoal_AtCover::execRateSlotMarker(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(ACoverSlotMarker, Marker);
	P_GET_OBJECT(APawn, Pawn);
	P_GET_INT(BaseRating);
	P_FINISH;

	if (Pawn != NULL && Pawn->PathGoalList == this && AI != NULL && AI->Squad != NULL && AI->MyGearPawn != NULL)
	{
		RateSlotMarker(Marker, Pawn, BaseRating);
	}
	else
	{
		debugf(NAME_Warning, TEXT("RateSlotMarker() called on %s when not fully initialized!"), *GetPathName());
	}
}

UBOOL UGoal_AtCover::DetermineFinalGoal( ANavigationPoint*& out_GoalNav )
{
#if DO_COVERCONSTRAINT_PROFILING
	debugf(TEXT("-----------------------COVER GOAL PATH CONSTRAINT STATS for %s---------------------"),*AI->GetName());
	for(TMap<FName,FCovConstraintProfileDatum>::TIterator Itt(ConstraintProfileData);Itt;++Itt)
	{
		FName ConstraintName = Itt.Key();
		debugf(TEXT("Time: %3.3fms (Per call: %3.3fms avg(%3.3fmsmax)) CallCount:%i PctOverall:%.2f%% -- %s"),
			Itt.Value().TotalTime* GSecondsPerCycle *1000.f,
			Itt.Value().AvgTime* GSecondsPerCycle *1000.f,
			Itt.Value().MaxTime* GSecondsPerCycle *1000.f,
			Itt.Value().CallCount,
			Itt.Value().TotalTime/RateSlotMarkerCallTotal*100.f,
			*ConstraintName.ToString());			
	}
	debugf(TEXT("OVERALL TIME:%3.3fms (per call %3.3fms avg, %3.3fms max), CallCount: %i"),RateSlotMarkerCallTotal*GSecondsPerCycle*1000.f,RateSlotMarkerCallAvg*GSecondsPerCycle *1000.f,RateSlotMarkerCallMax*GSecondsPerCycle *1000.f,RateSlotMarkerCallCount);
	debugf(TEXT("-------------------------------------------------------------------------------------------------"));
#endif

	//debugf(TEXT("UGoal_AtCover::DetermineFinalGoal - chose %s (%i) tested %i"),(BestMarker)? *BestMarker->GetName() : TEXT("NULL"),BestRating,NumMarkersTested);
	out_GoalNav = BestMarker;
	return BestMarker != NULL;
}

UBOOL UGoal_FarthestNavInRange::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	// if we've exceeded the max dist and have a result, return it
	if (PossibleGoal->visitedWeight > OptimalMaxDist && CurrentBest != NULL)
	{
		PossibleGoal = CurrentBest;
		return TRUE;
	}
	else
	{
		// otherwise see if this NavigationPoint is a goal node
		if ((CurrentBest == NULL || PossibleGoal->visitedWeight > CurrentBest->visitedWeight) && GoalList.ContainsItem(PossibleGoal))
		{
			if (PossibleGoal->visitedWeight > OptimalMaxDist)
			{
				return TRUE;
			}
			CurrentBest = PossibleGoal;
		}
		return FALSE;
	}
}

UBOOL UGoal_FarthestNavInRange::DetermineFinalGoal(ANavigationPoint*& out_GoalNav)
{
	if (CurrentBest != NULL)
	{
		out_GoalNav = CurrentBest;
		return TRUE;
	}
	else
	{
		return Super::DetermineFinalGoal(out_GoalNav);
	}
}


void AGearNavModificationVolume::UpdateAffectedNavPoints()
{
	// first, un-affect all affected points
	// removing all and re-adding affected is faster than checking if still affected and then having to check the array when checking for new items
	for (INT i = 0; i < AffectedPoints.Num(); i++)
	{
		if (AffectedPoints(i) != NULL)
		{
			AffectedPoints(i)->ExtraCost -= ExtraCost;
			if (bInvalidateAICover)
			{
				ACoverSlotMarker* Cover = Cast<ACoverSlotMarker>(AffectedPoints(i));
				if (Cover != NULL)
				{
					Cover->bLastChoice = Cover->GetClass()->GetDefaultObject<ACoverSlotMarker>()->bLastChoice;
				}
			}
		}
	}
	AffectedPoints.Reset();

	if (bEnabled && CollisionComponent != NULL)
	{
		// apply to all points that are inside the volume
		// with the potentially large volume size, it's faster to just check all NavigationPoints then doing a nav octree check
		// especially since the octree also contains ReachSpecs that we don't care about
		FBox CollisionBox = CollisionComponent->Bounds.GetBox();
		FCheckResult Hit(1.0f);
		for (ANavigationPoint* Nav = WorldInfo->NavigationPointList; Nav != NULL; Nav = Nav->nextNavigationPoint)
		{
			if ( CollisionBox.Intersect(Nav->CylinderComponent->Bounds.GetBox()) &&
				!CollisionComponent->PointCheck(Hit, Nav->CylinderComponent->Bounds.Origin, Nav->CylinderComponent->Bounds.BoxExtent, 0) )
			{
				Nav->ExtraCost += ExtraCost;
				AffectedPoints.AddItem(Nav);
				if (bInvalidateAICover)
				{
					if (Nav->AnchoredPawn != NULL)
					{
						AGearAI* AI = Cast<AGearAI>(Nav->AnchoredPawn->Controller);
						if (AI != NULL)
						{
							AI->AcquireCover = ACT_Immediate;
						}
					}
					ACoverSlotMarker* Cover = Cast<ACoverSlotMarker>(Nav);
					if (Cover != NULL)
					{
						Cover->bLastChoice = TRUE;
					}
				}
			}
		}
	}
}

void AGearNavModificationVolume::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	// if we have moved, update the list of NavigationPoints inside us
	if (CollisionComponent != NULL && (CollisionComponent->NeedsReattach() || CollisionComponent->NeedsUpdateTransform()))
	{
		FName TimerName = FName(TEXT("UpdateAffectedNavPoints"));
		if (!IsTimerActive(TimerName))
		{
			// delayed so that if we're constantly moving we aren't doing the expensive check every tick
			SetTimer(1.0f, FALSE, TimerName);
		}
	}

	Super::UpdateComponentsInternal(bCollisionUpdate);
}

UBOOL UGoal_SpawnPoints::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	ACoverSlotMarker* CovSlotMarker = Cast<ACoverSlotMarker>(PossibleGoal);
	if(CovSlotMarker != NULL && CovSlotMarker->IsValidClaim(AI->MyGearPawn))
	{

		// make sure it's not too close to the enemy, and if it's not exposed at all add it tot he choice list.. otherwise save it for backup
		if((!AI->Enemy || (AI->Enemy->Location - CovSlotMarker->Location).SizeSquared() > MinDistToEnemy*MinDistToEnemy))
		{
			if(!AI->IsCoverExposedToAnEnemy(CovSlotMarker->OwningSlot))
			{
				PickedSpawnPoints.AddUniqueItem(PossibleGoal);			
			}
			else if(AI->Enemy && !AI->IsCoverExposedToAnEnemy(CovSlotMarker->OwningSlot,AI->Enemy))
			{
				BackupSpawnPoints.AddUniqueItem(PossibleGoal);
			}
		}
	}

	return (PickedSpawnPoints.Num() >= NumSpawnpointsNeeded);
}


UBOOL UGoal_SpawnPoints::DetermineFinalGoal( ANavigationPoint*& out_GoalNav )
{
	if(PickedSpawnPoints.Num() >= NumSpawnpointsNeeded)
	{
		return TRUE;
	}

	INT NumAdditionalPointsNeeded = Min<INT>(NumSpawnpointsNeeded-PickedSpawnPoints.Num(),BackupSpawnPoints.Num());
	for(INT Idx=0;Idx<NumAdditionalPointsNeeded;Idx++)
	{
		PickedSpawnPoints.AddUniqueItem(BackupSpawnPoints(Idx));
	}
	return PickedSpawnPoints.Num() > 0;
}

UBOOL UGoal_AwayFromPosition::EvaluateGoal(ANavigationPoint*& PossibleGoal, APawn* Pawn)
{
	FLOAT Dot = (PossibleGoal->Location - Pawn->Location).SafeNormal() | AvoidDir;
	// rating prioritizes goals about 45 degrees away from straight back from target so that AI doesn't move predictably and in the open
	INT Rating = appTrunc(FLOAT(PossibleGoal->visitedWeight) * ((Dot > -0.7) ? (1.f - Dot) : (2.4 + Dot)));
	if (Rating > BestRating)
	{
		BestNode = PossibleGoal;
		BestRating = Rating;
	}

	// abort if our max distance was exceeded
	if (PossibleGoal->visitedWeight >= MaxDist)
	{
		PossibleGoal = BestNode;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

UBOOL UGoal_AwayFromPosition::DetermineFinalGoal(ANavigationPoint*& out_GoalNav)
{
	out_GoalNav = BestNode;
	return Super::DetermineFinalGoal(out_GoalNav);
}
