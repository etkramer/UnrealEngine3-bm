/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#include "GearGame.h"
#include "GearGameWeaponClasses.h"
#include "EngineMaterialClasses.h"
#include "GearGameVehicleClasses.h"

IMPLEMENT_CLASS(AGearWeapon)
IMPLEMENT_CLASS(AGearShield)
IMPLEMENT_CLASS(AGearDroppedPickup_Shield)
IMPLEMENT_CLASS(AGearWeap_HeavyBase)
IMPLEMENT_CLASS(AGearWeap_GrenadeBase)
IMPLEMENT_CLASS(AGearWeap_PistolBase)

IMPLEMENT_CLASS(AGearProjectile)
IMPLEMENT_CLASS(AGearProj_ExplosiveBase)
IMPLEMENT_CLASS(AGearProj_Grenade)
IMPLEMENT_CLASS(AGearProj_HomingRocket)

IMPLEMENT_CLASS(AGearExplosionActor)
IMPLEMENT_CLASS(AGearExplosionActorReplicated)

IMPLEMENT_CLASS(AFlameThrowerSprayBase)

IMPLEMENT_CLASS(AGearDynamicFogVolume_Spherical)
IMPLEMENT_CLASS(AGearFogVolume_Spawnable)
IMPLEMENT_CLASS(AGearFogVolume_SmokeGrenade)



/************************************************************************************
 * Internal Helpers
 ***********************************************************************************/

static void DrawDebugSweptBox(AActor* Owner, FVector const& StartLoc, FVector const& EndLoc, FVector const& Extent, BYTE R, BYTE G, BYTE B, UBOOL bPersistent)
{
	if( !Owner )
	{
		return;
	}

	ULineBatchComponent *LineBatcher = bPersistent ? GWorld->PersistentLineBatcher : GWorld->LineBatcher;
	FColor const Color(R, G, B);


	// Draw Start Box
	FVector Start_Vert0 = StartLoc + FVector( Extent.X,  Extent.Y,  Extent.Z);
	FVector Start_Vert1 = StartLoc + FVector( Extent.X, -Extent.Y, Extent.Z);
	FVector Start_Vert2 = StartLoc + FVector( -Extent.X, -Extent.Y, Extent.Z);
	FVector Start_Vert3 = StartLoc + FVector( -Extent.X, Extent.Y, Extent.Z);
	FVector Start_Vert4 = StartLoc + FVector( Extent.X, Extent.Y, -Extent.Z);
	FVector Start_Vert5 = StartLoc + FVector( Extent.X, -Extent.Y, -Extent.Z);
	FVector Start_Vert6 = StartLoc + FVector( -Extent.X, -Extent.Y, -Extent.Z);
	FVector Start_Vert7 = StartLoc + FVector( -Extent.X, Extent.Y, -Extent.Z);

	LineBatcher->DrawLine(Start_Vert0, Start_Vert1, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert1, Start_Vert2, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert2, Start_Vert3, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert3, Start_Vert0, Color, SDPG_World);

	LineBatcher->DrawLine(Start_Vert4, Start_Vert5, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert5, Start_Vert6, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert6, Start_Vert7, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert7, Start_Vert4, Color, SDPG_World);

	LineBatcher->DrawLine(Start_Vert0, Start_Vert4, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert1, Start_Vert5, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert2, Start_Vert6, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert3, Start_Vert7, Color, SDPG_World);


	// Draw End Box
	FVector End_Vert0 = EndLoc + FVector( Extent.X,  Extent.Y,  Extent.Z);
	FVector End_Vert1 = EndLoc + FVector( Extent.X, -Extent.Y, Extent.Z);
	FVector End_Vert2 = EndLoc + FVector( -Extent.X, -Extent.Y, Extent.Z);
	FVector End_Vert3 = EndLoc + FVector( -Extent.X, Extent.Y, Extent.Z);
	FVector End_Vert4 = EndLoc + FVector( Extent.X, Extent.Y, -Extent.Z);
	FVector End_Vert5 = EndLoc + FVector( Extent.X, -Extent.Y, -Extent.Z);
	FVector End_Vert6 = EndLoc + FVector( -Extent.X, -Extent.Y, -Extent.Z);
	FVector End_Vert7 = EndLoc + FVector( -Extent.X, Extent.Y, -Extent.Z);

	LineBatcher->DrawLine(End_Vert0, End_Vert1, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert1, End_Vert2, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert2, End_Vert3, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert3, End_Vert0, Color, SDPG_World);

	LineBatcher->DrawLine(End_Vert4, End_Vert5, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert5, End_Vert6, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert6, End_Vert7, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert7, End_Vert4, Color, SDPG_World);

	LineBatcher->DrawLine(End_Vert0, End_Vert4, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert1, End_Vert5, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert2, End_Vert6, Color, SDPG_World);
	LineBatcher->DrawLine(End_Vert3, End_Vert7, Color, SDPG_World);


	// Connect the boxes
	LineBatcher->DrawLine(Start_Vert0, End_Vert0, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert1, End_Vert1, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert2, End_Vert2, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert3, End_Vert3, Color, SDPG_World);

	LineBatcher->DrawLine(Start_Vert4, End_Vert4, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert5, End_Vert5, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert6, End_Vert6, Color, SDPG_World);
	LineBatcher->DrawLine(Start_Vert7, End_Vert7, Color, SDPG_World);
}


/************************************************************************************
 * AGearWeapon
 ***********************************************************************************/


