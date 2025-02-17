/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineAnimClasses.h"
#include "GenericOctree.h"

IMPLEMENT_CLASS(ACrowdAgent);
IMPLEMENT_CLASS(ACrowdAttractor);
IMPLEMENT_CLASS(USeqAct_CrowdSpawner);
IMPLEMENT_CLASS(ACrowdReplicationActor);

/** The octree semantics for crowd attractors. */
struct FCrowdAttractorOctreeSemantics
{
	enum { MaxElementsPerLeaf = 16 };
	enum { MinInclusiveElementsPerNode = 7 };
	enum { MaxNodeDepth = 12 };

	static FBoxCenterAndExtent GetBoundingBox(ACrowdAttractor* Attractor)
	{
		FVector BoxExtent = FVector(Attractor->AttractionRadius, Attractor->AttractionRadius, Attractor->AttractionHeight);
		return FBoxCenterAndExtent(Attractor->Location, BoxExtent);
	}

	static void SetElementId(ACrowdAttractor* Attractor, FOctreeElementId Id)
	{
		Attractor->OctreeId = Id;
	}
};


static FVector ClosestPointOnLine(const FVector& LineStart, const FVector& LineEnd, const FVector& Point)
{
	// Solve to find alpha along line that is closest point
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	FLOAT A = (LineStart-Point) | (LineEnd - LineStart);
	FLOAT B = (LineEnd - LineStart).SizeSquared();
	FLOAT T = ::Clamp(-A/B, 0.f, 1.f);

	// Generate closest point
	FVector ClosestPoint = LineStart + (T * (LineEnd - LineStart));

	return ClosestPoint;
}

/** Find the closts crowd-usable reach spec to the current position */
static UReachSpec* FindNearestReachspec(const FVector& Position, UBOOL bUseOnlyCrowdPaths, FLOAT PathDistance)
{
	// Find all nav objects within twice the distance we want to be to a path
	static TArray<FNavigationOctreeObject*> NavObjects;  // Temp storage - static so we don't allocate/free every time.

	NavObjects.Reset();
	GWorld->NavigationOctree->RadiusCheck(Position, 2.f*PathDistance, NavObjects);

	FLOAT ClosestDistSqr = BIG_NUMBER;
	UReachSpec* ClosestReachSpec = NULL;

	for(INT i=0; i<NavObjects.Num(); i++)
	{
		// Only interested in reach specs (edges)
		UReachSpec* Spec = NavObjects(i)->GetOwner<UReachSpec>();
		if(Spec)
		{
			ANavigationPoint* StartNav = Spec->Start;
			ANavigationPoint* EndNav = (ANavigationPoint*)Spec->End.Actor;
			// Check we have 2 nodes, and either we want to use any path, or both nodes are flagged for crowd usage
			if(StartNav && EndNav && (!bUseOnlyCrowdPaths || (StartNav->bCrowdPath && EndNav->bCrowdPath)))
			{
				FVector ReachStart = StartNav->Location;
				FVector ReachEnd = EndNav->Location;

				// Find closest point on edge from given location
				FVector ClosePoint = ClosestPointOnLine(ReachStart, ReachEnd, Position);

				// See how far that is
				FVector ThisToPath = (ClosePoint - Position);
				FLOAT ThisDistSqr = ThisToPath.SizeSquared();
				if(ThisToPath.SizeSquared() < ClosestDistSqr)
				{
					ClosestDistSqr = ThisDistSqr;
					ClosestReachSpec = Spec;
				}
			}
		}
	}

	return ClosestReachSpec;
}

/** Utility that returns sees if we are off the path network, and if so, gives unit dir vector and distance to get back onto it. */
static UBOOL IsOffPath(const UReachSpec* Spec, const FVector& Position, FLOAT DesiredPathDist, FLOAT& OutDist, FVector& ToPath, UBOOL bDrawDebugPathInfo)
{
	ToPath = FVector(0,0,0);
	OutDist = 0.f;
#if !FINAL_RELEASE
	FVector ClosestClosePoint = FVector(0,0,0);
#endif

	if(Spec)
	{
		ANavigationPoint* StartNav = Spec->Start;
		ANavigationPoint* EndNav = (ANavigationPoint*)Spec->End.Actor;

		if(StartNav && EndNav)
		{
			FVector ReachStart = StartNav->Location;
			FVector ReachEnd = EndNav->Location;

			// Find closest point on edge from given location
			FVector ClosePoint = ClosestPointOnLine(ReachStart, ReachEnd, Position);
			FVector ThisToPath = (ClosePoint - Position);
			FLOAT ThisDist = ThisToPath.Size() - DesiredPathDist;

			// If we are 'within' a path - return no vector - no need to move the agent
			if(ThisDist <= 0.f)
			{
				ToPath = (ReachStart - ReachEnd).SafeNormal();
				OutDist = 0.f;

#if !FINAL_RELEASE
				if(bDrawDebugPathInfo)
				{
					GWorld->LineBatcher->DrawLine(ReachStart, ReachEnd, FColor(255,255,255), SDPG_World);
					GWorld->LineBatcher->DrawLine(ClosePoint, Position, FColor(0,255,255), SDPG_World);
				}
#endif

				return FALSE;
			}
			// If not - remember how far away we were
			else
			{
#if !FINAL_RELEASE
				ClosestClosePoint = ClosePoint;
#endif
				ToPath = ThisToPath;
				OutDist = ThisDist;
			}

		}
	}

#if !FINAL_RELEASE
	if(bDrawDebugPathInfo)
	{
		GWorld->LineBatcher->DrawLine(ClosestClosePoint, Position, FColor(255,0,0), SDPG_World);
	}
#endif

	// Return vector to closest point
	ToPath.Normalize();
	return TRUE;
}

