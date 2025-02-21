/*=============================================================================
	UnPawn.cpp: APawn AI implementation

  This contains both C++ methods (movement and reachability), as well as some
  AI related natives

	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"
#include "FConfigCacheIni.h"
#include "UnPath.h"

DECLARE_STATS_GROUP(TEXT("Pathfinding"),STATGROUP_PathFinding);
DECLARE_CYCLE_STAT(TEXT("Various Reachable"),STAT_PathFinding_Reachable,STATGROUP_PathFinding);
DECLARE_CYCLE_STAT(TEXT("FindPathToward"),STAT_PathFinding_FindPathToward,STATGROUP_PathFinding);
DECLARE_CYCLE_STAT(TEXT("BestPathTo"),STAT_PathFinding_BestPathTo,STATGROUP_PathFinding);

#if LOG_DETAILED_PATHFINDING_STATS
/** Global detailed pathfinding stats. */
FDetailedTickStats GDetailedPathFindingStats( 30, 10, 1, 20, TEXT("pathfinding") );
#endif

/*-----------------------------------------------------------------------------
	APawn object implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(APawn);

void APawn::PostBeginPlay()
{
	Super::PostBeginPlay();
	if ( !bDeleteMe )
	{
		GWorld->AddPawn( this );
	}
}

void APawn::PostScriptDestroyed()
{
	Super::PostScriptDestroyed();
	GWorld->RemovePawn( this );
}

APawn* APawn::GetPlayerPawn() const
{
	if ( !Controller || !Controller->GetAPlayerController() )
		return NULL;

	return ((APawn *)this);
}

/** IsPlayerPawn()
return true if controlled by a Player (AI or human) on local machine (any controller on server, localclient's pawn on client)
*/
UBOOL APawn::IsPlayerPawn() const
{
	return ( Controller && Controller->bIsPlayer );
}

/** IsHumanControlled()
return true if controlled by a real live human on the local machine.
Returns true for all humans on server, and local client's pawn on client
*/
UBOOL APawn::IsHumanControlled()
{
	return ( Controller && Controller->GetAPlayerController() ); 
}

/** IsLocallyControlled()
return true if controlled by local (not network) player 
*/
UBOOL APawn::IsLocallyControlled()
{
	if ( !Controller )
		return false;
    if ( GWorld->GetNetMode() == NM_Standalone )
		return true;
	if ( !Controller->GetAPlayerController() )
		return true;

	return Controller->LocalPlayerController();
}


/**
@RETURN true if pawn is invisible to AI
*/
UBOOL APawn::IsInvisible()
{
	return FALSE;
}

UBOOL APawn::IsAliveAndWell() const
{
	return (Health > 0 && !bHidden && !bDeleteMe && !bPlayedDeath);
}

UBOOL APawn::ReachedDesiredRotation()
{
	// Only base success on Yaw 
	INT YawDiff = Abs((DesiredRotation.Yaw & 65535) - (Rotation.Yaw & 65535));
	return ( (YawDiff < AllowedYawError) || (YawDiff > 65535 - AllowedYawError) );
}

UBOOL APawn::ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags)
{
	return (TraceFlags & TRACE_Pawns) || (bStationary && (TraceFlags & TRACE_Others));
}

UBOOL APawn::DelayScriptReplication(FLOAT LastFullUpdateTime) 
{ 
	return ( LastFullUpdateTime != GWorld->GetTimeSeconds() );
}

void APawn::SetAnchor(ANavigationPoint *NewAnchor)
{
	//debug
//	debugf(TEXT("%s [%s] SET ANCHOR from %s to %s"), *GetName(), (GetStateFrame() && GetStateFrame()->StateNode) ? *GetStateFrame()->StateNode->GetName() : TEXT("NONE"), Anchor ? *Anchor->GetName() : TEXT("NONE"), NewAnchor ? *NewAnchor->GetName() : TEXT("NONE") );

	// clear the previous anchor reference
	if (Anchor != NULL &&
		Anchor->AnchoredPawn == this)
	{
		Anchor->AnchoredPawn = NULL;
		Anchor->LastAnchoredPawnTime = GWorld->GetTimeSeconds();
	}

	Anchor = NewAnchor;
	if ( Anchor )
	{
		LastValidAnchorTime = GWorld->GetTimeSeconds();
		LastAnchor = Anchor;
		// set the anchor reference for path finding purposes
		// only for AI pawns as PlayerControllers using pathfinding is usually sporadic, so this might not get updated for a long time
		if (!IsHumanControlled())
		{
			Anchor->AnchoredPawn = this;
		}
	}
}

#if 0
#define SCOPE_QUICK_TIMER(TIMERNAME) \
	static DOUBLE AverageTime##TIMERNAME = -1.0; \
	static INT NumItts##TIMERNAME = 0; \
	static UObject* OuterThis;\
	OuterThis=this;\