void AGearWeapon::GetActiveReloadValues(FLOAT& ActiveReloadStartTime,FLOAT& PreReactionWindowDuration,FLOAT& SuperSweetSpotDuration,FLOAT& SweetSpotDuration)
{
	const FLOAT PreReactionWindowDurationDefault = 0.250f;
	const FLOAT SuperSweetSpotDurationDefault = 0.125f;
	const FLOAT SweetSpotDurationDefault = 0.250f;
	//// Determine all of the values we need for the ActiveReload check using either the weapons's values or default values

	// get the owner of this Weapon
	const AGearPawn* WP = Cast<AGearPawn>(Owner);


	// use the AR_PossibleSuccessStartPoint time if there is one
	if( AR_PossibleSuccessStartPoint != -1 )
	{
		ActiveReloadStartTime = AR_PossibleSuccessStartPoint;
	}
	// else just use the ReloadDuration / 2
	else
	{
		ActiveReloadStartTime = ReloadDuration / 2.0f;
	}

	/** The time window before the TimeToStartTheMiniGame which if a button is pressed will result in a failure **/
	// check the weapon to see if it has the SuperSweetSpot value defined
	if( AR_PreReactionWindowDuration == -1 )
	{
		PreReactionWindowDuration = PreReactionWindowDurationDefault;
	}
	else
	{
		PreReactionWindowDuration = AR_PreReactionWindowDuration;
	}

	/** This is the time you must be within after the notification to get the uber fast reload **/
	// check the weapon to see if it has the SuperSweetSpot value defined
	if( AR_SuperSweetSpotDuration == -1 )
	{
		if(WP != NULL)
		{
			INT ARTier = WP->ActiveReload_CurrTier;
			SuperSweetSpotDuration = ActiveReload_SuperSweetSpotDurations.Value(ARTier);//AR_SuperSweetSpotDuration;
		}
		else
		{
			SuperSweetSpotDuration = ActiveReload_SuperSweetSpotDurations.Value(0);//AR_SuperSweetSpotDuration;
		}
		
	}
	else
	{
		SuperSweetSpotDuration = SuperSweetSpotDurationDefault;
	}

	/** This is the time you must be within after the notification to get the fast reload **/
	// check the weapon to see if it has the SweetSpot value defined
	if( AR_SweetSpotDuration == -1 )
	{
		SweetSpotDuration = SweetSpotDurationDefault;
	}
	else
	{
		SweetSpotDuration = AR_SweetSpotDuration;
	}
	AGearPRI *PRI = Cast<AGearPRI>(Instigator->PlayerReplicationInfo);
	if (PRI != NULL && PRI->Difficulty != NULL)
	{
		UDifficultySettings *Difficulty = Cast<UDifficultySettings>(PRI->Difficulty->GetDefaultObject());
		checkSlow(Difficulty != NULL && "No difficulty set!");
		PreReactionWindowDuration *= 1.f - Abs(1.f - Difficulty->PlayerActiveReloadWindowMod);
		SweetSpotDuration *= Difficulty->PlayerActiveReloadWindowMod;
		SuperSweetSpotDuration *= Difficulty->PlayerActiveReloadWindowMod;
	}
}

void AGearWeapon::DetermineActiveReloadSuccessOrFailure()
{
	FLOAT PreReactionWindowDuration, ReloadStartTime, SuperSweetSpotDuration, SweetSpotDuration;
	GetActiveReloadValues(ReloadStartTime,PreReactionWindowDuration,SuperSweetSpotDuration,SweetSpotDuration);

	// get the owner of this Weapon
	const AGearPawn* WP = Cast<AGearPawn>(Owner);

	// check to see if the Owner is in Cover and NOT leaning out
	if( WP == NULL || 
		(
			( WP != NULL )
			&& ( WP->CoverType != CT_None )
			//&& ( PC.IsLeaning() == FALSE ) // @todo need to native-ize these
		)
	  )
	{
		// increase the SweetSpot and SuperSweetSpot areas
		PreReactionWindowDuration *= 0.80f;
		SuperSweetSpotDuration *= 1.20f;
		SweetSpotDuration *= 1.20f;
	}


	///////////////  okie now we have all of our base data ////////////////

	const FLOAT RelativeTimePlayerPressedButton = GWorld->GetTimeSeconds() - AR_TimeReloadButtonWasPressed;


	// if we are able to fail the mini game ( inside the pre window )
	if( ( RelativeTimePlayerPressedButton >= ( ReloadStartTime - PreReactionWindowDuration ) )
		&& ( RelativeTimePlayerPressedButton < ReloadStartTime )
		)
	{
		eventLocalFailedActiveReload( TRUE );
	}
	// if we have a chance at succeeding with the Active Reload 
	else if( RelativeTimePlayerPressedButton >= ReloadStartTime )
	{
		// check to see if the button press was inside the SuperSweetSpot
		if( RelativeTimePlayerPressedButton <= ( ReloadStartTime + SuperSweetSpotDuration ) )
		{
			eventLocalActiveReloadSuccess( TRUE );
		}
		// check to see if the button press was inside the SweetSpot
		else if( RelativeTimePlayerPressedButton <= ( ReloadStartTime + SuperSweetSpotDuration +SweetSpotDuration ) )
		{  
			eventLocalActiveReloadSuccess( FALSE );
		}
		// else it was outside the SweetSpots and thus is a failure
		else
		{
			eventLocalFailedActiveReload( TRUE );
		}

	}
	// fail the Active Reload attempt but do NOT penalize the player.  They do a normal Reload
	else
	{
		eventLocalFailedActiveReload( TRUE );
	}
}


/** Weapon Ticking */
void AGearWeapon::TickSpecial(FLOAT DeltaSeconds)
{
	Super::TickSpecial(DeltaSeconds);

	// Muzzle flash light pulse
	if( MuzzleFlashLight && MuzzleFlashLight->bEnabled )
	{
		AGearWeapon*	DefaultWeap = Cast<AGearWeapon>(GetClass()->GetDefaultObject());

		if( DefaultWeap && DefaultWeap->MuzzleFlashLight )
		{
			// Calculate new brightness
			MuzzleFlashLight->Brightness = Clamp<FLOAT>(appCos(MuzzleLightPulseTime) * MuzzleLightPulseExp, 0.f, 1.f) * DefaultWeap->MuzzleFlashLight->Brightness;

			// Dirty component, so it gets updated.
			MuzzleFlashLight->BeginDeferredReattach();

			// Update pulse time.
			MuzzleLightPulseTime += DeltaSeconds * MuzzleLightPulseFreq;
		}
	}

	if (bSupportsBarrelHeat)
	{
		if (BarrelHeatCooldownTime > 0.f)
		{
			GoalBarrelHeat -= DeltaSeconds / BarrelHeatCooldownTime;
			GoalBarrelHeat = Max(GoalBarrelHeat, 0.f);
		}
		if (BarrelHeatInterpSpeed > 0.f)
		{
			CurrentBarrelHeat = FInterpTo(CurrentBarrelHeat, GoalBarrelHeat, DeltaSeconds, BarrelHeatInterpSpeed);
		}
		if (LastBarrelHeat != CurrentBarrelHeat)
		{
			eventUpdateBarrelHeatMaterial();
		}
		LastBarrelHeat = CurrentBarrelHeat;
	}
}

