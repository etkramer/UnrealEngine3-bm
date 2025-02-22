/*=============================================================================
	UnScript.cpp: UnrealScript engine support code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.

Description:
	UnrealScript execution and support code.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAudioDeviceClasses.h"
#include "UnNet.h"
#include "UnPhysicalMaterial.h"
#include "DemoRecording.h"
#include "EngineParticleClasses.h"
#include "EngineMaterialClasses.h"
#include "OpCode.h"

extern FParticleDataManager	GParticleDataManager;

//
// Generalize animation retrieval to work for both skeletal meshes (animation sits in Actor->SkelAnim->AnimSeqs) and
// classic meshes (Mesh->AnimSeqs) For backwards compatibility....
//

//
// Initialize execution.
//
void AActor::InitExecution()
{
	UObject::InitExecution();

	checkSlow(GetStateFrame());
	checkSlow(GetStateFrame()->Object==this);
	checkSlow(GWorld!=NULL);
}

/*-----------------------------------------------------------------------------
	Natives.
-----------------------------------------------------------------------------*/

//////////////////////
// Console Commands //
//////////////////////

FString AActor::ConsoleCommand(const FString& Cmd, UBOOL bWriteToLog)
{
	FStringOutputDevice StrOut(TEXT(""));
	FOutputDevice* OutputDevice = bWriteToLog ? (FOutputDevice*)GLog : (FOutputDevice*)&StrOut;

	const INT CmdLen = Cmd.Len();
	TCHAR* CommandBuffer = (TCHAR*)appMalloc((CmdLen+1)*sizeof(TCHAR));
	TCHAR* Line = (TCHAR*)appMalloc((CmdLen+1)*sizeof(TCHAR));

	const TCHAR* Command = CommandBuffer;
	// copy the command into a modifiable buffer
	appStrcpy(CommandBuffer, (CmdLen+1), *Cmd.Left(CmdLen)); 

	// iterate over the line, breaking up on |'s
	while (ParseLine(&Command, Line, CmdLen+1))		// The ParseLine function expects the full array size, including the NULL character.
	{
		// execute each command
		GEngine->Exec(Line, *OutputDevice);
	}

	// Free temp arrays
	appFree(CommandBuffer);
	CommandBuffer=NULL;

	appFree(Line);
	Line=NULL;

	// return the output from all commands, unless we were writing to log
	return bWriteToLog ? TEXT("") : *StrOut;
}

//////////////////////////
// Clientside functions //
//////////////////////////

void APlayerController::ClientTravel(const FString& URL, BYTE TravelType, UBOOL bSeamless, FGuid MapPackageGuid)
{
	// Warn the client.
	eventPreClientTravel(URL, TravelType, bSeamless);

	if (bSeamless && TravelType == TRAVEL_Relative)
	{
		WorldInfo->SeamlessTravel(URL);
	}
	else
	{
		if (bSeamless)
		{
			debugf(NAME_Warning, TEXT("Unable to perform seamless travel because TravelType was %i, not TRAVEL_Relative"), INT(TravelType));
		}
		// Do the travel.
		GEngine->SetClientTravel( *URL, (ETravelType)TravelType );
	}
}

FString APlayerController::GetPlayerNetworkAddress()
{
	if( Player && Player->IsA(UNetConnection::StaticClass()) )
		return Cast<UNetConnection>(Player)->LowLevelGetRemoteAddress();
	else
		return TEXT("");
}

FString APlayerController::GetServerNetworkAddress()
{
	if( GWorld->GetNetDriver() && GWorld->GetNetDriver()->ServerConnection )
	{
		return GWorld->GetNetDriver()->ServerConnection->LowLevelGetRemoteAddress();
	}
	return TEXT("");
}

void APlayerController::CopyToClipboard( const FString& Text )
{
	appClipboardCopy(*Text);
}

FString APlayerController::PasteFromClipboard()
{
	return appClipboardPaste();
}

FString AWorldInfo::GetLocalURL() const
{
	return GWorld->URL.String();
}

UBOOL AWorldInfo::IsDemoBuild() const
{
#if DEMOVERSION
	return TRUE;
#else
	return FALSE;
#endif
}

/** 
 * Returns whether we are running on a console platform or on the PC.
 *
 * @return TRUE if we're on a console, FALSE if we're running on a PC
 */
UBOOL AWorldInfo::IsConsoleBuild(BYTE ConsoleType) const
{
#if CONSOLE
	switch (ConsoleType)
	{
		case CONSOLE_Any:
			return TRUE;
		case CONSOLE_Xbox360:
#if XBOX
			return TRUE;
#else
			return FALSE;
#endif
		case CONSOLE_PS3:
#if PS3
			return TRUE;
#else
			return FALSE;
#endif
		default:
			debugf(NAME_Warning, TEXT("Unknown ConsoleType passed to IsConsoleBuild()"));
			return FALSE;
	}
#else
	return FALSE;
#endif
}

/** Returns whether script is executing within the editor. */
UBOOL AWorldInfo::IsPlayInEditor() const
{
	return GIsPlayInEditorWorld;
}

FString AWorldInfo::GetAddressURL() const
{
	return FString::Printf( TEXT("%s:%i"), *GWorld->URL.Host, GWorld->URL.Port );
}

USequence* AWorldInfo::GetGameSequence() const
{
	return GWorld->GetGameSequence();
}

void AWorldInfo::execAllNavigationPoints(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(ANavigationPoint, OutNav);
	P_FINISH;

	if (NavigationPointList == NULL)
	{
		debugf(NAME_Error, TEXT("(%s:%04X) Cannot use AllNavigationPoints() here - NavigationPointList not set up yet"), *Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0));
		// skip the iterator
		INT wEndOffset = Stack.ReadWord();
		Stack.Code = &Stack.Node->Script(wEndOffset + 1);
	}
	else
	{
		ANavigationPoint* CurrentNav = NavigationPointList;

		// if we have a valid subclass of NavigationPoint
		if (BaseClass && BaseClass != ANavigationPoint::StaticClass())
		{
			PRE_ITERATOR;
				// get the next NavigationPoint in the iteration
				OutNav = NULL;
				while (CurrentNav && OutNav == NULL)
				{
					if (CurrentNav->IsA(BaseClass))
					{
						OutNav = CurrentNav;
					}
					CurrentNav = CurrentNav->nextNavigationPoint;
				}
				if (OutNav == NULL)
				{
					Stack.Code = &Stack.Node->Script(wEndOffset + 1);
					break;
				}
			POST_ITERATOR;
		}
		else
		{
			// do a faster iteration that doesn't check IsA()
			PRE_ITERATOR;
				// get the next NavigationPoint in the iteration
				if (CurrentNav)
				{
					OutNav = CurrentNav;
					CurrentNav = CurrentNav->nextNavigationPoint;
				}
				else
				{
					// we're out of NavigationPoints
					OutNav = NULL;
					Stack.Code = &Stack.Node->Script(wEndOffset + 1);
					break;
				}
			POST_ITERATOR;
		}
 	}
}

void AWorldInfo::execRadiusNavigationPoints(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(ANavigationPoint, OutNav);
	P_GET_VECTOR(Point);
	P_GET_FLOAT(Radius);
	P_FINISH;

	if (NavigationPointList == NULL)
	{
		debugf(NAME_Error, TEXT("(%s:%04X) Cannot use RadiusNavigationPoints() here - navigation octree not set up yet"), *Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0));
		// skip the iterator
		INT wEndOffset = Stack.ReadWord();
		Stack.Code = &Stack.Node->Script(wEndOffset + 1);
	}
	else
	{
		TArray<FNavigationOctreeObject*> Objects;
		GWorld->NavigationOctree->RadiusCheck(Point, Radius, Objects);
		INT CurrentIndex = 0;

		// if we have a valid subclass of NavigationPoint
		if (BaseClass && BaseClass != ANavigationPoint::StaticClass())
		{
			PRE_ITERATOR;
				// get the next NavigationPoint in the iteration
				OutNav = NULL;
				while (CurrentIndex < Objects.Num() && OutNav == NULL)
				{
					ANavigationPoint* CurrentNav = Objects(CurrentIndex)->GetOwner<ANavigationPoint>();
					if (CurrentNav != NULL && CurrentNav->IsA(BaseClass))
					{
						OutNav = CurrentNav;
					}
					CurrentIndex++;
				}
				if (OutNav == NULL)
				{
					Stack.Code = &Stack.Node->Script(wEndOffset + 1);
					break;
				}
			POST_ITERATOR;
		}
		else
		{
			// do a faster iteration that doesn't check IsA()
			PRE_ITERATOR;
				// get the next NavigationPoint in the iteration
				OutNav = NULL;
				while (CurrentIndex < Objects.Num() && OutNav == NULL)
				{
					OutNav = Objects(CurrentIndex)->GetOwner<ANavigationPoint>();
					CurrentIndex++;
				}
				if (OutNav == NULL)
				{
					Stack.Code = &Stack.Node->Script(wEndOffset + 1);
					break;
				}
			POST_ITERATOR;
		}
 	}
}

void AWorldInfo::execNavigationPointCheck(FFrame& Stack, RESULT_DECL)
{
	P_GET_VECTOR(Point);
	P_GET_VECTOR(Extent);
	P_GET_TARRAY_REF(ANavigationPoint*, Navs); // optional
	P_GET_TARRAY_REF(UReachSpec*, Specs); // optional
	P_FINISH;

	if (pNavs == NULL && pSpecs == NULL)
	{
		debugf(NAME_Warning, TEXT("NavigationPointCheck() called without either out array specified from %s"), *Stack.Node->GetName());
	}

	TArray<FNavigationOctreeObject*> Objects;
	GWorld->NavigationOctree->PointCheck(Point, Extent, Objects);

	for (INT i = 0; i < Objects.Num(); i++)
	{
		ANavigationPoint* Nav = Objects(i)->GetOwner<ANavigationPoint>();
		if (Nav != NULL)
		{
			if (pNavs != NULL)
			{
				pNavs->AddItem(Nav);
			}
		}
		else
		{
			UReachSpec* Spec = Objects(i)->GetOwner<UReachSpec>();
			if (Spec != NULL && pSpecs != NULL)
			{
				pSpecs->AddItem(Spec);
			}
		}
	}
}

void AWorldInfo::execAllControllers(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(AController, OutC);
	P_FINISH;

	AController* CurrentC = ControllerList;

	// if we have a valid subclass of NavigationPoint
	if (BaseClass && BaseClass != AController::StaticClass())
	{
		PRE_ITERATOR;
			// get the next Controller in the iteration
			OutC = NULL;
			while (CurrentC && OutC == NULL)
			{
				if (CurrentC->IsA(BaseClass))
				{
					OutC = CurrentC;
				}
				CurrentC = CurrentC->NextController;
			}
			if (OutC == NULL)
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR;
	}
	else
	{
		// do a faster iteration that doesn't check IsA()
		PRE_ITERATOR;
			// get the next Controller in the iteration
			if (CurrentC)
			{
				OutC = CurrentC;
				CurrentC = CurrentC->NextController;
			}
			else
			{
				// we're out of Controllers
				OutC = NULL;
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR;
	}
}

void AWorldInfo::execAllPawns(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(APawn, OutP);
	P_GET_VECTOR_OPTX(TestLocation,FVector(0.f));
	P_GET_FLOAT_OPTX(TestRadius,0.f);
	P_FINISH;

	APawn* CurrentP = PawnList;

	// if we have a valid subclass of NavigationPoint
	if (BaseClass && BaseClass != APawn::StaticClass())
	{
		PRE_ITERATOR;
			// get the next Pawn in the iteration
			OutP = NULL;
			while (CurrentP && OutP == NULL)
			{
				// match the correct class and
				// if radius specified, make sure it's within it
				if (CurrentP->IsA(BaseClass) &&
					(TestRadius == 0.f || (CurrentP->Location - TestLocation).Size() <= TestRadius))
				{
					OutP = CurrentP;
				}
				CurrentP = CurrentP->NextPawn;
			}
			if (OutP == NULL)
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR;
	}
	else
	{
		// do a faster iteration that doesn't check IsA()
		PRE_ITERATOR;
			// get the next Pawn in the iteration
			OutP = NULL;
			while (CurrentP && OutP == NULL)
			{
				// if radius specified, make sure it's within it
				if (TestRadius == 0.f || (CurrentP->Location - TestLocation).Size() <= TestRadius)
				{
					OutP = CurrentP;
				}
				CurrentP = CurrentP->NextPawn;
			}
			if (OutP == NULL)
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR;
	}
}

////////////////////////////////
// Latent function initiators //
////////////////////////////////

void AActor::Sleep(FLOAT Seconds)
{
	GetStateFrame()->LatentAction = EPOLL_Sleep;
	LatentFloat  = Seconds;
}

///////////////////////////
// Slow function pollers //
///////////////////////////

void AActor::execPollSleep( FFrame& Stack, RESULT_DECL )
{
	FLOAT DeltaSeconds = *(FLOAT*)Result;
	if( (LatentFloat-=DeltaSeconds) < 0.5 * DeltaSeconds )
	{
		// Awaken.
		GetStateFrame()->LatentAction = 0;
	}
}
IMPLEMENT_FUNCTION( AActor, EPOLL_Sleep, execPollSleep );

///////////////
// Collision //
///////////////

void AActor::execSetCollision( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX( bNewCollideActors, bCollideActors );
	P_GET_UBOOL_OPTX( bNewBlockActors,  bBlockActors );
	P_GET_UBOOL_OPTX( bNewIgnoreEncroachers,  bIgnoreEncroachers );
	P_FINISH;

	SetCollision( bNewCollideActors, bNewBlockActors, bNewIgnoreEncroachers );

}

void AActor::execSetBase( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AActor,NewBase);
	P_GET_VECTOR_OPTX(NewFloor, FVector(0,0,1) );
	P_GET_OBJECT_OPTX(USkeletalMeshComponent,SkelComp, NULL);
	P_GET_NAME_OPTX(BoneName, NAME_None);
	P_FINISH;

	SetBase( NewBase, NewFloor, 1, SkelComp, BoneName );
}