class TimerClassName##TIMERNAME \
{ \
public:\
	TimerClassName##TIMERNAME() \
	{ \
		StartTimerTime = 0; \
		CLOCK_CYCLES(StartTimerTime);\
	} \
	\
	~TimerClassName##TIMERNAME() \
	{\
		UNCLOCK_CYCLES(StartTimerTime);\
		FLOAT Duration = (FLOAT)(StartTimerTime*GSecondsPerCycle);\
		if(AverageTime##TIMERNAME < 0.f)\
		{\
			AverageTime##TIMERNAME = Duration;\
		}\
		else\
		{\
			AverageTime##TIMERNAME += (Duration - AverageTime##TIMERNAME)/++NumItts##TIMERNAME;\
		}\
		\
		debugf(TEXT("(%s) Task %s took %.2f(ms) Avg %.2f(ms)"),(OuterThis)?*OuterThis->GetName():TEXT("NULL"),TEXT(#TIMERNAME),Duration*1000.f,AverageTime##TIMERNAME*1000.f);\
	}\
	DWORD StartTimerTime;\
};\
	TimerClassName##TIMERNAME TIMERNAME = TimerClassName##TIMERNAME();
#else
#define SCOPE_QUICK_TIMER(blah) {}
#endif

ANavigationPoint* APawn::GetBestAnchor( AActor* TestActor, FVector TestLocation, UBOOL bStartPoint, UBOOL bOnlyCheckVisible, FLOAT& out_Dist )
{
	//debugf(TEXT("----------------->> PHYS: %i"),Physics);
	SCOPE_QUICK_TIMER(GetBestAnchor)

	if (Controller != NULL)
	{
		return FindAnchor( TestActor, TestLocation, bStartPoint, bOnlyCheckVisible, out_Dist );
	}
	else
	{
		debugf(NAME_Warning, TEXT("GetBestAnchor(): No Controller! (%s)"),*GetName());
		return NULL;
	}
}

/** @see Pawn::SetRemoteViewPitch */
void APawn::SetRemoteViewPitch(INT NewRemoteViewPitch)
{
	RemoteViewPitch = (NewRemoteViewPitch & 65535) >> 8;
}

/* PlayerCanSeeMe()
	returns true if actor is visible to some player
*/
UBOOL AActor::PlayerCanSeeMe()
{
	if ( WorldInfo->NetMode == NM_Standalone || WorldInfo->NetMode == NM_Client || (WorldInfo->NetMode == NM_ListenServer && (bTearOff || (WorldInfo->Game && (WorldInfo->Game->NumPlayers + WorldInfo->Game->NumSpectators < 2)))) )
	{
		// just check local player visibility
		return (WorldInfo->TimeSeconds - LastRenderTime < 1.f);
	}
	else
	{
		for (AController *next = GWorld->GetFirstController(); next != NULL; next = next->NextController)
		{
			if (TestCanSeeMe(next->GetAPlayerController()))
			{
				return TRUE;
			}
		}

		return FALSE;
	}
}

INT AActor::TestCanSeeMe( APlayerController *Viewer )
{
	if ( !Viewer )
		return 0;
	if ( Viewer->GetViewTarget() == this )
		return 1;

	float distSq = (Location - Viewer->ViewTarget->Location).SizeSquared();

	FLOAT CollisionRadius, CollisionHeight;
	GetBoundingCylinder(CollisionRadius, CollisionHeight);

	return ( (distSq < 100000.f * ( Max(CollisionRadius, CollisionHeight) + 3.6 ))
		&& ( Viewer->PlayerCamera != NULL
			|| (Square(Viewer->Rotation.Vector() | (Location - Viewer->ViewTarget->Location)) >= 0.25f * distSq) )
		&& Viewer->LineOfSightTo(this) );

}

/*-----------------------------------------------------------------------------
	Pawn related functions.
-----------------------------------------------------------------------------*/

void APawn::ForceCrouch()
{
	Crouch(false);
}

/*MakeNoise
- check to see if other creatures can hear this noise
*/
void AActor::MakeNoise( FLOAT Loudness, FName NoiseType )
{
	if ( (GWorld->GetNetMode() != NM_Client) && Instigator )
	{
		Instigator->CheckNoiseHearing( this, Loudness, NoiseType );
	}
}

/* Dampen loudness of a noise instigated by this pawn
@PARAM NoiseMaker	is the actual actor who made the noise
@PARAM Loudness		is the loudness passed in (1.0 is typical value)
@PARAM NoiseType	is additional optional information for game specific implementations
@RETURN		dampening factor
*/
FLOAT APawn::DampenNoise( AActor* NoiseMaker, FLOAT Loudness, FName NoiseType )
{
	FLOAT DampeningFactor = 1.f;
	if ( (NoiseMaker == this) ||(NoiseMaker->Owner == this) )
		DampeningFactor *= Instigator->SoundDampening;

	return DampeningFactor;
}


/* Send a HearNoise() message instigated by this pawn to all Controllers which could possibly hear this noise
@PARAM NoiseMaker	is the actual actor who made the noise
@PARAM Loudness		is the loudness passed in (1.0 is typical value)
@PARAM NoiseType	is additional optional information for game specific implementations
*/
void APawn::CheckNoiseHearing( AActor* NoiseMaker, FLOAT Loudness, FName NoiseType )
{
	// only hear sounds from pawns with controllers, and not when playersonly
	if ( !Controller || WorldInfo->bPlayersOnly )
	{
		return;
	}

	Loudness *= DampenNoise(NoiseMaker, Loudness, NoiseType);

	FLOAT CurrentTime = WorldInfo->TimeSeconds;

	// allow only one noise per MAXSOUNDOVERLAPTIME seconds from a given instigator & area (within PROXIMATESOUNDTHRESHOLD units) unless much louder
	// check the two sound slots
	if ( (noise1time > CurrentTime - MAXSOUNDOVERLAPTIME)
		 && ((noise1spot - NoiseMaker->Location).SizeSquared() < PROXIMATESOUNDTHRESHOLDSQUARED)
		 && (noise1loudness >= 0.9f * Loudness) )
	{
		return;
	}

	if ( (noise2time > CurrentTime - MAXSOUNDOVERLAPTIME)
		 && ((noise2spot - NoiseMaker->Location).SizeSquared() < PROXIMATESOUNDTHRESHOLDSQUARED)
		 && (noise2loudness >= 0.9f * Loudness) )
	{
		return;
	}

	// put this noise in a slot
	if ( noise1time < CurrentTime - MINSOUNDOVERLAPTIME )
	{
		noise1time = CurrentTime;
		noise1spot = NoiseMaker->Location;
		noise1loudness = Loudness;
	}
	else if ( noise2time < CurrentTime - MINSOUNDOVERLAPTIME )
	{
		noise2time = CurrentTime;
		noise2spot = NoiseMaker->Location;
		noise2loudness = Loudness;
	}
	else if ( ((noise1spot - NoiseMaker->Location).SizeSquared() < PROXIMATESOUNDTHRESHOLDSQUARED)
			  && (noise1loudness <= Loudness) )
	{
		noise1time = CurrentTime;
		noise1spot = NoiseMaker->Location;
		noise1loudness = Loudness;
	}
	else if ( noise2loudness <= Loudness )
	{
		noise1time = CurrentTime;
		noise1spot = NoiseMaker->Location;
		noise1loudness = Loudness;
	}
//debugf(TEXT("Hear noise %s %f at %f"), *NoiseMaker->GetName(), Loudness, WorldInfo->TimeSeconds);
	// all controllers can hear this noise
	for ( AController *C=GWorld->GetFirstController(); C!=NULL; C=C->NextController )
	{
		if ( C->Pawn && C->Pawn != this )
		{
			C->HearNoise( NoiseMaker, Loudness, NoiseType);
		}
	}
}

FVector APawn::AdjustDestination( AActor* GoalActor, FVector Dest )
{
	return FVector(0,0,0);
}

//=================================================================================
void APawn::setMoveTimer(FVector MoveDir)
{
	if ( !Controller )
		return;

	if ( DesiredSpeed == 0.f )
		Controller->MoveTimer = 0.5f;
	else
	{
		FLOAT Extra = 2.f;
		if ( bIsCrouched )
			Extra = ::Max(Extra, 1.f/CrouchedPct);
		else if ( bIsWalking )
			Extra = ::Max(Extra, 1.f/WalkingPct);
		FLOAT MoveSize = MoveDir.Size();
		Controller->MoveTimer = 0.5f + Extra * MoveSize/(DesiredSpeed * 0.6f * GetMaxSpeed());
	}
	if ( Controller->bPreparingMove && Controller->PendingMover )
		Controller->MoveTimer += 2.f;
}

FLOAT APawn::GetMaxSpeed()
{
	if (Physics == PHYS_Flying)
		return AirSpeed;
	else if (Physics == PHYS_Swimming)
		return WaterSpeed;
	return GroundSpeed;
}

/* StartNewSerpentine()
pawn is using serpentine motion while moving to avoid being hit (while staying within the reachspec
its using.  At this point change direction (either reverse, or go straight for a while)
*/
void APawn::StartNewSerpentine(const FVector& Dir, const FVector& Start)
{
	FVector NewDir(Dir.Y, -1.f * Dir.X, Dir.Z);
	if ( (NewDir | (Location - Start)) > 0.f )
	{
		NewDir *= -1.f;
	}
	SerpentineDir = NewDir;

	if (!Controller->bAdvancedTactics || Controller->bUsingPathLanes)
	{
		ClearSerpentine();
	}
	else if (appFrand() < 0.2f)
	{
		SerpentineTime = 0.1f + 0.4f * appFrand();
	}
	else
	{
		SerpentineTime = 0.f;

		FLOAT ForcedStrafe = ::Min(1.f, 4.f * CylinderComponent->CollisionRadius/Controller->CurrentPath->CollisionRadius);
		SerpentineDist = (ForcedStrafe + (1.f - ForcedStrafe) * appFrand());
		SerpentineDist *= (Controller->CurrentPath->CollisionRadius - CylinderComponent->CollisionRadius);
	}
}

/* ClearSerpentine()
completely clear all serpentine related attributes
*/
void APawn::ClearSerpentine()
{
	SerpentineTime = 999.f;
	SerpentineDist = 0.f;
}

/* moveToward()
move Actor toward a point.  Returns 1 if Actor reached point
(Set Acceleration, let physics do actual move)
*/
UBOOL APawn::moveToward(const FVector &Dest, AActor *GoalActor )
{
	if ( !Controller )
		return false;

	if ( Controller->bAdjusting )
	{
		GoalActor = NULL;
	}
	FVector Direction = Dest - Location;
	FLOAT ZDiff = Direction.Z;

	if( Physics == PHYS_Walking )
	{
		Direction.Z = 0.f;
	}
	else if (Physics == PHYS_Falling)
	{
		// use air control if low grav or above destination and falling towards it
		if (Velocity.Z < 0.f && (ZDiff < 0.f || GetGravityZ() > 0.9f * GWorld->GetDefaultGravityZ()))
		{
			if ( ZDiff > 0.f )
			{
				if ( ZDiff > 2.f * MaxJumpHeight )
				{
					Controller->FailMove();
					Controller->eventNotifyMissedJump();
				}
			}
			else
			{
				if ( (Velocity.X == 0.f) && (Velocity.Y == 0.f) )
					Acceleration = FVector(0.f,0.f,0.f);
				else
				{
					FLOAT Dist2D = Direction.Size2D();
					Direction.Z = 0.f;
					Acceleration = Direction;
					Acceleration = Acceleration.SafeNormal();
					Acceleration *= AccelRate;
					if ( (Dist2D < 0.5f * Abs(Direction.Z)) && ((Velocity | Direction) > 0.5f*Dist2D*Dist2D) )
						Acceleration *= -1.f;

					if ( Dist2D < 1.5f*CylinderComponent->CollisionRadius )
					{
						Velocity.X = 0.f;
						Velocity.Y = 0.f;
						Acceleration = FVector(0.f,0.f,0.f);
					}
					else if ( (Velocity | Direction) < 0.f )
					{
						FLOAT M = ::Max(0.f, 0.2f - AvgPhysicsTime);
						Velocity.X *= M;
						Velocity.Y *= M;
					}
				}
			}
		}
		return false; // don't end move until have landed
	}
	else if ( (Physics == PHYS_Ladder) && OnLadder )
	{
		if ( ReachedDestination(Location, Dest, GoalActor) )
		{
			Acceleration = FVector(0.f,0.f,0.f);

			// if Pawn just reached a navigation point, set a new anchor
			ANavigationPoint *Nav = Cast<ANavigationPoint>(GoalActor);
			if ( Nav )
				SetAnchor(Nav);
			return true;
		}
		Acceleration = Direction.SafeNormal();
		if ( GoalActor && (OnLadder != GoalActor->PhysicsVolume)
			&& ((Acceleration | (OnLadder->ClimbDir + OnLadder->LookDir)) > 0.f)
			&& (GoalActor->Location.Z < Location.Z) )
			setPhysics(PHYS_Falling);
		Acceleration *= LadderSpeed;
		return false;
	}
	if ( Controller->MoveTarget && Controller->MoveTarget->IsA(APickupFactory::StaticClass()) 
		 && (Abs(Location.Z - Controller->MoveTarget->Location.Z) < CylinderComponent->CollisionHeight)
		 && (Square(Location.X - Controller->MoveTarget->Location.X) + Square(Location.Y - Controller->MoveTarget->Location.Y) < Square(CylinderComponent->CollisionRadius)) )
	{
		 Controller->MoveTarget->eventTouch(this, this->CollisionComponent, Location, (Controller->MoveTarget->Location - Location) );
	}
	
	FLOAT Distance = Direction.Size();
	UBOOL bGlider = ( !bCanStrafe && ((Physics == PHYS_Flying) || (Physics == PHYS_Swimming)) );
	FCheckResult Hit(1.f);

	if ( ReachedDestination(Location, Dest, GoalActor) )
	{
		if ( !bGlider )
		{
			Acceleration = FVector(0.f,0.f,0.f);
		}

		// if Pawn just reached a navigation point, set a new anchor
		ANavigationPoint *Nav = Cast<ANavigationPoint>(GoalActor);
		if ( Nav )
			SetAnchor(Nav);
		return true;
	}
	else 
	// if walking, and within radius, and goal is null or
	// the vertical distance is greater than collision + step height and trace hit something to our destination
	if (Physics == PHYS_Walking 
		&& Distance < CylinderComponent->CollisionRadius &&
		(GoalActor == NULL ||
		 (ZDiff > CylinderComponent->CollisionHeight + 2.f * MaxStepHeight && 
		  !GWorld->SingleLineCheck(Hit, this, Dest, Location, TRACE_World))))
	{
		Controller->eventMoveUnreachable(Dest,GoalActor);
		return true;
	}
	else if ( bGlider )
	{
		Direction = Rotation.Vector();
	}
	else if ( Distance > 0.f )
	{
		Direction = Direction/Distance;
		if (Controller != NULL && Controller->CurrentPath != NULL && Controller->CurrentPath->Start != NULL)
		{
			HandleSerpentineMovement(Direction, Distance, Dest);
		}
	}

	Acceleration = Direction * AccelRate;

	if ( !Controller->bAdjusting && Controller->MoveTarget && Controller->MoveTarget->GetAPawn() )
	{
		return (Distance < CylinderComponent->CollisionRadius + Controller->MoveTarget->GetAPawn()->CylinderComponent->CollisionRadius + 0.8f * MeleeRange);
	}

	FLOAT speed = Velocity.Size();

	if ( !bGlider && (speed > FASTWALKSPEED) )
	{
//		FVector VelDir = Velocity/speed;
//		Acceleration -= 0.2f * (1 - (Direction | VelDir)) * speed * (VelDir - Direction);
	}
	if ( Distance < 1.4f * AvgPhysicsTime * speed )
	{
		// slow pawn as it nears its destination to prevent overshooting
		if ( !bReducedSpeed ) 
		{
			//haven't reduced speed yet
			DesiredSpeed = 0.51f * DesiredSpeed;
			bReducedSpeed = 1;
		}
		if ( speed > 0.f )
			DesiredSpeed = Min(DesiredSpeed, (2.f*FASTWALKSPEED)/speed);
		if ( bGlider )
			return true;
	}
	return false;
}

void APawn::InitSerpentine()
{
	if( Controller->CurrentPath != NULL )
	{
		// round corners smoothly
		// start serpentine dir in current direction
		SerpentineTime = 0.f;
		SerpentineDir = Velocity.SafeNormal();
		SerpentineDist = Clamp(Controller->CurrentPath->CollisionRadius - CylinderComponent->CollisionRadius,0.f,4.f * CylinderComponent->CollisionRadius)
								* (0.5f + 1.f * appFrand());
		FLOAT DP = Controller->CurrentPathDir | SerpentineDir;
		FLOAT DistModifier = 1.f - DP*DP*DP*DP;
		if( (DP < 0) && (DistModifier < 0.5f) )
		{
			SerpentineTime = 0.8f;
		}
		else
		{
			SerpentineDist *= DistModifier;
		}
	}
}

void APawn::HandleSerpentineMovement(FVector& out_Direction, FLOAT Distance, const FVector& Dest)
{
	if( SerpentineTime > 0.f )
	{
		SerpentineTime -= AvgPhysicsTime;
		if( SerpentineTime <= 0.f )
		{
			StartNewSerpentine( Controller->CurrentPathDir, Controller->CurrentPath->Start->Location );
		}
		else if( SerpentineDist > 0.f )
		{
			if( Distance < 2.f * SerpentineDist )
			{
				ClearSerpentine();
			}
			else
			{
				// keep within SerpentineDist units of both the ReachSpec we're using and the line to our destination point
				const FVector& Start = Controller->CurrentPath->Start->Location;
				FVector PathLineDir = Location - (Start + (Controller->CurrentPathDir | (Location - Start)) * Controller->CurrentPathDir);
				FVector DestLine = (Dest - Start).SafeNormal();
				FVector DestLineDir = Location - (Start + (DestLine | (Location - Start)) * DestLine);
				if ( PathLineDir.SizeSquared() >= SerpentineDist * SerpentineDist && (PathLineDir.SafeNormal() | SerpentineDir) > 0.f ||
					 DestLineDir.SizeSquared() >= SerpentineDist * SerpentineDist && (DestLineDir.SafeNormal() | SerpentineDir) > 0.f )
				{
					out_Direction = (Dest - Location + SerpentineDir*SerpentineDist).SafeNormal();
				}
				else
				{
					out_Direction = (out_Direction + 0.2f * SerpentineDir).SafeNormal();
				}
			}
		}
	}
	if( SerpentineTime <= 0.f )
	{
		if( Distance < 2.f * SerpentineDist )
		{
			ClearSerpentine();
		}
		else
		{
			// keep within SerpentineDist units of both the ReachSpec we're using and the line to our destination point
			const FVector& Start = Controller->CurrentPath->Start->Location;
			FVector PathLineDir = Location - (Start + (Controller->CurrentPathDir | (Location - Start)) * Controller->CurrentPathDir);
			FVector DestLine = (Dest - Start).SafeNormal();
			FVector DestLineDir = Location - (Start + (DestLine | (Location - Start)) * DestLine);
			if ( PathLineDir.SizeSquared() >= SerpentineDist * SerpentineDist && (PathLineDir.SafeNormal() | SerpentineDir) > 0.f ||
				 DestLineDir.SizeSquared() >= SerpentineDist * SerpentineDist && (DestLineDir.SafeNormal() | SerpentineDir) > 0.f )
			{
				StartNewSerpentine(Controller->CurrentPathDir,Start);
			}
			else
			{
				out_Direction = (out_Direction + SerpentineDir).SafeNormal();
			}
		}
	}
}

UBOOL APawn::IsGlider()
{
	return !bCanStrafe && (Physics == PHYS_Flying || Physics == PHYS_Swimming);
}

/* rotateToward()
rotate Actor toward a point.  Returns 1 if target rotation achieved.
(Set DesiredRotation, let physics do actual move)
*/
void APawn::rotateToward(FVector FocalPoint)
{
	if( bRollToDesired || Physics == PHYS_Spider )
	{
		return;
	}

	if( IsGlider() )
	{
		Acceleration = Rotation.Vector() * AccelRate;
	}

	FVector Direction = FocalPoint - Location;
	if ( (Physics == PHYS_Flying) 
		&& Controller && Controller->MoveTarget && (Controller->MoveTarget != Controller->Focus) )
	{
		FVector MoveDir = (Controller->MoveTarget->Location - Location);
		FLOAT Dist = MoveDir.Size();
		if ( Dist < MAXPATHDIST )
		{
			Direction = Direction/Dist;
			MoveDir = MoveDir.SafeNormal();
			if ( (Direction | MoveDir) < 0.9f )
			{
				Direction = MoveDir;
				Controller->Focus = Controller->MoveTarget;
			}
		}
	}

	// Rotate toward destination
	DesiredRotation = Direction.Rotation();
	DesiredRotation.Yaw = DesiredRotation.Yaw & 65535;
	if ( (Physics == PHYS_Walking) && (!Controller || !Controller->MoveTarget || !Controller->MoveTarget->GetAPawn()) )
	{
		DesiredRotation.Pitch = 0;
	}
}

UBOOL APhysicsVolume::WillHurt(APawn *P)
{
	if ( !bPainCausing || (DamagePerSec <= 0) || bAIShouldIgnorePain )
		return FALSE;

	return P->HurtByDamageType(DamageType);
}

UBOOL APawn::HurtByDamageType(class UClass* DamageType)
{
	return TRUE;
}

INT APawn::actorReachable(AActor *Other, UBOOL bKnowVisible, UBOOL bNoAnchorCheck)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);
	if ( !Other || Other->bDeleteMe )
		return 0;

	if ( (Other->Physics == PHYS_Flying) && !bCanFly )
		return 0;

	if ( bCanFly && ValidAnchor() && Cast<AVolumePathNode>(Anchor) )
	{
		FVector Dir = Other->Location - Anchor->Location;
		if ( Abs(Dir.Z) < Anchor->CylinderComponent->CollisionHeight )
		{
			Dir.Z = 0.f;
			if ( Dir.SizeSquared() < Anchor->CylinderComponent->CollisionRadius * Anchor->CylinderComponent->CollisionRadius )
				return true;
		}
	}

	// If goal is on the navigation network, check if it will give me reachability
	ANavigationPoint *Nav = Cast<ANavigationPoint>(Other);
	if (Nav != NULL)
	{
		if (!bNoAnchorCheck)
		{
			if (ReachedDestination(Location, Other->Location,Nav))
			{
				SetAnchor( Nav );
				return 1;
			}
			else if( ValidAnchor() )
			{
				UReachSpec* Path = Anchor->GetReachSpecTo(Nav);
				if (Path == NULL)
				{
					// if it's a replacement PickupFactory, try redirecting to original
					APickupFactory* Pickup = Nav->GetAPickupFactory();
					if (Pickup != NULL)
					{
						while (Pickup->OriginalFactory != NULL && Path == NULL)
						{
							Path = Anchor->GetReachSpecTo(Pickup->OriginalFactory);
							Pickup = Pickup->OriginalFactory;
						}
					}
				}
				return ( Path != NULL && 
						!Path->End.Nav()->IsProbing(NAME_SpecialHandling) && // nodes requiring special handling might require extra pathfinding so force code through that path
						Path->supports(appTrunc(CylinderComponent->CollisionRadius), appTrunc(CylinderComponent->CollisionHeight), calcMoveFlags(), appTrunc(GetAIMaxFallSpeed())) && 
						Path->CostFor(this) < UCONST_BLOCKEDPATHCOST );
			}
			else if (Nav->bBlocked || (Nav->bBlockedForVehicles && bPathfindsAsVehicle))
			{
				return FALSE;
			}
		}
	}
	else if (!bNoAnchorCheck && ValidAnchor())
	{
		// find out if a path connected to our Anchor intersects with the Actor we want to reach
		FBox ActorBox = Other->GetComponentsBoundingBox(FALSE);
		INT Radius = appTrunc(CylinderComponent->CollisionRadius);
		INT Height = appTrunc(CylinderComponent->CollisionHeight);
		INT MoveFlags = calcMoveFlags();
		INT MaxFallSpeedInt = appTrunc(GetAIMaxFallSpeed());
		for (INT i = 0; i < Anchor->PathList.Num(); i++)
		{
			UReachSpec* Path = Anchor->PathList(i);
			// if the path is in the navigation octree, is usable by this pawn, and intersects with Other, then Other is reachable
			if(  Path != NULL && 
				 Path->NavOctreeObject != NULL && 
				 *Path->End != NULL &&
				!Path->End.Nav()->bSpecialMove &&	// Don't do for special move paths, it breaks special movement code
				 Path->supports(Radius, Height, MoveFlags, MaxFallSpeedInt) && 
			 	 Path->CostFor(this) < UCONST_BLOCKEDPATHCOST &&
				 Path->NavOctreeObject->BoundingBox.Intersect(ActorBox) && 
				 !Path->NavigationOverlapCheck(ActorBox) )
			{
				return TRUE;
			}
		}
	}

	FVector Dir = Other->Location - Location;

	if ( GWorld->HasBegunPlay() )
	{
		// prevent long reach tests during gameplay as they are expensive
		// Use the navigation network if more than MAXPATHDIST units to goal
		if (Dir.SizeSquared2D() > MAXPATHDISTSQ)
		{
			return 0;
		}
		if ( Other->PhysicsVolume )
		{
			if ( Other->PhysicsVolume->bWaterVolume )
			{
				if ( !bCanSwim )
					return 0;
			}
			else if ( !bCanFly && !bCanWalk )
				return 0;
			if ( Other->PhysicsVolume->WillHurt(this) )
				return 0;
		}
	}

	//check other visible
	if ( !bKnowVisible )
	{
		FCheckResult Hit(1.f);
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		GWorld->SingleLineCheck(Hit, this, Other->Location, ViewPoint, TRACE_World|TRACE_StopAtAnyHit);
		if( Hit.Time!=1.f && Hit.Actor!=Other )
			return 0;
	}

	if ( Other->GetAPawn() )
	{
		APawn* OtherPawn = (APawn*)Other;

		FLOAT Threshold = CylinderComponent->CollisionRadius + ::Min(1.5f * CylinderComponent->CollisionRadius, MeleeRange) + OtherPawn->CylinderComponent->CollisionRadius;
		if (Dir.SizeSquared() <= Square(Threshold))
			return 1;
	}
	FVector realLoc = Location;
	FVector aPoint = Other->Location; //adjust destination
	if ( Other->Physics == PHYS_Falling )
	{
		// check if ground below it
		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck(Hit, this, Other->Location - FVector(0.f,0.f,4.f*SHORTTRACETESTDIST), Other->Location, TRACE_World);
		if ( Hit.Time == 1.f )
			return false;
		aPoint = Hit.Location + FVector(0.f,0.f,CylinderComponent->CollisionRadius + MaxStepHeight);
		if ( GWorld->FarMoveActor(this, aPoint, 1) )
		{
			aPoint = Location;
			GWorld->FarMoveActor(this, realLoc,1,1);
			FVector	ViewPoint = Location;
			ViewPoint.Z += BaseEyeHeight; //look from eyes
			GWorld->SingleLineCheck(Hit, this, aPoint, ViewPoint, TRACE_World);
			if( Hit.Time!=1.f && Hit.Actor!=Other )
				return 0;
		}
		else
			return 0;
	}
	else
	{
		FLOAT OtherRadius, OtherHeight;
		Other->GetBoundingCylinder(OtherRadius, OtherHeight);

		if ( ((CylinderComponent->CollisionRadius > OtherRadius) || (CylinderComponent->CollisionHeight > OtherHeight))
			&& GWorld->FarMoveActor(this, aPoint, 1) )
		{
			aPoint = Location;
			GWorld->FarMoveActor(this, realLoc,1,1);
		}
	}
	return Reachable(aPoint, Other);
}

INT APawn::pointReachable(FVector aPoint, INT bKnowVisible)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	if (GWorld->HasBegunPlay())
	{
		FVector Dir2D = aPoint - Location;
		if (Dir2D.SizeSquared2D() > MAXPATHDISTSQ)
			return 0;
	}

