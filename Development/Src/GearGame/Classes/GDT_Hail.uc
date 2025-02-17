/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GDT_Hail extends GDT_Environment;

static function ModifyDamage(Pawn Victim, Controller InstigatedBy, out int Damage, TraceHitInfo HitInfo, vector HitLocation, vector Momentum)
{
	local GearPawn GP;
	GP = GearPawn(Victim);
	if (GP != None && GP.IsCarryingShield())
	{
		if ( GP.IsDoingSpecialMove(SM_RaiseShieldOverHead) || GP.TryToRaiseShieldOverHead() )
		{
			GP.LastAttemptedHailDamageTime = GP.WorldInfo.TimeSeconds;

			// shield protects from hail!
			Damage = 0.f;
			return;
		}
	}

	// in non MP games, 
	// don't let AI dudes in the player's squad get hurt by hail
	if(GP != none && 
		!GP.WorldInfo.GRI.IsMultiplayerGame() && 
		GP.GetTeamNum() == TEAM_COG &&
		!GP.IsHumanControlled() &&
		GP.MyGearAI != none &&
		GP.MyGearAI.Squad != none &&
		GP.MyGearAI.Squad.bPlayerSquad)
	{
		Damage = 0;
		return;
	}

	Super.ModifyDamage(Victim, InstigatedBy, Damage, HitInfo, HitLocation, Momentum);
}


/** Overridden to enforce default gibbing for explosive damage. */
static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return TRUE;
}

/** Should the Pawn transition to DBNO when killed with this damage type? */
static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	return FALSE;
}

/** This will spawn blood effects on the top torso portion of the pawn **/
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local Emitter BloodEmitter;
	local vector SpawnLocation, SocketLocation;
	local rotator SpawnRotation;
	local rotator SocketRotation;
	local bool bSocketFound;
	local float LocationPercent;
	local GearGRI GearGRI;
	local Pawn FakeInstigator;

	Super.HandleDamageFX( DamagedPawn, HitInfo );

	// okie for hail we will not get a valid hit location as the damage causer and hit data is from a volume (so we don't get a hit bone or anything really)
	// so we are going to use the head, and shoulder dust socket for the places to spawn the impact effect with some translation up

	GearGRI = GearGRI(DamagedPawn.WorldInfo.GRI);
	FakeInstigator = None;

	if( GearGRI.IsEffectRelevant( FakeInstigator, DamagedPawn.Location, 1024, FALSE, GearGRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		LocationPercent = FRand();

		// play hail impact
		if( LocationPercent > 0.75f )
		{
			bSocketFound = DamagedPawn.Mesh.GetSocketWorldLocationAndRotation( 'LeftShoulderCoverDust', SocketLocation );
			SpawnLocation = SocketLocation + vect(0,0,8);
			SpawnRotation = rot(0,0,1);
			//DamagedPawn.DrawDebugCoordinateSystem( SpawnLocation, SpawnRotation, 10.0f, TRUE );
		}
		else if( LocationPercent > 0.50f )
		{
			bSocketFound = DamagedPawn.Mesh.GetSocketWorldLocationAndRotation( 'RightShoulderCoverDust', SocketLocation );
			SpawnLocation = SocketLocation + vect(0,0,8);
			SpawnRotation = rot(0,0,1);
			//DamagedPawn.DrawDebugCoordinateSystem( SpawnLocation, SpawnRotation, 10.0f, TRUE );
		}
		// spawn on back which may be off on some pawns but looks decent enough for blood effect splattering
		else if( LocationPercent > 0.75f )
		{
			bSocketFound = DamagedPawn.Mesh.GetSocketWorldLocationAndRotation( DamagedPawn.HeadSocketName, SocketLocation, SocketRotation );
			SpawnLocation = SocketLocation + vect(0,0,5) + ( vector(DamagedPawn.Rotation) * -20.0f);
			SpawnRotation = rot(0,0,1);
			//DamagedPawn.DrawDebugCoordinateSystem( SpawnLocation, SpawnRotation, 10.0f, TRUE );
		}
		else
		{
			bSocketFound = DamagedPawn.Mesh.GetSocketWorldLocationAndRotation( DamagedPawn.HeadSocketName, SocketLocation );
			SpawnLocation = SocketLocation + vect(0,0,10);
			SpawnRotation = rot(0,0,1);
			//DamagedPawn.DrawDebugCoordinateSystem( SpawnLocation, SpawnRotation, 10.0f, TRUE );
		}


		if( bSocketFound == TRUE )
		{
			BloodEmitter = GearGRI.GOP.GetImpactEmitter( ParticleSystem'War_HailEffects.Effects.P_FX_Hail_PlayerImpact', SpawnLocation, SpawnRotation );
			BloodEmitter.ParticleSystemComponent.ActivateSystem();
		}
	}
}





defaultproperties
{
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.HUD.Gears_DeathIcons',U=78,V=150,UL=77,VL=29)
}
