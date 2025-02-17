//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"
#include "UnPath.h"
#include "DebugRenderSceneProxy.h"
#include "UTGameVehicleClasses.h"

IMPLEMENT_CLASS(AUTJumpPad);
IMPLEMENT_CLASS(UUTTrajectoryReachSpec);
IMPLEMENT_CLASS(UUTJumpPadReachSpec);
IMPLEMENT_CLASS(AUTTeamPlayerStart);
IMPLEMENT_CLASS(AUTScout);
IMPLEMENT_CLASS(AUTDefensePoint);
IMPLEMENT_CLASS(AUTHoldSpot);
IMPLEMENT_CLASS(AUTTranslocatorDest);

void AUTJumpPad::addReachSpecs(AScout *Scout, UBOOL bOnlyChanged)
{
	Super::addReachSpecs(Scout,bOnlyChanged);

	if ( GetGravityZ() >= 0.f )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("Jumppads not allowed in zero or positive gravity volumes!"));
		return;
	}

	// check that there are no other overlapping navigationpoints
	for (ANavigationPoint *Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		if (Nav != NULL &&
			!Nav->bDeleteMe &&
			Nav != this &&
			(!bOnlyChanged || bPathsChanged || Nav->bPathsChanged) ) 
		{
			FVector XYDir = Nav->Location - Location;
			FLOAT ZDiff = Abs(XYDir.Z);
			if ( ZDiff < CylinderComponent->CollisionHeight + Nav->CylinderComponent->CollisionHeight )
			{
				XYDir.Z = 0.f;
				FLOAT MinDist = CylinderComponent->CollisionRadius + Nav->CylinderComponent->CollisionRadius;
				if ( XYDir.SizeSquared() < MinDist*MinDist )
				{
					GWarn->MapCheck_Add( MCTYPE_ERROR, Nav, TEXT("NavigationPoints must not touch a jumppad!"));
				}
			}
		}
	}

	if ( !JumpTarget )
	{
		GWarn->MapCheck_Add( MCTYPE_ERROR, this, TEXT("No JumpTarget set for this JumpPad"));
		return;
	}

	if ( CalculateJumpVelocity(Scout) )
	{
		UReachSpec *newSpec = ConstructObject<UReachSpec>(UUTJumpPadReachSpec::StaticClass(),GWorld->GetOuter(),NAME_None);

		// debugf(TEXT("Add Jump Reachspec for JumpPad at (%f, %f, %f)"), Location.X,Location.Y,Location.Z);
		FVector HumanSize = Scout->GetSize(((AUTScout*)(AUTScout::StaticClass()->GetDefaultActor()))->SizePersonFindName);
		newSpec->CollisionRadius = appTrunc(HumanSize.X);
		newSpec->CollisionHeight = appTrunc(HumanSize.Y);
		newSpec->Start = this;
		newSpec->End = JumpTarget;
		newSpec->reachFlags = R_JUMP + R_WALK;

		FVector JumpXY = JumpVelocity;
		JumpXY.Z = 0.f;
		// scale distance based on speed
		newSpec->Distance = (INT)((Location - JumpTarget->Location).Size() * ::Min(1.f, 1.5f*Scout->TestGroundSpeed/JumpXY.Size()));
		PathList.AddItem(newSpec);
	}
	else
	{
		GWarn->MapCheck_Add(MCTYPE_ERROR, this, *FString::Printf(TEXT("Jump to %s cannot be successfully made"), *JumpTarget->GetName()));
	}
}

UBOOL AUTJumpPad::CalculateJumpVelocity(AScout *Scout)
{
	if ( !JumpTarget )
	{
		JumpVelocity = FVector(0.f, 0.f, 0.f);
		return FALSE;
	}

	FVector HumanSize = Scout->GetSize( ((AUTScout*)(AUTScout::StaticClass()->GetDefaultActor()))->SizePersonFindName);

	FVector Flight = JumpTarget->Location - Location;
	FLOAT FlightZ = Flight.Z;
	Flight.Z = 0.f;
	FLOAT FlightSize = Flight.Size();

	if ( FlightSize == 0.f )
	{
		JumpVelocity = FVector(0.f, 0.f, 0.f);
		return FALSE;
	}

	FLOAT Gravity = GetGravityZ(); 

	FLOAT XYSpeed = FlightSize/JumpTime;
	FLOAT ZSpeed = FlightZ/JumpTime - Gravity * JumpTime;

	// trace check trajectory
	UBOOL bFailed = TRUE;
	FVector FlightDir = Flight/FlightSize;
	FCheckResult Hit(1.f);

	// look for unobstructed trajectory, by increasing or decreasing flighttime
	UBOOL bDecreasing = TRUE;
	FLOAT AdjustedJumpTime = JumpTime;

	while ( bFailed )
	{
		FVector StartVel = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed);
		FLOAT StepSize = 0.0625f;
		FVector TraceStart = Location;
		bFailed = FALSE;

		// trace trajectory to make sure it isn't obstructed
		for ( FLOAT Step=0.f; Step<1.f; Step+=StepSize )
		{
			FLOAT FlightTime = (Step+StepSize) * AdjustedJumpTime;
			FVector TraceEnd = Location + StartVel*FlightTime + FVector(0.f, 0.f, Gravity * FlightTime * FlightTime);
			if ( !GWorld->SingleLineCheck( Hit, this, TraceEnd, TraceStart, TRACE_World|TRACE_StopAtAnyHit, HumanSize ) )
			{
				bFailed = TRUE;
				break;
			}
			TraceStart = TraceEnd;
		}

		if ( bFailed )
		{
			if ( bDecreasing )
			{
				AdjustedJumpTime -= 0.1f*JumpTime;
				if ( AdjustedJumpTime < 0.5f*JumpTime )
				{
					bDecreasing = FALSE;
					AdjustedJumpTime = JumpTime + 0.2f*JumpTime;
				}
			}
			else
			{
				AdjustedJumpTime += 0.2f*JumpTime;
				if ( AdjustedJumpTime > 2.f*JumpTime )
				{
					// no valid jump possible
					JumpVelocity = FVector(0.f, 0.f, 0.f);
					return FALSE;
				}

				XYSpeed = FlightSize/AdjustedJumpTime;
				ZSpeed = FlightZ/AdjustedJumpTime - Gravity * AdjustedJumpTime;
			}
		}
	}
	JumpVelocity = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed);
	return TRUE;
}


