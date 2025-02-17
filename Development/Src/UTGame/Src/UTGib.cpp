/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UTGame.h"

IMPLEMENT_CLASS(AUTEmitCameraEffect);

void AUTEmitCameraEffect::UpdateLocation( const FVector& CamLoc, const FRotator& CamRot, FLOAT CamFOVDeg )
{
	FRotationMatrix M(CamRot);

	// the particle is FACING X being parallel to the Y axis.  So we just flip the entire thing to face toward the player who is looking down X
 	const FVector& X = M.GetAxis(0);
	M.SetAxis(0, -M.GetAxis(0));
	M.SetAxis(1, -M.GetAxis(1));

  	const FRotator& NewRot = M.Rotator();

	const FLOAT DistAdjustedForFOV = DistFromCamera * appTan(float(CamFOVDeg*0.5f*PI/180.f)) / appTan(float(CamFOVDeg*0.5f*PI/180.f));

	SetLocation( CamLoc + X * DistAdjustedForFOV );
	SetRotation( NewRot );

	// have to do this, since in UWorld::Tick the actors do the component update for all
	// actors before the camera ticks.  without this, the blood appears to be a frame behind
	// the camera.
	ConditionalUpdateComponents();
}

