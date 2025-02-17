/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"

#include "GearGameVehicleClasses.h"
#include "GearGameAnimClasses.h"

IMPLEMENT_CLASS(AVehicle_Jack_Base);

FLOAT AVehicle_Jack_Base::GetGravityZ()
{
	return 0.f;
}

UBOOL AVehicle_Jack_Base::IsTooCloseToPlayerCamera( FVector* out_CamLoc, FRotator* out_CamRot )
{
	for( AController* C = GWorld->GetFirstController(); C; C = C->NextController )
	{
		AGearPC* PC = Cast<AGearPC>(C);
		if( PC == NULL || PC->Pawn == NULL )
			continue;

		FVector  CamLoc = PC->Pawn->Location;
		CamLoc.Z += PC->Pawn->GetDefaultCollisionSize().Z;
		FRotator CamRot = PC->Pawn->Rotation;

		if( Location.Z < CamLoc.Z + 300.f )
		{
			FLOAT CamDist  = 240.f;
			FLOAT JackDist = (Location - CamLoc).Size2D();
			if( JackDist < ::Max<FLOAT>(CamDist, 64.f) )
			{
				if( out_CamLoc != NULL ) *out_CamLoc = CamLoc;
				if( out_CamRot != NULL ) *out_CamRot = CamRot;

				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL AVehicle_Jack_Base::AdjustFlight( FLOAT ZDiff, UBOOL bFlyingDown, FLOAT Distance, AActor* GoalActor )
{
	const FLOAT Adjust = 0.8f;
	const FLOAT VelZ   = Velocity.Z * Adjust;
	
	if( ZDiff > 0.f )
	{
		Rise = 1.f;
		if( VelZ > ZDiff )
		{
			Rise = 1.f * (1.f - (VelZ / ZDiff));
		}
	}
	else
	if( ZDiff < 0.f )
	{
		Rise = -1.f;
		if( VelZ < ZDiff )
		{
			Rise = -1.f * (1.f - (VelZ / ZDiff));
		}
	}
	else
	{
		Rise = 0.f;
	}

	Rise = ::Clamp(Rise, -1.f, 1.f);

	// If Jack is closer to the pawn than the camera, fly up more
	if( IsTooCloseToPlayerCamera() )
	{
		Rise = ::Min( Rise + 0.5f, 1.f );
	}

	return FALSE;
}

FVector AVehicle_Jack_Base::AdjustDestination( AActor* GoalActor, FVector Dest ) 
{
	FVector Adjust(0,0,0);

	if( Cast<AKeypoint>(GoalActor) == NULL )
	{
		FVector Extent = GetCylinderExtent();
		FLOAT	ColHeight = Extent.Z;
		Extent   *= 0.5f;
		Extent.Z  = 1.f;

		FVector OrigDest	= !Dest.IsZero() ? Dest : GoalActor->Location + FVector(0,0,ColHeight);
		FVector StartTrace	= OrigDest;
		FVector EndTrace	= StartTrace;
		EndTrace.Z -= (StdAdjustHeight + ColHeight);

		UBOOL bAdjust = TRUE;
		

		// Trace down to find floor
		FCheckResult Hit;
		if( !GWorld->SingleLineCheck( Hit, this, EndTrace, StartTrace, TRACE_World, Extent ) )
		{
			StartTrace = Hit.Location;
			StartTrace.Z += ColHeight;
			EndTrace = StartTrace;
			EndTrace.Z += StdAdjustHeight;

			// Track back up to find location near ideal height
			UBOOL bHit = !GWorld->SingleLineCheck( Hit, this, EndTrace, StartTrace, TRACE_World, GetCylinderExtent() );
			if( bHit )
			{
				// Find good spot... use end trace if started penetrating geometry
				FVector FindLoc = (Hit.Time == 0.f) ? EndTrace : Hit.Location;
				UBOOL bGood = GWorld->FindSpot( GetCylinderExtent(), FindLoc, FALSE );
				if( bGood )
				{
					EndTrace = FindLoc;
				}
				else
				{
					bAdjust = FALSE;
				}
			}

			if( bAdjust )
			{
				if( GoalActor && Controller != NULL && Controller->CurrentPath != NULL && Controller->CurrentPath->End.Actor == GoalActor )
				{
					// Trace from current location to adjusted position above the nav point
					// If can't make it, don't adjust position... need to fly down to nav path height
					if( !GWorld->SingleLineCheck( Hit, this, EndTrace, Location, TRACE_World, GetCylinderExtent() ) )
					{
						bAdjust = FALSE;
					}
				}
			}

			if( bAdjust )
			{
				Adjust += (EndTrace - OrigDest);
			}			
		}
			
		// Adjust jack higher than normal when moving along a mantle reachspec
		if( Controller && Cast<UMantleReachSpec>(Controller->CurrentPath) )
		{
			Adjust.Z += MantleAdjustHeight;
		}

		ALadderMarker* LM = Cast<ALadderMarker>(GoalActor);
		if( LM != NULL && LM->LadderTrigger != NULL )
		{
			// If moving to the top of the ladder
			if( LM->bIsTopOfLadder && LM->LadderTrigger->BottomMarker != NULL )
			{
				// Fly out over the edge of the ladder so we can move directly down after
				FVector AdjustDest(LM->LadderTrigger->BottomMarker->Location.X,LM->LadderTrigger->BottomMarker->Location.Y,LM->Location.Z);
				Adjust += (AdjustDest - LM->Location);			
			}
		}
	}

	return Adjust;
}

void AVehicle_Jack_Base::AdjustThrottle( FLOAT Distance )
{
	FLOAT Speed = Velocity.Size(); 
	if (Speed > 0.f)
	{
		// Movement distance considers full route cache length
		AGearAI* AI = Cast<AGearAI>(Controller);
		Distance = ::Max( AI ? AI->GetRouteCacheDistance() : 0.f, Distance );

		const FLOAT Adjust = 1.4f;
		const FLOAT Ratio = Distance / Speed;
		const FLOAT	ScaleThrottle = Adjust * Ratio;

		Throttle *= ::Min(1.f, ScaleThrottle );

/*		debugf(TEXT("AdjustThrottle dist %f speed %f ratio %f scale %f throttle %f"),
			Distance,
			Speed,
			Ratio,
			ScaleThrottle,
			Throttle );*/
	}
}