void UUTTrajectoryReachSpec::AddToDebugRenderProxy(FDebugRenderSceneProxy* DRSP)
{
	if ( Start && End.Nav() )
	{
		FVector InitialVelocity = GetInitialVelocity();
		if ( InitialVelocity.IsZero() )
		{
			Super::AddToDebugRenderProxy(DRSP);
			return;
		}
		FPlane PathColorPlane = PathColor();
		FLinearColor PathColorValue = FLinearColor(PathColorPlane.X, PathColorPlane.Y, PathColorPlane.Z, PathColorPlane.W);
		FLOAT TotalFlightTime = (End->Location - Start->Location).Size2D()/InitialVelocity.Size2D();
		FLOAT Gravity = Start->GetGravityZ(); 
		FLOAT StepSize = 0.0625f;
		FVector TraceStart = Start->Location;

		for ( FLOAT Step=StepSize; Step<=1.f; Step+=StepSize )
		{
			FLOAT FlightTime = Step * TotalFlightTime;
			FVector TraceEnd = Start->Location + InitialVelocity*FlightTime + FVector(0.f, 0.f, Gravity * FlightTime * FlightTime);
			if (Step + StepSize > 1.f)
			{
				new(DRSP->ArrowLines) FDebugRenderSceneProxy::FArrowLine(TraceStart, TraceEnd, PathColorValue);
			}
			else
			{
				new(DRSP->Lines) FDebugRenderSceneProxy::FDebugLine(TraceStart, TraceEnd, PathColorValue);
			}
			TraceStart = TraceEnd;
		}
	}
}

FVector UUTJumpPadReachSpec::GetInitialVelocity()
{
	AUTJumpPad* JumpStart = Cast<AUTJumpPad>(Start);
	return JumpStart ? JumpStart->JumpVelocity : FVector(0.f,0.f,0.f);
}

FVector UUTTranslocatorReachSpec::GetInitialVelocity()
{
	return CachedVelocity;
}

INT UUTJumpPadReachSpec::CostFor(APawn* P)
{
	// vehicles can't use jump pads
	return P->IsA(AVehicle::StaticClass()) ? UCONST_BLOCKEDPATHCOST : Super::CostFor(P);
}

void AUTJumpPad::PostEditChange(UProperty* PropertyThatChanged)
{
	AScout*	Scout = FPathBuilder::GetScout();
	CalculateJumpVelocity(Scout);
	FPathBuilder::DestroyScout();

	Super::PostEditChange(PropertyThatChanged);
}

void AUTJumpPad::PostEditMove(UBOOL bFinished)
{
	if ( bFinished )
	{
		AScout*	Scout = FPathBuilder::GetScout();
		CalculateJumpVelocity(Scout);
	}

	Super::PostEditMove( bFinished );
}

void AUTPickupFactory::PostEditMove(UBOOL bFinished)
{
	if ( bFinished )
	{
		// align pickupbase mesh to floor
		if ( BaseMesh )
		{
			FCheckResult Hit(1.f);
			FLOAT CollisionHeight, CollisionRadius;
			GetBoundingCylinder(CollisionRadius, CollisionHeight);
			GWorld->SingleLineCheck( Hit, this, Location - FVector(0.f,0.f,1.5f*CollisionHeight), Location, TRACE_World, GetCylinderExtent() );
			if ( Hit.Time < 1.f )
			{
				Rotation = FindSlopeRotation(Hit.Normal, Rotation);
				FVector DefaultTranslation = Cast<AUTPickupFactory>(GetClass()->GetDefaultActor())->BaseMesh->Translation;
				BaseMesh->Translation = DefaultTranslation - FVector(CollisionRadius * (1.f - Hit.Normal.Z*Hit.Normal.Z));
				BaseMesh->ConditionalUpdateTransform();
			}
		}
	}

	Super::PostEditMove( bFinished );
}

void AUTPickupFactory::Spawned()
{
	Super::Spawned();

	if ( !GWorld->HasBegunPlay() )
	{
		PostEditMove( TRUE );
	}
}