void AGearWeapon::UpdateBarrelHeatMaterial()
{
	if ( (BarrelHeatMaterialParameterName != NAME_None) && MIC_WeaponSkin )
	{
		MIC_WeaponSkin->SetScalarParameterValue(BarrelHeatMaterialParameterName, CurrentBarrelHeat);
	}
}



/**
* This function returns the world location for spawning the projectile, pulled in to the Pawn's collision along the AimDir direction.
*/
FVector AGearWeapon::GetPhysicalFireStartLoc(FVector AimDir)
{
	if ( bUseMuzzleLocForPhysicalFireStartLoc || (Instigator == NULL) || (AimDir.IsNearlyZero()) )
		return eventGetMuzzleLoc();

	return Super::GetPhysicalFireStartLoc(AimDir);
}


/**
* This function returns the world location for spawning the visual effects.
* Network: ALL
*/
FVector AGearWeapon::GetMuzzleLoc()
{
	FVector Loc;
	FRotator Rot;
	GetMuzzleLocAndRot(Loc,Rot);
	return Loc;
}

void AGearWeapon::GetMuzzleLocAndRot(FVector& Loc, FRotator& Rot)
{
	USkeletalMeshComponent*	WeapMesh;
	FVector				X, Y, Z;

	WeapMesh = Cast<USkeletalMeshComponent>(Mesh);
	if( WeapMesh != NULL && MuzzleSocketName != NAME_None && (WeapMesh->SpaceBases.Num() > 0 || WeapMesh->ParentAnimComponent != NULL))
	{
		if( WeapMesh->GetSocketWorldLocationAndRotation(MuzzleSocketName, Loc, &Rot) )
		{
			return;
		}
	}

	AGearPawn* GP = Cast<AGearPawn>(Instigator);
	if( GP != NULL )
	{
		GP->GetWeaponHandPosition(Loc, Rot);
		FRotationMatrix RMat(Rot);
		RMat.GetAxes( X, Y, Z );
		Loc = (Loc + FireOffset.X*X + FireOffset.Y*Y + FireOffset.Z*Z);
		return;
	}
	else 
	{
		ATurret* Tur = Cast<ATurret>(Instigator);
		if( Tur != NULL )
		{
			Loc = Tur->eventGetPhysicalFireStartLoc(FireOffset);
			return;
		}
	}

	// Get here as final fallback (happens for dummy fire)
	Loc = Location;
}

/************************************************************************************
 * AGearProjectile
 ***********************************************************************************/

void AGearProjectile::TickSpecial( FLOAT DeltaSeconds )
{
	Super::TickSpecial(DeltaSeconds);

	// this will override what the super did
	if ( bRotationFollowsVelocity )
	{
		Rotation = (Velocity - InitialFrameOfRefVelocity).Rotation();
	}
}

UBOOL AGearProjectile::IgnoreBlockingBy( const AActor *Other ) const
{
	if (bIgnoreInstigatorCollision)
	{
		if (Other == Instigator)
		{
			return TRUE;
		}
		const AGearWeaponPawn* WP = ConstCast<AGearWeaponPawn>(Other);
		if (WP != NULL && WP->MyVehicle != NULL && Other == WP->MyVehicle)
		{
			return TRUE;
		}
	}

	return Super::IgnoreBlockingBy(Other);
}

/************************************************************************************
 * AGearProj_Grenade
 ***********************************************************************************/

void AGearProj_Grenade::processHitWall(FCheckResult const& Hit, FLOAT TimeSlice)
{
//	FLOAT StartVelZ = Velocity.Z;

	// update velocity 
	if( bPerformGravity )
	{
		// calculate time between last grav update and the collision time.  we will go ahead 
		// and simulate gravity for this delta, to be sure we have an accurate velocity at impact
		// for the bounce code.
		FLOAT GravAdvanceTime = ( (SimFixedTimeStep - TimeTilNextGravUpdate) - (TimeSlice * (1.f-Hit.Time)) ) - SimInternalGravityAdvanceTime;

		// keep track of how far we've advanced gravity simulation.  this handles multiple impacts within the same sim.
		SimInternalGravityAdvanceTime += GravAdvanceTime;

		// apply grav
		Velocity.Z += GravityScale * GetGravityZ() * GravAdvanceTime;
	}

//	FVector VelAtImpact = Velocity;

	// this will end up setting Velocity to the post-bounce velocity
	Super::processHitWall(Hit, TimeSlice);

//	if (bIsSimulating && (BounceCount == 1))
//	{
//		debugf( TEXT("%f : StartVelZ %f, VelAtImpact (%f, %f, %f), VelMagAtImpact %f, Post-bounce VelZ %f, TimeSlice %f, Hit.Time %f"), GWorld->GetWorldInfo()->TimeSeconds, StartVelZ, VelAtImpact.X, VelAtImpact.Y, VelAtImpact.Z, VelAtImpact.Size(), Velocity.Z, TimeSlice, Hit.Time);
//	}
}

/**
 * Special physProjectile for Grenades (in order to help with preview arc rendering)
 * Some explanation is in order here.  We have 2 high-level goals here.
 *		1) The real grenade should reliably go where the preview arc says it will.
 *		2) The preview arc should smoothly and predictably respond to aim adjustments (no big pops/jerks)
 * To accomplish 1) without to much processing time, we run the preview arc simulation at 20hz.  To make the 
 * real grenade match, we run it's gravity updates at 20hz as well, although we make sure to simulate each full
 * frame to avoid jerky movement.  To help with 2), we do make one exception and apply gravity when the grenade
 * hits something, to ensure we're bouncing with an accurate impact velocity.  We also restart the 20hz gravity
 * timer on collisions, to maintain consistent bounce arcs.
 */