///////////
// Audio //
///////////

UAudioComponent* AActor::CreateAudioComponent(class USoundCue* SoundCue, UBOOL bPlay, UBOOL bStopWhenOwnerDestroyed, UBOOL bUseLocation, FVector SourceLocation, UBOOL bAttachToSelf)
{
	return UAudioDevice::CreateComponent(SoundCue, GWorld->Scene, bAttachToSelf ? this : NULL, bPlay, bStopWhenOwnerDestroyed, bUseLocation ? &SourceLocation : NULL);
}

//////////////
// Movement //
//////////////

UBOOL AActor::Move(FVector Delta)
{
	FCheckResult Hit(1.0f);
	return GWorld->MoveActor( this, Delta, Rotation, 0, Hit );

}

UBOOL AActor::SetLocation(FVector NewLocation)
{
	return GWorld->FarMoveActor( this, NewLocation );
}

UBOOL AActor::SetRelativeLocation(FVector NewLocation)
{
	UBOOL Result = FALSE;
	if ( Base )
	{
		// Handle skeletal attachment case
		if(BaseSkelComponent)
		{
			RelativeLocation = NewLocation;

			const INT BoneIndex = BaseSkelComponent->MatchRefBone(BaseBoneName);
			if(BoneIndex != INDEX_NONE)
			{
				FMatrix BaseTM = BaseSkelComponent->GetBoneMatrix(BoneIndex);
				BaseTM.RemoveScaling();

				FRotationTranslationMatrix HardRelMatrix(RelativeRotation, RelativeLocation);
				const FMatrix& NewWorldTM = HardRelMatrix * BaseTM;
				NewLocation = NewWorldTM.GetOrigin();

				GWorld->FarMoveActor(this, NewLocation, FALSE, FALSE, TRUE);
			}
			else
			{
				debugf(TEXT("AActor::SetRelativeLocation for %s: BaseBoneName (%s) not found for attached Actor %s!"), *this->GetName(), *BaseBoneName.ToString(), *Base->GetName());
			}
		}
		// Regular 'actor to actor' case
		else
		{
			if(bHardAttach && !bBlockActors)
			{
				// Remember new relative position
				RelativeLocation = NewLocation;

				// Using this new relative transform, calc new wold transform and update current position to match.
				FRotationTranslationMatrix HardRelMatrix( RelativeRotation, RelativeLocation );
				FRotationTranslationMatrix BaseTM( Base->Rotation, Base->Location );
				FMatrix NewWorldTM = HardRelMatrix * BaseTM;

				NewLocation = NewWorldTM.GetOrigin();
				Result = GWorld->FarMoveActor( this, NewLocation,FALSE,FALSE,TRUE );
			}
			else
			{
				NewLocation = Base->Location + FRotationMatrix(Base->Rotation).TransformFVector(NewLocation);
				Result = GWorld->FarMoveActor( this, NewLocation,FALSE,FALSE,TRUE );
				if ( Base )
					RelativeLocation = Location - Base->Location;
			}
		}
	}

	return Result;
}

UBOOL AActor::SetRotation(FRotator NewRotation)
{
	FCheckResult Hit(1.0f);
	return GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
}

UBOOL AActor::SetRelativeRotation(FRotator NewRotation)
{
	if ( Base )
	{
		// Handle skeletal attachment case
		if(BaseSkelComponent)
		{
			RelativeRotation = NewRotation;

			const INT BoneIndex = BaseSkelComponent->MatchRefBone(BaseBoneName);
			if(BoneIndex != INDEX_NONE)
			{
				FMatrix BaseTM = BaseSkelComponent->GetBoneMatrix(BoneIndex);
				BaseTM.RemoveScaling();

				FRotationTranslationMatrix HardRelMatrix(RelativeRotation, RelativeLocation);
				const FMatrix& NewWorldTM = HardRelMatrix * BaseTM;
				NewRotation = NewWorldTM.Rotator();
			}
			else
			{
				debugf(TEXT("AActor::SetRelativeRotation for %s: BaseBoneName (%s) not found for attached Actor %s!"), *this->GetName(), *BaseBoneName.ToString(), *Base->GetName());
			}
		}
		// Regular 'actor to actor' case
		else
		{
			if(bHardAttach && !bBlockActors)
			{
				// We make a new HardRelMatrix using the new rotation and the existing position.
				FRotationTranslationMatrix HardRelMatrix( NewRotation, RelativeLocation );
				RelativeLocation = HardRelMatrix.GetOrigin();
				RelativeRotation = HardRelMatrix.Rotator();

				// Work out what the new child rotation is
				FRotationTranslationMatrix BaseTM( Base->Rotation, Base->Location );
				FMatrix NewWorldTM = HardRelMatrix * BaseTM;
				NewRotation = NewWorldTM.Rotator();
			}
			else
			{
				NewRotation = (FRotationMatrix( NewRotation ) * FRotationMatrix( Base->Rotation )).Rotator();
			}
		}
	}
	FCheckResult Hit(1.0f);
	return GWorld->MoveActor( this, FVector(0,0,0), NewRotation, 0, Hit );
}

void AActor::execSetZone(FFrame& Stack, RESULT_DECL)
{
	P_GET_UBOOL(bForceRefresh);
	P_FINISH;

	SetZone(FALSE, bForceRefresh);
}

//////////////////
// Line tracing //
//////////////////

void AActor::execTrace( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(HitLocation);
	P_GET_VECTOR_REF(HitNormal);
	P_GET_VECTOR(TraceEnd);
	P_GET_VECTOR_OPTX(TraceStart,Location);
	P_GET_UBOOL_OPTX(bTraceActors,bCollideActors);
	P_GET_VECTOR_OPTX(TraceExtent,FVector(0,0,0));
	P_GET_STRUCT_OPTX_REF(FTraceHitInfo,HitInfo,FTraceHitInfo());	// optional
	P_GET_INT_OPTX(ExtraTraceFlags,0);
	P_FINISH;

	// Trace the line.
	FCheckResult Hit(1.0f);
	DWORD TraceFlags;
	if( bTraceActors )
	{
		TraceFlags = (ExtraTraceFlags & UCONST_TRACEFLAG_Blocking) ? TRACE_AllBlocking : TRACE_ProjTargets;
	}
	else
	{
		TraceFlags = TRACE_World;
	}

	if( pHitInfo )
	{
		TraceFlags |= TRACE_Material;
	}
	if( ExtraTraceFlags & UCONST_TRACEFLAG_PhysicsVolumes )
	{
		TraceFlags |= TRACE_PhysicsVolumes;
	}
	if( ExtraTraceFlags & UCONST_TRACEFLAG_Bullet )
	{
		TraceFlags |= TRACE_ComplexCollision;
	}
	if( (ExtraTraceFlags & UCONST_TRACEFLAG_SkipMovers) && (TraceFlags & TRACE_Movers) )
	{
		TraceFlags -= TRACE_Movers;
	}

	AActor* TraceActor = this;
	AController* C = GetAController();
	if ( C && C->Pawn )
	{
		TraceActor = C->Pawn;
	}

	//If enabled, capture the callstack that triggered this script linecheck
	LINE_CHECK_TRACE_SCRIPT(TraceFlags, &Stack);
	GWorld->SingleLineCheck( Hit, TraceActor, TraceEnd, TraceStart, TraceFlags, TraceExtent );

	*(AActor**)Result = Hit.Actor;
	HitLocation      = Hit.Location;
	HitNormal        = Hit.Normal;

	if(pHitInfo)
	{
		HitInfo.PhysMaterial = DetermineCorrectPhysicalMaterial(Hit);

		HitInfo.Material = Hit.Material ? Hit.Material->GetMaterial() : NULL;

		HitInfo.Item = Hit.Item;
		HitInfo.LevelIndex = Hit.LevelIndex;
		HitInfo.BoneName = Hit.BoneName;
		HitInfo.HitComponent = Hit.Component;
	}
}

/** Run a line check against just this PrimitiveComponent. Return TRUE if we hit. */
void AActor::execTraceComponent( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR_REF(HitLocation);
	P_GET_VECTOR_REF(HitNormal);
	P_GET_OBJECT(UPrimitiveComponent, InComponent);
	P_GET_VECTOR(TraceEnd);
	P_GET_VECTOR_OPTX(TraceStart,Location);
	P_GET_VECTOR_OPTX(TraceExtent,FVector(0,0,0));
	P_GET_STRUCT_OPTX_REF(FTraceHitInfo, HitInfo, FTraceHitInfo());
	P_FINISH;

	UBOOL bNoHit = TRUE;
	FCheckResult Hit(1.0f);

	// Ensure the component is valid and attached before checking the line against it.
	// LineCheck needs a transform and IsValidComponent()==TRUE, both of which are implied by IsAttached()==TRUE.
	if( InComponent != NULL && InComponent->IsAttached() )
	{
		bNoHit = InComponent->LineCheck(Hit, TraceEnd, TraceStart, TraceExtent, TRACE_AllBlocking);

		HitLocation      = Hit.Location;
		HitNormal        = Hit.Normal;

		if(pHitInfo)
		{
			HitInfo.PhysMaterial = DetermineCorrectPhysicalMaterial(Hit);

			HitInfo.Material = Hit.Material ? Hit.Material->GetMaterial() : NULL;

			HitInfo.Item = Hit.Item;
			HitInfo.LevelIndex = Hit.LevelIndex;
			HitInfo.BoneName = Hit.BoneName;
			HitInfo.HitComponent = Hit.Component;
		}
	}

	*(DWORD*)Result = !bNoHit;
}

/**
 *	Run a point check against just this PrimitiveComponent. Return TRUE if we hit.
 *  NOTE: the actual Actor we call this on is irrelevant!
 */
void AActor::execPointCheckComponent( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UPrimitiveComponent, InComponent);
	P_GET_VECTOR(PointLocation);
	P_GET_VECTOR(PointExtent);
	P_FINISH;

	UBOOL bNoHit = TRUE;
	FCheckResult Hit(1.0f);

	if(InComponent != NULL && InComponent->IsAttached())
	{
		bNoHit = InComponent->PointCheck(Hit, PointLocation, PointExtent, 0);
	}

	*(DWORD*)Result = !bNoHit;
}

void AActor::execFastTrace( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(TraceEnd);
	P_GET_VECTOR_OPTX(TraceStart,Location);
	P_GET_VECTOR_OPTX(BoxExtent,FVector(0.f,0.f,0.f));
	P_GET_UBOOL_OPTX(bTraceComplex, FALSE);
	P_FINISH;

	DWORD TraceFlags = TRACE_World|TRACE_StopAtAnyHit;
	if( bTraceComplex )
	{
		TraceFlags |= TRACE_ComplexCollision;
	}

	// Trace the line.
	LINE_CHECK_TRACE_SCRIPT(TraceFlags, &Stack);
	FCheckResult Hit(1.f);
	GWorld->SingleLineCheck( Hit, this, TraceEnd, TraceStart, TraceFlags, BoxExtent );

	*(DWORD*)Result = !Hit.Actor;
}

void AActor::execTraceAllPhysicsAssetInteractions( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT( USkeletalMeshComponent, SkelMeshComp );
	P_GET_VECTOR( EndTrace );
	P_GET_VECTOR( StartTrace );
	P_GET_TARRAY_REF( FImpactInfo, out_Hits );
	P_GET_VECTOR_OPTX( Extent, FVector(0.f,0.f,0.f) );
	P_FINISH;

	UBOOL bResult = FALSE;
	out_Hits.Empty();

	if( SkelMeshComp != NULL && SkelMeshComp->PhysicsAsset != NULL )
	{
		FMemMark Mark(GMainThreadMemStack);

		FCheckResult* CheckResult = SkelMeshComp->PhysicsAsset->LineCheckAllInteractions( GMainThreadMemStack, SkelMeshComp, StartTrace, EndTrace, Extent, FALSE );
		for( FCheckResult* TempResult = CheckResult; TempResult != NULL; TempResult = TempResult->GetNext() )
		{
			INT Idx = out_Hits.AddZeroed();

			out_Hits(Idx).HitActor	  = TempResult->Actor;
			out_Hits(Idx).HitLocation = TempResult->Location;
			out_Hits(Idx).HitNormal	  = TempResult->Normal;

			out_Hits(Idx).HitInfo.PhysMaterial	= DetermineCorrectPhysicalMaterial(*TempResult);
			out_Hits(Idx).HitInfo.Material		= TempResult->Material ? TempResult->Material->GetMaterial() : NULL;

			out_Hits(Idx).HitInfo.Item			= TempResult->Item;
			out_Hits(Idx).HitInfo.LevelIndex	= TempResult->LevelIndex;
			out_Hits(Idx).HitInfo.BoneName		= TempResult->BoneName;
			out_Hits(Idx).HitInfo.HitComponent	= TempResult->Component;

			bResult = TRUE;
		}

		Mark.Pop();
	}


	*(DWORD*)Result = bResult;
}