void AUTTeamPlayerStart::PostEditChange(UProperty* PropertyThatChanged)
{
	UTexture2D* NewSprite = NULL;
	if (TeamNumber < TeamSprites.Num())
	{
		NewSprite = TeamSprites(TeamNumber);
	}
	else
	{
		// get sprite from defaults
		AUTTeamPlayerStart* Default = GetArchetype<AUTTeamPlayerStart>();
		for (INT i = 0; i < Default->Components.Num() && NewSprite == NULL; i++)
		{
			USpriteComponent* SpriteComponent = Cast<USpriteComponent>(Default->Components(i));
			if (SpriteComponent != NULL)
			{
				NewSprite = SpriteComponent->Sprite;
			}
		}
	}

	if (NewSprite != NULL)
	{
		// set the new sprite as the current one
		USpriteComponent* SpriteComponent = NULL;
		for (INT i = 0; i < Components.Num() && SpriteComponent == NULL; i++)
		{
			SpriteComponent = Cast<USpriteComponent>(Components(i));
		}
		if (SpriteComponent != NULL)
		{
			SpriteComponent->Sprite = NewSprite;
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AUTTeamPlayerStart::Spawned()
{
	Super::Spawned();

	PostEditChange(NULL);
}

void AUTDefensePoint::PostEditChange(UProperty* PropertyThatChanged)
{
	UTexture2D* NewSprite = NULL;
	if (DefendedObjective != NULL && DefendedObjective->DefenderTeamIndex < TeamSprites.Num())
	{
		NewSprite = TeamSprites(DefendedObjective->DefenderTeamIndex);
	}
	else
	{
		// get sprite from defaults
		AUTDefensePoint* Default = GetArchetype<AUTDefensePoint>();
		for (INT i = 0; i < Default->Components.Num() && NewSprite == NULL; i++)
		{
			USpriteComponent* SpriteComponent = Cast<USpriteComponent>(Default->Components(i));
			if (SpriteComponent != NULL)
			{
				NewSprite = SpriteComponent->Sprite;
			}
		}
	}

	if (NewSprite != NULL)
	{
		// set the new sprite as the current one
		USpriteComponent* SpriteComponent = NULL;
		for (INT i = 0; i < Components.Num() && SpriteComponent == NULL; i++)
		{
			SpriteComponent = Cast<USpriteComponent>(Components(i));
		}
		if (SpriteComponent != NULL)
		{
			SpriteComponent->Sprite = NewSprite;
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

void AUTDefensePoint::Spawned()
{
	Super::Spawned();

	// by default, all defense points have their own group
	DefenseGroup = GetFName();

	// set the correct sprite if we're in the editor
	if (!WorldInfo->bBegunPlay)
	{
		PostEditChange(NULL);
	}
}

/** flag used by FindBestJump() to prevent SuggestJumpVelocity() considering double jump */
static UBOOL GForceNoDoubleJump = FALSE;

//@todo FIXMESTEVE - eventually UTScout should become pathbuilder, spawns a UTPawn to do pathfinding
UBOOL AUTScout::SuggestJumpVelocity(FVector& JumpVelocity, FVector End, FVector Start)
{
	bRequiresDoubleJump = FALSE;
	if ( Super::SuggestJumpVelocity(JumpVelocity, End, Start) )
		return TRUE;

	if (GForceNoDoubleJump || PrototypePawnClass == NULL)
	{
		return FALSE;
	}

 	bRequiresDoubleJump = TRUE;
 
 	// calculate using effective JumpZ with doublejump
 	FLOAT RealJumpZ = JumpZ;
	AUTPawn* DefaultPawn = PrototypePawnClass->GetDefaultObject<AUTPawn>();
	JumpZ += DefaultPawn->JumpZ * 0.3 + DefaultPawn->MultiJumpBoost;

 	UBOOL bResult = Super::SuggestJumpVelocity(JumpVelocity, End, Start);
 	JumpZ = RealJumpZ;
 	return bResult;
}

ETestMoveResult AUTScout::FindBestJump(FVector Dest, FVector &CurrentPosition)
{
	// test both with and without double jump
	// if non-double jump check reached same Z, then don't require it
	GForceNoDoubleJump = TRUE;
	FVector SingleJumpPosition = CurrentPosition;
	ETestMoveResult SingleJumpResult = Super::FindBestJump(Dest, SingleJumpPosition);

	GForceNoDoubleJump = FALSE;
	FVector DoubleJumpPosition = CurrentPosition;
	ETestMoveResult DoubleJumpResult = Super::FindBestJump(Dest, DoubleJumpPosition);

	if (SingleJumpResult != TESTMOVE_Moved && DoubleJumpResult != TESTMOVE_Moved)
	{
		// both failed
		return SingleJumpResult;
	}
	else
	{
		if (SingleJumpResult != DoubleJumpResult)
		{
			bRequiresDoubleJump = (SingleJumpResult != TESTMOVE_Moved);
		}
		else
		{
			bRequiresDoubleJump = (SingleJumpPosition.Z <= DoubleJumpPosition.Z - MaxStepHeight);
		}
		
		CurrentPosition = bRequiresDoubleJump ? SingleJumpPosition : DoubleJumpPosition;
		return TESTMOVE_Moved;
	}
}

ETestMoveResult AUTScout::FindJumpUp(FVector Direction, FVector &CurrentPosition)
{
	bRequiresDoubleJump = FALSE;

	ETestMoveResult success = Super::FindJumpUp(Direction, CurrentPosition);
	if ( success != TESTMOVE_Stopped )
		return success;

	// only path build double jump up if human sized or smaller
	FVector HumanSize = GetSize(SizePersonFindName);
	if ( (HumanSize.X < CylinderComponent->CollisionRadius) || (HumanSize.Y < CylinderComponent->CollisionHeight) )
	{
		return success;
	}
	bRequiresDoubleJump = TRUE;
	FCheckResult Hit(1.f);
	FVector StartLocation = CurrentPosition;
	FVector CollisionExtent = GetDefaultCollisionSize();

	TestMove(FVector(0,0,MaxDoubleJumpHeight - MaxStepHeight), CurrentPosition, Hit, CollisionExtent);
	success = walkMove(Direction, CurrentPosition, CollisionExtent, Hit, NULL, MINMOVETHRESHOLD);

	StartLocation.Z = CurrentPosition.Z;
	if ( success != TESTMOVE_Stopped )
	{
		TestMove(-1.f*FVector(0,0,MaxDoubleJumpHeight), CurrentPosition, Hit, CollisionExtent);
		// verify walkmove didn't just step down
		StartLocation.Z = CurrentPosition.Z;
		if ((StartLocation - CurrentPosition).SizeSquared() < MINMOVETHRESHOLD * MINMOVETHRESHOLD)
			return TESTMOVE_Stopped;
	}
	else
		CurrentPosition = StartLocation;

	return success;
}

UBOOL AUTPawn::SuggestJumpVelocity(FVector& JumpVelocity, FVector End, FVector Start)
{
	bRequiresDoubleJump = FALSE;
	if ( Super::SuggestJumpVelocity(JumpVelocity, End, Start) )
		return TRUE;

	if ( !bCanDoubleJump )
		return FALSE;

 	bRequiresDoubleJump = TRUE;
 
 	// calculate using effective JumpZ with doublejump
 	FLOAT RealJumpZ = JumpZ;
 	JumpZ += JumpZ * 0.3f + MultiJumpBoost;
 	UBOOL bResult = Super::SuggestJumpVelocity(JumpVelocity, End, Start);
 
 	// set start jumpvelocity so double jump can be full
 	if ( JumpVelocity.Z <= RealJumpZ )
 	{
 		JumpVelocity.Z = RealJumpZ;
 		bRequiresDoubleJump = FALSE;
 	}
 	else
 	{
		JumpVelocity.Z -= JumpZ - RealJumpZ;
 	}
 	JumpZ = RealJumpZ;
 	return bResult;
}

ETestMoveResult AUTPawn::FindJumpUp(FVector Direction, FVector &CurrentPosition)
{
	bRequiresDoubleJump = FALSE;

	ETestMoveResult success = Super::FindJumpUp(Direction, CurrentPosition);
	if ( (success != TESTMOVE_Stopped) || !bCanDoubleJump )
		return success;

	bRequiresDoubleJump = TRUE;
	FCheckResult Hit(1.f);
	FVector StartLocation = CurrentPosition;
	FVector CollisionExtent = GetDefaultCollisionSize();

	TestMove(FVector(0,0,MaxDoubleJumpHeight - MaxStepHeight), CurrentPosition, Hit, CollisionExtent);
	success = walkMove(Direction, CurrentPosition, CollisionExtent, Hit, NULL, MINMOVETHRESHOLD);

	StartLocation.Z = CurrentPosition.Z;
	if ( success != TESTMOVE_Stopped )
	{
		TestMove(-1.f*FVector(0,0,MaxDoubleJumpHeight), CurrentPosition, Hit, CollisionExtent);
		// verify walkmove didn't just step down
		StartLocation.Z = CurrentPosition.Z;
		if ((StartLocation - CurrentPosition).SizeSquared() < MINMOVETHRESHOLD * MINMOVETHRESHOLD)
			return TESTMOVE_Stopped;
	}
	else
		CurrentPosition = StartLocation;

	return success;
}

/* TryJumpUp()
Check if could jump up over obstruction
*/
UBOOL AUTPawn::TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility)
{
	FVector Out = 14.f * Dir;
	FCheckResult Hit(1.f);
	FVector Up = FVector(0.f,0.f,MaxJumpHeight);

	if ( bNoVisibility )
	{
		// do quick trace check first
		FVector Start = Location + FVector(0.f, 0.f, CylinderComponent->CollisionHeight);
		FVector End = Start + Up;
		GWorld->SingleLineCheck(Hit, this, End, Start, TRACE_World);
		UBOOL bLowCeiling = Hit.Time < 1.f;
		if ( bLowCeiling )
		{
			End = Hit.Location;
		}
		GWorld->SingleLineCheck(Hit, this, Destination, End, TraceFlags);
		if ( (Hit.Time < 1.f) && (Hit.Actor != Controller->MoveTarget) )
		{
			if ( !bCanDoubleJump || bLowCeiling )
			{
				return FALSE;
			}

			// try double jump LOS
			Start = End;
			End += FVector(0.f, 0.f, MaxDoubleJumpHeight - MaxJumpHeight);
			GWorld->SingleLineCheck(Hit, this, End, Start, TRACE_World);
			if ( Hit.Time < 1.f )
				End = Hit.Location;
			GWorld->SingleLineCheck(Hit, this, Destination, End, TraceFlags);
			if ( (Hit.Time < 1.f) && (Hit.Actor != Controller->MoveTarget) )
			{
				return FALSE;
			}
		}
	}

	GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_World, GetCylinderExtent());
	FLOAT FirstHit = Hit.Time;
	if ( FirstHit > 0.5f )
	{
		GWorld->SingleLineCheck(Hit, this, Location + Up * Hit.Time + Out, Location + Up * Hit.Time, TraceFlags, GetCylinderExtent());
		if ( (Hit.Time < 1.f) && bCanDoubleJump && (FirstHit == 1.f) )
		{
			// try double jump
			Up = FVector(0.f,0.f,MaxJumpHeight);
			FVector doubleUp(0.f,0.f,MaxDoubleJumpHeight);
			GWorld->SingleLineCheck(Hit, this, Location + doubleUp, Location + Up, TRACE_World, GetCylinderExtent());
			if ( Hit.Time > 0.25f )
			{
				if ( Hit.Time == 1.f )
					Hit.Location = Location + doubleUp;
				GWorld->SingleLineCheck(Hit, this, Hit.Location + Out, Hit.Location, TraceFlags, GetCylinderExtent());
			}
		}
		return (Hit.Time == 1.f);
	}
	return FALSE;
}

INT AUTPawn::calcMoveFlags()
{
	return ( bCanWalk * R_WALK + bCanFly * R_FLY + bCanSwim * R_SWIM + bJumpCapable * R_JUMP + bCanDoubleJump * R_HIGHJUMP ); 
}


//============================================================
// UTScout path building

void AUTScout::SetPrototype()
{
	if ( PrototypePawnClass )
	{
		AUTPawn* DefaultPawn = Cast<AUTPawn>(PrototypePawnClass->GetDefaultActor());

		// override UTScout defaults
		PathSizes(0).Radius = DefaultPawn->CrouchRadius + 1;
		PathSizes(0).Height = DefaultPawn->CrouchHeight;

		PathSizes(1).Radius = DefaultPawn->CylinderComponent->CollisionRadius + 1;
		PathSizes(1).Height = DefaultPawn->CylinderComponent->CollisionHeight;

		TestJumpZ = DefaultPawn->JumpZ;
		TestGroundSpeed = DefaultPawn->GroundSpeed;
		MaxStepHeight = DefaultPawn->MaxStepHeight;
		MaxJumpHeight = DefaultPawn->MaxJumpHeight;
		MaxDoubleJumpHeight = DefaultPawn->MaxDoubleJumpHeight;
	}
}

void AUTScout::CreateTranslocatorPath(ANavigationPoint* Nav, ANavigationPoint* DestNav, FCheckResult Hit, UBOOL bOnlyChanged)
{
	if ( !DestNav->bNoAutoConnect && !DestNav->bDestinationOnly 
		&& ((DestNav->Location - Nav->Location).SizeSquared() < MaxTranslocDistSq) 
		&& Abs<FLOAT>(Nav->GetGravityZ() - DestNav->GetGravityZ()) < KINDA_SMALL_NUMBER
		&& GWorld->SingleLineCheck( Hit, Nav, DestNav->Location, Nav->Location, TRACE_World|TRACE_StopAtAnyHit )
		&& !Nav->CheckSatisfactoryConnection(DestNav) )
	{
		FVector TossVelocity(0.f,0.f,0.f);
		FLOAT DesiredZPct = 0.05f; // prefer nearly horizontal throws
		FLOAT FallbackDesiredZPct = 0.50f; // use more vertical throw if necessary
		AUTProjectile* DefaultProj = TranslocProjClass->GetDefaultObject<AUTProjectile>();
		FVector CollisionSize = DefaultProj->GetCylinderExtent();
		FLOAT TossSpeed = DefaultProj->Speed; 
		FLOAT BaseTossZ = DefaultProj->TossZ;
		FLOAT TerminalVelocity = (Nav->PhysicsVolume && Nav->PhysicsVolume->bWaterVolume) ? Nav->PhysicsVolume->TerminalVelocity : DefaultProj->TerminalVelocity;
		
		debugfSuppressed(NAME_DevPath,TEXT("ADDING transloc SPEC from %s to %s!"), *Nav->GetName(), *DestNav->GetName());
		// make sure can make throw
		if (	SuggestTossVelocity(&TossVelocity, DestNav->Location, Nav->Location, TossSpeed, BaseTossZ, DesiredZPct, CollisionSize,TerminalVelocity) 
			||	SuggestTossVelocity(&TossVelocity, DestNav->Location, Nav->Location+FVector(0.f,0.f,50.f), TossSpeed, BaseTossZ, DesiredZPct, CollisionSize,TerminalVelocity)
			||	SuggestTossVelocity(&TossVelocity, DestNav->Location, Nav->Location, TossSpeed, BaseTossZ, FallbackDesiredZPct, CollisionSize,TerminalVelocity) )
		{
			// add a new transloc spec
			UUTTranslocatorReachSpec* newSpec = ConstructObject<UUTTranslocatorReachSpec>(UUTTranslocatorReachSpec::StaticClass(), GWorld->GetOuter(), NAME_None);
			Nav->bPathsChanged = Nav->bPathsChanged || !bOnlyChanged;
			newSpec->CollisionRadius = appTrunc(PathSizes(1).Radius);
			newSpec->CollisionHeight = appTrunc(PathSizes(1).Height);
			newSpec->Start = Nav;
			newSpec->End = DestNav;
			newSpec->Distance = appTrunc((Nav->Location - DestNav->Location).Size());
			newSpec->CachedVelocity = TossVelocity + FVector(0.f,0.f,BaseTossZ);
			Nav->PathList.AddItem(newSpec);

			// find out if it could ever be possible to jump to Nav and if so what JumpZ would be required
			SetCollisionSize(newSpec->CollisionRadius, newSpec->CollisionHeight);
			if ( Nav->PlaceScout(this) )
			{
				FVector CurrentPosition = Location;
				if ( DestNav->PlaceScout(this) )
				{
					FVector DesiredPosition = Location;
					FVector JumpVelocity(0.f, 0.f, 0.f);
					if ( SuggestJumpVelocity(JumpVelocity, DesiredPosition, CurrentPosition) &&
						FindBestJump(DesiredPosition, CurrentPosition) == TESTMOVE_Moved &&
						ReachedDestination(CurrentPosition, DestNav->Location, DestNav) )
					{
						// record the JumpZ needed to make this jump and the current gravity
						newSpec->RequiredJumpZ = JumpVelocity.Z;
						newSpec->OriginalGravityZ = newSpec->Start->GetGravityZ();
						debugfSuppressed(NAME_DevPath,TEXT("Succeeded jump test with RequiredJumpZ %f"), newSpec->RequiredJumpZ);
					}
				}
			}
		}
		else
		{
			debugfSuppressed(NAME_DevPath,TEXT("Failed toss from %s to %s"), *Nav->GetName(), *DestNav->GetName());
		}
	}
}

void AUTScout::AddSpecialPaths(INT NumPaths, UBOOL bOnlyChanged)
{
	debugf(TEXT("Adding special UT paths to %s"), *GWorld->GetOutermost()->GetName());
	
	// skip translocator paths for onslaught levels
	UUTMapInfo* MapInfo = Cast<UUTMapInfo>(GWorld->GetWorldInfo()->GetMapInfo());
	if (MapInfo != NULL && !MapInfo->bBuildTranslocatorPaths)
	{
		debugf(TEXT("Skip translocator paths due to MapInfo setting"));
		return;
	}
	else if (TranslocProjClass == NULL)
	{
		debugf(TEXT("No translocator projectile class"));
		return;
	}
	INT NumDone = 0;

	// temporarily set Scout's JumpZ insanely high for lowgrav/jumpboots/etc jump reach test
	FLOAT OldJumpZ = JumpZ;
	JumpZ = 100000.f;
	FCheckResult Hit(1.f);
	INT TwiceNumPaths = 2 * NumPaths;

	// add translocator paths to reverse connected destinations
	for( ANavigationPoint *Nav=GWorld->GetFirstNavigationPoint(); Nav; Nav=Nav->nextNavigationPoint )
	{
		if ( (!bOnlyChanged || Nav->bPathsChanged) && !Nav->bNoAutoConnect && !Nav->PhysicsVolume->bWaterVolume )
		{
			GWarn->StatusUpdatef(NumDone, TwiceNumPaths, *SpecialReachSpecsWarningLog.ToString());
			NumDone++;

			for ( INT i=0; i<Nav->PathList.Num(); i++ )
			{
				CreateTranslocatorPath(Nav->PathList(i)->End.Nav(), Nav, Hit, bOnlyChanged);
			}
		}
	}

	// add translocator paths to unconnected destinations
	for( ANavigationPoint *Nav=GWorld->GetFirstNavigationPoint(); Nav; Nav=Nav->nextNavigationPoint )
	{
		if ( (!bOnlyChanged || Nav->bPathsChanged) && !Nav->bNoAutoConnect && !Nav->PhysicsVolume->bWaterVolume )
		{
			GWarn->StatusUpdatef(NumDone, TwiceNumPaths, *SpecialReachSpecsWarningLog.ToString());
			NumDone++;

			// check all visible paths within MaxTranslocDist that don't already have a good path to them
			for( ANavigationPoint *DestNav=GWorld->GetFirstNavigationPoint(); DestNav; DestNav=DestNav->nextNavigationPoint )
			{
				CreateTranslocatorPath(Nav, DestNav, Hit, bOnlyChanged);
			}
		}
	}

	JumpZ = OldJumpZ;
}

void AUTScout::ReviewPaths()
{
	Super::ReviewPaths();

	// add a warning for paths with only translocator specs, as that's equivalent to having no specs in most cases but won't trigger the usual "no paths from" warning
	for (ANavigationPoint* Nav = GWorld->GetFirstNavigationPoint(); Nav != NULL; Nav = Nav->nextNavigationPoint)
	{
		if (!Nav->bDestinationOnly && Nav->PathList.Num() > 0)
		{
			UBOOL bOnlyTranslocatorPaths = TRUE;
			for (INT i = 0; i < Nav->PathList.Num(); i++)
			{
				if (Nav->PathList(i) != NULL && Cast<UUTTranslocatorReachSpec>(Nav->PathList(i)) == NULL)
				{
					bOnlyTranslocatorPaths = FALSE;
					break;
				}
			}
			if (bOnlyTranslocatorPaths)
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, Nav, TEXT("Only translocator paths from this node"));
			}
		}
	}
}

void AUTTranslocatorDest::addReachSpecs(AScout* Scout, UBOOL bOnlyChanged)
{
	Super::addReachSpecs(Scout, bOnlyChanged);

	AUTScout* UTScout = Cast<AUTScout>(Scout);
	if (UTScout != NULL)
	{
		for (INT i = 0; i < StartPoints.Num(); i++)
		{
			if (StartPoints(i).Point != NULL)
			{
				AUTProjectile* DefaultProj = UTScout->TranslocProjClass->GetDefaultObject<AUTProjectile>();
				FLOAT BaseTossZ = DefaultProj->TossZ;
				if (StartPoints(i).bAutoDetectVelocity)
				{
					FVector CollisionSize = DefaultProj->GetCylinderExtent();
					FLOAT TossSpeed = DefaultProj->Speed; 
					FLOAT TerminalVelocity = (StartPoints(i).Point->PhysicsVolume && StartPoints(i).Point->PhysicsVolume->bWaterVolume) ? StartPoints(i).Point->PhysicsVolume->TerminalVelocity : DefaultProj->TerminalVelocity;
					
					// try various throws with different approximations to get one that works
					if ( !SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location, TossSpeed, BaseTossZ, 0.05f, CollisionSize, TerminalVelocity) 
						&& !SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location + FVector(0.f,0.f,50.f), TossSpeed, BaseTossZ, 0.05f, CollisionSize, TerminalVelocity)
						&& !SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location, TossSpeed, BaseTossZ, 0.50f, CollisionSize, TerminalVelocity)
						&& !SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location + FVector(0.f,0.f,50.f), TossSpeed, BaseTossZ, 0.50f, CollisionSize, TerminalVelocity)
						&& !SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location, TossSpeed, BaseTossZ, 0.95f, CollisionSize, TerminalVelocity) )
					{
						SuggestTossVelocity(&StartPoints(i).RequiredTransVelocity, Location, StartPoints(i).Point->Location + FVector(0.f,0.f,50.f), TossSpeed, BaseTossZ, 0.95f, CollisionSize, TerminalVelocity);
					}

					// try jump and set RequiredJumpZ if found
					UTScout->SetCollisionSize(UTScout->PathSizes(1).Radius, UTScout->PathSizes(1).Height);
					if (StartPoints(i).Point->PlaceScout(UTScout))
					{
						FVector CurrentPosition = UTScout->Location;
						if (PlaceScout(UTScout))
						{
							FVector DesiredPosition = UTScout->Location;
							FVector JumpVelocity(0.f, 0.f, 0.f);
							if ( UTScout->SuggestJumpVelocity(JumpVelocity, DesiredPosition, CurrentPosition) &&
								UTScout->FindBestJump(DesiredPosition, CurrentPosition) == TESTMOVE_Moved &&
								UTScout->ReachedDestination(CurrentPosition, Location, this) )
							{
								// record the JumpZ needed to make this jump and the current gravity
								StartPoints(i).RequiredJumpZ = JumpVelocity.Z;
							}
						}
					}
				}
				
				// add a new transloc spec
				UUTTranslocatorReachSpec* NewSpec = ConstructObject<UUTTranslocatorReachSpec>(UUTTranslocatorReachSpec::StaticClass(), GWorld->GetOuter(), NAME_None);
				StartPoints(i).Point->bPathsChanged = StartPoints(i).Point->bPathsChanged || !bOnlyChanged;
				NewSpec->CollisionRadius = appTrunc(UTScout->PathSizes(1).Radius);
				NewSpec->CollisionHeight = appTrunc(UTScout->PathSizes(1).Height);
				NewSpec->Start = StartPoints(i).Point;
				NewSpec->End = this;
				NewSpec->Distance = appTrunc((StartPoints(i).Point->Location - Location).Size());
				NewSpec->CachedVelocity = StartPoints(i).RequiredTransVelocity + FVector(0.f,0.f,BaseTossZ);
				if (StartPoints(i).RequiredJumpZ > 0.f)
				{
					NewSpec->RequiredJumpZ = StartPoints(i).RequiredJumpZ;
					NewSpec->OriginalGravityZ = StartPoints(i).Point->GetGravityZ();
				}
				StartPoints(i).Point->PathList.AddItem(NewSpec);
			}
		}
	}
}