	//check aPoint visible
	if ( !bKnowVisible )
	{
		FVector	ViewPoint = Location;
		ViewPoint.Z += BaseEyeHeight; //look from eyes
		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck( Hit, this, aPoint, ViewPoint, TRACE_World|TRACE_StopAtAnyHit );
		if ( Hit.Actor )
			return 0;
	}

	FVector realLoc = Location;
	if ( GWorld->FarMoveActor(this, aPoint, 1) )
	{
		aPoint = Location; //adjust destination
		GWorld->FarMoveActor(this, realLoc,1,1);
	}
	return Reachable(aPoint, NULL);

}

/**
 * Pawn crouches.
 * Checks if new cylinder size fits (no encroachment), and trigger Pawn->eventStartCrouch() in script if succesful.
 *
 * @param	bClientSimulation	true when called when bIsCrouched is replicated to non owned clients, to update collision cylinder and offset.
 */
void APawn::Crouch(INT bClientSimulation)
{
	// Do not perform if collision is already at desired size.
	if( (CylinderComponent->CollisionHeight == CrouchHeight) && (CylinderComponent->CollisionRadius == CrouchRadius) )
	{
		return;
	}

	// Change collision size to crouching dimensions
	FLOAT OldHeight = CylinderComponent->CollisionHeight;
	FLOAT OldRadius = CylinderComponent->CollisionRadius;
	SetCollisionSize(CrouchRadius, CrouchHeight);
	FLOAT HeightAdjust	= OldHeight - CrouchHeight;

	if( !bClientSimulation )
	{
		if ( (CrouchRadius > OldRadius) || (CrouchHeight > OldHeight) )
		{
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult* FirstHit = GWorld->Hash->ActorEncroachmentCheck
				(	GMainThreadMemStack,
					this, 
					Location - FVector(0,0,HeightAdjust), 
					Rotation, 
					TRACE_Pawns | TRACE_Movers | TRACE_Others 
				);

			UBOOL bEncroached	= false;
			for( FCheckResult* Test = FirstHit; Test!=NULL; Test=Test->GetNext() )
			{
				if ( (Test->Actor != this) && IsBlockedBy(Test->Actor,Test->Component) )
				{
					bEncroached = true;
					break;
				}
			}
			Mark.Pop();
			// If encroached, cancel

			if( bEncroached )
			{
				SetCollisionSize(OldRadius, OldHeight);
				return;
			}
		}

		bNetDirty = true;	// bIsCrouched replication controlled by bNetDirty
		bIsCrouched = true;
	}
	bForceFloorCheck = TRUE;
	eventStartCrouch( HeightAdjust );

}

/**
 * Pawn uncrouches.
 * Checks if new cylinder size fits (no encroachment), and trigger Pawn->eventEndCrouch() in script if succesful.
 *
 * @param	bClientSimulation	true when called when bIsCrouched is replicated to non owned clients, to update collision cylinder and offset.
 */
void APawn::UnCrouch(INT bClientSimulation)
{
	// see if space to uncrouch
	FCheckResult Hit(1.f);
	APawn* DefaultPawn = Cast<APawn>(GetClass()->GetDefaultObject());

	FLOAT	HeightAdjust	= DefaultPawn->CylinderComponent->CollisionHeight - CylinderComponent->CollisionHeight;
	FVector	NewLoc = Location + FVector(0.f,0.f,HeightAdjust);

	UBOOL bEncroached = false;

	// change cylinder directly rather than calling setcollisionsize(), since we don't want to cause touch/untouch notifications unless uncrouch succeeds
	check(CylinderComponent);
	CylinderComponent->SetCylinderSize(DefaultPawn->CylinderComponent->CollisionRadius, DefaultPawn->CylinderComponent->CollisionHeight);
	CylinderComponent->UpdateBounds(); // Force an update of the bounds with the new dimensions

	if( !bClientSimulation )
	{
		AActor* OldBase = Base;
		FVector OldFloor = Floor;
		SetBase(NULL,OldFloor,0);
		FMemMark Mark(GMainThreadMemStack);
		FCheckResult* FirstHit = GWorld->Hash->ActorEncroachmentCheck
			(	GMainThreadMemStack, 
				this, 
				NewLoc, 
				Rotation, 
				TRACE_Pawns | TRACE_Movers | TRACE_Others 
			);

		for( FCheckResult* Test = FirstHit; Test!=NULL; Test=Test->GetNext() )
		{
			if ( (Test->Actor != this) && IsBlockedBy(Test->Actor,Test->Component) )
			{
				bEncroached = true;
				break;
			}
		}
		Mark.Pop();
		// Attempt to move to the adjusted location
		if ( !bEncroached && !GWorld->FarMoveActor(this, NewLoc, 0, false, true) )
		{
			bEncroached = true;
		}

		// if encroached  then abort.
		if( bEncroached )
		{
			CylinderComponent->SetCylinderSize(CrouchRadius, CrouchHeight);
			CylinderComponent->UpdateBounds(); // Update bounds again back to old value
			SetBase(OldBase,OldFloor,0);
			return;
		}
	}	

	// now call setcollisionsize() to cause touch/untouch events
	SetCollisionSize(DefaultPawn->CylinderComponent->CollisionRadius, DefaultPawn->CylinderComponent->CollisionHeight);

	// space enough to uncrouch, so stand up
	if( !bClientSimulation )
	{
		bNetDirty = true;			// bIsCrouched replication controlled by bNetDirty
		bIsCrouched = false;
	}
	bForceFloorCheck = TRUE;
	eventEndCrouch( HeightAdjust );
}