///////////////////////
// Spawn and Destroy //
///////////////////////

#define SHOW_SLOW_SPAWN_CALLS 0
#define SHOW_SLOW_SPAWN_CALLS_TAKING_LONG_TIME_AMOUNT 1.0f // modify this value to look at larger or smaller sets of "bad" actors


void AActor::execSpawn( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,SpawnClass);
	P_GET_OBJECT_OPTX(AActor,SpawnOwner,NULL);
	P_GET_NAME_OPTX(SpawnName,NAME_None);
	P_GET_VECTOR_OPTX(SpawnLocation,Location);
	P_GET_ROTATOR_OPTX(SpawnRotation,Rotation);
	P_GET_OBJECT_OPTX(AActor,ActorTemplate,NULL);
	P_GET_UBOOL_OPTX(bNoCollisionFail,FALSE);
	P_FINISH;

#if SHOW_SLOW_SPAWN_CALLS || LOOKING_FOR_PERF_ISSUES
	 DOUBLE SpawnTime = 0.0f;
	 CLOCK_CYCLES( SpawnTime );
#endif

	 // Spawn and return actor.
	AActor* Spawned = SpawnClass ? GWorld->SpawnActor
	(
		SpawnClass,
		NAME_None,
		SpawnLocation,
		SpawnRotation,
		ActorTemplate,
		bNoCollisionFail,
		0,
		SpawnOwner,
		Instigator
	) : NULL;

	if( Spawned && (SpawnName != NAME_None) )
		Spawned->Tag = SpawnName;
	*(AActor**)Result = Spawned;


#if SHOW_SLOW_SPAWN_CALLS || LOOKING_FOR_PERF_ISSUES
	UNCLOCK_CYCLES( SpawnTime );
	const DOUBLE MSec = ( DOUBLE )SpawnTime * GSecondsPerCycle * 1000.0f;
	if( MSec > SHOW_SLOW_SPAWN_CALLS_TAKING_LONG_TIME_AMOUNT )
	{
		debugf( NAME_PerfWarning, TEXT( "Time: %10f  Spawning: %s  For: %s " ), MSec, *SpawnClass->GetName(), *SpawnOwner->GetName() );
	}
#endif
}

void AActor::execDestroy( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	// scripting support, fire off destroyed event
	//TODO:	
	*(DWORD*)Result = GWorld->DestroyActor( this );
}

////////////
// Timing //
////////////

/**
 * Sets a timer to call the given function at a set
 * interval.  Defaults to calling the 'Timer' event if
 * no function is specified.  If inRate is set to
 * 0.f it will effectively disable the previous timer.
 *
 * NOTE: Functions with parameters are not supported!
 *
 * @param Rate the amount of time to pass between firing
 * @param bLoop whether to keep firing or only fire once
 * @param TimerFunc the name of the function to call when the timer fires
 * @param inObj the object that this timer function should be called on
 */
void AActor::SetTimer( FLOAT Rate,UBOOL bLoop,FName FuncName, UObject* inObj )
{
	if (bStatic)
	{
		debugf(NAME_Error, TEXT("SetTimer() called on bStatic Actor %s"), *GetName());
	}
	else
	{
		if( !inObj ) { inObj = this; }

		// search for an existing timer first
		UBOOL bFoundEntry = 0;
		for (INT Idx = 0; Idx < Timers.Num() && !bFoundEntry; Idx++)
		{
			// If matching function and object
			if( Timers(Idx).FuncName == FuncName &&
				Timers(Idx).TimerObj == inObj )
			{
				bFoundEntry = 1;
				// if given a 0.f rate, disable the timer
				if (Rate == 0.f)
				{
					// timer will be cleared in UpdateTimers
					Timers(Idx).Rate = 0.f;
				}
				// otherwise update with new rate
				else
				{
					Timers(Idx).bLoop = bLoop;
					Timers(Idx).Rate = Rate;
					Timers(Idx).Count = 0.f;
				}
				Timers(Idx).bPaused = FALSE;
			}
		}
		// if no timer was found, add a new one
		if (!bFoundEntry)
		{
#ifdef _DEBUG
			// search for the function and assert that it exists
			UFunction *newFunc = inObj->FindFunctionChecked(FuncName);
			newFunc = NULL;
#endif
			const INT Idx = Timers.AddZeroed();
			Timers(Idx).TimerObj = inObj;
			Timers(Idx).FuncName = FuncName;
			Timers(Idx).bLoop = bLoop;
			Timers(Idx).Rate = Rate;
			Timers(Idx).Count = 0.f;
			Timers(Idx).bPaused = FALSE;
		}
	}
}

/**
 * Clears a previously set timer, identical to calling
 * SetTimer() with a <= 0.f rate.
 *
 * @param FuncName the name of the timer to remove or the default one if not specified
 */
void AActor::ClearTimer( FName FuncName, UObject* inObj )
{
	if( !inObj ) { inObj = this; }

	for (INT Idx = 0; Idx < Timers.Num(); Idx++)
	{
		// If matching function and object
		if( Timers(Idx).FuncName == FuncName &&
			Timers(Idx).TimerObj == inObj )
		{
			// set the rate to 0.f and let UpdateTimers clear it
			Timers(Idx).Rate = 0.f;
		}
	}
}

/**
 *	Pauses/Unpauses a previously set timer
 *
 * @param bPause whether to pause/unpause the timer
 * @param inTimerFunc the name of the timer to pause or the default one if not specified
 * @param inObj object timer is attached to
 */
void AActor::PauseTimer( UBOOL bPause, FName inTimerFunc, UObject* inObj )
{
	if( !inObj ) { inObj = this; }

	for( INT Idx = 0; Idx < Timers.Num(); Idx++ )
	{
		// If matching function and object
		if( Timers(Idx).FuncName == inTimerFunc &&
			Timers(Idx).TimerObj == inObj )
		{
			// Set paused value
			Timers(Idx).bPaused = bPause;
		}
	}
}

/**
 * Returns true if the specified timer is active, defaults
 * to 'Timer' if no function is specified.
 *
 * @param FuncName the name of the timer to remove or the default one if not specified
 */
UBOOL AActor::IsTimerActive( FName FuncName, UObject* inObj )
{
	if( !inObj ) { inObj = this; }

	UBOOL Return = FALSE;
	for (INT Idx = 0; Idx < Timers.Num(); Idx++)
	{
		if( Timers(Idx).FuncName == FuncName &&
			Timers(Idx).TimerObj == inObj )
		{
			Return = (Timers(Idx).Rate > 0.f);
			break;
		}
	}
	return Return;
}

/**
 * Gets the current rate for the specified timer.
 *
 * @note: GetTimerRate('SomeTimer') - GetTimerCount('SomeTimer') is the time remaining before 'SomeTimer' is called
 *
 * @param: TimerFuncName the name of the function to check for a timer for; 'Timer' is the default
 *
 * @return the rate for the given timer, or -1.f if that timer is not active
 */
FLOAT AActor::GetTimerCount( FName FuncName, UObject* inObj )
{
	if( !inObj ) { inObj = this; }

	FLOAT Result = -1.f;
	for (INT Idx = 0; Idx < Timers.Num(); Idx++)
	{
		if( Timers(Idx).FuncName == FuncName &&
			Timers(Idx).TimerObj == inObj )
		{
			Result = Timers(Idx).Count;
			break;
		}
	}
	return Result;
}

FLOAT AActor::GetTimerRate( FName FuncName, UObject* inObj )
{
	if( !inObj ) { inObj = this; }

	for (INT Idx = 0; Idx < Timers.Num(); Idx++)
	{
		if( Timers(Idx).FuncName == FuncName &&
			Timers(Idx).TimerObj == inObj )
		{
			return Timers(Idx).Rate;
		}
	}

	return -1.f;
}

/*-----------------------------------------------------------------------------
	Native iterator functions.
-----------------------------------------------------------------------------*/