void AUTTranslocatorDest::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if (PropertyThatChanged != NULL && (PropertyThatChanged->GetFName() == FName(TEXT("StartPoints")) || PropertyThatChanged->GetOwnerStruct()->GetFName() == FName(TEXT("TranslocatorSource"))))
	{
		WorldInfo->bPathsRebuilt = FALSE;
		bPathsChanged = TRUE;
	}
}

static AUTGameObjective* TestObjective = NULL;

IMPLEMENT_COMPARE_POINTER(ANavigationPoint, UTPathing, { return appTrunc(appSqrt((TestObjective->Location - A->Location).SizeSquared() - (TestObjective->Location - B->Location).SizeSquared())); })

void AUTGameObjective::AddForcedSpecs(AScout* Scout)
{
	// put the five closest visible NavigationPoints in the ShootSpots array

	// create list of all non-blocking non-flying source NavigationPoints
	TArray<ANavigationPoint*> NavList;
	for (ANavigationPoint* N = GWorld->GetFirstNavigationPoint(); N != NULL; N = N->nextNavigationPoint)
	{
		if (N != this && !N->bBlockActors && !N->bDestinationOnly && !N->bFlyingPreferred)
		{
			NavList.AddItem(N);
		}
	}

	// sort by distance
	TestObjective = this;
	Sort<USE_COMPARE_POINTER(ANavigationPoint,UTPathing)>(NavList.GetTypedData(), NavList.Num());
	TestObjective = NULL;

	// put the first five that succeed a visibility trace into the ShootSpots array
	ShootSpots.Empty();
	FCheckResult Hit(1.0f);
	FVector TargetLoc = GetTargetLocation();
	for (INT i = 0; i < NavList.Num(); i++)
	{
		if (GWorld->SingleLineCheck(Hit, Scout, TargetLoc, NavList(i)->Location, TRACE_World | TRACE_StopAtAnyHit))
		{
			ShootSpots.AddItem(NavList(i));
			if (ShootSpots.Num() >= 5)
			{
				break;
			}
		}
	}

	// if bAllowOnlyShootable, we don't need to be reachable if we found any ShootSpots
	if (bAllowOnlyShootable && ShootSpots.Num() > 0)
	{
		bMustBeReachable = FALSE;
	}
	else
	{
		bMustBeReachable = GetArchetype<ANavigationPoint>()->bMustBeReachable;
	}
}