INT APawn::Reachable(FVector aPoint, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);
	
	INT Result = 0;

	if(PhysicsVolume == NULL)
	{
		warnf(TEXT("WARNING! NULL PhysicsVolume in APawn::Reachable for %s"),*GetName());
		return 0;
	}
	if( PhysicsVolume->bWaterVolume )
	{
		Result = swimReachable( aPoint, Location, 0, GoalActor );
	}
	else if( PhysicsVolume->IsA(ALadderVolume::StaticClass()) )
	{
		Result = ladderReachable(aPoint, Location, 0, GoalActor);
	}
	else if( (Physics == PHYS_Walking)	|| 
			 (Physics == PHYS_Swimming) || 
			 (Physics == PHYS_Ladder)	|| 
			 (Physics == PHYS_Falling)	)
	{
		Result = walkReachable( aPoint, Location, 0, GoalActor );
	}
	else if( Physics == PHYS_Flying )
	{
		Result = flyReachable( aPoint, Location, 0, GoalActor );
	}
	else if( Physics == PHYS_Spider )
	{
		Result = spiderReachable( aPoint, Location, 0, GoalActor );
	}
	else
	{
		FCheckResult Hit(1.f);
		FVector Slice = GetDefaultCollisionSize();
		Slice.Z = 1.f;
		FVector Dest = aPoint + Slice.X * (Location - aPoint).SafeNormal();
		if (GWorld->SingleLineCheck(Hit, this, Dest, Location, TRACE_World | TRACE_StopAtAnyHit, Slice))
		{
			if (bCanFly)
			{
				Result = TRUE;
			}
			else
			{
				// if not a flyer, trace down and make sure there's a valid landing
				FLOAT Down = CylinderComponent->CollisionHeight;
				if (GoalActor != NULL)
				{
					FLOAT Radius, Height;
					GoalActor->GetBoundingCylinder(Radius, Height);
					Down += Height;
				}
				Result = (!GWorld->SingleLineCheck(Hit, this, Dest - FVector(0.f, 0.f, Down), Dest, TRACE_World | TRACE_StopAtAnyHit, Slice) && Hit.Normal.Z >= WalkableFloorZ);
			}
		}
		else
		{
			Result = FALSE;
		}
	}

	return Result;
}

UBOOL APawn::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	// get the pawn's normal height (might be crouching or a Scout, so use the max of current/default)
	FLOAT PawnHeight = Max<FLOAT>(P->CylinderComponent->CollisionHeight, ((APawn *)(P->GetClass()->GetDefaultObject()))->CylinderComponent->CollisionHeight);
	FLOAT UpThresholdAdjust = ::Max(0.f, CylinderComponent->CollisionHeight - PawnHeight);
	if ( Physics == PHYS_Falling )
		UpThresholdAdjust += 2.f * P->MaxJumpHeight;

	return P->ReachThresholdTest(TestPosition, Dest, this, 
		UpThresholdAdjust, 
		::Max(0.f,CylinderComponent->CollisionHeight-P->CylinderComponent->CollisionHeight), 
		::Min(1.5f * P->CylinderComponent->CollisionRadius, P->MeleeRange) + CylinderComponent->CollisionRadius);	
}

UBOOL AVehicle::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	// if enemy vehicle, use normal pawn check
	if (!bCollideActors || (P->Controller != NULL && P->Controller->Enemy == this))
	{
		return Super::ReachedBy(P, TestPosition, Dest);
	}

	FRadiusOverlapCheck CheckInfo(TestPosition, P->VehicleCheckRadius);
	for (INT i = 0; i < Components.Num(); i++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Components(i));

		// Only use collidable components to find collision bounding box.
		if (Primitive != NULL && Primitive->IsAttached() && Primitive->CollideActors && CheckInfo.SphereBoundsTest(Primitive->Bounds))
		{
			return TRUE;
		}
	}

	return FALSE;
}

UBOOL AActor::ReachedBy(APawn *P, const FVector &TestPosition, const FVector& Dest)
{
	if ( TouchReachSucceeded(P, TestPosition) )
		return true;

	FLOAT ColHeight, ColRadius;
	GetBoundingCylinder(ColRadius, ColHeight);
	if ( !bBlockActors && GWorld->HasBegunPlay() )
		ColRadius = 0.f;
	return P->ReachThresholdTest(TestPosition, Dest, this, ColHeight, ColHeight, ColRadius);	
}

UBOOL AActor::TouchReachSucceeded(APawn *P, const FVector &TestPosition)
{
	UBOOL bTouchTest = bCollideActors && P->bCollideActors && !GIsEditor;
	if ( bTouchTest )
	{
		if ( TestPosition == P->Location )
		{
			for (INT i = 0; i < Touching.Num(); i++)
			{
				if ( Touching(i) == P )
				{
					// succeed if touching
					return TRUE;
				}
			}
			return FALSE;
		}

		// if TestPosition != P->Location, can't use touching array
		UCylinderComponent* CylComp1 = Cast<UCylinderComponent>(this->CollisionComponent);
		if (CylComp1 != NULL && (!bBlockActors || !CylComp1->BlockActors))
		{
			return
				( (Square(Location.Z - TestPosition.Z) < Square(CylComp1->CollisionHeight + P->CylinderComponent->CollisionHeight))
					&&	( Square(Location.X - TestPosition.X) + Square(Location.Y - TestPosition.Y) <
								Square(CylComp1->CollisionRadius + P->CylinderComponent->CollisionRadius) ) );
		}
	}
	return FALSE;
}

UBOOL APawn::ReachedDestination(class AActor* GoalActor)
{
	if( GoalActor != NULL )
	{
		return ReachedDestination(Location, GoalActor->GetDestination( Controller ), GoalActor);
	}

	return false;
}

/*
	ReachedDestination()
	Return true if sufficiently close to destination.
*/
UBOOL APawn::ReachedDestination(const FVector &TestPosition, const FVector &Dest, AActor* GoalActor)
{
	if ( GoalActor && (!Controller || !Controller->bAdjusting) )
	{
		return GoalActor->ReachedBy(this, TestPosition, Dest);
	}

	return ReachThresholdTest(TestPosition, Dest, NULL, 0.f, 0.f, 0.f);
}

UBOOL APawn::ReachedPoint( FVector Point, AActor* NewAnchor )
{
	if( ReachedDestination( Location, Point, NULL ) )
	{
		ANavigationPoint* Nav = Cast<ANavigationPoint>(NewAnchor);
		if( Nav )
		{
			SetAnchor( Nav );
		}
		return TRUE;
	}

	return FALSE;
}


UBOOL APawn::ReachThresholdTest(const FVector &TestPosition, const FVector &Dest, AActor* GoalActor, FLOAT UpThresholdAdjust, FLOAT DownThresholdAdjust, FLOAT ThresholdAdjust)
{
	// get the pawn's normal height (might be crouching or a Scout, so use the max of current/default)
	const FLOAT PawnFullHeight = Max<FLOAT>(CylinderComponent->CollisionHeight, ((APawn*)(GetClass()->GetDefaultObject()))->CylinderComponent->CollisionHeight);
	FLOAT UpThreshold = UpThresholdAdjust + PawnFullHeight + PawnFullHeight - CylinderComponent->CollisionHeight;
	FLOAT DownThreshold = DownThresholdAdjust + CylinderComponent->CollisionHeight;
	FLOAT Threshold = ThresholdAdjust + CylinderComponent->CollisionRadius;

	FVector Dir = Dest - TestPosition;

	if ( GoalActor )
		Threshold += DestinationOffset;

	// give gliding pawns more leeway
	if ( !bCanStrafe && ((Physics == PHYS_Flying) || (Physics == PHYS_Swimming)) 
		&& ((Velocity | Dir) < 0.f) )
	{
		UpThreshold = 2.f * UpThreshold;
		DownThreshold = 2.f * DownThreshold;
		Threshold = 2.f * Threshold;
	}
	else if ( Physics == PHYS_RigidBody )
	{
		if ( GoalActor )
		{
			FLOAT GoalRadius, GoalHeight;
			GoalActor->GetBoundingCylinder(GoalRadius, GoalHeight);
			UpThreshold = ::Max(UpThreshold, GoalHeight);
		}
		UpThreshold = ::Max(UpThreshold, CylinderComponent->CollisionHeight);
		DownThreshold = ::Max(3.f * CylinderComponent->CollisionHeight, DownThreshold);
	}

	FLOAT Zdiff = Dir.Z;
	Dir.Z = 0.f;
	if ( Dir.SizeSquared() > Threshold * Threshold )
		return false;

	if ( (Zdiff > 0.f) ? (Abs(Zdiff) > UpThreshold) : (Abs(Zdiff) > DownThreshold) )
	{
		if ( (Zdiff > 0.f) ? (Abs(Zdiff) > 2.f * UpThreshold) : (Abs(Zdiff) > 2.f * DownThreshold) )
			return false;

		// Check if above or below because of slope
		FCheckResult Hit(1.f);
		UBOOL bCheckSlope = false;
		if ( (Zdiff < 0.f) && (CylinderComponent->CollisionRadius > CylinderComponent->CollisionHeight) )
		{
			// if wide pawn, use a smaller trace down and check for overlap 
			GWorld->SingleLineCheck( Hit, this, TestPosition - FVector(0.f,0.f,CylinderComponent->CollisionHeight), TestPosition, TRACE_World, FVector(CylinderComponent->CollisionHeight,CylinderComponent->CollisionHeight,CylinderComponent->CollisionHeight));
			bCheckSlope = ( Hit.Time < 1.f );
			Zdiff = Dest.Z - Hit.Location.Z;
		}
		else
		{
			// see if on slope
			GWorld->SingleLineCheck( Hit, this, TestPosition - FVector(0.f,0.f,LEDGECHECKTHRESHOLD+MAXSTEPHEIGHTFUDGE), TestPosition, TRACE_World, FVector(CylinderComponent->CollisionRadius,CylinderComponent->CollisionRadius,CylinderComponent->CollisionHeight));
			bCheckSlope = ( (Hit.Normal.Z < 0.95f) && (Hit.Normal.Z >= WalkableFloorZ) );
		}
		if ( bCheckSlope )
		{
			// check if above because of slope
			if ( (Zdiff < 0.f)
				&& (Zdiff * -1.f < PawnFullHeight + CylinderComponent->CollisionRadius * appSqrt(1.f/(Hit.Normal.Z * Hit.Normal.Z) - 1.f)) )
			{
				return true;
			}
			else
			{
				// might be below because on slope
				FLOAT adjRad = 0.f;
				if ( GoalActor )
				{
					FLOAT GoalHeight;
					GoalActor->GetBoundingCylinder(adjRad, GoalHeight);
				}
				else
				{
					ANavigationPoint* DefaultNavPoint = (ANavigationPoint*)ANavigationPoint::StaticClass()->GetDefaultActor();
					adjRad = DefaultNavPoint->CylinderComponent->CollisionRadius;
				}
				if ( (CylinderComponent->CollisionRadius < adjRad)
					&& (Zdiff < PawnFullHeight + (adjRad + 15.f - CylinderComponent->CollisionRadius) * appSqrt(1.f/(Hit.Normal.Z * Hit.Normal.Z) - 1.f)) )
				{
					return true;
				}
			}
		}
		return false;
	}
	return true;
}

/* ladderReachable()
Pawn is on ladder. Return false if no GoalActor, else true if GoalActor is on the same ladder
*/
INT APawn::ladderReachable(const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);

	if ( !OnLadder || !GoalActor || ((GoalActor->Physics != PHYS_Ladder) && !GoalActor->IsA(ALadder::StaticClass())) )
		return walkReachable(Dest, Start, reachFlags, GoalActor);
	
	ALadder *L = Cast<ALadder>(GoalActor);
	ALadderVolume *GoalLadder = NULL;
	if ( L )
		GoalLadder = L->MyLadder;
	else
	{
		APawn *GoalPawn = GoalActor->GetAPawn(); 
		if ( GoalPawn && GoalPawn->OnLadder )
			GoalLadder = GoalPawn->OnLadder;
		else
			return walkReachable(Dest, Start, reachFlags, GoalActor);
	}

	if ( GoalLadder == OnLadder )
		return bCanClimbLadders;
	return walkReachable(Dest, Start, reachFlags, GoalActor);
}

INT APawn::flyReachable(const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);

	reachFlags = reachFlags | R_FLY;
	INT success = 0;
	FVector CurrentPosition = Start;
	ETestMoveResult stillmoving = TESTMOVE_Moved;
	FVector Direction = Dest - CurrentPosition;
	FLOAT Movesize = ::Max(MAXTESTMOVESIZE, CylinderComponent->CollisionRadius);
	FLOAT MoveSizeSquared = Movesize * Movesize;
	INT ticks = 100; 
	if ( !GWorld->HasBegunPlay() )
		ticks = 10000;

	while (stillmoving != TESTMOVE_Stopped )
	{
		Direction = Dest - CurrentPosition;
		if ( !ReachedDestination(CurrentPosition, Dest, GoalActor) )
		{
			if ( Direction.SizeSquared() < MoveSizeSquared )
				stillmoving = flyMove(Direction, CurrentPosition, GoalActor, 2.f*MINMOVETHRESHOLD);
			else
			{
				Direction = Direction.SafeNormal();
				stillmoving = flyMove(Direction * Movesize, CurrentPosition, GoalActor, MINMOVETHRESHOLD);
			}
			if ( stillmoving == TESTMOVE_HitGoal ) //bumped into goal
			{
				stillmoving = TESTMOVE_Stopped;
				success = 1;
			}
			else if ( (stillmoving != TESTMOVE_Stopped) )
			{
				APhysicsVolume *NewVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);
				if ( NewVolume->bWaterVolume )
				{
					stillmoving = TESTMOVE_Stopped;
					if ( bCanSwim && !NewVolume->WillHurt(this) )
					{
						reachFlags = swimReachable(Dest, CurrentPosition, reachFlags, GoalActor);
						success = reachFlags;
					}
				}
			}
		}
		else
		{
			stillmoving = TESTMOVE_Stopped;
			success = 1;
		}
		ticks--;
		if (ticks < 0)
			stillmoving = TESTMOVE_Stopped;
	}

	if (success)
		return reachFlags;
	else
		return 0;
}