void AActor::execAllActors( FFrame& Stack, RESULT_DECL )
{
	// Get the parms.
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FActorIterator It;

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		while( It && OutActor==NULL )
		{
			AActor* TestActor = *It; ++It;
			if(	TestActor && 
                !TestActor->bDeleteMe &&
                TestActor->IsA(BaseClass) )
				OutActor = TestActor;
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execDynamicActors( FFrame& Stack, RESULT_DECL )
{
	// Get the parms.
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;
	
	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FDynamicActorIterator It;

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		while( It && OutActor==NULL )
		{
			AActor* TestActor = *It; ++It;
			if(	TestActor && 
                !TestActor->bDeleteMe &&
                TestActor->IsA(BaseClass) )
				OutActor = TestActor;
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execChildActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FActorIterator It;

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		while( It && OutActor==NULL )
		{
			AActor* TestActor = *It; ++It;
			if(	TestActor && 
                !TestActor->bDeleteMe &&
                TestActor->IsA(BaseClass) && 
                TestActor->IsOwnedBy( this ) )
				OutActor = TestActor;
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execBasedActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	INT iBased=0;

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		for( iBased; iBased<Attached.Num() && OutActor==NULL; iBased++ )
		{
			AActor* TestActor = Attached(iBased);
			if(	TestActor &&
                !TestActor->bDeleteMe &&
                TestActor->IsA(BaseClass))
				OutActor = TestActor;
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execComponentList( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_OBJECT_REF(UObject,OutComp);
	P_FINISH;

	if( !BaseClass )
		return;

	INT iComp = 0;
	PRE_ITERATOR;
		OutComp = NULL;

		// Go through component list and fetch each one
		for( iComp; iComp < Components.Num() && OutComp == NULL; iComp++ )
		{
			UObject* Obj = Components(iComp);
			if(	Obj && Obj->IsA( BaseClass ) )
			{
				OutComp = Obj;
			}
		}

		if( OutComp == NULL )
		{
			Stack.Code = &Stack.Node->Script( wEndOffset + 1 );
			break;
		}
	POST_ITERATOR;
}

void AActor::execAllOwnedComponents( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_OBJECT_REF(UActorComponent,OutComp);
	P_FINISH;

	if( !BaseClass )
		return;

	INT ComponentIndex = 0;
	PRE_ITERATOR;
		OutComp = NULL;

		for(;ComponentIndex < AllComponents.Num();ComponentIndex++)
		{
			UActorComponent* Obj = AllComponents(ComponentIndex);
			if(	Obj && Obj->IsA( BaseClass ) )
			{
				OutComp = Obj;
				break;
			}
		}

		ComponentIndex++;

		if( OutComp == NULL )
		{
			Stack.Code = &Stack.Node->Script( wEndOffset + 1 );
			break;
		}
	POST_ITERATOR;
}

void AActor::execTouchingActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	INT iTouching=0;

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		for( iTouching; iTouching<Touching.Num() && OutActor==NULL; iTouching++ )
		{
			AActor* TestActor = Touching(iTouching);
			if(	TestActor &&
                !TestActor->bDeleteMe &&
                TestActor->IsA(BaseClass) )
			{
				OutActor = TestActor;
			}
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execTraceActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_GET_VECTOR_REF(HitLocation);
	P_GET_VECTOR_REF(HitNormal);
	P_GET_VECTOR(End);
	P_GET_VECTOR_OPTX(Start,Location);
	P_GET_VECTOR_OPTX(TraceExtent,FVector(0,0,0));
	P_GET_STRUCT_OPTX_REF(FTraceHitInfo,HitInfo,FTraceHitInfo());	// optional
	P_GET_INT_OPTX(ExtraTraceFlags,0);
	P_FINISH;

	DWORD TraceFlags = (ExtraTraceFlags & UCONST_TRACEFLAG_Blocking) ? TRACE_AllBlocking : TRACE_AllColliding;

	if( pHitInfo )
	{
		TraceFlags |= TRACE_Material;
	}
	if( ExtraTraceFlags & UCONST_TRACEFLAG_PhysicsVolumes )
	{
		TraceFlags |= TRACE_PhysicsVolumes;
	}
	if( ExtraTraceFlags & UCONST_TRACEFLAG_Bullet )
	{
		TraceFlags |= TRACE_ComplexCollision;
	}
	if( (ExtraTraceFlags & UCONST_TRACEFLAG_SkipMovers) && (TraceFlags & TRACE_Movers) )
	{
		TraceFlags -= TRACE_Movers;
	}

	FMemMark Mark(GMainThreadMemStack);
	BaseClass         = BaseClass ? BaseClass : AActor::StaticClass();
	FCheckResult* Hit = GWorld->MultiLineCheck( GMainThreadMemStack, End, Start, TraceExtent, TraceFlags, this );

	PRE_ITERATOR;
		if (Hit != NULL)
		{
			if ( Hit->Actor && 
                 !Hit->Actor->bDeleteMe &&
                 Hit->Actor->IsA(BaseClass))
			{
				OutActor    = Hit->Actor;
				HitLocation = Hit->Location;
				HitNormal   = Hit->Normal;
				if(pHitInfo)
				{
					HitInfo.PhysMaterial = DetermineCorrectPhysicalMaterial(*Hit);
			
					HitInfo.Material = Hit->Material ? Hit->Material->GetMaterial() : NULL;
			
					HitInfo.Item = Hit->Item;
					HitInfo.LevelIndex = Hit->LevelIndex;
					HitInfo.BoneName = Hit->BoneName;
					HitInfo.HitComponent = Hit->Component;
				}
				Hit = Hit->GetNext();
			}
			else
			{
				Hit = Hit->GetNext();
				continue;
			}
		}
		else
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			OutActor = NULL;
			break;
		}
	POST_ITERATOR;
	Mark.Pop();

}

void AActor::execVisibleActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_GET_FLOAT_OPTX(Radius,0.0f);
	P_GET_VECTOR_OPTX(TraceLocation,Location);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FActorIterator It;
	FCheckResult Hit(1.f);

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		while( It && OutActor==NULL )
		{
			AActor* TestActor = *It; ++It;
			if
			(	TestActor
			&& !TestActor->bHidden
            &&  !TestActor->bDeleteMe
			&&	TestActor->IsA(BaseClass)
			&&	(Radius==0.0f || (TestActor->Location-TraceLocation).SizeSquared() < Square(Radius)) )
			{
				GWorld->SingleLineCheck( Hit, this, TestActor->Location, TraceLocation, TRACE_World|TRACE_StopAtAnyHit );
				if ( !Hit.Actor || (Hit.Actor == TestActor) )
					OutActor = TestActor;
			}
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

void AActor::execVisibleCollidingActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_GET_FLOAT(Radius);
	P_GET_VECTOR_OPTX(TraceLocation,Location);
	P_GET_UBOOL_OPTX(bIgnoreHidden, FALSE);
	P_GET_VECTOR_OPTX(Extent,FVector(0.f));
	P_GET_UBOOL_OPTX(bTraceActors, FALSE);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* Link = GWorld->Hash->ActorRadiusCheck( GMainThreadMemStack, TraceLocation, Radius, TRUE );

	DWORD TraceFlags = TRACE_World;
	if (bTraceActors)
	{
		TraceFlags |= TRACE_ProjTargets;
	}
	
	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		FCheckResult Hit(1.f);

		if ( Link )
		{
			while ( Link )
			{
				if( !Link->Actor ||  
					Link->Actor == this ||
					!Link->Actor->bCollideActors ||	
					Link->Actor->bDeleteMe ||
					!Link->Actor->IsA(BaseClass) ||
					(bIgnoreHidden && Link->Actor->bHidden) )
				{
					Link = Link->GetNext();
				}
				else
				{
					// instead of Actor->Location, we use center of bounding box. It gives better results.
					FBox Box			= Link->Actor->GetComponentsBoundingBox();
					FVector ActorOrigin = Box.GetCenter();
					GWorld->SingleLineCheck( Hit, this, ActorOrigin, TraceLocation, TraceFlags, Extent );
					if( Hit.Actor && (Hit.Actor != Link->Actor))
					{
						Link = Link->GetNext();
					}
					else
					{
						break;
					}
				}
			}

			if ( Link )
			{
				OutActor = Link->Actor;
				Link = Link->GetNext();
			}
		}
		if ( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

	Mark.Pop();
}

void AActor::execCollidingActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_GET_FLOAT(Radius);
	P_GET_VECTOR_OPTX(TraceLocation,Location);
	P_GET_UBOOL_OPTX(bUseOverlapCheck, FALSE);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* Link=GWorld->Hash->ActorRadiusCheck( GMainThreadMemStack, TraceLocation, Radius, bUseOverlapCheck );
	
	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		if ( Link )
		{
			while ( Link )
			{
				if( !Link->Actor ||
					!Link->Actor->bCollideActors ||	
					Link->Actor->bDeleteMe ||
					!Link->Actor->IsA(BaseClass) )
				{
					Link = Link->GetNext();
				}
				else
					break;
			}

			if ( Link )
			{
				OutActor = Link->Actor;
				Link=Link->GetNext();
			}
		}
		if ( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

	Mark.Pop();
}

void AActor::execOverlappingActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_GET_FLOAT(Radius);
	P_GET_VECTOR_OPTX(TraceLocation,Location);
	P_GET_UBOOL_OPTX(bIgnoreHidden, 0);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AActor::StaticClass();
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* Link = GWorld->Hash->ActorOverlapCheck(GMainThreadMemStack, Owner, TraceLocation, Radius);

	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		if ( Link )
		{
			while ( Link )
			{
				if( !Link->Actor
                    || Link->Actor->bDeleteMe
					|| !Link->Actor->IsA(BaseClass)
					|| (bIgnoreHidden && Link->Actor->bHidden) )
				{
					Link = Link->GetNext();
				}
				else
				{
					break;
				}
			}

			if ( Link )
			{
				OutActor = Link->Actor;
				Link=Link->GetNext();
			}
		}
		if ( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

	Mark.Pop();
}

/* execInventoryActors
	Iterator for InventoryManager
	Note: Watch out for Iterator being used to remove Inventory items!
*/
void AInventoryManager::execInventoryActors( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	BaseClass = BaseClass ? BaseClass : AInventory::StaticClass();
	AInventory	*InvItem;
	InvItem = InventoryChain;

	INT InventoryCount = 0;
	PRE_ITERATOR;
		// Fetch next actor in the iteration.
		OutActor = NULL;
		while ( InvItem )
		{	
			// limit inventory checked, since temporary loops in linked list may sometimes be created on network clients while link pointers are being replicated
			InventoryCount++;
			if ( InventoryCount > 100 )
				break;
			if ( InvItem->IsA(BaseClass) )
			{
				OutActor	= InvItem;
				InvItem		= InvItem->Inventory; // Jump to next InvItem in case OutActor is removed from inventory, for next iteration
				break;
			}
			InvItem	= InvItem->Inventory;
		}
		if( OutActor == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
	POST_ITERATOR;

}

/**
 iterator execLocalPlayerControllers()
 returns all locally rendered/controlled player controllers (typically 1 per client, unless split screen)
*/
void AActor::execLocalPlayerControllers( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_ACTOR_REF(OutActor);
	P_FINISH;

	if (BaseClass == NULL)
	{
		debugf(NAME_Error, TEXT("(%s:%04X) Call to LocalPlayerControllers() with BaseClass of None"), *Stack.Node->GetFullName(), Stack.Code - &Stack.Node->Script(0));
		// skip the iterator
		INT wEndOffset = Stack.ReadWord();
		Stack.Code = &Stack.Node->Script(wEndOffset + 1);
	}
	else
	{
		INT iPlayers = 0;

		PRE_ITERATOR;
			// Fetch next actor in the iteration.
			OutActor = NULL;
			for( iPlayers; iPlayers<GEngine->GamePlayers.Num() && OutActor==NULL; iPlayers++ )
			{
				if (GEngine->GamePlayers(iPlayers) && GEngine->GamePlayers(iPlayers)->Actor && GEngine->GamePlayers(iPlayers)->Actor->IsA(BaseClass))
					OutActor = GEngine->GamePlayers(iPlayers)->Actor;
			}
			if( OutActor == NULL )
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR;
	}
}

UBOOL AActor::ContainsPoint( FVector Spot )
{
	UBOOL HaveHit = 0;
	for(UINT ComponentIndex = 0;ComponentIndex < (UINT)this->Components.Num();ComponentIndex++)
	{
		UPrimitiveComponent*	primComp = Cast<UPrimitiveComponent>(this->Components(ComponentIndex));

		if(primComp && primComp->ShouldCollide())
		{
			FCheckResult Hit(0);
			HaveHit = ( primComp->PointCheck( Hit, Spot, FVector(0,0,0), 0 ) == 0 );
			if(HaveHit)
			{
				return 1;
			}
		}
	}	

	return 0;

}

void AActor::execIsOverlapping(FFrame& Stack, RESULT_DECL)
{
	P_GET_ACTOR(A);
	P_FINISH;

	*(UBOOL*)Result = IsOverlapping(A);
}

void AActor::execPlaySound( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USoundCue,InSoundCue);
	P_GET_UBOOL_OPTX(bNotReplicated, FALSE);
	P_GET_UBOOL_OPTX(bNoRepToOwner, FALSE);
	P_GET_UBOOL_OPTX(bStopWhenOwnerDestroyed, FALSE);
	P_GET_VECTOR_OPTX(SoundLocation, Location);
	P_GET_UBOOL_OPTX(bNoRepToRelevant, FALSE);
	P_FINISH;

	if ( InSoundCue == NULL )
	{
		debugf(TEXT("%s PLAYSOUND with no sound cue called from %s"), *GetName(), *Stack.Node->GetName());
		debugf(TEXT("%s"), *Stack.GetStackTrace() );

		return;
	}

	PlaySound(InSoundCue, bNotReplicated, bNoRepToOwner, bStopWhenOwnerDestroyed, &SoundLocation, bNoRepToRelevant);
}

void AActor::PlaySound(USoundCue* InSoundCue, UBOOL bNotReplicated/*=FALSE*/, UBOOL bNoRepToOwner/*=FALSE*/, UBOOL bStopWhenOwnerDestroyed/*=FALSE*/, FVector* pSoundLocation/*=NULL*/, UBOOL bNoRepToRelevant/*=FALSE*/)
{
	if ( InSoundCue == NULL )
	{
		debugf(NAME_Warning, TEXT("%s::PlaySound: NULL sound cue specified!"), *GetName());
		return;
	}

	// @todo demo: do something like 2k4 demo recording with the eventDemoPlaySound with server side recording

	// if no location was specified, use this actor's location
	const FVector& SoundLocation = pSoundLocation
		? *pSoundLocation
		: Location;

	if ( !bNotReplicated && WorldInfo->NetMode != NM_Standalone && GWorld->GetNetDriver() != NULL)
	{
		UNetDriver* NetDriver = GWorld->GetNetDriver();
		// replicate sound
		for( INT iPlayers=0; iPlayers < NetDriver->ClientConnections.Num(); iPlayers++ )
		{
			if ( NetDriver->ClientConnections(iPlayers) )
			{
				APlayerController *NextPlayer = NetDriver->ClientConnections(iPlayers)->Actor;
				if ( bNoRepToOwner && NextPlayer && (GetTopPlayerController() == NextPlayer) )
				{
					NextPlayer = NULL;
					bNoRepToOwner = false; // found the owner, so can stop looking
				}
				if ( NextPlayer )
				{
					if (bNoRepToRelevant)
					{
						UNetConnection* Connection = Cast<UNetConnection>(NextPlayer->Player);
						if (Connection != NULL && Connection->ActorChannels.Find(this) != NULL)
						{
							// don't replicate to this player because this Actor is relevant to it
							NextPlayer = NULL;
						}
					}
					if (NextPlayer != NULL)
					{
						NextPlayer->HearSound(InSoundCue, this, SoundLocation, bStopWhenOwnerDestroyed);
					}
				}
			}
		}
	}

	if ( GWorld->GetNetMode() != NM_DedicatedServer )
	{
		// Play sound locally
		for( INT iPlayers=0; iPlayers<GEngine->GamePlayers.Num(); iPlayers++ )
		{
			if ( GEngine->GamePlayers(iPlayers) )
			{
				APlayerController *NextListener = GEngine->GamePlayers(iPlayers)->Actor;
				if( NextListener && NextListener->LocalPlayerController() )
				{
					// break once someone plays the sound ( so we don't double up on splitscreen )
					if(NextListener->HearSound(InSoundCue, this, SoundLocation, bStopWhenOwnerDestroyed))
					{
						//debugf(TEXT("%s played %s"),*NextListener->GetName(),*InSoundCue->GetPathName());
						break;
					}
				}
			}
		}
	}
}

/** checks whether the passed in SoundPlayer is valid for replicating in a HearSound() call and sets it to NULL if not */
void APlayerController::ValidateSoundPlayer(AActor*& SoundPlayer)
{
	if (SoundPlayer != NULL)
	{
		UNetConnection* Connection = Cast<UNetConnection>(Player);
		if (Connection != NULL && !Connection->PackageMap->CanSerializeObject(SoundPlayer))
		{
			SoundPlayer = NULL;
		}
	}
}

/** 
 * HearSound()
 * If sound is audible, calls eventClientHearSound() so local or remote player will hear it. Listener position is either the Pawn
 * or the Pawn's ViewTarget.
 *
 * @param USoundCue		Sound cue to play
 * @param AActor*		Actor that owns the sound
 * @param SoundLocation	Location of the sound. Most of the time, this is the same as AActor->Location
 * @param UBOOL			Stop when owner is destroyed
*/
UBOOL APlayerController::HearSound( USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed )
{
	INT bIsOccluded = 0;
	if( SoundPlayer == this || InSoundCue->IsAudible( SoundLocation, ( ViewTarget != NULL ) ? ViewTarget->Location : Location, SoundPlayer, bIsOccluded, bCheckSoundOcclusion ) )
	{
		// don't pass SoundLocation if it is the same as the Actor's Location and that Actor exists on the client for this player
		// as in that case we want to attach the sound to the Actor itself
		ValidateSoundPlayer(SoundPlayer);
		eventClientHearSound( InSoundCue, SoundPlayer, (SoundPlayer != NULL && SoundPlayer->Location == SoundLocation) ? FVector(0.f) : SoundLocation, bStopWhenOwnerDestroyed, bIsOccluded );
		return TRUE;
	}
	return FALSE;
}

/** get an audio component from the HearSound pool
 * creates a new component if the pool is empty and MaxConcurrentHearSounds has not been exceeded
 * the component is initialized with the values passed in, ready to call Play() on
 * its OnAudioFinished delegate is set to this PC's HearSoundFinished() function
 * @param ASound - the sound to play
 * @param SourceActor - the Actor to attach the sound to (if None, attached to self)
 * @param bStopWhenOwnerDestroyed - whether the sound stops if SourceActor is destroyed
 * @param bUseLocation (optional) - whether to use the SourceLocation parameter for the sound's location (otherwise, SourceActor's location)
 * @param SourceLocation (optional) - if bUseLocation, the location for the sound
 * @return the AudioComponent that was found/created
 */
UAudioComponent* APlayerController::GetPooledAudioComponent(USoundCue* ASound, AActor* SourceActor, UBOOL bStopWhenOwnerDestroyed, UBOOL bUseLocation, FVector SourceLocation)
{
	UAudioComponent* Result = NULL;

	// try to grab one from the pool
	while (HearSoundPoolComponents.Num() > 0)
	{
		INT i = HearSoundPoolComponents.Num() - 1;
		Result = HearSoundPoolComponents(i);
		HearSoundPoolComponents.Remove(i, 1);
		if (Result != NULL && !Result->IsPendingKill())
		{
			break;
		}
		else
		{
			Result = NULL;
		}
	}

	if (Result == NULL)
	{
		// clear out old entries
		INT i = 0;
		while (i < HearSoundActiveComponents.Num())
		{
			if (HearSoundActiveComponents(i) != NULL && !HearSoundActiveComponents(i)->IsPendingKill())
			{
				i++;
			}
			else
			{
				HearSoundActiveComponents.Remove(i, 1);
			}
		}

		if (MaxConcurrentHearSounds > 0 && HearSoundActiveComponents.Num() >= MaxConcurrentHearSounds)
		{
			if (bLogHearSoundOverflow)
			{
				debugf(NAME_Warning, TEXT("Exceeded max concurrent active HearSound() sounds! Sound list:"));
				for (i = 0; i < MaxConcurrentHearSounds; i++)
				{
					debugf(*HearSoundActiveComponents(i)->SoundCue->GetPathName());
				}
			}
			// overwrite oldest sound
			Result = HearSoundActiveComponents(0);
			OBJ_SET_DELEGATE(Result, OnAudioFinished, NULL, NAME_None); // so HearSoundFinished() doesn't get called and mess with the arrays
			Result->ResetToDefaults();
			HearSoundActiveComponents.Remove(0, 1);
		}
		else
		{
			Result = CreateAudioComponent(ASound, FALSE, FALSE, FALSE, FVector(), FALSE);
			if (Result == NULL)
			{
				// sound is disabled
				return NULL;
			}
		}
	}

	Result->SoundCue = ASound;
	Result->bStopWhenOwnerDestroyed = bStopWhenOwnerDestroyed;
	if (SourceActor != NULL && !SourceActor->IsPendingKill())
	{
		Result->bUseOwnerLocation = !bUseLocation;
		Result->Location = SourceLocation;
		SourceActor->eventModifyHearSoundComponent(Result);
		SourceActor->AttachComponent(Result);
	}
	else
	{
		Result->bUseOwnerLocation = FALSE;
		if (bUseLocation)
		{
			Result->Location = SourceLocation;
		}
		else if (SourceActor != NULL)
		{
			Result->Location = SourceActor->Location;
		}
		AttachComponent(Result);
	}
	HearSoundActiveComponents.AddItem(Result);
	OBJ_SET_DELEGATE(Result, OnAudioFinished, this, NAME_HearSoundFinished);
	return Result;
}

/** internal - detaches the given PSC and returns it to the pool */
void AEmitterPool::ReturnToPool(UParticleSystemComponent* PSC)
{
	// if the component is already pending kill then we can't put it back in the pool
	if (PSC != NULL && !PSC->IsPendingKill())
	{
		GParticleDataManager.RemoveParticleSystemComponent(PSC);
		FreeStaticMeshComponents(PSC);
		PSC->DetachFromAny();
		OBJ_SET_DELEGATE(PSC, OnSystemFinished, NULL, NAME_None);
		PoolComponents.AddItem(PSC);
	}
}

/** internal - moves the SMComponents from given PSC to the pool free list */
void AEmitterPool::FreeStaticMeshComponents(class UParticleSystemComponent* PSC)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticlePoolTime);

	for (INT SMIndex = 0; SMIndex < PSC->SMComponents.Num(); SMIndex++)
	{
		UStaticMeshComponent* SMC = PSC->SMComponents(SMIndex);
		// Make sure it is not NULL and this pool created it...
		if ((SMC != NULL) && (SMC->GetOuter() == this))
		{
			// Make sure it has not been tagged for being destroyed
			if (!(SMC->HasAnyFlags(RF_Unreachable) || SMC->IsPendingKill()))
			{
				FreeMaterialInstanceConstants(SMC);
				SMC->Materials.Empty();
				FreeSMComponents.AddItem(SMC);
			}
		}
		PSC->SMComponents(SMIndex) = NULL;
	}
	PSC->SMComponents.Empty();
}

/** 
*	internal - retrieves a SMComponent from the pool free list 
*
*	@param	bCreateNewObject	If TRUE, create an SMC w/ the pool as its outer
*
*	@return	StaticMeshComponent	The SMC, NULL if none was available/created
*/
UStaticMeshComponent* AEmitterPool::GetFreeStaticMeshComponent(UBOOL bCreateNewObject)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticlePoolTime);

	UStaticMeshComponent* Result = NULL;
	while (FreeSMComponents.Num() > 0)
	{
		Result = FreeSMComponents.Pop();
		// Make sure it is not NULL and this pool created it...
		if ((Result != NULL) && (Result->GetOuter() == this))
		{
			// Make sure it has not been tagged for being destroyed
			if (!(Result->HasAnyFlags(RF_Unreachable) || Result->IsPendingKill()))
			{
				break;
			}
			else
			{
				Result = NULL;
			}
		}
		else
		{
			Result = NULL;
		}
	}

	if ((Result == NULL) && (bCreateNewObject == TRUE))
	{
		Result = ConstructObject<UStaticMeshComponent>(UStaticMeshComponent::StaticClass(), this);
		Result->bAcceptsStaticDecals = FALSE;
		Result->bAcceptsDynamicDecals = FALSE;
		Result->CollideActors		= FALSE;
		Result->BlockActors			= FALSE;
		Result->BlockZeroExtent		= FALSE;
		Result->BlockNonZeroExtent	= FALSE;
		Result->BlockRigidBody		= FALSE;
	}

	return Result;
}

/** internal - moves the MIConstants from given SMComponent to the pool free list */
void AEmitterPool::FreeMaterialInstanceConstants(UStaticMeshComponent* SMC)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticlePoolTime);

	for (INT MICIndex = 0; MICIndex < SMC->Materials.Num(); MICIndex++)
	{
		UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(SMC->Materials(MICIndex));
		// Make sure it is not NULL and this pool created it...
		if ((MIC != NULL) && (MIC->GetOuter() == this))
		{
			// Make sure it has not been tagged for being destroyed
			if (!(MIC->HasAnyFlags(RF_Unreachable) || MIC->IsPendingKill()))
			{
				FreeMatInstConsts.AddItem(MIC);
			}
		}
		SMC->Materials(MICIndex) = NULL;
	}
	SMC->Materials.Empty();
}