void AUTGameObjective::SetNetworkID(INT InNetworkID)
{
	if (bAllowOnlyShootable)
	{
		// steal network ID from ShootSpots
		if (NetworkID == INDEX_NONE)
		{
			for (INT i = 0; i < ShootSpots.Num(); i++)
			{
				if (ShootSpots(i)->NetworkID != INDEX_NONE)
				{
					InNetworkID = ShootSpots(i)->NetworkID;
					break;
				}
			}
		}

		Super::SetNetworkID(InNetworkID);

		// also pass ID to ShootSpots
		for (INT i = 0; i < ShootSpots.Num(); i++)
		{
			ShootSpots(i)->SetNetworkID(InNetworkID);
		}
	}
	else
	{
		Super::SetNetworkID(InNetworkID);
	}
}

/** Check that game objectives are not non-uniformly scaled. */
void AUTGameObjective::CheckForErrors()
{
	Super::CheckForErrors();

	// Then iterate over components
	UBOOL bUniformScaling = TRUE;
	for(INT i=0; i<Components.Num(); i++)
	{
		if(Components(i) && Components(i)->IsAttached())
		{
			// Only care about primitive components
			USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(Components(i));
			if(SkelComp && SkelComp->ShouldCollide())
			{
				FVector TotalScale = SkelComp->Scale3D;
				if(SkelComp->GetOwner())
				{
					TotalScale *= SkelComp->GetOwner()->DrawScale3D;
				}

				if(!TotalScale.IsUniform())
				{
					bUniformScaling = FALSE;
					break;				
				}
			}
		}
	}

	// If something is non-uniformly scaled, add warning.
	if(!bUniformScaling)
	{
		GWarn->MapCheck_Add(MCTYPE_WARNING, this, TEXT("Objective with colliding component using non-uniform scaling!") );
	}
}