INT APawn::swimReachable(const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);
	reachFlags = reachFlags | R_SWIM;
	INT success = 0;
	FVector CurrentPosition = Start;
	ETestMoveResult stillmoving = TESTMOVE_Moved;
	FVector CollisionExtent = GetDefaultCollisionSize();
	FLOAT Movesize = ::Max(MAXTESTMOVESIZE, CollisionExtent.X);
	FLOAT MoveSizeSquared = Movesize * Movesize;
	INT ticks = 100; 
	if ( !GWorld->HasBegunPlay() )
		ticks = 1000;

	while ( stillmoving != TESTMOVE_Stopped )
	{
		FVector Direction = Dest - CurrentPosition;
		if ( !ReachedDestination(CurrentPosition, Dest, GoalActor) )
		{
			if ( Direction.SizeSquared() < MoveSizeSquared )
				stillmoving = swimMove(Direction, CurrentPosition, GoalActor, 2.f*MINMOVETHRESHOLD);
			else
			{
				Direction = Direction.SafeNormal();
				stillmoving = swimMove(Direction * Movesize, CurrentPosition, GoalActor, MINMOVETHRESHOLD);
			}
			if ( stillmoving == TESTMOVE_HitGoal ) //bumped into goal
			{
				stillmoving = TESTMOVE_Stopped;
				success = 1;
			}

			APhysicsVolume *NewVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);

			if ( NewVolume->bWaterVolume && (stillmoving == TESTMOVE_Stopped) && bCanWalk )
			{
				// allow move up in case close to edge of wall, if still in water
				FCheckResult Hit(1.f);
				TestMove(FVector(0,0,MaxStepHeight), CurrentPosition, Hit, CollisionExtent);
				NewVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);
			}
			if ( !NewVolume->bWaterVolume )
			{
				stillmoving = TESTMOVE_Stopped;
				if (bCanFly)
				{
					reachFlags = flyReachable(Dest, CurrentPosition, reachFlags, GoalActor);
					success = reachFlags;
				}
				else if ( bCanWalk && (Dest.Z < CurrentPosition.Z + CollisionExtent.Z + MaxStepHeight) )
				{
					FCheckResult Hit(1.f);
					TestMove(FVector(0,0,::Max(CollisionExtent.Z + MaxStepHeight,Dest.Z - CurrentPosition.Z)), CurrentPosition, Hit, CollisionExtent);
					if (Hit.Time == 1.f)
					{
						success = flyReachable(Dest, CurrentPosition, reachFlags, GoalActor);
						reachFlags = success & !R_FLY;
						reachFlags |= R_WALK;
					}
				}
			}
			else if ( NewVolume->WillHurt(this) )
			{
				stillmoving = TESTMOVE_Stopped;
				success = 0;
			}
		}
		else
		{
			stillmoving = TESTMOVE_Stopped;
			success = 1;
		}
		ticks--;
		if (ticks < 0)
		{
			stillmoving = TESTMOVE_Stopped;
		}
	}

	if (success)
		return reachFlags;
	else
		return 0;
}

INT APawn::spiderReachable( const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);
	FVector CollisionExtent = GetDefaultCollisionSize();
	reachFlags = reachFlags | R_WALK;
	UBOOL bSuccess = 0;

	FVector CurrentPosition = Start;
	ETestMoveResult stillmoving = TESTMOVE_Moved;
	FVector Direction;

	FLOAT Movesize = CollisionExtent.X;
	INT ticks = 100; 
	if( GWorld->HasBegunPlay() )
	{
		// use longer steps if have begun play and can jump
		if( bJumpCapable )
		{
			Movesize = ::Max(TESTMINJUMPDIST, Movesize);
		}
	}
	else
	{
		// allow longer walks when path building in editor
		ticks = 1000;
	}
	
	FLOAT MoveSizeSquared = Movesize * Movesize;
	FCheckResult Hit(1.f);
	APhysicsVolume *CurrentVolume = GWorld->GetWorldInfo()->GetPhysicsVolume( CurrentPosition, this, FALSE );
	FVector CheckDown = GetGravityDirection();
	CheckDown.Z *= (0.5f * CollisionExtent.Z + MaxStepHeight + 2.f * MAXSTEPHEIGHTFUDGE);

	while( stillmoving == TESTMOVE_Moved )
	{
		if( ReachedDestination( CurrentPosition, Dest, GoalActor ) )
		{
			// Success!
			stillmoving = TESTMOVE_Stopped;
			bSuccess = TRUE;
		}
		else
		{
			Direction = Dest - CurrentPosition;
			Direction.Z = 0; //this is a 2D move
			if( Direction.SizeSquared() < MoveSizeSquared )
			{
				// shorten this step since near end
				stillmoving = walkMove( Direction, CurrentPosition, CollisionExtent, Hit, GoalActor, 2.f*MINMOVETHRESHOLD );
			}
			else
			{
				// step toward destination
				Direction   = Direction.SafeNormal();
				Direction  *= Movesize;
				stillmoving = walkMove( Direction, CurrentPosition, CollisionExtent, Hit, GoalActor, MINMOVETHRESHOLD );
			}

			if( stillmoving != TESTMOVE_Moved )
			{
				if( stillmoving == TESTMOVE_HitGoal ) 
				{
					// bumped into goal - success!
					stillmoving = TESTMOVE_Stopped;
					bSuccess = TRUE;
				}
				else 
				if( bCanFly )
				{
					// no longer walking - see if can fly to destination
					stillmoving = TESTMOVE_Stopped;
					reachFlags	= flyReachable( Dest, CurrentPosition, reachFlags, GoalActor );
					bSuccess	= reachFlags;
				}
				else 
				if( bJumpCapable )
				{
					// try to jump to destination
					reachFlags = reachFlags | R_JUMP;

					if( stillmoving == TESTMOVE_Fell )
					{
						// went off ledge
						FVector Landing = Dest;
						if( GoalActor )
						{
							FLOAT GoalRadius, GoalHeight;
							GoalActor->GetBoundingCylinder(GoalRadius, GoalHeight);
							Landing.Z = Landing.Z - GoalHeight + CollisionExtent.Z;
						}

						stillmoving = FindBestJump(Landing, CurrentPosition);
					}
					else if (stillmoving == TESTMOVE_Stopped)
					{
						// try to jump up over barrier
						stillmoving = FindJumpUp(Direction, CurrentPosition);
						if ( stillmoving == TESTMOVE_HitGoal ) 
						{
							// bumped into goal - success!
							stillmoving = TESTMOVE_Stopped;
							bSuccess = TRUE;
						}
					}
					if ( SetHighJumpFlag() )
						reachFlags = reachFlags | R_HIGHJUMP;
				}
				else 
				if( (stillmoving == TESTMOVE_Fell) && (Movesize > MaxStepHeight) ) 
				{
					// try smaller step
					stillmoving = TESTMOVE_Moved;
					Movesize = MaxStepHeight;
				}
			}
			else
			if ( !GWorld->HasBegunPlay() )
			{
				// FIXME - make sure fully on path
				GWorld->SingleLineCheck(Hit, this, CurrentPosition + CheckDown, CurrentPosition, TRACE_World|TRACE_StopAtAnyHit, 0.5f * CollisionExtent);
				if( Hit.Time == 1.f )
				{
					reachFlags = reachFlags | R_JUMP;	
				}
			}

			// check if entered new physics volume
			APhysicsVolume *NewVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);
			if( NewVolume != CurrentVolume )
			{
				if( NewVolume->WillHurt(this) )
				{
					// failed because entered pain volume
					stillmoving = TESTMOVE_Stopped;
					bSuccess = FALSE;
				}
				else 
				if( NewVolume->bWaterVolume )
				{
					// entered water - try to swim to destination
					stillmoving = TESTMOVE_Stopped;
					if( bCanSwim )
					{
						reachFlags = swimReachable( Dest, CurrentPosition, reachFlags, GoalActor );
						bSuccess = reachFlags;
					}
				}
				else 
				if( bCanClimbLadders && 
					GoalActor && 
					(GoalActor->PhysicsVolume == NewVolume) && 
					NewVolume->IsA(ALadderVolume::StaticClass()) )
				{
					// entered ladder, and destination is on same ladder - success!
					stillmoving = TESTMOVE_Stopped;
					bSuccess = TRUE;
				}
			}

			CurrentVolume = NewVolume;
			if( ticks-- < 0 )
			{
				//debugf(TEXT("OUT OF TICKS"));
				stillmoving = TESTMOVE_Stopped;
			}
		}
	}
	return (bSuccess ? reachFlags : 0);

//	return 1;
}

void APawn::GetBoundingCylinder(FLOAT& CollisionRadius, FLOAT& CollisionHeight) const
{
	// if we are a template, no components will be attached and the default implementation of calling GetComponentsBoundingBox() will do nothing
	// so use our CylinderComponent instead if possible
	if (CylinderComponent != CollisionComponent && IsTemplate() && CylinderComponent != NULL)
	{
		CollisionRadius = CylinderComponent->CollisionRadius;
		CollisionHeight = CylinderComponent->CollisionHeight;
	}
	else
	{
		Super::GetBoundingCylinder(CollisionRadius, CollisionHeight);
	}
}

FVector APawn::GetDefaultCollisionSize()
{
	UCylinderComponent* Cylinder = GWorld->HasBegunPlay() ? GetClass()->GetDefaultObject<APawn>()->CylinderComponent : CylinderComponent;
	return (Cylinder != NULL) ? FVector(Cylinder->CollisionRadius, Cylinder->CollisionRadius, Cylinder->CollisionHeight) : FVector(0.f, 0.f, 0.f);
}

FVector APawn::GetCrouchSize()
{
	return FVector(CrouchRadius, CrouchRadius, CrouchHeight);
}

/*walkReachable() -
walkReachable returns 0 if Actor cannot reach dest, and 1 if it can reach dest by moving in
 straight line
 actor must remain on ground at all times
 Note that Actor is not actually moved
*/
INT APawn::walkReachable(const FVector &Dest, const FVector &Start, INT reachFlags, AActor* GoalActor)
{
	SCOPE_CYCLE_COUNTER(STAT_PathFinding_Reachable);
	TRACK_DETAILED_PATHFINDING_STATS(this);
	FVector CollisionExtent = bCanCrouch ? GetCrouchSize() : GetDefaultCollisionSize();
	reachFlags = reachFlags | R_WALK;
	INT success = 0;
	FVector CurrentPosition = Start;
	ETestMoveResult stillmoving = TESTMOVE_Moved;
	FVector Direction;

	FLOAT Movesize = CollisionExtent.X;
	INT ticks = 100; 
	if (GWorld->HasBegunPlay())
	{
		// use longer steps if have begun play and can jump
		if ( bJumpCapable )
			Movesize = ::Max(TESTMINJUMPDIST, Movesize);
	}
	else
	{
		// allow longer walks when path building in editor
		ticks = 1000;
	}
	
	FLOAT MoveSizeSquared = Movesize * Movesize;
	FCheckResult Hit(1.f);
	APhysicsVolume *CurrentVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);
	FVector CheckDown = FVector(0,0,-1 * (0.5f * CollisionExtent.Z + MaxStepHeight + 2*MAXSTEPHEIGHTFUDGE));

	// make sure pawn starts on ground
	TestMove(CheckDown, CurrentPosition, Hit, CollisionExtent);

	while ( stillmoving == TESTMOVE_Moved )
	{
		if ( ReachedDestination(CurrentPosition, Dest, GoalActor) )
		{
			// Success!
			stillmoving = TESTMOVE_Stopped;
			success = 1;
		}
		else
		{
			FVector LastPosition = CurrentPosition;
			Direction = Dest - CurrentPosition;
			Direction.Z = 0; //this is a 2D move
			if ( Direction.SizeSquared() < MoveSizeSquared )
			{
				// shorten this step since near end
				stillmoving = walkMove(Direction, CurrentPosition, CollisionExtent, Hit, GoalActor, 2.f*MINMOVETHRESHOLD);
			}
			else
			{
				// step toward destination
				Direction = Direction.SafeNormal();
				Direction *= Movesize;
				stillmoving = walkMove(Direction, CurrentPosition, CollisionExtent, Hit, GoalActor, MINMOVETHRESHOLD);
			}
			if (stillmoving != TESTMOVE_Moved)
			{
				if ( stillmoving == TESTMOVE_HitGoal ) 
				{
					// bumped into goal - success!
					stillmoving = TESTMOVE_Stopped;
					success = 1;
				}
				else if ( bCanFly )
				{
					// no longer walking - see if can fly to destination
					stillmoving = TESTMOVE_Stopped;
					reachFlags = flyReachable(Dest, CurrentPosition, reachFlags, GoalActor);
					success = reachFlags;
				}
				else if ( bJumpCapable )
				{
					// try to jump to destination
					reachFlags = reachFlags | R_JUMP;	
					if (stillmoving == TESTMOVE_Fell)
					{
						// went off ledge
						FVector Landing = Dest;
						if ( GoalActor )
						{
							FLOAT GoalRadius, GoalHeight;
							GoalActor->GetBoundingCylinder(GoalRadius, GoalHeight);
							Landing.Z = Landing.Z - GoalHeight + CollisionExtent.Z;
						}
						stillmoving = FindBestJump(Landing, CurrentPosition);
					}
					else if (stillmoving == TESTMOVE_Stopped)
					{
						// try to jump up over barrier
						stillmoving = FindJumpUp(Direction, CurrentPosition);
						if ( stillmoving == TESTMOVE_HitGoal ) 
						{
							// bumped into goal - success!
							stillmoving = TESTMOVE_Stopped;
							success = 1;
						}
					}
					if ( SetHighJumpFlag() )
						reachFlags = reachFlags | R_HIGHJUMP;
				}
				else if ( (stillmoving == TESTMOVE_Fell) && (Movesize > MaxStepHeight) ) 
				{
					// try smaller step
					CurrentPosition = LastPosition;
					stillmoving = TESTMOVE_Moved;
					Movesize = MaxStepHeight;
					MoveSizeSquared = Square(Movesize);
				}
			}
			else if ( !GWorld->HasBegunPlay() )
			{
				// FIXME - make sure fully on path
				GWorld->SingleLineCheck(Hit, this, CurrentPosition + CheckDown, CurrentPosition, TRACE_World|TRACE_StopAtAnyHit, 0.5f * CollisionExtent);
				if ( Hit.Time == 1.f )
					reachFlags = reachFlags | R_JUMP;	
			}

			// check if entered new physics volume
			APhysicsVolume *NewVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(CurrentPosition,this,false);
			if ( NewVolume != CurrentVolume )
			{
				if ( NewVolume->WillHurt(this) )
				{
					// failed because entered pain volume
					stillmoving = TESTMOVE_Stopped;
					success = 0;
				}
				else if ( NewVolume->bWaterVolume )
				{
					// entered water - try to swim to destination
					stillmoving = TESTMOVE_Stopped;
					if (bCanSwim )
					{
						reachFlags = swimReachable(Dest, CurrentPosition, reachFlags, GoalActor);
						success = reachFlags;
					}
				}
				else if ( bCanClimbLadders && GoalActor && (GoalActor->PhysicsVolume == NewVolume)
						&& NewVolume->IsA(ALadderVolume::StaticClass()) )
				{
					// entered ladder, and destination is on same ladder - success!
					stillmoving = TESTMOVE_Stopped;
					success = 1;
				}
			}
			CurrentVolume = NewVolume;
			if (ticks-- < 0)
			{
				//debugf(TEXT("OUT OF TICKS"));
				stillmoving = TESTMOVE_Stopped;
			}
		}
	}
	return (success ? reachFlags : 0);
}