/** 
*	internal - retrieves a MaterialInstanceConstant from the pool free list 
*
*	@param	bCreateNewObject			If TRUE, create an MIC w/ the pool as its outer
*
*	@return	MaterialInstanceConstant	The MIC, NULL if none was available/created
*/
UMaterialInstanceConstant* AEmitterPool::GetFreeMatInstConsts(UBOOL bCreateNewObject)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticlePoolTime);

	UMaterialInstanceConstant* Result = NULL;
	while (FreeMatInstConsts.Num() > 0)
	{
		Result = FreeMatInstConsts.Pop();
		// Make sure it is not NULL and this pool created it...
		if ((Result != NULL) && (Result->GetOuter() == this))
		{
			// Make sure it has not been tagged for being destroyed
			if (!(Result->HasAnyFlags(RF_Unreachable) || Result->IsPendingKill()))
			{
				break;
			}
			else
			{
				Result = NULL;
			}
		}
		else
		{
			Result = NULL;
		}
	}

	if ((Result == NULL) && (bCreateNewObject == TRUE))
	{
		Result = ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass(), this);
	}

	return Result;
}

DECLARE_CYCLE_STAT(TEXT("Particle Pool Time"),STAT_ParticlePoolTime,STATGROUP_Particles);

/** internal - helper for spawning functions
 * gets a component from the appropriate pool array (checks PerEmitterPools)
 * includes creating a new one if necessary as well as taking one from the active list if the max number active has been exceeded
 * @return the ParticleSystemComponent to use
 */
UParticleSystemComponent* AEmitterPool::GetPooledComponent(UParticleSystem* EmitterTemplate)
{
	SCOPE_CYCLE_COUNTER(STAT_ParticlePoolTime);

	UParticleSystemComponent* Result = NULL;

	//@todo: have some hashing or something to try to find pooled component of same template as that passed in
	while (PoolComponents.Num() > 0)
	{
		INT i = PoolComponents.Num() - 1;
		Result = PoolComponents(i);
		PoolComponents.Remove(i, 1);
		if (Result != NULL && !Result->IsPendingKill())
		{
			break;
		}
		else
		{
			Result = NULL;
		}
	}

	if (Result == NULL)
	{
		// clear out old entries
		INT i = 0;
		while (i < ActiveComponents.Num())
		{
			if (ActiveComponents(i) != NULL && !ActiveComponents(i)->IsPendingKill())
			{
				i++;
			}
			else
			{
				ActiveComponents.Remove(i, 1);
			}
		}

		if (MaxActiveEffects > 0 && ActiveComponents.Num() >= MaxActiveEffects)
		{
			if (bLogPoolOverflow)
			{
				debugf(NAME_Warning, TEXT("Exceeded max active pooled emitters!"));
				if (bLogPoolOverflowList)
				{
					debugf(TEXT("Effect list:"));
					for (INT i = 0; i < MaxActiveEffects; i++)
					{
						debugf(TEXT("%s"), *ActiveComponents(i)->Template->GetPathName());
					}
				}
			}
			// overwrite oldest emitter
			Result = ActiveComponents(0);

			// Deactivate it, and make sure to remove it from the data manager!
			Result->DeactivateSystem();
			GParticleDataManager.RemoveParticleSystemComponent(Result);

			// pretend the system finished if it's a custom lifetime emitter, so the caller always gets the callback
			if (OBJ_DELEGATE_IS_SET(Result, OnSystemFinished) && Result->__OnSystemFinished__Delegate.Object != NULL && Result->__OnSystemFinished__Delegate.Object != this)
			{
				Result->delegateOnSystemFinished(Result);
			}
			OBJ_SET_DELEGATE(Result, OnSystemFinished, NULL, NAME_None); // so OnParticleSystemFinished() doesn't get called and mess with the arrays
			ActiveComponents.Remove(0, 1);
		}
		else if (PSCTemplate == NULL)
		{
			debugf(NAME_Error, TEXT("%s is missing its PSCTemplate so it cannot spawn emitters!"), *GetName());
			return NULL;
		}
		else
		{
			Result = ConstructObject<UParticleSystemComponent>(PSCTemplate->GetClass(), this, NAME_None, 0, PSCTemplate, INVALID_OBJECT);
		}
	}

	if (Result->Template == EmitterTemplate)
	{
		// Rewind the emitter instances (to reset the emitter times)
		// 'Kill' the particles (sets the ActiveParticle value to 0)
		Result->RewindEmitterInstances();
		Result->KillParticlesForced();
		Result->bJustAttached = TRUE;
		// Re-activate the system
		Result->ActivateSystem();
	}
	else
	{
		Result->ResetToDefaults();
		Result->SetTemplate(EmitterTemplate);
	}
	ActiveComponents.AddItem(Result);

#define TRACK_LOOPING_POOLED_EMITTERS 0
#if TRACK_LOOPING_POOLED_EMITTERS
	if( EmitterTemplate )
	{
		UParticleSystem* ParticleSystem = EmitterTemplate;

		UBOOL bIsInfinitelyLooping = FALSE;
		// Loop over all emitters and LOD levels, looking for 0 EmitterLoops.
		for( INT EmitterIndex=0; EmitterIndex<ParticleSystem->Emitters.Num(); EmitterIndex++ )
		{
			UParticleEmitter* ParticleEmitter = ParticleSystem->Emitters(EmitterIndex);
			if( ParticleEmitter )
			{
				for( INT LODIndex=0; LODIndex < ParticleEmitter->LODLevels.Num(); LODIndex++ )
				{
					UParticleLODLevel* ParticleLODLevel = ParticleEmitter->LODLevels(LODIndex);
					if( ParticleLODLevel && ParticleLODLevel->RequiredModule && ParticleLODLevel->RequiredModule->EmitterLoops == 0 )
					{
						bIsInfinitelyLooping = TRUE;
					}
				}					
			}
		}
		// Warn about infinitely looping pooled particles.
		if( bIsInfinitelyLooping )
		{
			debugf(NAME_Warning,TEXT("Pool request for infinitely looping %s"),*ParticleSystem->GetFullName());
		}
	}
#endif

	return Result;
}

/*-----------------------------------------------------------------------------
	Script processing function.
-----------------------------------------------------------------------------*/

