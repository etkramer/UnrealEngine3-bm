/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Brumak_SideGun extends GearAI_Brumak_Slave
	native(AI)
	config(AI);

var() GearPawn_LocustBrumak_SideGun	Gun;

function Possess( Pawn NewPawn, bool bVehicleTransition )
{
	super.Possess( NewPawn, bVehicleTransition );

	Gun = GearPawn_LocustBrumak_SideGun(Pawn);
}

function String GetGunName()
{
	if( Gun.bIsLeftGun )
	{
		return "LEFT Brumak Gun";
	}
	return "RIGHT Brumak Gun";
}

function float RateEnemy( GearAI_Brumak AI, GearPawn EnemyWP )
{
	local float		Rating, Distance;
	local Vector	EnemyLocation, X, Y, Z;
	local Vector	VectToEnemy;
	local float		EnemyDotY;
	local Pawn		OtherEnemy;
	
	EnemyLocation = AI.GetEnemyLocation( EnemyWP );

	//debug
	`log( GetGunName()@"RateEnemy"@Enemy@EnemyWP, bDebug );

	// Make sure enemy is within FOV
	if( Gun.bIsLeftGun )
	{
		if( !Brumak.IsWithinLeftGunFOV(EnemyLocation) )
		{
			return 0.f;
		}
	}
	else
	{
		if( !Brumak.IsWithinRightGunFOV(EnemyLocation) )
		{
			return 0.f;
		}
	}

	// Base rating based on distance - closer == higher rating
	Distance = VSize( EnemyLocation - Pawn.GetWeaponStartTraceLocation() );
	Rating = 1.f - (Distance / AI.EnemyDistance_Long);

	//debug
	`log( GetGunName()@EnemyWP@"Base Rating"@Rating, bDebug );

	if (EnemyWP.IsHumanControlled())
	{
		Rating *= 1.5f;
		if (EnemyWP.IsDoingSpecialMove(SM_RoadieRun))
		{
			Rating *= 1.5f;
		}
	}
	
	//debug
	`log( GetGunName()@EnemyWP@"After Human"@Rating, bDebug );

	// Penalize enemies that would require this gun to shoot across the body
	GetAxes( Brumak.Rotation, X, Y, Z );
	VectToEnemy = Normal(EnemyLocation - Brumak.Location);
	EnemyDotY = VectToEnemy DOT Y;
	if( ( Gun.bIsLeftGun && EnemyDotY > 0.f) ||
		(!Gun.bIsLeftGun && EnemyDotY < 0.f) )
	{
		Rating *= 0.15f;
	}

	//debug
	`log( GetGunName()@EnemyWP@"After Side"@Rating, bDebug );
	
	// Prefer to shoot at a different enemy then the other gun
	OtherEnemy = Brumak.GetOtherGunEnemy( Gun );
	if( OtherEnemy == EnemyWP )
	{
		Rating *= 0.85f;
	}

	//debug
	`log( GetGunName()@EnemyWP@"After Other"@Rating, bDebug );

	// If enemy is not visible - ignore
	if( !AI.IsEnemyVisible( EnemyWP ) )
	{
		Rating *= 0.f;
	}

	//debug
	`log( GetGunName()@EnemyWP@"After Visible Enemy"@Rating, bDebug );

	return Rating;
}

function SetEnemy( Pawn NewEnemy )
{
	Super.SetEnemy( NewEnemy );

	if( !BrumakAI.bHijackedByLD )
	{
		// Update the brumak arm to look at this
		Brumak.SetSideGunAimTarget( Enemy, Gun.bIsLeftGun );
	
		if( Enemy == None )
		{
			ClearFirePattern();
		}
	}

	`log( GetGunName()@"Got enemy"@Enemy, bDebug );
}

function SetScriptedFireTarget( Actor Targ, optional bool bShouldFire, optional float Duration )
{
	Super.SetScriptedFireTarget( Targ, bShouldFire, Duration );

	if( Brumak != None )
	{
		Brumak.SetSideGunAimTarget( ScriptedFireTarget, Gun.bIsLeftGun );
	}
	ClearFirePattern();
}

function ClearFirePattern()
{
	if( BrumakAI != None && BrumakAI.FirePatternIdx >= 0 )
	{
		if( Gun.bIsLeftGun )
		{
			BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer = -1.f;
		}
		else
		{
			BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer = -1.f;
		}
	}
}

function Tick( float DeltaTime )
{
	local bool			bTimerActive;

	Super.Tick( DeltaTime );

	if( BrumakAI != None && !BrumakAI.bHijackedByLD )
	{
		// If we have an enemy, make sure he stays within FOV
		if( Enemy != None )
		{
			if( Gun.bIsLeftGun )
			{
				if( !Brumak.IsWithinLeftGunFOV(BrumakAI.GetEnemyLocation(Enemy)) )
				{
					NotifyEnemyOutsideOfGunFOV();
				}
			}
			else
			{
				if( !Brumak.IsWithinRightGunFOV(BrumakAI.GetEnemyLocation(Enemy)) )
				{
					NotifyEnemyOutsideOfGunFOV();
				}
			}
		}

		if(  BrumakAI.FirePatternIdx >= 0 && 
			!BrumakAI.IsFiringRockets() ) // Don't update timers when rockets are firing
		{
			if( Gun.bIsLeftGun )
			{
				bTimerActive = (BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer >= 0);
				if( bTimerActive )
				{
					//debug
					`log( "LEFT GUN TICK"@bTimerActive@BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer, bDebug );

					BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer -= DeltaTime;
					if( BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer < 0 )
					{
						//debug
						`log( "LEFT GUN FIRE", bDebug  );

						bAbleToFire = TRUE;
					}
				}
			}
			else
			{
				bTimerActive = (BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer >= 0);
				if( bTimerActive )
				{
					//debug
					`log( "RIGHT GUN TICK"@bTimerActive@BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer, bDebug );

					BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer -= DeltaTime;
					if( BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer < 0 )
					{
						//debug
						`log( "RIGHT GUN FIRE", bDebug  );

						bAbleToFire = TRUE;
					}
				}
			}
		}	
	}
}

function float GetTimerStatus()
{
	if( BrumakAI != None && BrumakAI.FirePatternIdx >= 0 )
	{
		if( Gun.bIsLeftGun )
		{
			return BrumakAI.FirePattern[BrumakAI.FirePatternIdx].LeftGunTimer;
		}
		return BrumakAI.FirePattern[BrumakAI.FirePatternIdx].RightGunTimer;
	}
	return -99999.f;
}

defaultproperties
{
	bIsPlayer=TRUE
}