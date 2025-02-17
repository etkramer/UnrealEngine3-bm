/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class FlockTestLocust extends CrowdAgent
	native;

/** Cached pointer to gears-specific spawner. */
var	SeqAct_FlockSpawner	FlockSpawner;
/** Where our action effects are targetted. */
var vector	FireTarget;
/** World time when PlayWeaponFire should be called. */
var	float	FireTime;
/** A PlayWeaponFire call is pending. */
var	bool	bFirePending;

cpptext
{
	virtual void SpawnActionEffect(const FVector& ActionTarget);
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual void PlayWeaponFire();
};



/** Allows you to kill crowd members. */
simulated function TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local Emitter ImpactEmitter;
	local GearPC PC;

	Health -= DamageAmount;

	if(Health <= 0)
	{
		if(FlockSpawner == None)
		{
			FlockSpawner = SeqAct_FlockSpawner(Spawner);
		}

		PC = GearPC(EventInstigator);
		if (Role == ROLE_Authority && PC != None && (FlockSpawner != None && FlockSpawner.bCountForSeriouslyAchievement))
		{
			PC.UpdateSeriously();
		}
		// If no death anims, or explosive damage, use explosive death
		if(Spawner.DeathAnimNames.length == 0 || ClassIsChildOf(DamageType, class'GDT_Explosive') || ClassIsChildOf(DamageType, class'GDT_HeavyMiniGun') || ClassIsChildOf(DamageType, class'GDT_BrumakBullet'))
		{
			if( WorldInfo.GRI.ShouldShowGore() )
			{
				ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(Spawner.ExplosiveDeathEffect, Location, Rotation);
			}
			else
			{
				ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(Spawner.ExplosiveDeathEffectNonExtremeContent, Location, Rotation);
			}

			ImpactEmitter.SetDrawScale(Spawner.ExplosiveDeathEffectScale);
			ImpactEmitter.ParticleSystemComponent.ActivateSystem();
			Destroy();
		}
		else
		{
			PlayDeath();
			SetCollision(FALSE, FALSE, FALSE); // Turn off all collision when dead.
		}
	}
}

/** Look for being run over by a vehicle. */
simulated event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	if(Vehicle(Other) != None)
	{
		PlayDeath();
		SetCollision(FALSE, FALSE, FALSE); // Turn off all collision when dead.
	}
}


/** Called when crowd agent overlaps something in the ReportOverlapsWithClass list */
event OverlappedActorEvent(Actor A)
{
	local Actor TraceActor;
	local vector out_HitLocation, out_HitNormal;
	local vector TraceStart, TraceEnd;
	local TraceHitInfo HitInfo;
	local GearDecal GD;
	local Emitter ExplodeEmitter;
	local float Distance2D;

	// If lifespan already shortened, skip
	if(LifeSpan != 0.0 || Health == 0)
	{
		return;
	}	

	// Get our SeqAct_FlockSpawner
	if(FlockSpawner == None)
	{
		FlockSpawner = SeqAct_FlockSpawner(Spawner);
	}

	if(FlockSpawner == None)
	{
		return;
	}

	// check speed
	if(VSize2D(A.Velocity) < 10.0)
	{
		return;
	}

	// Check close enough
	Distance2D = VSize2D(Location - A.Location);
	if(Distance2D > FlockSpawner.OverlapDeathRadius)
	{
		return;
	}

	// If desired, play death anim on overlap
	if(FlockSpawner.bOverlapDeathPlayAnim)
	{
		PlayDeath();
		SetCollision(FALSE, FALSE, FALSE); // Turn off all collision when dead.
		Health = 0;
		return;
	}

	// Kill it!
	LifeSpan = 0.001f;

	// Spawn decal
	if(FlockSpawner.OverlapDeathDecalInfo.DecalMaterial != None)
	{
		// Trace to find spawn location
		TraceStart = Location + vect(0,0,100);
		TraceEnd = Location - vect(0,0,100);
		TraceActor = Trace( out_HitLocation, out_HitNormal, TraceEnd, TraceStart, FALSE, vect(0,0,0), HitInfo );
		if( TraceActor != None )
		{

			// create GearDecal
			GD = GearGRI(WorldInfo.GRI).GOP.GetDecal_Blood( out_HitLocation );
			if( GD != none )
			{
				// Create material
				GD.MITV_Decal.SetParent( FlockSpawner.OverlapDeathDecalInfo.DecalMaterial );

				// Set up
				WorldInfo.MyDecalManager.SetDecalParameters( 
					GD, 
					GD.MITV_Decal, 
					out_HitLocation, 
					rotator(-out_HitNormal), 
					FlockSpawner.OverlapDeathDecalInfo.Width, 
					FlockSpawner.OverlapDeathDecalInfo.Height, 
					FlockSpawner.OverlapDeathDecalInfo.Thickness, 
					!FlockSpawner.OverlapDeathDecalInfo.ClipDecalsUsingFastPath, 
					FRand() * 360.0f, 
					HitInfo.HitComponent, 
					TRUE, 
					FALSE, 
					HitInfo.BoneName, 
					INDEX_NONE, 
					INDEX_NONE, 
					FlockSpawner.OverlapDeathDecalInfo.LifeSpan,
					INDEX_NONE,
					class'GearDecal'.default.DepthBias,
					class'GearDecal'.default.BlendRange
					);

				GearGRI(WorldInfo.GRI).GOP.AttachComponent(GD);

				GD.MITV_Decal.SetDuration( FlockSpawner.OverlapDeathDecalInfo.LifeSpan );

				//DrawDebugCoordinateSystem( out_HitLocation, rotator(-out_HitNormal), 3.0f, TRUE );
			}
		}
	}

	// Play sound
	if(FlockSpawner.OverlapDeathSound != None)
	{
		PlaySound(FlockSpawner.OverlapDeathSound, TRUE, TRUE, FALSE, Location, TRUE);
	}

	// Play effect
	if(FlockSpawner.OverlapDeathEffect != None)
	{
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( FlockSpawner.OverlapDeathEffect, Location, rotator(vect(0,0,1)) );
		ExplodeEmitter.ParticleSystemComponent.ActivateSystem();
	}
}