ETestMoveResult APawn::FindJumpUp(FVector Direction, FVector &CurrentPosition)
{
	FCheckResult Hit(1.f);
	FVector StartLocation = CurrentPosition;
	FVector CollisionExtent = GetDefaultCollisionSize();

	TestMove(FVector(0,0,MaxJumpHeight - MaxStepHeight), CurrentPosition, Hit, CollisionExtent);
	ETestMoveResult success = walkMove(Direction, CurrentPosition, CollisionExtent, Hit, NULL, MINMOVETHRESHOLD);

	StartLocation.Z = CurrentPosition.Z;
	if ( success != TESTMOVE_Stopped )
	{
		TestMove(-1.f*FVector(0,0,MaxJumpHeight), CurrentPosition, Hit, CollisionExtent);
		// verify walkmove didn't just step down
		StartLocation.Z = CurrentPosition.Z;
		if ((StartLocation - CurrentPosition).SizeSquared() < MINMOVETHRESHOLD * MINMOVETHRESHOLD)
			return TESTMOVE_Stopped;
	}
	else
		CurrentPosition = StartLocation;

	return success;

}

/* SuggestTossVelocity()
returns a recommended Toss velocity vector, given a destination and a Toss speed magnitude
*/
UBOOL AActor::SuggestTossVelocity(FVector* TossVelocity, const FVector& End, const FVector& Start, FLOAT TossSpeed, FLOAT BaseTossZ, FLOAT DesiredZPct, const FVector& CollisionSize, FLOAT TerminalVelocity, FLOAT OverrideGravityZ)
{
	FLOAT StartXYPct = 1.f - DesiredZPct; 
	FLOAT XYPct = StartXYPct;
	FVector Flight = End - Start;
	FLOAT FlightZ = Flight.Z;
	Flight.Z = 0.f;
	FLOAT FlightSize = Flight.Size();

	if ( (FlightSize == 0.f) || (TossSpeed == 0.f) )
	{
		*TossVelocity = FVector(0.f, 0.f, TossSpeed);
		return false;
	}

	FLOAT Gravity;
	if (OverrideGravityZ != 0.f)
	{
		Gravity = OverrideGravityZ;
	}
	else
	{
		Gravity = (PhysicsVolume != NULL) ? PhysicsVolume->GetGravityZ() : GWorld->GetGravityZ();
	}
	FLOAT XYSpeed = XYPct*TossSpeed;
	FLOAT FlightTime = FlightSize/XYSpeed;
	FLOAT ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
	FLOAT TossSpeedSq = TossSpeed*TossSpeed;
	if ( TerminalVelocity == 0.f )
	{
		TerminalVelocity = GetTerminalVelocity();
	}
	TossSpeedSq = ::Min(TossSpeedSq, Square(TerminalVelocity));

	// iteratively find a working toss velocity with magnitude <= TossSpeed
	if ( StartXYPct == 1.f )
	{
		while ( (XYPct > 0.f) && (ZSpeed*ZSpeed + XYSpeed*XYSpeed > TossSpeedSq) )
		{
			// pick an XYSpeed
			XYPct -= 0.05f;
			XYSpeed = XYPct*TossSpeed;
			FlightTime = FlightSize/XYSpeed;
			ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
		}
	}
	else
	{
		while ( (XYPct < 1.f) && (ZSpeed*ZSpeed + XYSpeed*XYSpeed > TossSpeedSq) )
		{
			// pick an XYSpeed
			XYPct += 0.05f;
			XYSpeed = XYPct*TossSpeed;
			FlightTime = FlightSize/XYSpeed;
			ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
		}
	}

	FVector FlightDir = Flight/FlightSize;

	// make sure we ended with a valid toss velocity
	if ( ZSpeed*ZSpeed + XYSpeed*XYSpeed > (1.2f*TossSpeed * 1.2f*TossSpeed))
	{
		*TossVelocity = TossSpeed * (XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed)).SafeNormal();
		return false;
	}

	// trace check trajectory
	UBOOL bFailed = true;
	FCheckResult Hit(1.f);

	while ( bFailed && (XYPct > 0.f) )
	{
		FVector StartVel = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed+BaseTossZ);
		FLOAT StepSize = 0.125f;
		FVector TraceStart = Start;
		bFailed = false;

		if ( GWorld->HasBegunPlay() )
		{
			// if game in progress, do fast line traces, moved down by collision box size (assumes that obstacles more likely to be below)
			for ( FLOAT Step=0.f; Step<1.f; Step+=StepSize )
			{
				FlightTime = (Step+StepSize) * FlightSize/XYSpeed;
				FVector TraceEnd = Start + StartVel*FlightTime + FVector(0.f, 0.f, Gravity * FlightTime * FlightTime - CollisionSize.Z);
				if ( !GWorld->SingleLineCheck( Hit, this, TraceEnd, TraceStart, TRACE_World|TRACE_StopAtAnyHit ) )
				{
					bFailed = true;
					break;
				}
				TraceStart = TraceEnd;
			}
		}
		else
		{
			// if no game in progress, do slower but more accurate box traces
			for ( FLOAT Step=0.f; Step<1.f; Step+=StepSize )
			{
				FlightTime = (Step+StepSize) * FlightSize/XYSpeed;
				FVector TraceEnd = Start + StartVel*FlightTime + FVector(0.f, 0.f, Gravity * FlightTime * FlightTime);
				if ( !GWorld->SingleLineCheck( Hit, this, TraceEnd, TraceStart, TRACE_World|TRACE_StopAtAnyHit, CollisionSize ) )
				{
					bFailed = true;
					break;
				}
				TraceStart = TraceEnd;
			}
		}

		if ( bFailed )
		{
			if ( XYPct >= StartXYPct )
			{
				// no valid toss possible - return an average arc
				XYSpeed = 0.7f*TossSpeed;
				FlightTime = FlightSize/XYSpeed;
				ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
				*TossVelocity = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed);
				return false;
			}
			else
			{
				// raise trajectory
				XYPct -= 0.1f;
				XYSpeed = XYPct*TossSpeed;
				FlightTime = FlightSize/XYSpeed;
				ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
				if ( ZSpeed*ZSpeed + XYSpeed * XYSpeed > TossSpeed * TossSpeed )
				{
					// no valid toss possible - return an average arc
					XYSpeed = 0.7f*TossSpeed;
					FlightTime = FlightSize/XYSpeed;
					ZSpeed = FlightZ/FlightTime - Gravity * FlightTime - BaseTossZ;
					*TossVelocity = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed);
					return false;
				}
			}
		}
	}
	*TossVelocity = XYSpeed*FlightDir + FVector(0.f,0.f,ZSpeed);
	return true;
}

UBOOL APawn::SuggestJumpVelocity(FVector& JumpVelocity, FVector End, FVector Start)
{
	FVector JumpDir = End - Start;
	FLOAT JumpDirZ = JumpDir.Z;
	JumpDir.Z = 0.f;
	FLOAT JumpDist = JumpDir.Size();

	if ( (JumpDist == 0.f) || (JumpZ <= 0.f) )
	{
		JumpVelocity = FVector(0.f, 0.f, JumpZ);
		return false;
	}

	FLOAT Gravity = GetGravityZ();
	FLOAT XYSpeed = GroundSpeed;
	FLOAT JumpTime = JumpDist/XYSpeed;
	FLOAT ZSpeed = JumpDirZ/JumpTime - Gravity * JumpTime;

	if ( (ZSpeed < 0.25f * JumpZ) && (JumpDirZ < 0.f) )
	{
		ZSpeed = 0.25f * JumpZ;

		// calculate XYSpeed by solving this quadratic equation for JumpTime
		// Gravity*JumpTime*JumpTime + ZSpeed*JumpTime - JumpDirZ = 0;
		JumpTime = (-1.f*ZSpeed - appSqrt(ZSpeed*ZSpeed + 4.f*Gravity*JumpDirZ))/(2.f*Gravity);
		XYSpeed = JumpDist/JumpTime;
	}
	else if ( ZSpeed > JumpZ )
	{
		// see if could make jump if we gave more time
		const FLOAT TimeSlice = 0.1f;
		FLOAT LastZSpeed = ZSpeed;
		do
		{
			JumpTime += TimeSlice;
			FLOAT NewZSpeed = JumpDirZ/JumpTime - Gravity * JumpTime;
			if (NewZSpeed > LastZSpeed)
			{
				JumpVelocity = XYSpeed * JumpDir/JumpDist + FVector(0.f,0.f,JumpZ);
				return false;
			}
			LastZSpeed = NewZSpeed;
		} while (LastZSpeed > JumpZ);
		// found usable jump speed
		ZSpeed = LastZSpeed;
		XYSpeed = JumpDist/JumpTime;
	}
   
	JumpVelocity = XYSpeed * JumpDir/JumpDist + FVector(0.f,0.f,ZSpeed);
	return true;
}