////////////////////////////////////////////////////
// ACROWDAGENT
////////////////////////////////////////////////////

void ACrowdAgent::PreBeginPlay()
{
	GetLevel()->CrossLevelActors.AddItem(this);

	Super::PreBeginPlay();
}

void ACrowdAgent::PostScriptDestroyed()
{
	//@note: this won't be called if the agent is simply GC'ed due to level change/removal,
	// but in that case the level must be being GC'ed as well, making this unnecessary
	GetLevel()->CrossLevelActors.RemoveItem(this);

	Super::PostScriptDestroyed();
}

void ACrowdAgent::GetActorReferences(TArray<FActorReference*>& ActorRefs, UBOOL bIsRemovingLevel)
{
	if (bIsRemovingLevel)
	{
		// simply clear our references regardless as we can easily regenerate them from the remaining levels
		NearestPath = NULL;
	}
	Super::GetActorReferences(ActorRefs, bIsRemovingLevel);
}

/** Override physics update to do our own movement stuff */
void ACrowdAgent::performPhysics(FLOAT DeltaTime)
{
	// if the spawner no longer exists or isn't currently initialized (streamed out)
	if(!Spawner)// || !Spawner->bScriptInitialized)
	{
		// delete and return
		GWorld->DestroyActor(this);
		return;
	}

	// If crowds disable - keep active but don't spawn any crowd members
	if(GWorld->bDisableCrowds)
	{
		return;
	}

	Spawner->UpdateAgent(this, DeltaTime);
}

void ACrowdAgent::SetAgentMoveState(BYTE NewState)
{
	AgentState = NewState;

	if((NewState == EAMS_Move) && ActionSeqNode)
	{
		ActionSeqNode->RootBoneOption[0] = RBA_Default;
		ActionSeqNode->RootBoneOption[1] = RBA_Default;
		ActionSeqNode->RootBoneOption[2] = RBA_Default;
		SkeletalMeshComponent->RootMotionMode = RMM_Ignore;
	}
}

void ACrowdAgent::DoAction(UBOOL bAtTarget, const FVector& TargetLoc)
{
	SetAgentMoveState(EAMS_Idle);

	if(ActionSeqNode)
	{
		// Choose from different set if a 'targetted' action
		if(bAtTarget)
		{
			if(Spawner->TargetActions.Num() > 0)
			{
				const INT AnimIndex = appRand() % Spawner->TargetActions.Num();
				ActionSeqNode->SetAnim( Spawner->TargetActions(AnimIndex).AnimName );
				ActionSeqNode->PlayAnim(FALSE, 1.0, 0.0);

				// If desired, call script event to fire particle effects (if we are allowed more this frame)
				if(Spawner->TargetActions(AnimIndex).bFireEffects)
				{
					if(Spawner->RemaingEffectsThisFrame > 1.f)
					{
						SpawnActionEffect(TargetLoc);
						// Decrement remaining calls allowed.
						Spawner->RemaingEffectsThisFrame -= 1.f;
					}
				}			
			}

			// Rotate agent to target
			FVector ToTargetDir = (TargetLoc - Location);
			ToTargetRot = ToTargetDir.Rotation();
			ToTargetRot.Pitch = 0;
			bRotateToTargetRot = TRUE;
		}
		else
		{
			if(Spawner->ActionAnimNames.Num() > 0)
			{
				const INT AnimIndex = appRand() % Spawner->ActionAnimNames.Num();
				ActionSeqNode->SetAnim( Spawner->ActionAnimNames(AnimIndex) );
				ActionSeqNode->PlayAnim(FALSE, 1.0, 0.0);
			}
			bRotateToTargetRot = FALSE;
		}
	}

	if(ActionBlendNode)
	{
		ActionBlendNode->SetBlendTarget( 1.f, Spawner->ActionBlendTime );
	}

	EndActionTime = GWorld->GetTimeSeconds() + Spawner->ActionDuration.GetValue(0.f, NULL);
}

/** Stop agent moving and pay death anim */
void ACrowdAgent::PlayDeath()
{
	SetAgentMoveState(EAMS_Idle);

	if((Spawner->DeathAnimNames.Num() > 0) && ActionSeqNode && ActionBlendNode)
	{
		const INT AnimIndex = appRand() % Spawner->DeathAnimNames.Num();
		ActionSeqNode->SetAnim( Spawner->DeathAnimNames(AnimIndex) );
		ActionSeqNode->PlayAnim(FALSE, 1.0, 0.0);

		ActionBlendNode->SetBlendTarget( 1.f, Spawner->ActionBlendTime );
	}

	bRotateToTargetRot = FALSE;
	EndActionTime = BIG_NUMBER; // Never end this move.
}