//
// Execute the state code of the actor.
//
void AActor::ProcessState( FLOAT DeltaSeconds )
{
	if
	(	GetStateFrame()
	&&	GetStateFrame()->Code
	&&	(Role>=ROLE_Authority || (GetStateFrame()->StateNode->StateFlags & STATE_Simulated))
	&&	!ActorIsPendingKill() )
	{
		// If a latent action is in progress, update it.
		if (GetStateFrame()->LatentAction != 0)
		{
			(this->*GNatives[GetStateFrame()->LatentAction])(*GetStateFrame(), (BYTE*)&DeltaSeconds);
		}

		if (GetStateFrame()->LatentAction == 0)
		{
			// Execute code.
			INT NumStates = 0;
			BYTE Buffer[MAX_SIMPLE_RETURN_VALUE_SIZE];
			// create a copy of the state frame to execute state code from so that if the state is changed from within the code, the executing frame's code pointer isn't modified while it's being used
			FStateFrame ExecStateFrame(*GetStateFrame());
			while (!bDeleteMe && ExecStateFrame.Code != NULL && GetStateFrame()->LatentAction == 0)
			{
				// if we are continuing interrupted state code, we need to manually push the frame onto the script debugger's stack
				if (GetStateFrame()->bContinuedState)
				{
#if !FINAL_RELEASE
					if (GDebugger != NULL)
					{
						GDebugger->DebugInfo(this, &ExecStateFrame, DI_NewStack, 0, 0);
					}
#endif
					GetStateFrame()->bContinuedState = FALSE;
				}
				// remember old starting point (+1 for the about-to-be-executed byte so we can detect a state/label jump back to the same byte we're at now)
				BYTE* OldCode = ++GetStateFrame()->Code;

				ExecStateFrame.Step( this, Buffer ); 
				// if a state was pushed onto the stack, we need to correct the originally executing state's code pointer to reflect the code *after* the last state command was executed
				if (GetStateFrame()->StateStack.Num() > ExecStateFrame.StateStack.Num())
				{
					GetStateFrame()->StateStack(ExecStateFrame.StateStack.Num()).Code = ExecStateFrame.Code;
				}
				// if the state frame's code pointer was directly modified by a state or label change, we need to update our copy
				if (GetStateFrame()->Node != ExecStateFrame.Node)
				{
					// we have changed states
					if( ++NumStates > 4 )
					{
						//debugf(TEXT("%s pause going from state %s to %s"), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						// shouldn't do any copying as the StateFrame was modified for the new state/label
						break;
					}
					else
					{
						//debugf(TEXT("%s went from state %s to %s"), *GetName(), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						ExecStateFrame = *GetStateFrame();
					}
				}
				else if (GetStateFrame()->Code != OldCode)
				{
					// transitioned to a new label
					//debugf(TEXT("%s went to new label in state %s"), *GetName(), *GetStateFrame()->StateNode->GetName());
					ExecStateFrame = *GetStateFrame();
				}
				else
				{
					// otherwise, copy the new code pointer back to the original state frame
					GetStateFrame()->Code = ExecStateFrame.Code;
				}
			}

#if !FINAL_RELEASE
			// notify the debugger if state code ended prematurely due to this Actor being destroyed
			if (bDeleteMe && GDebugger != NULL)
			{
				GDebugger->DebugInfo(this, &ExecStateFrame, DI_PrevStackState, 0, 0);
			}
#endif
		}
	}
}

//
// Internal RPC calling.
//
static inline void InternalProcessRemoteFunction
(
	AActor*			Actor,
	UNetConnection*	Connection,
	UFunction*		Function,
	void*			Parms,
	FFrame*			Stack,
	UBOOL			IsServer
)
{
//#if WITH_UE3_NETWORKING
#if 1
	// Route RPC calls to actual connection
	if (Connection->GetUChildConnection())
	{
		Connection = ((UChildConnection*)Connection)->Parent;
	}

	// Make sure this function exists for both parties.
	FClassNetCache* ClassCache = Connection->PackageMap->GetClassNetCache( Actor->GetClass() );
	if( !ClassCache )
		return;
	FFieldNetCache* FieldCache = ClassCache->GetFromField( Function );
	if( !FieldCache )
		return;

	// Get the actor channel.
	UActorChannel* Ch = Connection->ActorChannels.FindRef(Actor);
	if( !Ch )
	{
		if( IsServer )
		{
			if (Connection->ClientWorldPackageName == GWorld->GetOutermost()->GetFName() && Connection->ClientHasInitializedLevelFor(Actor))
			{
				Ch = (UActorChannel *)Connection->CreateChannel( CHTYPE_Actor, 1 );
			}
			else
			{
				debugf(NAME_DevNet, TEXT("Error: Can't send function '%s' on '%s': Client hasn't loaded the level for this Actor"), *Function->GetName(), *Actor->GetName());
			}
		}
		if( !Ch )
			return;
		if( IsServer )
			Ch->SetChannelActor( Actor );
	}

	// Make sure initial channel-opening replication has taken place.
	if( Ch->OpenPacketId==INDEX_NONE )
	{
		if( !IsServer )
			return;

		// triggering replication of an Actor while already in the middle of replication can result in invalid data being sent and is therefore illegal
		if (Ch->bIsReplicatingActor)
		{
			FString Error(FString::Printf(TEXT("Attempt to replicate function '%s' on Actor '%s' while it is in the middle of variable replication!"), *Function->GetName(), *Actor->GetName()));
			debugf(NAME_Error, *Error);
			appErrorfSlow(*Error);
			return;
		}
		Ch->ReplicateActor();
	}

#if WITH_UE3_NETWORKING
	// Form the RPC preamble.
	FOutBunch Bunch( Ch, 0 );
	//debugf(TEXT("   Call %s"),Function->GetFullName());
	Bunch.WriteInt( FieldCache->FieldNetIndex, ClassCache->GetMaxIndex() );
#endif	//#if WITH_UE3_NETWORKING

	// Form the RPC parameters.
	if( Stack )
	{
		// this only happens for native replicated functions called from script
		// because in that case, the C++ function itself handles evaluating the parameters
		// so we cannot do that before calling ProcessRemoteFunction() as we do with all other cases
		appMemzero( Parms, Function->ParmsSize );

		// if this function has optional parameters, we'll need to process the default value opcodes
		FFrame* NewFunctionStack=NULL;
		if ( Function->HasAnyFunctionFlags(FUNC_HasOptionalParms) )
		{
			NewFunctionStack = new FFrame( Actor, Function, 0, Parms, Stack );
		}

		for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
		{
			// reset the runtime flag that indicates whether the user specified a value for this parm
			GRuntimeUCFlags &= ~RUC_SkippedOptionalParm;

			BYTE* CurrentPropAddr = (BYTE*)Parms + It->Offset;
			if ( Cast<UBoolProperty>(*It,CLASS_IsAUBoolProperty) && It->ArrayDim == 1 )
			{
				// we're going to get '1' returned for bools that are set, so we need to manually mask it in to the proper place
				UBOOL bValue = FALSE;
				Stack->Step(Stack->Object, &bValue);
				if (bValue)
				{
					*(BITFIELD*)(CurrentPropAddr) |= ((UBoolProperty*)*It)->BitMask;
				}
			}
			else
			{
				Stack->Step(Stack->Object, CurrentPropAddr);
			}

			// if this is an optional parameter, evaluate (or skip) the default value of the parameter
			if ( It->HasAnyPropertyFlags(CPF_OptionalParm) )
			{
				checkSlow(NewFunctionStack);
				// if this is a struct property and no value was passed into the function, initialize the property value with the defaults from the struct
				if ( (GRuntimeUCFlags&RUC_SkippedOptionalParm) != 0 )
				{
					UStructProperty* StructProp = Cast<UStructProperty>(*It, CLASS_IsAUStructProperty);
					if ( StructProp != NULL )
					{
						StructProp->InitializeValue( CurrentPropAddr );
					}
				}

				// now evaluate the default value of the optional parameter (which will be stored in the new function's script
				NewFunctionStack->Step(Actor, CurrentPropAddr);
			}
		}
		checkSlow(*Stack->Code==EX_EndFunctionParms);
		delete NewFunctionStack;
	}
#if WITH_UE3_NETWORKING
	// verify we haven't overflowed unacked bunch buffer
	//@warning: needs to be after parameter evaluation for script stack integrity
	if (Bunch.IsError())
	{
		debugf(NAME_DevNet, TEXT("Error: Can't send function '%s' on '%s': Reliable buffer overflow"), *Function->GetName(), *Actor->GetName());
		appErrorfSlow(TEXT("Can't send function '%s' on '%s': Reliable buffer overflow"), *Function->GetName(), *Actor->GetName());
		return;
	}

	for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
	{
		if( Connection->PackageMap->SupportsObject(*It) )
		{
			UBOOL Send = 1;
			if( !Cast<UBoolProperty>(*It,CLASS_IsAUBoolProperty) )
			{
				// check for a complete match, including arrays
				// (we're comparing against zero data here, since 
				// that's the default.)
				Send = 0;

				for (INT i = 0; i < It->ArrayDim; i++)
				{
					if (!It->Matches(Parms, NULL, i))
					{
						Send = 1;
						break;
					}
				}
				Bunch.WriteBit(Send);
			}
			if (Send)
			{
				for (INT i = 0; i < It->ArrayDim; i++)
				{
					It->NetSerializeItem(Bunch, Connection->PackageMap, (BYTE*)Parms + It->Offset + (i * It->ElementSize));
				}
			}
		}
	}

	// Reliability.
	//warning: RPC's might overflow, preventing reliable functions from getting thorough.
	if( Function->FunctionFlags & FUNC_NetReliable )
		Bunch.bReliable = 1;

	// Send the bunch.
	if (Bunch.IsError())
	{
		debugf(NAME_DevNet, TEXT("Error: Can't send function '%s' on '%s': RPC bunch overflowed (too much data in parameters?)"), *Function->GetName(), *Actor->GetName());
		appErrorfSlow(TEXT("Error: Can't send function '%s' on '%s': RPC bunch overflowed (too much data in parameters?)"), *Function->GetName(), *Actor->GetName());
	}
	else if (Ch->Closing)
	{
		debugfSuppressed( NAME_DevNetTraffic, TEXT("RPC bunch on closing channel") );
	}
	else
	{
		debugfSuppressed( NAME_DevNetTraffic, TEXT("      Sent RPC: %s::%s"), *Actor->GetName(), *Function->GetName() );
		Ch->SendBunch( &Bunch, 1 );
	}
#endif	//#if WITH_UE3_NETWORKING
#else	//#if WITH_UE3_NETWORKING
warnf(TEXT("InternalProcessRemoteFunction> Networking disabled..."));
#endif	//#if WITH_UE3_NETWORKING
}

//
// Return whether a function should be executed remotely.
//
UBOOL AActor::ProcessRemoteFunction( UFunction* Function, void* Parms, FFrame* Stack )
{
	// Quick reject 1.
	if( (Function->FunctionFlags & FUNC_Static) || ActorIsPendingKill() )
	{
		return FALSE;
	}

	UBOOL Absorb = (Role<=ROLE_SimulatedProxy) && !(Function->FunctionFlags & (FUNC_Simulated | FUNC_Native));
	
	// we should only be calling script on actors inside valid, visible levels
	checkSlow(WorldInfo != NULL);

	// let the demo record system have a go
	if( GWorld->DemoRecDriver )
	{
		if( GWorld->DemoRecDriver->ServerConnection )
		{
			return Absorb;
		}
		ProcessDemoRecFunction( Function, Parms, Stack );
		// absorb client replicated functions on the demo spectator 
		if (Function->FunctionFlags & FUNC_NetClient)
		{
			APlayerController* Top = GetTopPlayerController();
			if ( Top != NULL && Top->bDemoOwner && GWorld->DemoRecDriver->ClientConnections.Num() > 0 &&
				GWorld->DemoRecDriver->ClientConnections(0) != NULL && GWorld->DemoRecDriver->ClientConnections(0)->Actor == Top )
			{
				return TRUE;
			}
		}
	}

	// Quick reject 2.
	if( WorldInfo->NetMode == NM_Standalone )
	{
		return FALSE;
	}
	if (!(Function->FunctionFlags & FUNC_Net) || GWorld->GetNetDriver() == NULL)
	{
		return Absorb;
	}

//#if WITH_UE3_NETWORKING
#if 1
	// Check if the actor can potentially call remote functions.
    APlayerController* Top = GetTopPlayerController();
	UNetConnection* ClientConnection = NULL;
	if (Top == NULL || (Role == ROLE_Authority && (ClientConnection = Cast<UNetConnection>(Top->Player)) == NULL) )
	{
		return Absorb;
	}

	// Route RPC calls to actual connection
	if (ClientConnection != NULL && ClientConnection->GetUChildConnection() != NULL)
	{
		ClientConnection = ((UChildConnection*)ClientConnection)->Parent;
	}

	// Get the connection.
	UBOOL           IsServer   = WorldInfo->NetMode == NM_DedicatedServer || WorldInfo->NetMode == NM_ListenServer;
	UNetConnection* Connection = IsServer ? ClientConnection : GWorld->GetNetDriver()->ServerConnection;
	if ( Connection == NULL )
	{
		return TRUE;
	}

	// get the top most function
	while (Function->GetSuperFunction() != NULL)
	{
		Function = Function->GetSuperFunction();
	}

	// if we are the server, and it's not a send-to-client function,
	if (IsServer && !(Function->FunctionFlags & FUNC_NetClient))
	{
		// don't replicate
		return Absorb;
	}
	// if we aren't the server, and it's not a send-to-server function,
	if (!IsServer && !(Function->FunctionFlags & FUNC_NetServer))
	{
		// don't replicate
		return Absorb;
	}

	// If saturated and function is unimportant, skip it.
	if( !(Function->FunctionFlags & FUNC_NetReliable) && !Connection->IsNetReady(0) )
		return TRUE;
#else	//#if WITH_UE3_NETWORKING
	UBOOL           IsServer   = FALSE;
	UNetConnection* Connection = NULL;
#endif	//#if WITH_UE3_NETWORKING

	// Send function data to remote.
	InternalProcessRemoteFunction( this, Connection, Function, Parms, Stack, IsServer );
	return TRUE;
}