/* Find best jump from current position toward destination.  Assumes that there is no immediate
barrier.  
*/
ETestMoveResult APawn::FindBestJump(FVector Dest, FVector &CurrentPosition)
{
	// Calculate jump velocity to get as close as possible to Dest
	FVector StartVel(0.f,0.f,0.f); 
	SuggestJumpVelocity(StartVel, Dest, CurrentPosition);

	// trace jump to validate that it is not obstructed
	FVector StartPos = CurrentPosition;
	FVector TraceStart = CurrentPosition;
	FVector CollisionSize = GetDefaultCollisionSize();
	UBOOL bFailed = FALSE;
	FLOAT FlightSize = (CurrentPosition - Dest).Size();
	if ( FlightSize < CollisionSize.X )
	{
		CurrentPosition = Dest;
		return TESTMOVE_Moved;
	}
	FLOAT FlightTotalTime = FlightSize/GroundSpeed;
	FLOAT StepSize = ::Max(0.03f,CylinderComponent->CollisionRadius/FlightSize);
	FCheckResult Hit(1.f);
	FLOAT Step = 0.f;
	FVector TraceEnd = CurrentPosition;

	FLOAT GravityNet;
	do
	{
		APhysicsVolume *CurrentPhysicsVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(TraceStart,NULL,false);
		FLOAT FlightTime = (Step+StepSize) * FlightTotalTime;
		FLOAT BaseGravity = CurrentPhysicsVolume->GetGravityZ();
		GravityNet = BaseGravity * FlightTime * FlightTime;
		TraceEnd = CurrentPosition + StartVel*FlightTime + FVector(0.f, 0.f, GravityNet);
		if ( GravityNet < -1.f * GetAIMaxFallSpeed() )
		{
			bFailed = TRUE;
		}
		else if ( CurrentPhysicsVolume->bWaterVolume )
		{
			break;
		}
		else if ( !GWorld->SingleLineCheck( Hit, this, TraceEnd, TraceStart, TRACE_AllBlocking, CollisionSize ) )
		{
			if ( Hit.Normal.Z < WalkableFloorZ )
			{
				// hit a wall
				// if we're still moving upward, see if we can get over the wall
				UBOOL bMustFall = TRUE;
				if (TraceEnd.Z > TraceStart.Z)
				{
					// trace upwards as high as our jump will get us
					FLOAT CurrentVelocityZ = StartVel.Z + BaseGravity * FlightTime;
					FlightTime = (-StartVel.Z / BaseGravity) - FlightTime;
					TraceEnd = Hit.Location + FVector(0.f, 0.f, (CurrentVelocityZ * FlightTime) + (0.5f * BaseGravity * FlightTime * FlightTime));
					if (!GWorld->SingleLineCheck(Hit, this, TraceEnd, Hit.Location, TRACE_AllBlocking, CollisionSize))
					{
						TraceEnd = Hit.Location;
					}
					// now trace one step in the direction we want to go to see if we got over the wall
					TraceStart = TraceEnd;
					TraceEnd += FVector(StartVel.X, StartVel.Y, 0.f) * StepSize;
					bMustFall = !GWorld->SingleLineCheck(Hit, this, TraceEnd, TraceStart, TRACE_AllBlocking, CollisionSize);
					// add the time it took to reach the jump apex
					Step += FlightTime / FlightTotalTime;
					// adjust CurrentPosition to account for only moving upward during this part of the jump
					CurrentPosition -= FVector(StartVel.X, StartVel.Y, 0.f) * FlightTime;
				}
				if (bMustFall)
				{
					// see if acceptable landing below
					// limit trace to point at which would reach MaxFallSpeed
					FlightTime = appSqrt(Abs(GetAIMaxFallSpeed()/BaseGravity)) - FlightTime;
					TraceEnd = Hit.Location + FVector(0.f, 0.f, BaseGravity*FlightTime*FlightTime);
					GWorld->SingleLineCheck( Hit, this, TraceEnd, Hit.Location, TRACE_AllBlocking, CollisionSize );

					bFailed = ( (Hit.Time == 1.f) || (Hit.Normal.Z < WalkableFloorZ) );
					if ( bFailed )
					{
						// check if entered water
						APhysicsVolume* PossibleWaterPhysicsVolume = GWorld->GetWorldInfo()->GetPhysicsVolume(Hit.Location,NULL,false);
						if ( PossibleWaterPhysicsVolume->bWaterVolume )
						{
							bFailed = FALSE;
						}
					}
					TraceEnd = Hit.Location;
					break;
				}
			}
			else
			{
				TraceEnd = Hit.Location;
				break;
			}
		}
		TraceStart = TraceEnd;
		Step += StepSize;
	} while (!bFailed);

	if ( bFailed )
	{
		CurrentPosition = StartPos;
		return TESTMOVE_Stopped;
	}

	// if we're building paths, set the Scout's MaxLandingVelocity to its largest fall
	SetMaxLandingVelocity(GravityNet);
	CurrentPosition = TraceEnd;
	return TESTMOVE_Moved;
}

FVector APawn::GetGravityDirection()
{
	if( Physics == PHYS_Spider )
	{
		return -Floor;
	}

	return FVector(0,0,-1);
}

ETestMoveResult APawn::HitGoal(AActor *GoalActor)
{
	if ( GoalActor->IsA(ANavigationPoint::StaticClass()) && !GoalActor->bBlockActors )
		return TESTMOVE_Stopped;

	return TESTMOVE_HitGoal;
}

void APawn::TestMove(const FVector &Delta, FVector &CurrentPosition, FCheckResult& Hit, const FVector &CollisionExtent)
{
	GWorld->SingleLineCheck(Hit, this, CurrentPosition+Delta, CurrentPosition, TRACE_Others|TRACE_Volumes|TRACE_World|TRACE_Blocking, CollisionExtent);
	if ( Hit.Actor )
	{
        CurrentPosition = Hit.Location;
	}
	else
	{
		CurrentPosition = CurrentPosition+Delta;
	}
}

/* walkMove()
Move direction must not be adjusted.
*/
ETestMoveResult APawn::walkMove(FVector Delta, FVector &CurrentPosition, const FVector &CollisionExtent, FCheckResult& Hit, AActor* GoalActor, FLOAT threshold)
{
	FVector StartLocation = CurrentPosition;
	Delta.Z = 0.f;

	//-------------------------------------------------------------------------------------------
	//Perform the move
	FVector GravDir = GetGravityDirection();
	FVector Down = GravDir * MaxStepHeight;

	TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
	if( GoalActor && (Hit.Actor == GoalActor) )
	{
		return HitGoal( GoalActor );
	}

	FVector StopLocation = Hit.Location;
	if(Hit.Time < 1.f) //try to step up
	{
		Delta = Delta * (1.f - Hit.Time);
		TestMove(-1.f*Down, CurrentPosition, Hit, CollisionExtent);
		TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
		if( GoalActor && (Hit.Actor == GoalActor) )
		{
			return HitGoal(GoalActor);
		}

		TestMove(Down, CurrentPosition, Hit, CollisionExtent);
		if( Hit.Time < 1.f )
		{
			if( (GravDir.Z < 0.f && Hit.Normal.Z <  WalkableFloorZ) ||	// valid floor for walking
				(GravDir.Z > 0.f && Hit.Normal.Z > -WalkableFloorZ) )   // valid floor for spidering
			{
				// Want only good floors, else undo move
				CurrentPosition = StopLocation;
				return TESTMOVE_Stopped;
			}
		}
	}

	//drop to floor
	FVector Loc = CurrentPosition;
	Down = GravDir * (MaxStepHeight + MAXSTEPHEIGHTFUDGE);
	TestMove( Down, CurrentPosition, Hit, CollisionExtent );

	// If there was no hit OR
	// If gravity is down and hit normal is too steep for floor OR
	// If gravity is up and hit normal is too steep for ceiling
	if( (Hit.Time == 1.f) || 
		(GravDir.Z < 0.f && Hit.Normal.Z <  WalkableFloorZ) || 
		(GravDir.Z > 0.f && Hit.Normal.Z > -WalkableFloorZ) )	// occurs w/ phys_spider
	{
		// Then falling
		CurrentPosition = Loc;
		return TESTMOVE_Fell;
	}

	if( GoalActor && (Hit.Actor == GoalActor) )
	{
		return HitGoal(GoalActor);
	}

	//check if move successful
	if( (CurrentPosition - StartLocation).SizeSquared() < threshold * threshold )
	{
		return TESTMOVE_Stopped;
	}
	return TESTMOVE_Moved;
}

ETestMoveResult APawn::flyMove(FVector Delta, FVector &CurrentPosition, AActor* GoalActor, FLOAT threshold)
{
	FVector StartLocation = Location;
	FVector Down = FVector(0,0,-1) * MaxStepHeight;
	FVector Up = -1 * Down;
	FVector CollisionExtent = GetDefaultCollisionSize();
	FCheckResult Hit(1.f);

	TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
	if ( GoalActor && (Hit.Actor == GoalActor) )
		return HitGoal(GoalActor);
	if (Hit.Time < 1.f) //try to step up
	{
		Delta = Delta * (1.f - Hit.Time);
		TestMove(Up, CurrentPosition, Hit, CollisionExtent);
		TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return HitGoal(GoalActor);
	}

	if ((CurrentPosition - StartLocation).SizeSquared() < threshold * threshold)
		return TESTMOVE_Stopped;

	return TESTMOVE_Moved;
}

ETestMoveResult APawn::swimMove(FVector Delta, FVector &CurrentPosition, AActor* GoalActor, FLOAT threshold)
{
	FVector StartLocation = CurrentPosition;
	FVector Down = FVector(0,0,-1) * MaxStepHeight;
	FVector Up = -1 * Down;
	FCheckResult Hit(1.f);
	FVector CollisionExtent = GetDefaultCollisionSize();

	TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
	if ( GoalActor && (Hit.Actor == GoalActor) )
		return HitGoal(GoalActor);
	if ( !PhysicsVolume->bWaterVolume )
	{
		FVector End = findWaterLine(StartLocation, CurrentPosition);
		if (End != CurrentPosition)
				TestMove(End - CurrentPosition, CurrentPosition, Hit, CollisionExtent);
		return TESTMOVE_Stopped;
	}
	else if (Hit.Time < 1.f) //try to step up
	{
		Delta = Delta * (1.f - Hit.Time);
		TestMove(Up, CurrentPosition, Hit, CollisionExtent);
		TestMove(Delta, CurrentPosition, Hit, CollisionExtent);
		if ( GoalActor && (Hit.Actor == GoalActor) )
			return HitGoal(GoalActor); //bumped into goal
	}

	if ((CurrentPosition - StartLocation).SizeSquared() < threshold * threshold)
		return TESTMOVE_Stopped;

	return TESTMOVE_Moved;
}

/* TryJumpUp()
Check if could jump up over obstruction
*/
UBOOL APawn::TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility)
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
		if ( Hit.Time < 1.f )
			End = Hit.Location;
		GWorld->SingleLineCheck(Hit, this, Destination, End, TraceFlags);
		if ( (Hit.Time < 1.f) && (Hit.Actor != Controller->MoveTarget) )
			return false;
	}

	GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TRACE_World, GetCylinderExtent());
	FLOAT FirstHit = Hit.Time;
	if ( FirstHit > 0.5f )
	{
		GWorld->SingleLineCheck(Hit, this, Location + Up * Hit.Time + Out, Location + Up * Hit.Time, TraceFlags, GetCylinderExtent());
		return (Hit.Time == 1.f);
	}
	return false;
}

/* PickWallAdjust()
Check if could jump up over obstruction (only if there is a knee height obstruction)
If so, start jump, and return current destination
Else, try to step around - return a destination 90 degrees right or left depending on traces
out and floor checks

returns true if successful adjustment was found
*/
UBOOL APawn::PickWallAdjust(FVector WallHitNormal, AActor* HitActor)
{
	if ( (Physics == PHYS_Falling) || !Controller )
		return FALSE;

	if ( (Physics == PHYS_Flying) || (Physics == PHYS_Swimming) )
		return Pick3DWallAdjust(WallHitNormal, HitActor);

	DWORD TraceFlags = TRACE_World | TRACE_StopAtAnyHit;
	if ( HitActor && !HitActor->bWorldGeometry )
		TraceFlags = TRACE_AllBlocking | TRACE_StopAtAnyHit;

	// first pick likely dir with traces, then check with testmove
	FCheckResult Hit(1.f);
	FVector ViewPoint = Location + FVector(0.f,0.f,BaseEyeHeight);
	FVector Dir = Controller->DesiredDirection();
	FVector Dest = Controller->GetDestinationPosition();
	FLOAT zdiff = Dir.Z;
	Dir.Z = 0.f;
	FLOAT AdjustDist = 2.5f * CylinderComponent->CollisionRadius;
	AActor *MoveTarget = ( Controller->MoveTarget ? Controller->MoveTarget->AssociatedLevelGeometry() : NULL );

	if ( (zdiff < CylinderComponent->CollisionHeight) && ((Dir | Dir) - CylinderComponent->CollisionRadius * CylinderComponent->CollisionRadius < 0.f) )
	{
		return FALSE;
	}
	FLOAT Dist = Dir.Size();
	if ( Dist == 0.f )
	{
		return FALSE;
	}
	Dir = Dir/Dist;
	GWorld->SingleLineCheck( Hit, this, Dest, ViewPoint, TraceFlags );
	if ( Hit.Actor && (Hit.Actor != MoveTarget) )
		AdjustDist += CylinderComponent->CollisionRadius;

	//look left and right
	FVector Left = FVector(Dir.Y, -1 * Dir.X, 0);
	INT bCheckRight = 0;
	FVector CheckLeft = Left * 1.4f * CylinderComponent->CollisionRadius;
	GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
	if ( Hit.Actor && (Hit.Actor != MoveTarget) ) //try right
	{
		bCheckRight = 1;
		Left *= -1;
		CheckLeft *= -1;
		GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
	}

	UBOOL bNoVisibility = Hit.Actor && (Hit.Actor != MoveTarget);

	if ( (Physics == PHYS_Walking) && bCanJump && TryJumpUp(Dir, Dest, TraceFlags, bNoVisibility) )
	{
		Controller->JumpOverWall(WallHitNormal);
		return TRUE;
	}

	if ( bNoVisibility )
	{
		return FALSE;
	}

	//try step left or right
	FVector Out = 14.f * Dir;
	Left *= AdjustDist;
	GWorld->SingleLineCheck(Hit, this, Location + Left, Location, TraceFlags, GetCylinderExtent());
	if (Hit.Time == 1.f)
	{
		GWorld->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TraceFlags, GetCylinderExtent());
		if (Hit.Time == 1.f)
		{
			Controller->SetAdjustLocation( Location + Left, TRUE );
			return TRUE;
		}
	}
	
	if ( !bCheckRight ) // if didn't already try right, now try it
	{
		CheckLeft *= -1;
		GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
		if ( Hit.Time < 1.f )
			return FALSE;
		Left *= -1;
		GWorld->SingleLineCheck(Hit, this, Location + Left, Location, TraceFlags, GetCylinderExtent());
		if (Hit.Time == 1.f)
		{
			GWorld->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TraceFlags, GetCylinderExtent());
			if (Hit.Time == 1.f)
			{
				Controller->SetAdjustLocation( Location + Left, TRUE );
				return TRUE;
			}
		}
	}
	return FALSE;
}