void AUTGameObjective::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.Ver() < VER_FIXED_NAV_VEHICLE_USAGE_FLAGS)
	{
		// these flags got incorrectly modified by LDs in some cases so reset them
		bBlockedForVehicles = GetClass()->GetDefaultObject<AUTGameObjective>()->bBlockedForVehicles;
		bPreferredVehiclePath = GetClass()->GetDefaultObject<AUTGameObjective>()->bPreferredVehiclePath;
	}
}

/*
 * Returns navigation anchor associated with this actor
 * (rather than actually checking if anchor is also reachable)
*/
ANavigationPoint* AUTCarriedObject::SpecifyEndAnchor(APawn* RouteFinder)
{
	if (bHome && HomeBase != NULL && ((HomeBase->Location + HomeBaseOffset) - Location).SizeSquared() <= 100.0f)
	{
		LastAnchor = HomeBase;
		return HomeBase;
	}
	else if (WorldInfo->TimeSeconds - LastValidAnchorTime < 0.25f)
	{
		return LastAnchor;
	}
	else
	{
		return NULL;
	}
}

/*
 * Notify actor of anchor finding result
 * @PARAM EndAnchor is the anchor found
 * @PARAM RouteFinder is the pawn which requested the anchor finding
 */
void AUTCarriedObject::NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder)
{
	if ( EndAnchor )
	{
		// save valid anchor info
		LastValidAnchorTime = WorldInfo->TimeSeconds;
		LastAnchor = EndAnchor;
	}
	else
	{
		eventNotReachableBy(RouteFinder);
	}
}