void AGearProj_Grenade::physProjectile( FLOAT DeltaTime, INT Iterations )
{
	//debugf( TEXT("physProjectile called with DeltaTime %f  Iterations %d"), DeltaTime, Iterations );
	//debugf( TEXT("AProjectile::physProjectile Velocity X:%f, Y:%f, Z:%f"), Velocity.X, Velocity.Y, Velocity.Z);

	FLOAT SimTimeRemainingThisTick = DeltaTime;
	
	while (SimTimeRemainingThisTick > 0.f)
	{
		// run grav
		if (TimeTilNextGravUpdate <= 0.f)
		{
			// Simulate gravity
			if( bPerformGravity )
			{
				Velocity.Z += GravityScale * GetGravityZ() * SimFixedTimeStep;
			}

			TimeTilNextGravUpdate = SimFixedTimeStep;
		}

		FLOAT const PreGravTimeSlice = Min(TimeTilNextGravUpdate, SimTimeRemainingThisTick);

		// run pre-grav
		if (PreGravTimeSlice > 0.f)
		{
			// do this before calling the super, since processHitWall can modify TimeTilNextGravUpdate
			TimeTilNextGravUpdate -= PreGravTimeSlice;

			Super::physProjectile(PreGravTimeSlice, 0);

			if (SimInternalGravityAdvanceTime > 0.f)
			{
				// we hit something and reset the grav timing.  consume the portion of time AFTER the last hit
				// so we're always doing grav again exactly SimFixedTimeStep after the impact.
				TimeTilNextGravUpdate += SimInternalGravityAdvanceTime;
				SimInternalGravityAdvanceTime = 0.f;
			}

			SimTimeRemainingThisTick -= PreGravTimeSlice;
		}
	}
}