// Replicate a function call to a demo recording file
void AActor::ProcessDemoRecFunction( UFunction* Function, void* Parms, FFrame* Stack )
{
	// Check if the function is replicatable
	if( (Function->FunctionFlags & (FUNC_Static|FUNC_Net))!=FUNC_Net || bNetTemporary )
	{
		return;
	}

#if CLIENT_DEMO
	UBOOL IsNetClient = (WorldInfo->NetMode == NM_Client);

	// Check if actor was spawned locally in a client-side demo 
	if(IsNetClient && Role == ROLE_Authority)
		return;
#endif

	// See if UnrealScript replication condition is met.
	while( Function->GetSuperFunction() )
	{
		Function = Function->GetSuperFunction();
	}

	UBOOL bNeedsReplication = FALSE;

#if MAYBE_NEEDED
	bDemoRecording = 1;

#if CLIENT_DEMO
	if(IsNetClient)
	{
		Exchange(RemoteRole, Role);
	}
	bClientDemoRecording = IsNetClient;
#endif

#endif
	// check to see if there is function replication
	// @todo demo: Is this really the right thing to do? Don't need to check FUNC_NetServer
	// because we don't support client side recording
	if (Function->FunctionFlags & FUNC_NetClient)
	{
		bNeedsReplication = TRUE;
	}

#if MAYBE_NEEDED
	bDemoRecording = 0;
#endif

#if CLIENT_DEMO
	bClientDemoRecording = 0;

	if(IsNetClient)
		Exchange(RemoteRole, Role);
	bClientDemoNetFunc = 0;
#endif

	if( !bNeedsReplication)
	{
		return;
	}

	// Get the connection.
	if( !GWorld->DemoRecDriver->ClientConnections.Num() )
	{
		return;
	}
	UNetConnection* Connection = GWorld->DemoRecDriver->ClientConnections(0);
	if (!Connection)
	{
		return;
	}

	//@todo: FIXME: UActorChannel::ReceivedBunch() currently doesn't handle receiving non-owned PCs (tries to hook them up to a splitscreen viewport). Do demos need those other PCs in them?
	if (this != Connection->Actor && this->GetAPlayerController() != NULL)
	{
		return;
	}

	// Send function data to remote.
	BYTE* SavedCode = Stack ? Stack->Code : NULL;
	InternalProcessRemoteFunction( this, Connection, Function, Parms, Stack, 1 );
	if( Stack )
	{
		Stack->Code = SavedCode;
	}
}


/*-----------------------------------------------------------------------------
	GameInfo
-----------------------------------------------------------------------------*/

/** Update navigation point fear cost fall off. */
void AGameInfo::DoNavFearCostFallOff()
{
	INT TotalFear = 0;
	bDoFearCostFallOff = (TotalFear > 0);
}


//
// Network
//
FString AGameInfo::GetNetworkNumber()
{
	return GWorld->GetNetDriver() ? GWorld->GetNetDriver()->LowLevelGetNetworkNumber() : FString(TEXT(""));
}

/** used to swap a viewport/connection's PlayerControllers when seamless travelling and the new gametype's
 * controller class is different than the previous
 * includes network handling
 * @param OldPC - the old PC that should be discarded
 * @param NewPC - the new PC that should be used for the player
 */
void AGameInfo::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	if (OldPC != NULL && !OldPC->bDeleteMe && NewPC != NULL && !NewPC->bDeleteMe && OldPC->Player != NULL)
	{
		// move the Player to the new PC
		UPlayer* Player = OldPC->Player;
		NewPC->SetPlayer(Player);
		NewPC->NetPlayerIndex = OldPC->NetPlayerIndex;
		NewPC->RemoteRole = OldPC->RemoteRole;
		// send destroy event to old PC immediately if it's local
		if (Cast<ULocalPlayer>(Player))
		{
			GWorld->DestroyActor(OldPC);
		}
		else
		{
			OldPC->PendingSwapConnection = Cast<UNetConnection>(Player);
			//@note: at this point, any remaining RPCs sent by the client on the old PC will be discarded
			// this is consistent with general owned Actor destruction,
			// however in this particular case it could easily be changed
			// by modifying UActorChannel::ReceivedBunch() to account for PendingSwapConnection when it is setting bNetOwner
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("SwapPlayerControllers(): Invalid OldPC, invalid NewPC, or OldPC has no Player!"));
	}
}

/**
 * Forcibly removes an object's CanUnpause delegates from the list of pausers.  If any of the object's CanUnpause delegate
 * handlers were in the list, triggers a call to ClearPause().
 *
 * Called when the player controller is being destroyed to prevent the game from being stuck in a paused state when a PC that
 * paused the game is destroyed before the game is unpaused.
 */
void AGameInfo::ForceClearUnpauseDelegates( AActor* PauseActor )
{
	if ( PauseActor != NULL )
	{
		UBOOL bUpdatePausedState = FALSE;
		for ( INT PauserIdx = Pausers.Num() - 1; PauserIdx >= 0; PauserIdx-- )
		{
			FScriptDelegate& CanUnpauseDelegate = Pausers(PauserIdx);
			if ( CanUnpauseDelegate.Object == PauseActor )
			{
				Pausers.Remove(PauserIdx);
				bUpdatePausedState = TRUE;
			}
		}

		// if we removed some CanUnpause delegates, we may be able to unpause the game now
		if ( bUpdatePausedState )
		{
			eventClearPause();
		}

		APlayerController* PC = Cast<APlayerController>(PauseActor);
		if ( PC != NULL && PC->PlayerReplicationInfo != NULL && WorldInfo != NULL && WorldInfo->Pauser == PC->PlayerReplicationInfo )
		{
			// try to find another player to be the worldinfo's Pauser
			for ( AController* C = WorldInfo->ControllerList; C != NULL; C = C->NextController )
			{
				APlayerController* Player = Cast<APlayerController>(C);
				if (Player != NULL && Player->PlayerReplicationInfo != NULL
				&&	Player->PlayerReplicationInfo != PC->PlayerReplicationInfo
				&&	!Player->ActorIsPendingKill() && !Player->PlayerReplicationInfo->ActorIsPendingKill()
				&&	!Player->bPendingDelete && !Player->PlayerReplicationInfo->bPendingDelete)
				{
					WorldInfo->Pauser = Player->PlayerReplicationInfo;
					break;
				}
			}

			// if it's still pointing to the original player's PRI, clear it completely
			if ( WorldInfo->Pauser == PC->PlayerReplicationInfo )
			{
				WorldInfo->Pauser = NULL;
			}
		}
	}
}

void AVolume::execEncompasses( FFrame& Stack, RESULT_DECL )
{
	P_GET_ACTOR(InActor);
	P_FINISH;

	*(DWORD*)Result = Encompasses(InActor->Location);
}

void AHUD::Draw3DLine(FVector Start, FVector End, FColor LineColor)
{
	GWorld->LineBatcher->DrawLine(Start,End,LineColor,SDPG_World);
}

void AHUD::Draw2DLine(INT X1,INT Y1,INT X2,INT Y2,FColor LineColor)
{
	check(Canvas);

	DrawLine2D(Canvas->Canvas, FVector2D(X1, Y1), FVector2D(X2, Y2), LineColor);
}


/** Flush persistent lines */
void AActor::FlushPersistentDebugLines()
{
	GWorld->PersistentLineBatcher->BatchedLines.Empty();
	GWorld->PersistentLineBatcher->BeginDeferredReattach();
}


/** Draw a debug line */
void AActor::DrawDebugLine(FVector LineStart, FVector LineEnd, BYTE R, BYTE G, BYTE B, UBOOL bPersistentLines)
{
	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;
	LineBatcher->DrawLine(LineStart, LineEnd, FColor(R, G, B), SDPG_World);
}