UBOOL ACrowdAgent::ShouldTrace(UPrimitiveComponent* Primitive, AActor *SourceActor, DWORD TraceFlags)
{
	if(SourceActor && SourceActor->IsA(AWeapon::StaticClass()) && SourceActor->Instigator == NULL)
	{
		return FALSE;
	}

	return Super::ShouldTrace(Primitive, SourceActor, TraceFlags);
}

////////////////////////////////////////////////////
// USEQACT_CROWDSPAWNER
////////////////////////////////////////////////////

void USeqAct_CrowdSpawner::PostLoad()
{
	Super::PostLoad();

	// Convert from TargetActionAnimNames to TargetActions
	if(TargetActionAnimNames.Num() > 0 && TargetActions.Num() == 0)
	{
		TargetActions.AddZeroed(TargetActionAnimNames.Num());

		for(INT i=0; i<TargetActionAnimNames.Num(); i++)
		{
			TargetActions(i).AnimName = TargetActionAnimNames(i);
		}
	}

	// Convert from FlockMesh to FlockMeshes
	if(FlockMeshes.Num() == 0 && FlockMesh != NULL)
	{
		FlockMeshes.AddItem(FlockMesh);
		FlockMesh = NULL;
	}

	// Convert from WalkAnimName to WalkAnimNames
	if(WalkAnimNames.Num() == 0 && WalkAnimName != NAME_None)
	{
		WalkAnimNames.AddItem(WalkAnimName);
		WalkAnimName = NAME_None;
	}

	// Convert from RunAnimName to RunAnimNames
	if(RunAnimNames.Num() == 0 && RunAnimName != NAME_None)
	{
		RunAnimNames.AddItem(RunAnimName);
		RunAnimName = NAME_None;
	}
}

void USeqAct_CrowdSpawner::KillAgents()
{
	// Iterate over list of dudes
	for(INT i=0; i<SpawnedList.Num(); i++)
	{
		ACrowdAgent* Agent = SpawnedList(i);
		if(Agent)
		{
			GWorld->DestroyActor(Agent);
		}
	}
	SpawnedList.Empty();

	// Tell clients if necessary
	if(GWorld->GetNetMode() != NM_Client && RepActor)
	{
		RepActor->DestroyAllCount++;
		RepActor->bNetDirty = TRUE;
	}
}

void USeqAct_CrowdSpawner::UpdateSpawning(float DeltaSeconds)
{
	UpdateOp(DeltaSeconds);
}


/** Cache SpawnLocs and AssignedMoveTargets from attached Kismet vars. */
void USeqAct_CrowdSpawner::CacheSpawnerVars()
{
	// Cache spawn locations.
	SpawnLocs.Empty();
	TArray<UObject**> Objs;
	GetObjectVars(Objs,TEXT("Target"));
	for(INT Idx = 0; Idx < Objs.Num(); Idx++)
	{
		AActor* TestActor = Cast<AActor>( *(Objs(Idx)) );
		if (TestActor != NULL)
		{
			// use the pawn instead of the controller
			AController* C = Cast<AController>(TestActor);
			if (C != NULL && C->Pawn != NULL)
			{
				TestActor = C->Pawn;
			}

			SpawnLocs.AddItem(TestActor);
		}
	}

	// Cache the set of assigned movetargets and action targets
	AssignedMoveTargets.Empty();
	AssignedActionTargets.Empty();
	TArray<UObject**> OutObjects;
	GetObjectVars(OutObjects, TEXT("Attractors"));

	for(INT i=0; i<OutObjects.Num(); i++)
	{
		if(OutObjects(i))
		{
			ACrowdAttractor* TmpAttractor = Cast<ACrowdAttractor>(*OutObjects(i));
			if(TmpAttractor)
			{
				if((TmpAttractor->Mode == ECAM_MoveTarget) && (TmpAttractor->Attraction > KINDA_SMALL_NUMBER))
				{
					AssignedMoveTargets.AddItem(TmpAttractor);
				}

				if(TmpAttractor->bActionAtThisAttractor)
				{
					AssignedActionTargets.AddItem(TmpAttractor);
				}
			}
		}
	}
}

void USeqAct_CrowdSpawner::Activated()
{
	// START
	if( InputLinks(0).bHasImpulse )
	{
		bSpawningActive = TRUE;

		CacheSpawnerVars();

		if(bReduceNumInSplitScreen && GEngine->IsSplitScreen() && !bHasReducedNumberDueToSplitScreen)
		{
			SpawnNum = appCeil(SpawnNum * SplitScreenNumReduction);
		}
	}
	// STOP
	else if( InputLinks(1).bHasImpulse )
	{
		bSpawningActive = FALSE;
	}
	// DESTROY ALL
	else if (InputLinks(2).bHasImpulse)
	{
		KillAgents();

		// Stop spawning as well
		bSpawningActive = FALSE;
	}
}