/* Pick3DWallAdjust()
pick wall adjust when swimming or flying
*/
UBOOL APawn::Pick3DWallAdjust(FVector WallHitNormal, AActor* HitActor)
{
	DWORD TraceFlags = TRACE_World | TRACE_StopAtAnyHit;
	if ( HitActor && !HitActor->bWorldGeometry )
		TraceFlags = TRACE_AllBlocking | TRACE_StopAtAnyHit;

	// first pick likely dir with traces, then check with testmove
	FCheckResult Hit(1.f);
	FVector ViewPoint = Location + FVector(0.f,0.f,BaseEyeHeight);
	FVector Dir = Controller->DesiredDirection();
	FVector Dest = Controller->GetDestinationPosition();
	FLOAT zdiff = Dir.Z;
	Dir.Z = 0.f;
	FLOAT AdjustDist = 2.5f * CylinderComponent->CollisionRadius;
	AActor *MoveTarget = ( Controller->MoveTarget ? Controller->MoveTarget->AssociatedLevelGeometry() : NULL );

	// if swimming, maybe just want to get out of water
	if ( bCanWalk && (Physics == PHYS_Swimming) && MoveTarget && MoveTarget->PhysicsVolume && !MoveTarget->PhysicsVolume->bWaterVolume )
	{
		FVector Up = FVector(0,0,4.f*CylinderComponent->CollisionHeight) - CylinderComponent->CollisionRadius * (MoveTarget->Location - Location).SafeNormal();
		GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TraceFlags, GetCylinderExtent());
		if (Hit.Time > 0.5f )
		{
			Controller->SetAdjustLocation( Location + Up, TRUE );
			return TRUE;
		}
	}		
	INT bCheckUp = 0;
	if ( zdiff < CylinderComponent->CollisionHeight )
	{
		if ( (Dir | Dir) - CylinderComponent->CollisionRadius * CylinderComponent->CollisionRadius < 0 )
			return FALSE;
		if ( Dir.SizeSquared() < 4 * CylinderComponent->CollisionHeight * CylinderComponent->CollisionHeight )
		{
			FVector Up = FVector(0,0,2.f*CylinderComponent->CollisionHeight);
			bCheckUp = 1;
			if ( Location.Z > Dest.Z )
			{
				bCheckUp = -1;
				Up *= -1;
			}
			GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TraceFlags, GetCylinderExtent());
			if (Hit.Time == 1.f)
			{
				FVector ShortDir = Dir.SafeNormal();
				ShortDir *= CylinderComponent->CollisionRadius;
				GWorld->SingleLineCheck(Hit, this, Location + Up + ShortDir, Location + Up, TraceFlags, GetCylinderExtent());
				if (Hit.Time == 1.f)
				{
					Controller->SetAdjustLocation( Location + Up, TRUE );
					return TRUE;
				}
			}
		}
	}

	FLOAT Dist = Dir.Size();
	if ( Dist == 0.f )
		return FALSE;
	Dir = Dir/Dist;
	GWorld->SingleLineCheck( Hit, this, Dest, ViewPoint, TraceFlags );
	if ( (Hit.Actor != MoveTarget) && (zdiff > 0) )
	{
		FVector Up = FVector(0,0, 2.f*CylinderComponent->CollisionHeight);
		GWorld->SingleLineCheck(Hit, this, Location + 2 * Up, Location, TraceFlags, GetCylinderExtent());
		if (Hit.Time == 1.f)
		{
			Controller->SetAdjustLocation( Location + Up, TRUE );
			return TRUE;
		}
	}

	//look left and right
	FVector Left = FVector(Dir.Y, -1 * Dir.X, 0);
	INT bCheckRight = 0;
	FVector CheckLeft = Left * 1.4f * CylinderComponent->CollisionRadius;
	GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
	if ( Hit.Actor != MoveTarget ) //try right
	{
		bCheckRight = 1;
		Left *= -1;
		CheckLeft *= -1;
		GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
	}

	if ( Hit.Actor != MoveTarget ) //neither side has visibility
		return FALSE;

	FVector Out = 14.f * Dir;

	//try step left or right
	Left *= AdjustDist;
	GWorld->SingleLineCheck(Hit, this, Location + Left, Location, TraceFlags, GetCylinderExtent());
	if (Hit.Time == 1.f)
	{
		GWorld->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TraceFlags, GetCylinderExtent());
		if (Hit.Time == 1.f)
		{
			Controller->SetAdjustLocation( Location + Left, TRUE );
			return TRUE;
		}
	}
	
	if ( !bCheckRight ) // if didn't already try right, now try it
	{
		CheckLeft *= -1;
		GWorld->SingleLineCheck(Hit, this, Dest, ViewPoint + CheckLeft, TraceFlags); 
		if ( Hit.Time < 1.f )
			return FALSE;
		Left *= -1;
		GWorld->SingleLineCheck(Hit, this, Location + Left, Location, TraceFlags, GetCylinderExtent());
		if (Hit.Time == 1.f)
		{
			GWorld->SingleLineCheck(Hit, this, Location + Left + Out, Location + Left, TraceFlags, GetCylinderExtent());
			if (Hit.Time == 1.f)
			{
				Controller->SetAdjustLocation( Location + Left, TRUE );
				return TRUE;
			}
		}
	}

	//try adjust up or down if swimming or flying
	FVector Up = FVector(0,0,2.5f*CylinderComponent->CollisionHeight);

	if ( bCheckUp != 1 )
	{
		GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TraceFlags, GetCylinderExtent());
		if ( Hit.Time > 0.7f )
		{
			GWorld->SingleLineCheck(Hit, this, Location + Up + Out, Location + Up, TraceFlags, GetCylinderExtent());
			if ( (Hit.Time == 1.f) || (Hit.Normal.Z > 0.7f) )
			{
				Controller->SetAdjustLocation( Location + Up, TRUE );
				return TRUE;
			}
		}
	}

	if ( bCheckUp != -1 )
	{
		Up *= -1; //try adjusting down
		GWorld->SingleLineCheck(Hit, this, Location + Up, Location, TraceFlags, GetCylinderExtent());
		if ( Hit.Time > 0.7f )
		{
			GWorld->SingleLineCheck(Hit, this, Location + Up + Out, Location + Up, TraceFlags, GetCylinderExtent());
			if (Hit.Time == 1.f)
			{
				Controller->SetAdjustLocation( Location + Up, TRUE );
				return TRUE;
			}
		}
	}

	return FALSE;
}

/*-----------------------------------------------------------------------------
	Networking functions.
-----------------------------------------------------------------------------*/
/** GetNetPriority()
@param Viewer		PlayerController owned by the client for whom net priority is being determined
@param InChannel	Channel on which this actor is being replicated.
@param Time			Time since actor was last replicated
@return				Priority of this actor for replication
*/
FLOAT APawn::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, FLOAT Time, UBOOL bLowBandwidth)
{
	return 1;
}

/** GetNetPriority()
@param Viewer		PlayerController owned by the client for whom net priority is being determined
@param InChannel	Channel on which this actor is being replicated.
@param Time			Time since actor was last replicated
@return				Priority of this actor for replication
*/
FLOAT AProjectile::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, FLOAT Time, UBOOL bLowBandwidth)
{
	return 1;
}

UBOOL APawn::SharingVehicleWith(APawn *P)
{
	return ( P && ((Base == P) || (P->Base == this)) );
}

void APawn::PushedBy(AActor* Other)
{
	bForceFloorCheck = TRUE;
}

/** Update controller's view rotation as pawn's base rotates
*/
void APawn::UpdateBasedRotation(FRotator &FinalRotation, const FRotator& ReducedRotation)
{
	FLOAT ControllerRoll = 0;
	if( Controller != NULL && !bIgnoreBaseRotation )
	{
		Controller->OldBasedRotation = Controller->Rotation;
		ControllerRoll = Controller->Rotation.Roll;
		Controller->Rotation += ReducedRotation;
	}

	// If its a pawn, and its not a crawler, remove roll.
	if( !bCrawler )
	{
		FinalRotation.Roll = Rotation.Roll;
		if( Controller )
		{
			Controller->Rotation.Roll = appTrunc(ControllerRoll);
		}
	}
}

void APawn::ReverseBasedRotation()
{
	if (Controller != NULL && !bIgnoreBaseRotation)
	{
		Controller->Rotation = Controller->OldBasedRotation;
	}
}

/*
 * Route finding notification (sent to target)
 * Returns whether only visible anchors should be looked for if EndAnchor is not returned
 * (rather than actually checking if anchor is also reachable)
 */
ANavigationPoint* APawn::SpecifyEndAnchor(APawn* RouteFinder)
{
	ANavigationPoint* EndAnchor = NULL;

	if ( ValidAnchor() )
	{
		// use currently valid anchor
		EndAnchor = Anchor;
	}
	else
	{
		// use this pawn's destination as endanchor for routfinder
		if ( Controller && (Controller->GetStateFrame()->LatentAction == UCONST_LATENT_MOVETOWARD) )
		{
			EndAnchor = Cast<ANavigationPoint>(Controller->MoveTarget);
		}
	}

	// maybe we can just use a recently valid anchor for this pawn
	FLOAT MaxAnchorAge = (Physics == PHYS_Falling) ? 1.f : 0.25f;
	if ( !EndAnchor && LastAnchor && (RouteFinder->Anchor != LastAnchor) && (GWorld->GetTimeSeconds() - LastValidAnchorTime < MaxAnchorAge)
		&& Controller && Controller->LineOfSightTo(LastAnchor) )
	{
		EndAnchor = LastAnchor;
	}

	if(EndAnchor != NULL && EndAnchor->IsUsableAnchorFor(RouteFinder))
	{
		return EndAnchor;
	}
	else
	{
		return NULL;
	}	
}

/**
  * RETURN TRUE if will accept as anchor visible nodes that are not reachable
  */
UBOOL APawn::AnchorNeedNotBeReachable()
{
	if ( Physics == PHYS_Falling )
	{
		 return TRUE;
	}

	APlayerController *PC = Controller ? Controller->GetAPlayerController() : NULL;
	if ( PC && (Location == PC->FailedPathStart) )
	{
		return TRUE;
	}
	return FALSE;
}

/*
 * Notify actor of anchor finding result
 * @PARAM EndAnchor is the anchor found
 * @PARAM RouteFinder is the pawn which requested the anchor finding
 */
void APawn::NotifyAnchorFindingResult(ANavigationPoint* EndAnchor, APawn* RouteFinder)
{
	if ( EndAnchor )
	{
		// save valid anchor info
		LastValidAnchorTime = GWorld->GetTimeSeconds();
		LastAnchor = EndAnchor;
	}
}

/* epic ===============================================
* ::GetTeamNum
*
 * Queries the PRI and returns our current team index.
* =====================================================
*/
BYTE APawn::GetTeamNum()
{
	if ( Controller )
	{
		return Controller->GetTeamNum();
	}
	else if ( PlayerReplicationInfo )
	{
		return (PlayerReplicationInfo->Team) ? PlayerReplicationInfo->Team->TeamIndex : 255;
	}
	else if ( DrivenVehicle )
	{
		return DrivenVehicle->GetTeamNum();
	}
	else
	{
		return 255;
	}
}

FVector APawn::GetPawnViewLocation()
{
	return Location + FVector(0.0f,0.0f,1.0f) * BaseEyeHeight;
}

FRotator APawn::GetViewRotation()
{
	APlayerController* PC = NULL;

	if ( Controller != NULL )
	{
		return Controller->Rotation;
	}
	else if ( Role < ROLE_Authority )
	{
		// check if being spectated
		
		for( INT Idx=0; Idx<GEngine->GamePlayers.Num(); Idx++ )
		{
			if (GEngine->GamePlayers(Idx) && GEngine->GamePlayers(Idx)->Actor)
			{
				PC = Cast<APlayerController>(GEngine->GamePlayers(Idx)->Actor);
				if(PC && PC->ViewTarget == this)
				{
					return PC->BlendedTargetViewRotation;
				}
			}
		}
	}

	return Rotation;

}

/* epic ===============================================
* ::GetTeamNum
*
 * Queries the PRI and returns our current team index.
* =====================================================
*/
BYTE AProjectile::GetTeamNum()
{
	if ( InstigatorController )
	{
		return InstigatorController->GetTeamNum();
	}
	else if ( Instigator )
	{
		return Instigator->GetTeamNum();
	}
	else
	{
		return 255;
	}
}

