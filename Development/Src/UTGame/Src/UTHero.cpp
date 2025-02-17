//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "UTGame.h"

void AUTPawn::SetMaxStepHeight(FLOAT NewMaxStepHeight)
{
	MaxStepHeight = NewMaxStepHeight;
}

/** Returns whether hero will fit in current spawn position
 */
UBOOL AUTPawn::HeroFits( AUTPawn *P, FLOAT NewRadius, FLOAT NewHeight)
{
	// see if space to become giant
	FVector NewLocation = Location;

	if ( !FindSpot(FVector(NewRadius, NewRadius, 0.5f*NewHeight), NewLocation) )
	{
		return FALSE;
	}

	// start off crouched
	bIsCrouched = TRUE;
	CylinderComponent->SetCylinderSize(NewRadius, 0.5f*NewHeight);
	CylinderComponent->UpdateBounds(); // Force an update of the bounds with the new dimensions

	GWorld->FarMoveActor(this, NewLocation, FALSE, FALSE, FALSE);

	return TRUE;
}


/**
 * Pawn uncrouches.
 * Checks if new cylinder size fits (no encroachment), and trigger Pawn->eventEndCrouch() in script if succesful.
 *
 * @param	bClientSimulation	true when called when bIsCrouched is replicated to non owned clients, to update collision cylinder and offset.
 */
void AUTPawn::UnCrouch(INT bClientSimulation)
{
	// see if space to uncrouch
	FCheckResult Hit(1.f);

	FLOAT	HeightAdjust = DefaultHeight - CylinderComponent->CollisionHeight;
	FVector	NewLoc = Location + FVector(0.f,0.f,HeightAdjust);

	UBOOL bEncroached = false;

	// change cylinder directly rather than calling setcollisionsize(), since we don't want to cause touch/untouch notifications unless uncrouch succeeds
	check(CylinderComponent);
	CylinderComponent->SetCylinderSize(DefaultRadius, DefaultHeight);
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
	SetCollisionSize(DefaultRadius, DefaultHeight);

	// space enough to uncrouch, so stand up
	if( !bClientSimulation )
	{
		bNetDirty = true;			// bIsCrouched replication controlled by bNetDirty
		bIsCrouched = false;
	}
	bForceFloorCheck = TRUE;
	eventEndCrouch( HeightAdjust );
}