UBOOL USeqAct_CrowdSpawner::UpdateOp(FLOAT DeltaSeconds)
{
	// START
	if( InputLinks(0).bHasImpulse )
	{
		bSpawningActive = TRUE;
	}
	// STOP
	else if( InputLinks(1).bHasImpulse )
	{
		bSpawningActive = FALSE;
	}
	// DESTROY ALL
	else if (InputLinks(2).bHasImpulse)
	{
		KillAgents();

		// Stop spawning as well
		bSpawningActive = FALSE;
	}

	InputLinks(0).bHasImpulse = 0;
	InputLinks(1).bHasImpulse = 0;
	InputLinks(2).bHasImpulse = 0;

	// Create actor to replicate crowd control to clients
	if(GWorld->GetNetMode() != NM_Client)
	{
		if(!RepActor)
		{
			RepActor = (ACrowdReplicationActor*)GWorld->SpawnActor(ACrowdReplicationActor::StaticClass());
			RepActor->Spawner = this;
		}

		if(RepActor)
		{
			if(RepActor->bSpawningActive != bSpawningActive)
			{
				RepActor->bSpawningActive = bSpawningActive;
				RepActor->bNetDirty = TRUE;
			}
		}
	}


	INT Idx = 0;
	while(Idx < SpawnedList.Num())
	{
		if(!SpawnedList(Idx) || SpawnedList(Idx)->bDeleteMe)
		{
			SpawnedList.Remove(Idx--);

			// If we don't want to spawn more 
			if(!bRespawnDeadAgents)
			{
				SpawnNum--;
			}
		}
		Idx++;
	}

	// If crowds disable - keep active but don't spawn any crowd members
	if(GWorld->bDisableCrowds)
	{
		return FALSE;
	}

	// No longer spawning - stop updating
	if(!bSpawningActive)
	{
		return TRUE;
	}

	// Active but too many out - keep active
	if(SpawnedList.Num() >= SpawnNum)
	{
		return FALSE;
	}

	// No locations found - don't spawn - stop being active
	if(SpawnLocs.Num() == 0)
	{
		return TRUE;
	}

	Remainder += (DeltaSeconds * SpawnRate);

	while(Remainder > 1.f && SpawnedList.Num() < SpawnNum)
	{
		INT SpawnIndex = appRand() % SpawnLocs.Num();
		ACrowdAgent* NewAgent = eventSpawnAgent(SpawnLocs(SpawnIndex));
		if(NewAgent)
		{
			SpawnedList.AddItem(NewAgent);

			// If there is a spawning anim, play that now.
			if((SpawnAnimName != NAME_None) && NewAgent->ActionSeqNode && NewAgent->ActionBlendNode)
			{
				NewAgent->SetAgentMoveState(EAMS_Idle);
				NewAgent->ActionSeqNode->SetAnim(SpawnAnimName);
				NewAgent->ActionSeqNode->PlayAnim(FALSE, 1.0, 0.0);
				NewAgent->bRotateToTargetRot = FALSE;
				NewAgent->ActionBlendNode->SetBlendTarget( 1.f, 0.f );
				NewAgent->EndActionTime = GWorld->GetTimeSeconds() + NewAgent->ActionSeqNode->GetAnimPlaybackLength();

				// Use root motion for spawn anim
				NewAgent->ActionSeqNode->RootBoneOption[0] = RBA_Translate;
				NewAgent->ActionSeqNode->RootBoneOption[1] = RBA_Translate;
				NewAgent->ActionSeqNode->RootBoneOption[2] = RBA_Translate;
				NewAgent->SkeletalMeshComponent->RootMotionMode = RMM_Translate;
			}
			// Otherwise, just init the first action time.
			else
			{
				if(ActionAnimNames.Num() > 0)
				{
					NewAgent->NextActionTime = GWorld->GetTimeSeconds() + ActionInterval.GetValue(0.f, NULL);
				}
			}
		}

		Remainder -= 1.0;
	}

	// Re-allocate number of times we can call SpawnActionEffect this frame.
	RemaingEffectsThisFrame += (MaxEffectsPerSecond * DeltaSeconds);
	// Make sure we don't stockpile too many effects!
	// Allow no more than 2 or MaxEffectsPerSecond (whichever is greater)
	FLOAT MaxRemaining = ::Max(2.f, MaxEffectsPerSecond);
	RemaingEffectsThisFrame = ::Min(RemaingEffectsThisFrame, MaxRemaining);

	// Keep active
	return FALSE;
}

/** Called when level is unstreamed */
void USeqAct_CrowdSpawner::CleanUp()
{
	// Destroy actor used for replication when action is cleaned up
	if(RepActor)
	{
		GWorld->DestroyActor(RepActor);
		RepActor = NULL;
	}
}


INT FindDeltaYaw(const FRotator& A, const FRotator& B)
{
	INT DeltaYaw = A.Yaw - B.Yaw;
	if(DeltaYaw > 32768)
		DeltaYaw -= 65536;
	else if(DeltaYaw < -32768)
		DeltaYaw += 65536;

	return DeltaYaw;
}

