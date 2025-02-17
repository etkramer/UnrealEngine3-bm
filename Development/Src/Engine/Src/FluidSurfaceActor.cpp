/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineFluidClasses.h"

IMPLEMENT_CLASS(AFluidSurfaceActorMovable);
IMPLEMENT_CLASS(AFluidSurfaceActor);

void AFluidSurfaceActor::PostEditImport()
{
	Super::PostEditImport();
}

void AFluidSurfaceActor::PostEditChange(UProperty* PropertyThatChanged)
{
	// AActor::PostEditChange will ForceUpdateComponents()
	Super::PostEditChange( PropertyThatChanged );
}

void AFluidSurfaceActor::PostEditMove(UBOOL bFinished)
{
	Super::PostEditMove( bFinished );

	if ( bFinished )
	{
		// Propagate scale changes to the fluid vertices instead.
		FluidComponent->OnScaleChange();
		DrawScale = 1.0f;
		DrawScale3D = FVector(1.0f, 1.0f, 1.0f);
		GCallbackEvent->Send( CALLBACK_UpdateUI );
	}
}

void AFluidSurfaceActor::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	Super::EditorApplyScale( DeltaScale, ScaleMatrix, PivotLocation, bAltDown, bShiftDown, bCtrlDown );
}

void AFluidSurfaceActor::CheckForErrors()
{
	Super::CheckForErrors();
}

void AFluidSurfaceActor::TickSpecial( FLOAT DeltaSeconds )
{
	for ( INT ActorIndex=0; ActorIndex < Touching.Num(); ++ActorIndex )
	{
		AActor* Actor = Touching( ActorIndex );
		if (Actor && Actor->bAllowFluidSurfaceInteraction)
		{
			FLOAT ActorVelocity = Actor->Velocity.Size();
			if ( ActorVelocity > KINDA_SMALL_NUMBER )
			{
				if(Actor->CollisionComponent)
				{
					FLOAT Radius = Actor->CollisionComponent->Bounds.SphereRadius;
					FluidComponent->ApplyForce( Actor->Location, FluidComponent->ForceContinuous, Radius*0.3f, FALSE );
				}
			}
		}
	}
}