/** Used to see when the 'base most' (ie end of Base chain) changes. */
void AUTCarriedObject::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	AActor* NewBase = GetBase();
	AActor* NewBaseBase = NULL;
	if(NewBase)
	{
		NewBaseBase = NewBase->GetBase();
	}
	

	if(NewBase != OldBase || NewBaseBase != OldBaseBase)
	{
		// Call script event when this happens.
		eventOnBaseChainChanged();
		OldBase = NewBase;
		OldBaseBase = NewBaseBase;

	}
}

void AUTCarriedObject::ForceUpdateComponents(UBOOL bCollisionUpdate,UBOOL bTransformOnly)
{
	// For carried objects, we update _all_ components.
	Super::ForceUpdateComponents(FALSE, TRUE);
}

void AUTVehicle::InitForPathfinding(AActor* Goal, ANavigationPoint* EndAnchor)
{
	Super::InitForPathfinding(Goal, EndAnchor);
}

ANavigationPoint* AUTHoldSpot::SpecifyEndAnchor(APawn* RouteFinder)
{
	if (LastAnchor != NULL && !LastAnchor->IsUsableAnchorFor(RouteFinder))
	{
		LastAnchor = NULL;
	}
	return LastAnchor;
}

void AUTHoldSpot::NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder)
{
	LastAnchor = EndAnchor;
}

UBOOL AUTHoldSpot::AnchorNeedNotBeReachable()
{
	return TRUE;
}