/** Util to see if a position is contained in an array of positions. */
static UBOOL ArrayContainsPosition(const TArray<FVector>& Positions, const FVector& Pos)
{
	for(INT i=0; i<Positions.Num(); i++)
	{
		if(Positions(i).Equals(Pos))
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Update position of an 'agent' in a crowd, using properties in the action. */
void USeqAct_CrowdSpawner::UpdateAgent(ACrowdAgent* Agent, FLOAT DeltaTime)
{
	check(Agent);

	FVector DesireForce = Agent->ExternalForce;
	Agent->ExternalForce = FVector(0,0,0);

	FVector	InhibitForce(0,0,0);

	FVector NearestActionTarget(0,0,0);
	FLOAT NearestActionTargetDist = BIG_NUMBER;

	TArray<FVector> BestAttractorPositions;
	FLOAT BestAttractorValue = -BIG_NUMBER;

	FVector	RepulsorForce(0,0,0);

	UBOOL bNearbyTarget = FALSE;

#if !FINAL_RELEASE
	// If showing DEBUG info - show 
	if(bDrawDebugHitBox)
	{
		FVector CollisionExtent = Agent->SkeletalMeshComponent->LineCheckBoundsScale * Agent->SkeletalMeshComponent->Bounds.BoxExtent;
		FLOAT ZOffset = (Agent->SkeletalMeshComponent->Bounds.BoxExtent.Z - CollisionExtent.Z); // Offset box down so bottom stays in same place
		FVector CollisionOrigin = Agent->SkeletalMeshComponent->Bounds.Origin - (FVector(0,0,1) * ZOffset);
		FBox CollisionBox(CollisionOrigin-CollisionExtent, CollisionOrigin+CollisionExtent);
		GWorld->LineBatcher->DrawBox(CollisionBox, FMatrix::Identity, FColor(255,255,255), SDPG_World);
	}
#endif

	// Update aware check frame count and see if its time to do it again.
	Agent->AwareUpdateFrameCount++;
	if(Agent->AwareUpdateFrameCount >= AwareUpdateInterval)
	{
		Agent->AwareUpdateFrameCount = 0;

		// If crowd attractor octree present, query it for attractors
		Agent->RelevantAttractors.Reset();
		if(GWorld->CrowdAttractorOctree)
		{
			FBoxCenterAndExtent QueryBox(Agent->Location, FVector(AwareRadius,AwareRadius,AwareRadius));

			// Iterate over the octree nodes containing the query point.
			for(FCrowdAttractorOctreeType::TConstElementBoxIterator<> OctreeIt(*(GWorld->CrowdAttractorOctree),QueryBox); OctreeIt.HasPendingElements(); OctreeIt.Advance())
			{
				ACrowdAttractor* Attractor = OctreeIt.GetCurrentElement();
				check(Attractor);

				Agent->RelevantAttractors.AddItem(Attractor);
			}
		}

		// Query main collision octree for other agents nearby
		Agent->NearbyAgents.Reset();
		FMemMark Mark(GMainThreadMemStack);
		FCheckResult* Link = GWorld->Hash->ActorOverlapCheck(GMainThreadMemStack, Agent, Agent->Location, AwareRadius);
		for( FCheckResult* result=Link; result; result=result->GetNext())
		{
			check(result->Actor);
			// Look for nearby agents, find average velocity, and repel from them
			ACrowdAgent* FlockActor = Cast<ACrowdAgent>(result->Actor);
			if(FlockActor)
			{
				Agent->NearbyAgents.AddItem(FlockActor);
			}

			// Check if we want notification of this overlap
			for(INT i=0; i<ReportOverlapsWithClass.Num(); i++)
			{
				// Check class
				if(result->Actor->IsA(ReportOverlapsWithClass(i)))
				{
					// fire event
					Agent->eventOverlappedActorEvent(result->Actor);
				}
			}
		}
		Mark.Pop();

		// Find nearest reachspec to us
		Agent->NearestPath = FindNearestReachspec(Agent->Location, bUseOnlyCrowdPaths, PathDistance);
	}

	// Iterate over relevant attractors to update attraction/repulsion
	for(INT i=0; i<Agent->RelevantAttractors.Num(); i++)
	{
		ACrowdAttractor* Attractor = Agent->RelevantAttractors(i);
		if(Attractor)
		{
			FVector ToAttractor = Attractor->Location - Agent->Location;

			// If enabled, have it add force to agent
			if(Attractor->bAttractorEnabled)
			{
				const FLOAT Distance = ToAttractor.Size();
				FLOAT AttractRadius = Attractor->CylinderComponent->CollisionRadius;

				// If desired, kill actor when it reaches an attractor
				if(Attractor->bKillWhenReached && Distance < Attractor->KillDist)
				{
					Agent->LifeSpan = 0.01f;
				}

				if(Distance <= AttractRadius)
				{
					// Normalize vector from location to actor.
					ToAttractor = ToAttractor / Distance;

					FLOAT Attraction = Attractor->Attraction;

					// If desired, do falloff
					if(Attractor->bAttractionFalloff)
					{
						Attraction *= (1.f - (Distance / AttractRadius));
					}

					if(Attractor->Mode == ECAM_Repulsor)
					{
						RepulsorForce += (ToAttractor * Attraction);
#if !FINAL_RELEASE
						if(bDrawDebugMoveTarget)
						{
							GWorld->LineBatcher->DrawLine(Agent->Location, Agent->Location + (ToAttractor * Attraction), FColor(255,255,0), SDPG_World);
						}
#endif
					}
					// Only consider this move target if none assigned, or its one of the assigned ones
					// Also, must have non-zero attraction
					else if((AssignedMoveTargets.Num() == 0 || AssignedMoveTargets.ContainsItem(Attractor)) && Attraction > KINDA_SMALL_NUMBER)
					{
						FLOAT AttractionValue = Abs(Attraction);
						// See if it is better than our current set
						if(AttractionValue > (BestAttractorValue + KINDA_SMALL_NUMBER))
						{
							BestAttractorPositions.Reset();
							BestAttractorPositions.AddItem(Attractor->Location);

							BestAttractorValue = AttractionValue;
						}
						// The same as our current set - add to it.
						else if(Abs(AttractionValue - BestAttractorValue) < KINDA_SMALL_NUMBER)
						{
							BestAttractorPositions.AddItem(Attractor->Location);
						}
					}
				}

				FLOAT ActionRadius = AttractRadius / Attractor->ActionRadiusScale;

				// If its an action target, see if its the closest one so far and save location.
				// Also if we assigned some action targets, see if its in that subset
				if(	Attractor->bActionAtThisAttractor && 
					(AssignedActionTargets.Num() == 0 || AssignedActionTargets.ContainsItem(Attractor)) &&
					(Distance < ActionRadius) && 
					(Distance < NearestActionTargetDist))
				{
					// Use 'action target' location if present.
					if(Attractor->ActionTarget)
					{
						NearestActionTarget = Attractor->ActionTarget->Location;
					}
					else
					{
						NearestActionTarget = Attractor->Location;
					}

					NearestActionTargetDist = Distance;
					bNearbyTarget = TRUE;
				}
			}
		}
	}


	// Iterate over nearby agents to update velocity, repulsive forces etc
	FVector FlockVel(0,0,0);
	INT FlockCount = 0;

	for(INT i=0; i<Agent->NearbyAgents.Num(); i++)
	{
		ACrowdAgent* FlockActor = Agent->NearbyAgents(i);
		if(FlockActor)
		{
			// Update
			FlockVel += FlockActor->Velocity;

			FVector ToFlockActor = FlockActor->Location - Agent->Location;
			FLOAT ToFlockActorMag = ToFlockActor.Size();
			FLOAT Overlap = AvoidOtherRadius - ToFlockActorMag;
			if(Overlap > 0.f)
			{
				// normalize
				ToFlockActor /= ToFlockActorMag;

				InhibitForce += ((Overlap/AvoidOtherRadius) * -ToFlockActor * AvoidOtherStrength);
			}

			FlockCount++;
		}
	}

	// Average location and velocity for nearby agents.
	if(FlockCount > 0)
	{
		FlockVel /= (FLOAT)FlockCount;
	}

	// Match velocity
	DesireForce += (FlockVel - Agent->Velocity) * MatchVelStrength;

	// To target attraction
	FVector ToAttractor(0,0,0); 
	if(BestAttractorPositions.Num() > 0)
	{
		// See if our current move target is still a possibility, If not, pick a new one.
		if(!ArrayContainsPosition(BestAttractorPositions, Agent->CurrentMoveTargetPos))
		{
			Agent->CurrentMoveTargetPos = BestAttractorPositions(appRand() % BestAttractorPositions.Num());
		}

		ToAttractor = (Agent->CurrentMoveTargetPos - Agent->Location).SafeNormal();
		DesireForce += ToAttractorStrength * ToAttractor;

#if !FINAL_RELEASE
		if(bDrawDebugMoveTarget)
		{
			GWorld->LineBatcher->DrawLine(Agent->Location, Agent->CurrentMoveTargetPos, FColor(200,0,255), SDPG_World);
		}
#endif // !FINAL_RELEASE
	}

	// Repulsor force
	InhibitForce += RepulsorForce;

	// To path attraction 
	FLOAT DistToPath;
	FVector DirToPath;
	if(IsOffPath(Agent->NearestPath, Agent->Location, PathDistance, DistToPath, DirToPath, bDrawDebugPathInfo))
	{
		FVector PathForce = (DirToPath * ToPathStrength);
		//GWorld->LineBatcher->DrawLine(Agent->Location, Agent->Location + (PathForce * 50.f), FColor(0,255,255), SDPG_World);
		InhibitForce += (PathForce);
	}
	else
	{
		if((ToAttractor | DirToPath) > 0.f)
		{
			DesireForce += DirToPath * FollowPathStrength;
		}
		else
		{
			DesireForce -= DirToPath * FollowPathStrength;
		}
	}

	// Velocity damping
	FLOAT VMag = Agent->Velocity.Size();
	InhibitForce -= (Agent->Velocity * VMag * Agent->VelDamping);

	// Ensure forces only in Z plane.
	DesireForce.Z = 0;
	InhibitForce.Z = 0;

	// If we just saw a target (and we have target action anims) - reset 'next action' time to use target action interval (more frequent)
	if(bNearbyTarget && !Agent->bHadNearbyTarget && TargetActions.Num() > 0)
	{
		Agent->NextActionTime = GWorld->GetTimeSeconds() + TargetActionInterval.GetValue(0.f, NULL);
	}

	// If currently doing action.
	if(Agent->AgentState == EAMS_Idle)
	{
		// If action just finished
		if(GWorld->GetTimeSeconds() > Agent->EndActionTime)
		{
			if(Agent->ActionBlendNode)
			{
				Agent->ActionBlendNode->SetBlendTarget( 0.f, ActionBlendTime );
			}
			Agent->SetAgentMoveState(EAMS_Move);


			if(bNearbyTarget)
			{
				if(TargetActions.Num() > 0)
				{
					Agent->NextActionTime = GWorld->GetTimeSeconds() + TargetActionInterval.GetValue(0.f, NULL);
				}
			}
			else
			{
				if(ActionAnimNames.Num() > 0)
				{
					Agent->NextActionTime = GWorld->GetTimeSeconds() + ActionInterval.GetValue(0.f, NULL);
				}
			}
		}
	}
	else if(Agent->ActionSeqNode && (TargetActions.Num() > 0 || ActionAnimNames.Num() > 0))
	{
		UBOOL bOkToAction = (GWorld->GetTimeSeconds() - Agent->EndActionTime > ReActionDelay);
		UBOOL bTimeToAction = (GWorld->GetTimeSeconds() > Agent->NextActionTime);

		// First see if we want to do an action
		if( bOkToAction && bTimeToAction )
		{
			Agent->DoAction(bNearbyTarget, NearestActionTarget);
		}
		else
		{
			Agent->SetAgentMoveState(EAMS_Move);
		}
	}

	// Blend between running and walking anim
	if(Agent->SpeedBlendNode)
	{
		FLOAT CurrentWeight = Agent->SpeedBlendNode->Child2Weight;

		FLOAT TargetWeight = ((VMag - SpeedBlendStart)/(SpeedBlendEnd - SpeedBlendStart));
		TargetWeight = ::Clamp<FLOAT>(TargetWeight, 0.f, 1.f);

		// limit how quickly anim rate can change
		FLOAT DeltaWeight = (TargetWeight - CurrentWeight);
		FLOAT MaxScaleChange = MaxSpeedBlendChangeSpeed * DeltaTime;
		DeltaWeight = Clamp(DeltaWeight, -MaxScaleChange, MaxScaleChange);

		Agent->SpeedBlendNode->SetBlendTarget(CurrentWeight + DeltaWeight, 0.f);
	}

	// Change anim rate based on speed
	Agent->AgentTree->SetGroupRateScale(MoveSyncGroupName, VMag * AnimVelRate);
		
	// Force velocity to zero while idling.
	if(Agent->AgentState == EAMS_Idle)
	{
		Agent->Velocity = FVector(0,0,0);
		DesireForce = FVector(0,0,0);
		InhibitForce = FVector(0,0,0);

		// If desired, rotate pawn to look at target when performing action.
		if(Agent->bRotateToTargetRot)
		{
			INT DeltaYaw = FindDeltaYaw(Agent->ToTargetRot, Agent->Rotation);
			FRotator NewRotation = Agent->Rotation;
			NewRotation.Yaw += appRound((FLOAT)DeltaYaw * RotateToTargetSpeed);
			Agent->SetRotation(NewRotation);
		}
	}
	else
	{
		// Integrate force to get new velocity
		Agent->Velocity += (DesireForce + InhibitForce) * DeltaTime;

		// Integrate velocity to get new position.
		FVector NewLocation = (Agent->Location + (Agent->Velocity * DeltaTime));

		// If desired, use ZELC to follow ground
		Agent->ConformTraceFrameCount++;
		if(bConformToBSP || bConformToWorld)
		{
			// See if enough frames have passed to update target Z again
			if(Agent->ConformTraceFrameCount >= ConformTraceInterval)
			{
				Agent->ConformTraceFrameCount = 0;

				FVector LineStart = NewLocation + (ConformTraceDist * FVector(0,0,1));
				FVector LineEnd = NewLocation - (ConformTraceDist * FVector(0,0,1));

				DWORD TraceFlags = bConformToWorld ? TRACE_World : TRACE_Level;

#if !FINAL_RELEASE
				if(bDrawDebugPathInfo)
				{
					GWorld->LineBatcher->DrawLine(LineStart, LineEnd, FColor(0,128,200), SDPG_World);
				}
#endif

				// Ground-conforming stuff
				UBOOL bHitGround = FALSE;
				FLOAT GroundHitTime = 1.f;
				FVector GroundHitLocation(0,0,0);

				// Agent-killing stuff
				UBOOL bKillAgent = FALSE;
				UBOOL bPlayDeathAnim = FALSE;

				// Line trace down and look through results
				FMemMark LineMark(GMainThreadMemStack);
				FCheckResult* Result = GWorld->MultiLineCheck(GMainThreadMemStack, LineEnd, LineStart, FVector(0,0,0), TraceFlags | TRACE_Volumes, Agent);
				for(FCheckResult* Temp=Result; Temp; Temp=Temp->GetNext())
				{
					// If we hit something
					if(Result->Actor)
					{
						// If volume, look for a pain volume that would kill agent
						AVolume* Volume = Result->Actor->GetAVolume();
						if(Volume)
						{
							APhysicsVolume* PhysVol = Cast<APhysicsVolume>(Volume);
							if(PhysVol && PhysVol->bPainCausing)
							{
								// See if we want to destroy actor, or play the death anim
								if(PhysVol->bCrowdAgentsPlayDeathAnim)
								{
									bPlayDeathAnim = TRUE;
								}
								else
								{
									bKillAgent = TRUE;
								}

								break;
							}
						}
						// Didn't hit a volume, so treat this as ground - look for nearest hit
						else if(Result->Time < GroundHitTime)
						{
							bHitGround = TRUE;
							GroundHitTime = Result->Time;
							GroundHitLocation = Result->Location;
						}
					}
				}
				LineMark.Pop();

				// First handle dying cases
				if(bKillAgent)
				{
					Agent->LifeSpan = 0.01f;
					return;
				}
				else if(bPlayDeathAnim)
				{
					Agent->PlayDeath();
					return;
				}

				FLOAT TargetZ = NewLocation.Z;
				// If we hit something - move to that point
				if(bHitGround)
				{
					// If you end up embedded in the world - kill the crowd member
					if(GroundHitTime < KINDA_SMALL_NUMBER)
					{
						Agent->LifeSpan = 0.01f;
					}
					// Otherwise just position at end of line trace.
					else
					{
						TargetZ = GroundHitLocation.Z;
					}
				}
				// If we didn't move to bottom of line check
				else
				{
					TargetZ = LineEnd.Z;
				}

				Agent->InterpZTranslation = (TargetZ - NewLocation.Z)/((FLOAT)ConformTraceInterval);

				Agent->bTargetZPosInitialized = TRUE;
			}

			// Move location towards Z target, at ConformZVel speed
			NewLocation.Z += Agent->InterpZTranslation;
		}

		// Point in direction of travel
		FRotator NewRotation = Agent->Velocity.Rotation();

		// Cap the maximum yaw rate
		INT DeltaYaw = FindDeltaYaw(NewRotation, Agent->Rotation);
		INT MaxYaw = appRound(DeltaTime * MaxYawRate);
		DeltaYaw = ::Clamp(DeltaYaw, -MaxYaw, MaxYaw);
		NewRotation.Yaw = Agent->Rotation.Yaw + DeltaYaw;

		// Actually move the Actor
		FCheckResult Hit(1.f);
		GWorld->MoveActor(Agent, NewLocation - Agent->Location, NewRotation, 0, Hit);
	}

	Agent->bHadNearbyTarget = bNearbyTarget;
}

void ACrowdAttractor::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	// Change colour based on usage
	if(Mode == ECAM_MoveTarget)
	{
		CylinderComponent->CylinderColor = FColor(0,255,0);
	}
	else
	{
		CylinderComponent->CylinderColor = FColor(0,0,255);
	}
}

void ACrowdAttractor::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	if ( bCtrlDown )
	{
		// CTRL+Scaling modifies trigger collision height.  This is for convenience, so that height
		// can be changed without having to use the non-uniform scaling widget (which is
		// inaccessable with spacebar widget cycling).
		CylinderComponent->CollisionHeight += ModifiedScale.X;
		CylinderComponent->CollisionHeight = Max( 0.0f, CylinderComponent->CollisionHeight );
	}
	else
	{
		CylinderComponent->CollisionRadius += ModifiedScale.X;
		CylinderComponent->CollisionRadius = Max( 0.0f, CylinderComponent->CollisionRadius );

		// If non-uniformly scaling, Z scale affects height and Y can affect radius too.
		if ( !ModifiedScale.AllComponentsEqual() )
		{
			CylinderComponent->CollisionHeight += -ModifiedScale.Z;
			CylinderComponent->CollisionHeight = Max( 0.0f, CylinderComponent->CollisionHeight );

			CylinderComponent->CollisionRadius += ModifiedScale.Y;
			CylinderComponent->CollisionRadius = Max( 0.0f, CylinderComponent->CollisionRadius );
		}
	}
}