void AGearProj_Grenade::RunPhysicsSimulationTilEnd(FLOAT GrenadeLifeSpan)
{
	// start the sim off right!
//	static FLOAT JCF = 0.f;
//	TimeTilNextGravUpdate = SimFixedTimeStep + JCF;
	TimeTilNextGravUpdate = 0.f;

	// We need collision on to bounce off pawns and stuff
	SetCollision(TRUE, FALSE, FALSE);

	// make all of the particles not display in the scene 
	for( INT ComponentIndex = 0; ComponentIndex < this->Components.Num(); ComponentIndex++ )
	{
		UParticleSystemComponent* PSC = Cast<UParticleSystemComponent>(this->Components(ComponentIndex));
		if( PSC != NULL )
		{  
			for( INT i = 0; i < PSC->EmitterInstances.Num(); i++ )
			{
				// destroys all of the particles that are alive in the level
				if( PSC->EmitterInstances(i) != NULL )
				{
					PSC->EmitterInstances(i)->KillParticlesForced();  
				}
			}
			// Need to tick the emitter the first time so the end of the trail is at the 'start' of the toss
			PSC->SetSkipUpdateDynamicDataDuringTick(TRUE);
			PSC->Tick(SimFixedTimeStep);
		}
	}

	FLOAT	CurrSimulationTime	= 0.f;
	//INT		IterationCount		= 0;

//	FVector	OldLocation = Location;
	while( 	(Physics != PHYS_None)								// if grenade stopped moving, then abort simulation
		    && (CurrSimulationTime < GrenadeLifeSpan) )			// when will the grenade explode due to cooking
	{
		//	debugf( TEXT( "RunPhysicsSimulationTilEnd Velocity X:%f, Y:%f, Z:%f,  IterationCount: %d, TimeTilExplosion %f, CurrSimulationTime %f" ), Velocity.X, Velocity.Y, Velocity.Z, IterationCount, TimeTilExplosion, CurrSimulationTime  );
		AGearProj_Grenade::physProjectile(SimFixedTimeStep, 0);

		// @note: Uncomment this to debug the actual trajectory arc
		// 
		//GWorld->LineBatcher->DrawLine(OldLocation, Location, FColor(255, 0, 0), SDPG_World);
		//OldLocation = Location;
		//
		for (INT ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
		{
			UActorComponent* Cpnt = Components(ComponentIndex);
			if (Cpnt != NULL)
			{
				UParticleSystemComponent* const PSC = Cast<UParticleSystemComponent>(Cpnt);
				if (PSC)
				{
					PSC->bJustAttached = TRUE;
				}

				Cpnt->ConditionalTick(SimFixedTimeStep);
			}
		}

		//IterationCount++;	
		CurrSimulationTime += SimFixedTimeStep;
	}

	// Need to tick the particle system components one last time to udpate the dynamic data
	for( INT ComponentIndex = 0; ComponentIndex < this->Components.Num(); ComponentIndex++ )
	{
		UParticleSystemComponent* PSC = Cast<UParticleSystemComponent>(this->Components(ComponentIndex));
		if( PSC != NULL )
		{  
			PSC->SetSkipUpdateDynamicDataDuringTick(FALSE);
			PSC->ConditionalTick(0.0f);
		}
	}
	//debugf( TEXT( "======= SIM COMPLETED ======= IterationCount: %d" ), IterationCount );
}


/************************************************************************************
 * AFlameThrowerSprayBase
 ***********************************************************************************/

void AFlameThrowerSprayBase::ParticleSystemCleanUp()
{
	// call ResetParticles on all contained PSCs
	for (INT BoneIdx=0; BoneIdx<BoneChain.Num(); ++BoneIdx)
	{
		FFlameBoneInfo& BoneInfo = BoneChain(BoneIdx);
		if (BoneInfo.BonePSC0)
		{
			BoneInfo.BonePSC0->ResetParticles(TRUE);
		}
		if (BoneInfo.BonePSC1)
		{
			BoneInfo.BonePSC1->ResetParticles(TRUE);
		}
	}

	if (SplashGlancingPSC)
	{
		SplashGlancingPSC->ResetParticles(TRUE);
	}
	if (SplashDirectPSC)
	{
		SplashDirectPSC->ResetParticles(TRUE);
	}
	if (SplashPawnPSC)
	{
		SplashPawnPSC->ResetParticles(TRUE);
	}
	if (SplashMaterialBasedPSC)
	{
		SplashMaterialBasedPSC->ResetParticles(TRUE);
	}

	if (PSC_OwnerGlow)
	{
		PSC_OwnerGlow->ResetParticles(TRUE);
	}
	if (StartFirePSC)
	{
		StartFirePSC->ResetParticles(TRUE);
	}
}


void AFlameThrowerSprayBase::DoFlameCollisionDetection(FLOAT DeltaTime)
{
	if ( bWaitingToDestroy || 
		 !bSkeletonHasBeenUpdated ||
		 bStasis ||
		 !SkeletalSprayMesh || 
		 !SkeletalSprayMesh->SkeletalMesh || 
		 (SkeletalSprayMesh->SkeletalMesh->RefSkeleton.Num() == 0) )
	{
		bFlameMeshCollidedThisTick = FALSE;
		return;
	}

	// if we hit a pawn last time, we'll do a special pawn-detecting NZE trace
	// since ZE traces suck for hitting pawns
	INT SegmentToDoExtraPawnTest = INDEX_NONE;
	if (bFlameMeshCollidedLastTick)
	{
		AGearPawn* LastHitPawn = Cast<AGearPawn>(HighestFlameMeshContactThisTick.Actor);
		if (LastHitPawn)
		{
			SegmentToDoExtraPawnTest = HighestFlameMeshContactThisTick.BoneChainIndex;
		}
	}

	// easy method, just do incremental traces between bones
	// walk down the chain, test each link
	// @optimize later if necessary

	TArray<FMeshBone> const& RefSkel = SkeletalSprayMesh->SkeletalMesh->RefSkeleton;

	if (!bDetached)
	{
		// if we're detached, we won't clear this.  we don't want the mesh to reexpand
		// once it detaches.
		bFlameMeshCollidedThisTick = FALSE;
	}

	// set up our trace flags
	DWORD TraceFlags = TRACE_ProjTargets | TRACE_Material;
	if (bTestCollideComplex)
	{
		TraceFlags |= TRACE_ComplexCollision;
	}
	// in case we tweak them later, we can restore
	DWORD const BaseTraceFlags = TraceFlags;

	//
	// First, make sure the path is clear from the pawn to the first chain bone.
	// It might not be, if the tip of the gun is penetrating the world, for instance.
	// 
	FVector const Bone0Loc = SkeletalSprayMesh->GetBoneMatrix(BoneChain(0).BoneIndex).GetOrigin();

	// @fixme, do all this stuff for bone 0 too!!
	//FLOAT const SeedLoc = BoneChain(ChainIdx).SeedChainLoc;
	//INT const SeedIdx = appTrunc(BoneChain(ChainIdx).SeedChainLoc);
	//FLOAT const SeedFraction = BoneChain(ChainIdx).SeedChainLoc - ((FLOAT)SeedIdx);

	//FVector LocBasedOnSeeds(0.f);
	//if (SeedIdx >= Seeds.Num()-1)
	//{
	//	LocBasedOnSeeds = Seeds.Last().Location;
	//}
	//else
	//{
	//	FVector const SeedDir = Seeds(SeedIdx+1).Location - Seeds(SeedIdx).Location;
	//	FLOAT const SeedDirMag = SeedDir.Size();
	//	LocBasedOnSeeds = Seeds(SeedIdx).Location + (SeedDir * SeedFraction / SeedDirMag);
	//}
	//BoneLoc = LocBasedOnSeeds;

	if (OwningGearPawn != NULL) 
	{
		FVector const StartTrace = OwningGearPawn->eventGetWeaponStartTraceLocation(OwningGearPawn->Weapon);

		FMemMark Mark(GMainThreadMemStack);
		FCheckResult *Hit = GWorld->MultiLineCheck(GMainThreadMemStack, Bone0Loc, StartTrace, FVector(0.f), TraceFlags/*^TRACE_SingleResult*/, this);
		
		if (Hit != NULL)
		{
			for(FCheckResult* Temp=Hit;Temp;Temp=Temp->GetNext())
			{
				if(Temp->Actor != NULL && (Temp->Actor->Base == OwningGearPawn || Temp->Actor == OwningGearPawn || Temp->Actor->IsA(ATrigger::StaticClass()) || Temp->Actor->IsA(AVolume::StaticClass()))) 
				{
					continue;
				}

				// record the hit
				bFlameMeshCollidedThisTick = TRUE;
				HighestFlameMeshContactThisTick.Actor = Temp->Actor;
				HighestFlameMeshContactThisTick.BoneChainIndex = INDEX_NONE;
				HighestFlameMeshContactThisTick.ContactNormal = Temp->Normal;
				HighestFlameMeshContactThisTick.ContactPosition = Temp->Location;
				HighestFlameMeshContactThisTick.PhysicalMaterial = Temp->PhysMaterial;

				if(bDebugShowCollision)
				{
					DrawDebugCoordinateSystem(Temp->Location,FRotator(0,0,0),100.f,FALSE);
				}
				break;
			}
		}
		Mark.Pop();
	}

	//
	// Now walk the bone chain, looking for world intersections.
	//
	if ( (bFlameMeshCollidedThisTick == FALSE) && (Seeds.Num() > 0) )
	{
		FVector PrevBoneLoc = Bone0Loc;
//		FVector PrevExtent(0.f);
			
		INT const NumChainBones = BoneChain.Num();
		for (INT ChainIdx=1; ChainIdx<NumChainBones; ChainIdx++)
		{
			FFlameBoneInfo const& BoneChainEntry = BoneChain(ChainIdx);
			//FMatrix const BoneMatrix = SkeletalSprayMesh->GetBoneMatrix(BoneChainEntry.BoneIndex);

			//// scale trace extents to match mesh scaling
			//FVector Extent = BoneChainEntry.TraceExtent + CurrentTraceExtentInflation;

			//FLOAT const Axis0Size = BoneMatrix.GetAxis(0).Size();
			//if (Axis0Size > KINDA_SMALL_NUMBER)
			//{
			//	Extent.X *= Axis0Size;
			//	Extent.Y *= BoneMatrix.GetAxis(1).Size();
			//	Extent.Z *= BoneMatrix.GetAxis(2).Size();
			//}
			//else
			//{
			//	// for bones scaled to zero, use the extent from the previous bone
			//	Extent = PrevExtent;
			//}

			FVector Extent(0.f);
			if (ChainIdx == SegmentToDoExtraPawnTest)
			{
				Extent = FVector(8.f,8.f,8.f);
				TraceFlags = TRACE_Actors | TRACE_ComplexCollision | TRACE_Material;
			}

			INT const SeedIdx = appTrunc(BoneChainEntry.SeedChainLoc);
			FVector BoneLoc;
			if (SeedIdx >= Seeds.Num()-1)
			{
				BoneLoc = Seeds.Last().Location;
			}
			else
			{
				FVector const SeedDelta = Seeds(SeedIdx+1).Location - Seeds(SeedIdx).Location;
				FLOAT const SeedFraction = BoneChainEntry.SeedChainLoc - ((FLOAT)SeedIdx);
				BoneLoc = Seeds(SeedIdx).Location + (SeedDelta * SeedFraction);
			}

			// trace from PrevBoneLoc to BoneLoc
			//FCheckResult Hit(1.f);
			//@fixme - extent traces won't collide with BlockWeapons since that disables BlockNonZero
			//UBOOL bHit = !GWorld->SingleLineCheck(Hit, this, BoneLoc, PrevBoneLoc, TraceFlags, Extent);
			// switched from single to multi so we can filter certain actors
#if !FINAL_RELEASE
			if (bDebugShowCollision)
			{
				if (Extent.IsZero())
				{
					DrawDebugLine(PrevBoneLoc,BoneLoc,255,255,255,FALSE);				
				}
				else
				{
					DrawDebugBox(BoneLoc, Extent, 255, 255, 255, FALSE);
				}
			}
#endif
			FMemMark Mark(GMainThreadMemStack);
			FCheckResult *Hit = GWorld->MultiLineCheck(GMainThreadMemStack, BoneLoc, PrevBoneLoc, Extent, TraceFlags/*^TRACE_SingleResult*/, this);
			if (Hit != NULL)
			{
				for(FCheckResult* Temp=Hit;Temp;Temp=Temp->GetNext())
				{
					if(Temp->Actor != NULL && (Temp->Actor->Base == OwningGearPawn || Temp->Actor == OwningGearPawn || Temp->Actor->IsA(ATrigger::StaticClass()) || Temp->Actor->IsA(AVolume::StaticClass()))) 
					{
						continue;
					}
					bFlameMeshCollidedThisTick = TRUE;
					HighestFlameMeshContactThisTick.Actor = Temp->Actor;
					HighestFlameMeshContactThisTick.BoneChainIndex = ChainIdx-1;
					HighestFlameMeshContactThisTick.ContactNormal = Temp->Normal;
					HighestFlameMeshContactThisTick.ContactPosition = Temp->Location;
					HighestFlameMeshContactThisTick.PhysicalMaterial = Temp->PhysMaterial;

	#if !FINAL_RELEASE
					if(bDebugShowCollision)
					{
						DrawDebugCoordinateSystem(Temp->Location,FRotator(0,0,0),100.f,FALSE);
					}
	#endif
					// since we're going front to end, we can safely bail on first hit
					Mark.Pop();
					return;
				}
			}
			Mark.Pop();

			if (ChainIdx == SegmentToDoExtraPawnTest)
			{
				// if we got here, extent test for pawn didn't hit anything
				// so drop back to zero-extent and do this trace again
				ChainIdx--;
				SegmentToDoExtraPawnTest = INDEX_NONE;
				TraceFlags = BaseTraceFlags;		// restore traceflags
			}
			else
			{
				// prime for next loop
				PrevBoneLoc = BoneLoc;
	//			PrevExtent = Extent;
			}
		}
	}
}

void AFlameThrowerSprayBase::DebugRenderBones()
{
	TArray<FMeshBone> const& RefSkel = SkeletalSprayMesh->SkeletalMesh->RefSkeleton;

	for (INT i=0; i<RefSkel.Num(); ++i)
	{
		INT R=255,G=255,B=0;

		for (INT BoneChainIdx=0; BoneChainIdx<BoneChain.Num(); ++BoneChainIdx)
		{
			if (BoneChain(BoneChainIdx).BoneIndex == i)
			{
				B = 255;
				break;
			}
		}

		FVector BoneLoc = SkeletalSprayMesh->GetBoneLocation(RefSkel(i).Name);
		DrawDebugBox(BoneLoc, FVector(4,4,4), R, G, B, FALSE);
	}
}


void AFlameThrowerSprayBase::UpdateFlameSeeds(FLOAT DeltaTime)
{
	// spew new seed
	// @fixme, try a ring buffer approach?
	// or at the very least, handle the array up-shifting in the for loop below
	// so we don't have to traverse it twice.

	if (!bWaitingToDestroy)
	{
		UBOOL const bARActive = (OwningGearPawn && OwningGearPawn->bActiveReloadBonusActive);
		FLOAT const Vel = bARActive ? SeedSprayVelAR : SeedSprayVel;
		FLOAT const Decel = bARActive ? SeedDecelAR : SeedDecel;

		// work on a fixed timestep so fps burps don't hose the flame sim
		SeedSimTimeRemaining += DeltaTime;

		FLOAT const SimTime = 1.f / SeedSimFreq;

		while (SeedSimTimeRemaining >= SimTime)
		{
			Seeds.Insert(0, 1);
			Seeds(0).Velocity = Vel * Rotation.Vector();
			Seeds(0).Location = Location;
			Seeds(0).Age = 0.f;

			FLOAT LengthSum = 0;
			FFlameSpraySeed const* PrevSeed = &Seeds(0);

			for (INT Idx=1; Idx<Seeds.Num(); ++Idx)
			{
				FFlameSpraySeed& Seed = Seeds(Idx);

				// update seed for time advance
				Seed.Age += SimTime;

				// deceleration on the particles, as an experiment.
				FLOAT const OldSeedSpeed = Seed.Velocity.Size();
				if (OldSeedSpeed > 0.f)
				{
					FLOAT NewSeedSpeed = OldSeedSpeed - Decel * SimTime;
					NewSeedSpeed = Max(NewSeedSpeed, 0.f);
					Seed.Velocity = Seed.Velocity * NewSeedSpeed / OldSeedSpeed;
					Seed.Location += Seed.Velocity * SimTime;
				}

				LengthSum += (PrevSeed->Location - Seed.Location).Size();

				if ( (Seed.Age >= SeedMaxAge) && (LengthSum > SeedMinChainLength) )
				{
					// expire all subsequent seeds
					Seeds.Remove(Idx, Seeds.Num()-Idx);
					break;
				}

				PrevSeed = &Seed;
			}

			SeedSimTimeRemaining -= SimTime;
		}

	#if !FINAL_RELEASE
		if (bDebugShowSeeds)
		{
			for (INT Idx=0; Idx<Seeds.Num(); ++Idx)
			{
				DrawDebugBox(Seeds(Idx).Location, FVector(4.f,4.f,4.f), 0, 255, 0);
			}
		}
	#endif
	}
}


void AFlameThrowerSprayBase::DestroyIfAllEmittersFinished()
{
	UBOOL bAllFinished = TRUE;

	bAllFinished = TRUE;

	// aer all of the bonespawns done?
	for (INT Idx=0; Idx<BoneChain.Num(); ++Idx)
	{
		FFlameBoneInfo& BI = BoneChain(Idx);
		if ( (BI.BonePSC0 && !BI.BonePSC0->HasCompleted()) ||
			 (BI.BonePSC1 && !BI.BonePSC1->HasCompleted()) )
		{
			bAllFinished = FALSE;
			break;
		}
	}

	// are the splashes done?
	if (bAllFinished)
	{
		if ( (SplashMaterialBasedPSC && !SplashMaterialBasedPSC->HasCompleted()) ||
			 (SplashGlancingPSC && !SplashGlancingPSC->HasCompleted()) ||
			 (SplashPawnPSC && !SplashPawnPSC->HasCompleted()) ||
			 (SplashDirectPSC && !SplashDirectPSC->HasCompleted()) )
		{
			bAllFinished = FALSE;
		}
	}

	if (bAllFinished)
	{
		Seeds.Empty();
		bWaitingToDestroy = FALSE;

		ClearTimer(FName(TEXT("DestroyIfAllEmittersFinished")));

		SetHidden(TRUE);
		bStasis = TRUE;
	}
}

FVector AGearWeap_GrenadeBase::GetPhysicalFireStartLoc(FVector AimDir)
{
	FVector				StartLoc, CamLoc(0.0f), Offset, FinalOffset, Test;
	ECoverDirection		CoverDir;
	ECoverAction		CoverAction;
	FRotator			CamRot;
	CamRot.Yaw = 0;
	CamRot.Pitch = 0;
	CamRot.Roll = 0;
	AGearPC*			PC;
	AGearPawn*			WP;
	FLOAT				Pct, NormPitch;

	ECoverType CoverType;

	// adjust based on cover direction

	WP = Cast<AGearPawn>(Instigator);

	if(WP == NULL || Mesh == NULL)
	{
		return FVector(0.f);
	}

	if( !bDisplayingArc )
	{
		// for blindfires and such, grenade comes from the hand
		WP->Mesh->GetSocketWorldLocationAndRotation(eventGetDefaultHandSocketName(WP), FinalOffset, NULL);

		// we need to trace from the actor origin though, to ensure hand isn't through something
		FCheckResult Hit(1.f);
		UBOOL bHitsWorld = !GWorld->SingleLineCheck(Hit, this, FinalOffset, WP->Location, TRACE_World);
		if( bHitsWorld )
		{
			FinalOffset = Hit.Location;
		}

		return FinalOffset;
	}

	// for targeted grenades with preview arc, grenade follows the arc.
	// note that this code also determines start loc of the arc.

	PC = Cast<AGearPC>(Instigator->Controller);
	if( PC == NULL )
	{
		return Super::GetPhysicalFireStartLoc(AimDir);
	}

	PC->eventGetPlayerViewPoint( CamLoc, CamRot );
	StartLoc = Instigator->Location;

	CoverDir	= (ECoverDirection)PC->GetCoverDirection();
	CoverAction	= (ECoverAction)WP->CoverAction;
	CoverType	= (ECoverType)WP->CoverType;

	Offset = FireStartOffset_Base;

	switch( CoverDir )
	{
		case CD_Left:
		case CD_Right:
			// this will happen during leans, as well as during blind tosses left and right
			// @fixme, blind tosses now use different code path, can we get rid of this?
			Offset += FireStartOffset_CoverLeftOrRight;
			break;
		default:
			// this actually gets applied not in cover as well
			Offset += FireStartOffset_CoverDefault;
			break;
	}

	switch( CoverAction )
	{
		case CA_LeanLeft:
		case CA_LeanRight:
			// applied when leaning out of cover
			Offset += (CoverType == CT_MidLevel) ? FireStartOffset_CoverLean_Low : FireStartOffset_CoverLean_High;
			break;
		case CA_BlindUp:
			// @fixme, blind tosses now use different code path, can we get rid of this?
			Offset += FireStartOffset_CoverBlindUp;
			break;
	}

	if( WP->bIsMirrored )
	{
		Offset.Y = -Offset.Y;
	}

	NormPitch = FRotator::NormalizeAxis(CamRot.Pitch);

	// in cover, adjust z a little to help out
	if( CoverType != CT_None )
	{
		if( CamRot.Pitch >= 0.f )
		{
			Pct	= NormPitch / Instigator->ViewPitchMax;
			Offset.Z += Lerp( FireStartOffset_CoverZOffset_Mid, FireStartOffset_CoverZOffset_Low, Pct );
		}
		else
		{
			Pct = NormPitch / Instigator->ViewPitchMin;
			Offset.Z += Lerp( FireStartOffset_CoverZOffset_Mid, FireStartOffset_CoverZOffset_High, Pct );
		}
	}

	// Move final offset away from the wall
	FinalOffset = StartLoc + FRotationMatrix(CamRot).TransformFVector(Offset);

	Test = Instigator->Location;
	Test.Z += Instigator->GetCylinderExtent().Z - 1.f;
	if( FinalOffset.Z < Test.Z )
	{
		Test.Z = FinalOffset.Z;
	}

	//FlushPersistentDebugLines();
	//DrawDebugLine(FinalOffset, Test, 255, 0, 0, TRUE);

	// Safer way, although more expensive! :|
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* FirstHit = GWorld->MultiLineCheck( GMainThreadMemStack, FinalOffset, Test, FVector(0,0,0),TRACE_ProjTargets|TRACE_World|TRACE_ComplexCollision,this );

	// Iterate over each thing we hit, adding an impulse to the components we hit.
	for( FCheckResult* Check = FirstHit; Check!=NULL; Check=Check->GetNext() )
	{
		// If that's not volume or trigger, then yes, go ahead. Otherwise, ignore, please! 
		// could actor be null and have valid location?
		if( Check->Actor && ( !Check->Actor->IsA(AVolume::StaticClass()) && !Check->Actor->IsA(ATrigger::StaticClass() ) ))
		{
			FinalOffset = Check->Location;
			break;
		}
	}

	Mark.Pop();

	return FinalOffset;
}

/** Grenades don't have muzzles so we just want the hand location**/
FVector AGearWeap_GrenadeBase::GetMuzzleLoc()
{
	FVector Loc, X, Y, Z;
	FRotator Rot;

	AGearPawn* GP = Cast<AGearPawn>(Instigator);
	if(  GP != NULL )
	{
		GP->GetWeaponHandPosition(Loc, Rot);
		FRotationMatrix(Rot).GetAxes( X, Y, Z );
		return (Loc + FireOffset.X*X + FireOffset.Y*Y + FireOffset.Z*Z);
	}
	else
	{
		return Location;
	}
}

extern FVector CalcAngularVelocity(FRotator const& OldRot, FRotator const& NewRot, FLOAT DeltaTime);

void AGearProj_HomingRocket::physProjectile(FLOAT DeltaTime, INT Iterations)
{
	if(!bUseInterpPhysics)
	{
		Super::physProjectile(DeltaTime, Iterations);
		return;
	}

	if(TargetActor && SourceActor && InterpTime > 0.f)
	{
		CheckStillInWorld();
		if(bDeleteMe || Physics != PHYS_Projectile)
		{
			return;
		}

		CurrentAlpha += (DeltaTime/InterpTime);

		FMatrix SourceL2W = SourceActor->LocalToWorld();
		SourceL2W.RemoveScaling();
		FVector WorldSpaceStart = SourceL2W.TransformFVector(SourcePosActorSpace);

		FVector WorldSpaceEnd = TargetActor->Location;

		FVector NewPos = Lerp(WorldSpaceStart, WorldSpaceEnd, CurrentAlpha);

		// INstead of using world-space velocity for rotation, orient along vector we are interpolating along
		FVector PointDir = WorldSpaceEnd - WorldSpaceStart;
		PointDir.X *= 0.5f; // Hack to not make rockets point up/down as much
		FRotator const NewRot = PointDir.Rotation();
		AngularVelocity = CalcAngularVelocity(Rotation, NewRot, DeltaTime);

		if( bCollideActors )
		{
			GrowCollision();
		}

		FVector StartLocation = Location;
		FCheckResult Hit(1.f);

		// If move is massive, teleport projectile. This is a hack to handle tunnels level where we teleport hydra/reaver across the level each time.
		if((NewPos - Location).SizeSquared() > 20000.f*20000.f)
		{
			GWorld->FarMoveActor(this, NewPos, FALSE, FALSE, FALSE);
			SetRotation(NewRot);
		}
		else
		{
			GWorld->MoveActor( this, NewPos - Location, NewRot, 0, Hit );	
		}

		if( bDeleteMe )
		{
			return;
		}

		if( Hit.Time < 1.f)
		{
			if( ShrinkCollision(Hit.Actor, Hit.Component, StartLocation) )
			{
				//
			}
			else
			{
				processHitWall(Hit, 0.0);
				if( bDeleteMe )
				{
					return;
				}
			}
		}
	}
}


void AGearProj_HomingRocket::TickSpecial( FLOAT DeltaSeconds )
{
	// Skip the orientation code in AGearPojectile
	AProjectile::TickSpecial(DeltaSeconds);
}

/** Ignore collisions with source */
UBOOL AGearProj_HomingRocket::IgnoreBlockingBy( const AActor *Other ) const
{
	if(SourceActor && Other == SourceActor)
	{
		return TRUE;
	}

	return Super::IgnoreBlockingBy(Other);
}

/** This will set the correct Material Paramters on the Overlay to make the snper work in 9:6 and 4:3 SP/SplitScreen **/
void AGearHUD_Base::SetSniperAspectRatio( UMaterialInstanceConstant* SniperOverlay )
{

/*
For:   1280x720    (One player Hi def)

Material:   Warfare_HUD.HUD_Sniper_overlay
			Parm Name:   SwitchHorizontalResolution   (set to =0)
			Parm Name:   SwitchVerticalResolution       (set to =0)
			-----------------------------------------------------------------------------------------

For:   960x720    (One player low def)

Material:   Warfare_HUD.HUD_Sniper_overlay

			Parm Name:   SwitchHorizontalResolution   (set to =1)
			Parm Name:   SwitchVerticalResolution        (set to =0)
			-----------------------------------------------------------------------------------------

For:   1280x360    (SplitScreen  Hi def)

Material:   Warfare_HUD.HUD_Sniper_overlay
			Parm Name:   SwitchHorizontalResolution   (set to =0)
			Parm Name:   SwitchVerticalResolution       (set to =1)
			-----------------------------------------------------------------------------------------

For:   960x360    (SplitScreen  Low def)

Material:   Warfare_HUD.HUD_Sniper_overlay
			Parm Name:   SwitchHorizontalResolution   (set to =1)
			Parm Name:   SwitchVerticalResolution       (set to =1)
*/

// so 9:6 and 4:3 vs splitscreen and SP

	extern INT GScreenWidth;
	INT SwitchHorizontalResolution = 0;
	INT SwitchVerticalResolution = 0;
	if( GEngine->IsSplitScreen() == TRUE )
	{
		if( GScreenWidth == 1280 )
		{
			SwitchHorizontalResolution = 0;
			SwitchVerticalResolution = 1;
		}
		// everything else gets the 4:3 ratio
		else
		{
			SwitchHorizontalResolution = 1;
			SwitchVerticalResolution = 1;
		}
	}
	else
	{
		if( GScreenWidth == 1280 )
		{
			SwitchHorizontalResolution = 0;
			SwitchVerticalResolution = 0;
		}
		// everything else gets the 4:3 ratio
		else
		{
			SwitchHorizontalResolution = 1;
			SwitchVerticalResolution = 0;
		}
	}

	SniperOverlay->SetScalarParameterValue(FName(TEXT("SwitchHorizontalResolution")), SwitchHorizontalResolution );
	SniperOverlay->SetScalarParameterValue(FName(TEXT("SwitchVerticalResolution")), SwitchVerticalResolution );
}

UBOOL AGearFogVolume_SmokeGrenade::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	return (TraceFlags & TRACE_GearAIVisibility) ? TRUE : FALSE;
}

UBOOL AGearDroppedPickup_Shield::ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags)
{
	return (bDeployed && (TraceFlags & TRACE_ComplexCollision)) || Super::ShouldTrace(Primitive,SourceActor,TraceFlags);
}


