/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "UTGame.h"
#include "EngineAnimClasses.h"
#include "EngineMaterialClasses.h"
#include "UTGameVehicleClasses.h"
#include "UTGameAnimationClasses.h"											   

IMPLEMENT_CLASS(AUTVehicle_Scorpion);

/**********************************************************************************
 * Scorpion Vehicle
 **********************************************************************************/

UBOOL AUTVehicle_Scorpion::ReadyToSelfDestruct()
{
	return (bBoostersActivated && (Velocity.SizeSquared() > SelfDestructSpeedSquared));
}

void AUTVehicle_Scorpion::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	// ready sound above everything else so that it can be stopped if dead
	if ( SelfDestructReadyComponent )
	{
		// stop self destruct ready sound if no longer ready
		if( !bDriving || SelfDestructInstigator || !IsLocallyControlled() || !ReadyToSelfDestruct() ) 
		{
			SelfDestructReadyComponent->Stop();
			SelfDestructReadyComponent = NULL;
		}
	}
	else if( bDriving && !bDeadVehicle && IsLocallyControlled() && ReadyToSelfDestruct() )
	{
		// play sound when ready to self destruct
		SelfDestructReadyComponent = CreateAudioComponent(SelfDestructReadyCue,TRUE,TRUE,FALSE);
	}

	if ( bDeadVehicle )
		return;

	if ( SelfDestructInstigator )
	{
		if ( (WorldInfo->TimeSeconds - BoostStartTime > MaxBoostDuration) )
		{
			// blow up
			eventSelfDestruct(NULL);
			return;
		}

		ATeamInfo* InstigatorTeam = SelfDestructInstigator->PlayerReplicationInfo ? SelfDestructInstigator->PlayerReplicationInfo->Team : NULL;
		if ( CheckAutoDestruct(InstigatorTeam, BoosterCheckRadius) )
			return;
	}
	else
	{
		if ( bTryToBoost )
		{
			// turbo mode
			if ( !bBoostersActivated )
			{
				if ( WorldInfo->TimeSeconds - BoostChargeTime > BoostChargeDuration ) // Starting boost
				{
					eventActivateRocketBoosters();
					bBoostersActivated = TRUE;
					BoostStartTime = WorldInfo->TimeSeconds;
				}
			}
		}

		bTryToBoost = false;

		if ( (Role == ROLE_Authority) || IsLocallyControlled() )
		{
			if ( bBoostersActivated )
			{
				if ( WorldInfo->TimeSeconds - BoostStartTime > MaxBoostDuration ) // Ran out of Boost
				{
					eventDeactivateRocketBoosters();
					bBoostersActivated = FALSE;
					BoostChargeTime = WorldInfo->TimeSeconds;
				}

				if ( (Throttle <= 0) && (WorldInfo->TimeSeconds - BoostReleaseTime > BoostReleaseDelay) ) // Stopped in middle of boost
				{
					eventDeactivateRocketBoosters();
					bBoostersActivated = FALSE;
					FLOAT BoostRemaining = MaxBoostDuration - WorldInfo->TimeSeconds + BoostStartTime;
					BoostChargeTime = WorldInfo->TimeSeconds - ::Min(BoostChargeDuration - 2.f, BoostRemaining * BoostChargeDuration/MaxBoostDuration);
				}
				else
				{
					BoostReleaseTime = WorldInfo->TimeSeconds;
				}
			}
			else if ( bSteeringLimited && (Velocity.SizeSquared() < Square(AirSpeed)) )
			{
				eventEnableFullSteering();
			}
		}
	}

	if ( bBoostersActivated )
	{
		FVector BoostDir = Rotation.Vector();
		if ( Velocity.SizeSquared() < BoostPowerSpeed*BoostPowerSpeed )
		{
			if ( BoostDir.Z > 0.7f )
				AddForce( (1.f - BoostDir.Z) * BoosterForceMagnitude * BoostDir );
			else
				AddForce( BoosterForceMagnitude * BoostDir );
		}
		else
			AddForce( 0.25f * BoosterForceMagnitude * BoostDir );
	}

	if (bBladesExtended)
	{
		if (!bRightBladeBroken)
		{
			// trace across right blade
			FVector Start, End;
			Mesh->GetSocketWorldLocationAndRotation(RightBladeStartSocket, Start, NULL);
			Mesh->GetSocketWorldLocationAndRotation(RightBladeEndSocket, End, NULL);
			FCheckResult Hit(1.0f);
			GWorld->SingleLineCheck(Hit, this, End, Start, TRACE_ProjTargets);
			if ( Hit.Actor && (Hit.Actor->GetAPawn() || (Hit.Time < BladeBreakPoint)) )
			{
				eventBladeHit(Hit.Actor, Hit.Location, false);
			}
		}
		if (!bLeftBladeBroken)
		{
			// trace across the left blade
			FVector Start, End;
			Mesh->GetSocketWorldLocationAndRotation(LeftBladeStartSocket, Start, NULL);
			Mesh->GetSocketWorldLocationAndRotation(LeftBladeEndSocket, End, NULL);
			FCheckResult Hit(1.0f);
			GWorld->SingleLineCheck(Hit, this, End, Start, TRACE_ProjTargets);
			if ( Hit.Actor && (Hit.Actor->GetAPawn() || (Hit.Time < BladeBreakPoint)) )
			{
				eventBladeHit(Hit.Actor, Hit.Location, true);
			}
		}
	}

	// client side effects follow - return if server
	//@todo FIXMESTEVE - skip if far away or not rendered
	if (LastRenderTime < WorldInfo->TimeSeconds - 1.0f)
		return;

	// Update brake light and reverse light
	// Both lights default to off.
	UBOOL bSetBrakeLightOn = FALSE;
	UBOOL bSetReverseLightOn = FALSE;

	// check if scorpion is braking
	if( ( (OutputBrake > 0.f) || bOutputHandbrake ) && (Velocity.Size() > 2.0f) )
	{
		bSetBrakeLightOn = TRUE;
		if ( !bBrakeLightOn )
		{	
			// turn on brake light
			bBrakeLightOn = TRUE;
			if(DamageMaterialInstance[0])
			{
				DamageMaterialInstance[0]->SetScalarParameterValue(BrakeLightParameterName, 60.f );
			}
		}
	}

	// check if scorpion is in reverse
	if ( Throttle < 0.0f )
	{
		bSetReverseLightOn = true;
		if ( !bReverseLightOn )
		{
			// turn on reverse light
			bReverseLightOn = true;
			if(DamageMaterialInstance[0])
			{
				DamageMaterialInstance[0]->SetScalarParameterValue(ReverseLightParameterName, 50.f );
			}
		}
	}

	if ( bBrakeLightOn && !bSetBrakeLightOn )
	{
		// turn off brake light
		bBrakeLightOn = false;
		if(DamageMaterialInstance[0])
		{
			DamageMaterialInstance[0]->SetScalarParameterValue(BrakeLightParameterName, 0.f );
		}
	}
	if ( bReverseLightOn && !bSetReverseLightOn )
	{
		// turn off reverse light
		bReverseLightOn = false;
		if(DamageMaterialInstance[0])
		{
			DamageMaterialInstance[0]->SetScalarParameterValue(ReverseLightParameterName, 0.f );
		}
	}

	// update headlights
	if ( bHeadlightsOn )
	{
		if ( !PlayerReplicationInfo )
		{
			// turn off headlights
			bHeadlightsOn = false;
			if(DamageMaterialInstance[0])
			{
				DamageMaterialInstance[0]->SetScalarParameterValue(HeadLightParameterName, 0.f );
			}
		}
	}
	else if ( PlayerReplicationInfo )
	{
		// turn on headlights
		bHeadlightsOn = true;
		if(DamageMaterialInstance[0])
		{
			DamageMaterialInstance[0]->SetScalarParameterValue(HeadLightParameterName, 100.f );
		}
	}
}