void ACrowdAttractor::PreSave()
{
	Super::PreSave();

	if(CylinderComponent)
	{
		AttractionRadius = CylinderComponent->CollisionRadius;
		AttractionHeight = CylinderComponent->CollisionHeight;
	}
}

void ACrowdAttractor::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	Super::UpdateComponentsInternal();

	if(GWorld)
	{
		// Create octree if not present
		if(!GWorld->CrowdAttractorOctree)
		{
			GWorld->CrowdAttractorOctree = new FCrowdAttractorOctreeType(FVector(0,0,0), HALF_WORLD_MAX);
			check(GWorld->CrowdAttractorOctree);
		}

		if(OctreeId.IsValidId())
		{
			check(GWorld->CrowdAttractorOctree->GetElementById(OctreeId) == this);
			GWorld->CrowdAttractorOctree->RemoveElement(OctreeId);
			OctreeId = FOctreeElementId(); // clear Id
		}

		check(!OctreeId.IsValidId());
		GWorld->CrowdAttractorOctree->AddElement(this);
		check(OctreeId.IsValidId());
	}
}

void ACrowdAttractor::ClearComponents()
{
	Super::ClearComponents();

	if(GWorld)
	{
		// Do nothing if no crowd octree around
		if(!GWorld->CrowdAttractorOctree)
		{
			return;
		}

		if(OctreeId.IsValidId())
		{
			check(GWorld->CrowdAttractorOctree->GetElementById(OctreeId) == this);
			GWorld->CrowdAttractorOctree->RemoveElement(OctreeId);
			OctreeId = FOctreeElementId(); // clear Id
		}
		check(!OctreeId.IsValidId());
	}
}

/** Delete the crowd octree if present. */
void UWorld::DeleteCrowdOctree()
{
	if(CrowdAttractorOctree)
	{
		delete CrowdAttractorOctree;
		CrowdAttractorOctree = NULL;
	}
}