/** Draw a debug box */
void AActor::DrawDebugBox(FVector Center, FVector Box, BYTE R, BYTE G, BYTE B, UBOOL bPersistentLines)
{
	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;

	LineBatcher->DrawLine(Center + FVector( Box.X,  Box.Y,  Box.Z), Center + FVector( Box.X, -Box.Y, Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector( Box.X, -Box.Y,  Box.Z), Center + FVector(-Box.X, -Box.Y, Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X, -Box.Y,  Box.Z), Center + FVector(-Box.X,  Box.Y, Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X,  Box.Y,  Box.Z), Center + FVector( Box.X,  Box.Y, Box.Z), FColor(R, G, B), SDPG_World);

	LineBatcher->DrawLine(Center + FVector( Box.X,  Box.Y, -Box.Z), Center + FVector( Box.X, -Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector( Box.X, -Box.Y, -Box.Z), Center + FVector(-Box.X, -Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X, -Box.Y, -Box.Z), Center + FVector(-Box.X,  Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X,  Box.Y, -Box.Z), Center + FVector( Box.X,  Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);

	LineBatcher->DrawLine(Center + FVector( Box.X,  Box.Y,  Box.Z), Center + FVector( Box.X,  Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector( Box.X, -Box.Y,  Box.Z), Center + FVector( Box.X, -Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X, -Box.Y,  Box.Z), Center + FVector(-Box.X, -Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
	LineBatcher->DrawLine(Center + FVector(-Box.X,  Box.Y,  Box.Z), Center + FVector(-Box.X,  Box.Y, -Box.Z), FColor(R, G, B), SDPG_World);
}


/** Draw Debug coordinate system */
void AActor::DrawDebugCoordinateSystem(FVector AxisLoc, FRotator AxisRot, FLOAT Scale, UBOOL bPersistentLines)
{
	FRotationMatrix R(AxisRot);
	FVector X = R.GetAxis(0);
	FVector Y = R.GetAxis(1);
	FVector Z = R.GetAxis(2);

	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;

	LineBatcher->DrawLine(AxisLoc, AxisLoc + X*Scale, FColor(255, 000, 000), SDPG_World );
	LineBatcher->DrawLine(AxisLoc, AxisLoc + Y*Scale, FColor(000, 255, 000), SDPG_World );
	LineBatcher->DrawLine(AxisLoc, AxisLoc + Z*Scale, FColor(000, 000, 255), SDPG_World );
}


/** Draw a debug sphere */
void AActor::DrawDebugSphere(FVector Center, FLOAT Radius, INT Segments, BYTE R, BYTE G, BYTE B, UBOOL bPersistentLines)
{
	// Need at least 4 segments
	Segments = Max(Segments, 4);

	FVector Vertex1, Vertex2, Vertex3, Vertex4;
	const INT AngleInc = 65536 / Segments;
	INT NumSegmentsY = Segments, Latitude = AngleInc;
	INT NumSegmentsX, Longitude;
	FLOAT SinY1 = 0.0f, CosY1 = 1.0f, SinY2, CosY2;
	FLOAT SinX, CosX;
	const FColor Color = FColor(R, G, B);

	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;

	while( NumSegmentsY-- )
	{
		SinY2 = GMath.SinTab(Latitude);
		CosY2 = GMath.CosTab(Latitude);

		Vertex1 = FVector(SinY1, 0.0f, CosY1) * Radius + Center;
		Vertex3 = FVector(SinY2, 0.0f, CosY2) * Radius + Center;
		Longitude = AngleInc;

		NumSegmentsX = Segments;
		while( NumSegmentsX-- )
		{
			SinX = GMath.SinTab(Longitude);
			CosX = GMath.CosTab(Longitude);

			Vertex2 = FVector((CosX * SinY1), (SinX * SinY1), CosY1) * Radius + Center;
			Vertex4 = FVector((CosX * SinY2), (SinX * SinY2), CosY2) * Radius + Center;

			LineBatcher->DrawLine(Vertex1, Vertex2, Color, SDPG_World);
			LineBatcher->DrawLine(Vertex1, Vertex3, Color, SDPG_World);

			Vertex1 = Vertex2;
			Vertex3 = Vertex4;
			Longitude += AngleInc;
		}
		SinY1 = SinY2;
		CosY1 = CosY2;
		Latitude += AngleInc;
	}
}


/** Draw a debug cylinder */
void AActor::DrawDebugCylinder(FVector Start, FVector End, FLOAT Radius, INT Segments, BYTE R, BYTE G, BYTE B, UBOOL bPersistentLines)
{
	// Need at least 4 segments
	Segments = Max(Segments, 4);

	// Rotate a point around axis to form cylinder segments
	FVector Segment;
	FVector P1, P2, P3, P4;
	const INT AngleInc = 65536 / Segments;
	INT Angle = AngleInc;
	const FColor Color = FColor(R, G, B);

	// Default for Axis is up
	FVector Axis = (End - Start).SafeNormal();
	if( Axis.IsZero() )
	{
		Axis = FVector(0.f, 0.f, 1.f);
	}

	FVector Perpendicular;
	FVector Dummy;

	Axis.FindBestAxisVectors(Perpendicular, Dummy);


	Segment = Perpendicular.RotateAngleAxis(0, Axis) * Radius;
	P1 = Segment + Start;
	P3 = Segment + End;

	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;

	while( Segments-- )
	{
		Segment = Perpendicular.RotateAngleAxis(Angle, Axis) * Radius;
		P2 = Segment + Start;
		P4 = Segment + End;

		LineBatcher->DrawLine(P2, P4, Color, SDPG_World);
		LineBatcher->DrawLine(P1, P2, Color, SDPG_World);
		LineBatcher->DrawLine(P3, P4, Color, SDPG_World);

		P1 = P2;
		P3 = P4;
		Angle += AngleInc;
	}
}

void AActor::DrawDebugCone(FVector Origin, FVector Direction, FLOAT Length, FLOAT AngleWidth, FLOAT AngleHeight, INT NumSides, FColor DrawColor, UBOOL bPersistentLines)
{
	// Need at least 4 sides
	NumSides = Max(NumSides, 4);

	const FLOAT Angle1 = Clamp<FLOAT>(AngleHeight, (FLOAT)KINDA_SMALL_NUMBER, (FLOAT)(PI - KINDA_SMALL_NUMBER));
	const FLOAT Angle2 = Clamp<FLOAT>(AngleWidth, (FLOAT)KINDA_SMALL_NUMBER, (FLOAT)(PI - KINDA_SMALL_NUMBER));

	const FLOAT SinX_2 = appSin(0.5f * Angle1);
	const FLOAT SinY_2 = appSin(0.5f * Angle2);

	const FLOAT SinSqX_2 = SinX_2 * SinX_2;
	const FLOAT SinSqY_2 = SinY_2 * SinY_2;

	const FLOAT TanX_2 = appTan(0.5f * Angle1);
	const FLOAT TanY_2 = appTan(0.5f * Angle2);

	TArray<FVector> ConeVerts(NumSides);

	for(INT i = 0; i < NumSides; i++)
	{
		const FLOAT Fraction	= (FLOAT)i/(FLOAT)(NumSides);
		const FLOAT Thi			= 2.f * PI * Fraction;
		const FLOAT Phi			= appAtan2(appSin(Thi)*SinY_2, appCos(Thi)*SinX_2);
		const FLOAT SinPhi		= appSin(Phi);
		const FLOAT CosPhi		= appCos(Phi);
		const FLOAT SinSqPhi	= SinPhi*SinPhi;
		const FLOAT CosSqPhi	= CosPhi*CosPhi;

		const FLOAT RSq			= SinSqX_2*SinSqY_2 / (SinSqX_2*SinSqPhi + SinSqY_2*CosSqPhi);
		const FLOAT R			= appSqrt(RSq);
		const FLOAT Sqr			= appSqrt(1-RSq);
		const FLOAT Alpha		= R*CosPhi;
		const FLOAT Beta		= R*SinPhi;

		ConeVerts(i).X = (1 - 2*RSq);
		ConeVerts(i).Y = 2 * Sqr * Alpha;
		ConeVerts(i).Z = 2 * Sqr * Beta;
	}

	// Calculate transform for cone.
	FVector YAxis, ZAxis;
	Direction = Direction.SafeNormal();
	Direction.FindBestAxisVectors(YAxis, ZAxis);
	const FMatrix	ConeToWorld	= FScaleMatrix(FVector(Length)) * FMatrix(Direction, YAxis, ZAxis, Origin);

	ULineBatchComponent *LineBatcher = bPersistentLines ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;

	FVector CurrentPoint, PrevPoint, FirstPoint;
	for(INT i = 0; i < NumSides; i++)
	{
		CurrentPoint = ConeToWorld.TransformFVector(ConeVerts(i));
		LineBatcher->DrawLine(ConeToWorld.GetOrigin(), CurrentPoint, DrawColor, SDPG_World);

		// PrevPoint must be defined to draw junctions
		if( i > 0 )
		{
			LineBatcher->DrawLine(PrevPoint, CurrentPoint, DrawColor, SDPG_World);
		}
		else
		{
			FirstPoint = CurrentPoint;
		}

		PrevPoint = CurrentPoint;
	}
	// Connect last junction to first
	LineBatcher->DrawLine(CurrentPoint, FirstPoint, DrawColor, SDPG_World);
}

#if WITH_UE3_NETWORKING

/**
 * Finds the net connection for the specified player controller or NULL if not found
 *
 * @param PC the player controller to search for
 *
 * @return The net connection for the specified player controller or NULL if not found
 */
static inline UNetConnection* GetNetConnectionFromPlayerController(APlayerController* PC)
{
	// Skip if not networked
	if (GWorld &&
		GWorld->GetNetDriver() &&
		GWorld->GetWorldInfo()->NetMode != NM_Standalone)
	{
		UNetDriver* NetDriver = GWorld->GetNetDriver();
		for (INT Index = 0; Index < NetDriver->ClientConnections.Num(); Index++)
		{
			UNetConnection* Conn = NetDriver->ClientConnections(Index);
			// Does this connection own the player controller in question?
			if (Conn && Conn->Actor == PC)
			{
				return Conn;
			}
		}
	}
	return NULL;
}

/**
 * Used to respond to a product key auth request. Uses the product key to create
 * a response that can be verified on the GameSpy backend
 *
 * @param Challenge the string sent by the backend/server
 */
void APlayerController::ClientConvolve(const FString& Challenge,INT Hint)
{
#if WITH_GAMESPY
	// Skip non-networked games
	if (GWorld &&
		GWorld->GetNetDriver())
	{
		// Get the connection so we can attach the challenge/response information
		UNetConnection* Connection = GWorld->GetNetDriver()->ServerConnection;
		if (Connection)
		{
			// Set the new challenge
			Connection->Challenge = Challenge;
			extern void appGetGameSpyChallengeResponse(UNetConnection*,UBOOL);
			// Calculate the response string based upon the user's CD Key
			appGetGameSpyChallengeResponse(Connection,TRUE);
			// Now send the response to the server for processing
			eventServerProcessConvolve(Connection->ClientResponse,Hint);
		}
	}
#endif
}

/**
 * Forwards the client's response to the GameSpy backend for authorization
 *
 * @param Response the string sent by the cleint in response to the challenge message
 */
void APlayerController::ServerProcessConvolve(const FString& Response,INT Hint)
{
#if WITH_GAMESPY
	// Get the connection so we can pass that to the GameSpy code
	UNetConnection* Connection = GetNetConnectionFromPlayerController(this);
	if (Connection)
	{
		Connection->ClientResponse = Response;
		extern void appSubmitGameSpyReauthRequest(UNetConnection*,INT);
		// Submit the async reauth request
		appSubmitGameSpyReauthRequest(Connection,Hint);
	}
#endif
}
#else	//#if WITH_UE3_NETWORKING
void APlayerController::ClientConvolve(const FString&,INT)
{
}

void APlayerController::ServerProcessConvolve(const FString&,INT)
{
}
#endif	//#if WITH_UE3_NETWORKING

void AActor::Clock(FLOAT& Time)
{
	CLOCK_CYCLES(Time);
}

void AActor::UnClock(FLOAT& Time)
{
	UNCLOCK_CYCLES(Time);
	Time = Time * GSecondsPerCycle * 1000.f;
}

/**
 * Adds a component to the actor's components array, attaching it to the actor.
 * @param NewComponent - The component to attach.
 */
void AActor::AttachComponent(class UActorComponent* NewComponent)
{
	checkf(!HasAnyFlags(RF_Unreachable), TEXT("%s"), *GetFullName());
	checkf(!NewComponent || !NewComponent->HasAnyFlags(RF_Unreachable), TEXT("%s"), *GetFullName());

	if( ActorIsPendingKill() )
	{
		debugf( TEXT("AActor::AttachComponent: Trying to attach '%s' to '%s' which IsPendingKill. Aborting"), *(NewComponent->GetDetailedInfo()), *(this->GetDetailedInfo()) );
		return;
	}

	if ( NewComponent )
	{
		checkf(!NewComponent->HasAnyFlags(RF_Unreachable), TEXT("%s"), *NewComponent->GetFullName());
		NewComponent->ConditionalAttach(GWorld->Scene,this,LocalToWorld());
		Components.AddItem(NewComponent);
	}
}

/**
 * Removes a component from the actor's components array, detaching it from the actor.
 * @param ExComponent - The component to detach.
 */
void AActor::DetachComponent(UActorComponent* ExComponent)
{
	if(ExComponent)
	{
		if(Components.RemoveItem(ExComponent) > 0)
		{
			// If the component passed in was actually in the components array, detach the component from this actor.
			ExComponent->ConditionalDetach();
		}
	}
}

/**
* Removes a component from the actor's components array, detaching it from the actor.
* @param ExComponent - The component to detach.
*/
void AActor::ReattachComponent(UActorComponent* ComponentToReattach)
{
	if(ComponentToReattach)
	{
		if(Components.RemoveItem(ComponentToReattach) > 0)
		{
			// If the component passed in was actually in the components array, detach the component from this actor.
			// Note: setting bWillReattach to TRUE
			ComponentToReattach->ConditionalDetach(TRUE);
		}

		AttachComponent(ComponentToReattach);
	}
}


/*
 // ======================================================================================
 // ClampRotation
 // Clamps the given rotation accoring to the base and the limits passed
 // input:	Rotator RotToLimit - Rotation to limit
 //			Rotator Base	   - Base reference for limiting
 //			Rotator Limits	   - Component wise limits
 // output: out RotToLimit	   - Actual limited rotator
 //			return UBOOL	true if within limits, false otherwise
 // effects:  Alters the passed in Rotation by reference and calls event OverRotated
 //			  when limit has been exceeded
 // notes:	  It cannot change const Rotations passed in (ie Actor::Rotation) so you need to
 //			  make a copy first
 // ======================================================================================
*/
UBOOL AActor::ClampRotation(FRotator& out_RotToLimit, FRotator rBase, FRotator rUpperLimits, FRotator rLowerLimits)
{
	FRotator rOriginal;
	FRotator rAdjusted;

	rOriginal = out_RotToLimit;
	rOriginal = rOriginal.GetNormalized();
	rAdjusted = rOriginal;

	rBase = rBase.GetNormalized();

	rAdjusted = (rAdjusted-rBase).GetNormalized();

	if( rUpperLimits.Pitch >= 0 ) 
	{
		rAdjusted.Pitch = Min( rAdjusted.Pitch,  rUpperLimits.Pitch	);
	}
	if( rLowerLimits.Pitch >= 0 ) 
	{
		rAdjusted.Pitch = Max( rAdjusted.Pitch, -rLowerLimits.Pitch	);
	}
		
	if( rUpperLimits.Yaw >= 0 )
	{
		rAdjusted.Yaw   = Min( rAdjusted.Yaw,    rUpperLimits.Yaw	);
	}
	if( rLowerLimits.Yaw >= 0 )
	{
		rAdjusted.Yaw   = Max( rAdjusted.Yaw,   -rLowerLimits.Yaw	);
	}
		
	if( rUpperLimits.Roll >= 0 )
	{
		rAdjusted.Roll  = Min( rAdjusted.Roll,   rUpperLimits.Roll	);
	}
	if( rLowerLimits.Roll >= 0 )
	{
		rAdjusted.Roll  = Max( rAdjusted.Roll,  -rLowerLimits.Roll	);
	}

	rAdjusted = (rAdjusted + rBase).GetNormalized();

	out_RotToLimit = rAdjusted;
	if( out_RotToLimit != rOriginal ) 
	{
		eventOverRotated( rOriginal, out_RotToLimit );
		return false;
	}
	else 
	{
		return true;
	}
}

void AActor::execIsBasedOn(FFrame& Stack, RESULT_DECL)
{
	P_GET_ACTOR(TestActor);
	P_FINISH;

	*(UBOOL*)Result = IsBasedOn(TestActor);
}

void AActor::execIsOwnedBy(FFrame& Stack, RESULT_DECL)
{
	P_GET_ACTOR(TestActor);
	P_FINISH;

	*(UBOOL*)Result = IsOwnedBy(TestActor);
}

/** Script to C++ thunking code */
void AActor::execSetHardAttach( FFrame& Stack, RESULT_DECL )
{
	P_GET_UBOOL_OPTX(bNewHardAttach,bHardAttach);
	P_FINISH;
	// Just foward the call
	SetHardAttach(bNewHardAttach);
}

/** Looks up the GUID of a package on disk. The package must NOT be in the autodownload cache.
 * This may require loading the header of the package in question and is therefore slow.
 */
FGuid AActor::GetPackageGuid(FName PackageName)
{
	FGuid Result(0,0,0,0);
	for (TObjectIterator<UGuidCache> It; It; ++It)
	{
		if (It->GetPackageGuid(PackageName, Result))
		{
			return Result;
		}
	}

	BeginLoad();
	ULinkerLoad* Linker = GetPackageLinker(NULL, *PackageName.ToString(), LOAD_NoWarn | LOAD_NoVerify, NULL, NULL);
	if (Linker != NULL && Linker->LinkerRoot != NULL)
	{
		Result = Linker->LinkerRoot->GetGuid();
	}
	EndLoad();

	return Result;
}
