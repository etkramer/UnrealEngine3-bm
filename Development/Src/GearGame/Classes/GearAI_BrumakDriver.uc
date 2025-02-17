/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */ 
class GearAI_BrumakDriver extends GearAI_Brumak_Slave
	native(AI)
	config(AI);

/** GoW global macros */

/** Pass on sight information from driver to brumak */
event SeePlayer( Pawn Seen )
{
	if( Brumak != None && Brumak.Controller != None )
	{
		Brumak.Controller.SeePlayer( Seen );
	}
}

function String GetGunName()
{
	return "MAIN Brumak Gun";
}

function float RateEnemy( GearAI_Brumak AI, GearPawn EnemyWP )
{
	local float Rating, Distance;
	local Vector EnemyLocation;

	EnemyLocation = AI.GetEnemyLocation( EnemyWP );

	//debug
	`log( GetGunName()@"RateEnemy"@Enemy@EnemyWP, bDebug );

	// Make sure Enemy can be targeted
	if( !Brumak.IsWithinMainGunFOV(EnemyLocation) )
	{
		return 0.f;
	}

	// Base rating by distance
	Distance = VSize( EnemyLocation - Pawn.GetWeaponStartTraceLocation() );
	Rating = 1.f - (Distance / AI.EnemyDistance_Long);

	//debug
	`log( GetGunName()@EnemyWP@"Base Rating"@Rating, bDebug );

	// If it's the current enemy
	if( EnemyWP == Enemy )
	{
		// Probably prefer to switch
		Rating *= 0.5 + FRand()*0.5;
	}

	//debug
	`log( GetGunName()@EnemyWP@"After Cur Enemy"@Rating, bDebug );

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
	local Pawn OldEnemy;
	
	OldEnemy = Enemy;

	Super.SetEnemy( NewEnemy );

	if( !BrumakAI.bHijackedByLD )
	{
		// Update the brumak cannon to look at this
		Brumak.SetMainGunAimTarget( Enemy );
		if( Enemy == None && OldEnemy != Enemy )
		{
			ClearFirePattern();
		}
	}

	//debug
	`log( GetGunName()@"Got enemy"@Enemy, bDebug );
}

function SetScriptedFireTarget( Actor Targ, optional bool bShouldFire, optional float Duration )
{
	Super.SetScriptedFireTarget( Targ, bShouldFire, Duration );

	Brumak.SetMainGunAimTarget( ScriptedFireTarget );
	ClearFirePattern();
}

state RocketAttack
{
	function ClearFirePattern();

	function float RateEnemy( GearAI_Brumak AI, GearPawn EnemyWP )
	{
		local float	 Rating;
		local Vector EnemyLocation;

		EnemyLocation = AI.GetEnemyLocation( EnemyWP );

		//debug
		`log( GetGunName()@"RateEnemy"@Enemy@EnemyWP@GetStateName(), bDebug );

		// Make sure Enemy can be targeted
		if( !Brumak.IsWithinMainGunFOV(EnemyLocation) )
		{
			return 0.f;
		}

		Rating = FRand();

		//debug
		`log( GetGunName()@EnemyWP@"Rand"@Rating@GetStateName(), bDebug );

		return Rating;
	}
Begin:
	//debug
	`log( GetGunName()@GetStateName()@"BEGIN TAG"@Enemy@BrumakAI.Enemy, bDebug );

	ClearTimer( 'SelectEnemy' );
	ClearTimer( 'RefireWeapon' );
	StopFiring();

	// Enemy is selected by GearAI_Brumak before calling this state.
	if( BrumakAI.FireTarget == None )
	{
		`log( GetGunName()@GetStateName()@"Failed to find fire target", bDebug );
		StopFiring();
		Sleep( 1.f );
		Goto( 'End' );
	}

	Sleep( 2.5f );
	Pawn.StartFire( class'GearWeapon'.const.ALTFIRE_FIREMODE ); // Brumak rocket mode
	do
	{
		Sleep( 0.25f );
	} until( Pawn == None || !Pawn.Weapon.IsFiring() );
	StopFiring();
	Sleep( 2.5f );

End:
	SetScriptedFireTarget( None, FALSE );
	GotoState( 'Active' );
}

function ClearFirePattern()
{
	//debug
	`log( GetGunName()@GetFuncName()@BrumakAI.FirePatternIdx, bDebug );

	if( BrumakAI != None && BrumakAI.FirePatternIdx >= 0 )
	{
		BrumakAI.FirePattern[BrumakAI.FirePatternIdx].MainGunTimer  = -1.f;
		BrumakAI.FirePattern[BrumakAI.FirePatternIdx].SuperGunTimer = -1.f;
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
			if( !Brumak.IsWithinMainGunFOV(BrumakAI.GetEnemyLocation(Enemy)) )
			{
				NotifyEnemyOutsideOfGunFOV();
			}
		}

		if(  BrumakAI.FirePatternIdx >= 0 && 
			!BrumakAI.IsFiringRockets() ) // Don't update timers when rockets are firing
		{
			bTimerActive = (BrumakAI.FirePattern[BrumakAI.FirePatternIdx].MainGunTimer >= 0);
			if( bTimerActive )
			{
				BrumakAI.FirePattern[BrumakAI.FirePatternIdx].MainGunTimer -= DeltaTime;
				if( BrumakAI.FirePattern[BrumakAI.FirePatternIdx].MainGunTimer < 0 )
				{
					bAbleToFire = TRUE;
				}
			}

			bTimerActive = (BrumakAI.FirePattern[BrumakAI.FirePatternIdx].SuperGunTimer >= 0);
			if( bTimerActive )
			{
				BrumakAI.FirePattern[BrumakAI.FirePatternIdx].SuperGunTimer -= DeltaTime;
				if( BrumakAI.FirePattern[BrumakAI.FirePatternIdx].SuperGunTimer < 0 )
				{
					FireRockets( BrumakAI );
				}
			}
		}
	}
}

function FireRockets( GearAI_Brumak AI )
{
	//debug
	`log( GetFuncName()@AI@ScriptedFireTarget@Brumak.IsDoingASpecialMove(), bDebug );

	if( !Brumak.IsDoingASpecialMove() )
	{
		class'AICmd_Attack_Brumak_Rockets'.static.InitCommand( AI );
	}
}

function float GetTimerStatus()
{
	if( BrumakAI != None && BrumakAI.FirePatternIdx >= 0 )
	{
		return BrumakAI.FirePattern[BrumakAI.FirePatternIdx].MainGunTimer;
	}
	return -99999.f;
}

defaultproperties
{
	bIsPlayer=TRUE
	bGodMode=TRUE // Start in god mode, disabled when guns are destroyed

//	bDebug=TRUE
}