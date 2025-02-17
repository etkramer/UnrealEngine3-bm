/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Brumak extends GearAI
	native(AI)
	config(AI);

var() GearPawn_LocustBrumakBase	Brumak;
var() GearAI_BrumakDriver		DriverAI;
var() GearAI_Brumak_SideGun		LeftGunAI,
								RightGunAI;

var() bool						bHijackedByLD;

/** Rough offset from brumak location of the guns */
var Vector						MainGunFireOffset,
								LeftGunFireOffset,
								RightGunFireOffset;

/** Last time we completed a move to adjust to enemy locations */
var				float			LastMoveTime;

enum EBrumakSearchType
{
	BrumakSearchType_All,
	BrumakSearchType_Direct,
	BrumakSearchType_Flank,
};
/** List of active enemies for pathfinding use */
var array<Pawn>	Internal_ActiveList;

/** Fire patterns */
struct native BrumakFirePattern
{
	var()	float	LeftGunFireTime,	LeftGunTimer;
	var()	float	RightGunFireTime,	RightGunTimer;
	var()	float	MainGunFireTime,	MainGunTimer;
	var()	float	SuperGunFireTime,	SuperGunTimer;
	var()	float	DelayEndTime,		DelayEndTimer;
	var()	Name	PatternName;

	structdefaultproperties
	{
		LeftGunFireTime=-1.f
		RightGunFireTime=-1.f
		MainGunFireTime=-1.f
		SuperGunFireTime=-1.f
		DelayEndTime=-1.f
		LeftGunTimer=-1.f
		RightGunTimer=-1.f
		MainGunTimer=-1.f
		SuperGunTimer=-1.f
		DelayEndTimer=-1.f
	}
};
var()	array<BrumakFirePattern>	FirePattern;
var()	int							FirePatternIdx;
var()	Vector2D					FirePatternDelayRange;


struct native BrumakMeleeAttackDef
{
	/** Special move to use for attack */
	var()	ESpecialMove	AttackSpecialMove;
	/** If brumak can do this attack while moving, or if he has to stop. */
	var()	bool			bRequiresBrumakToStop;
	/** Chance to be picked. Higher means more chances. */
	var()	float			Chance;
};

var		BrumakMeleeAttackDef		CurrentBrumakMeleeAttack;
var		Array<BrumakMeleeAttackDef>	BrumakMeleeAttackList;

/** Linearly loop through available fire patterns */
var(Debug) bool bDebugCyclePatterns;

/** BEGIN IGNORED FUNCTIONS */
function bool IsTooLongSinceEnemySeen( Pawn EnemyPawn );
function SetupEnemyDistances(optional GearWeapon Wpn);
/** END IGNORED FUNCTIONS */

/** Get list of current enemies held by weapons */
native function bool GetActiveEnemyList( out array<Pawn> ActiveList );
/** Check to see if specific gun can fire at a target */
native function bool CanFireAtTarget( GearAI_Brumak_Slave GunAI, optional Pawn TestEnemy, optional Vector TestBaseLocation, optional Vector GunOffset, optional Rotator PawnRot );
/** Check to see if we can fire at any active enemies */
native function bool CanFireAtAnyTarget( optional Pawn TestEnemy, optional Vector TestBaseLocation, optional Rotator PawnRot );
/** Gets average vector between the list of pawns */
native function Vector GetVectorAvg( array<Pawn> PawnList, Vector TestBaseLocation );

simulated function DrawDebug(GearHUD_Base HUD, Name Category)
{
	local vector GroundLocation;
	//local Vector AIAimOffsetViewLocation;

	Super.DrawDebug( HUD, Category );

	if(Category == 'Default')
	{
		DrawDebugBox( Pawn.Location, Pawn.GetCollisionExtent(), 255, 0, 0 );

		GearWeapon(DriverAI.Pawn.Weapon).DebugDrawFireLocation();

		if( DriverAI != None && DriverAI.Pawn != None && DriverAI.Enemy != None )
		{
			// RED
			DrawDebugLine( DriverAI.Pawn.GetPawnViewLocation(), GetEnemyLocation(DriverAI.Enemy, LT_InterpVisibility ), 255, 0, 0 );
		}
		if( LeftGunAI != None && LeftGunAI.Pawn != None && LeftGunAI.Enemy != None )
		{
			// RED
			DrawDebugLine( LeftGunAI.Pawn.GetPawnViewLocation(), GetEnemyLocation(LeftGunAI.Enemy, LT_InterpVisibility ), 255, 0, 0 );
		}
		if( RightGunAI != None && RightGunAI.Pawn != None && RightGunAI.Enemy != None )
		{
			// RED
			DrawDebugLine( RightGunAI.Pawn.GetPawnViewLocation(), GetEnemyLocation(RightGunAI.Enemy, LT_InterpVisibility ), 255, 0, 0 );
		}


		// Aim Offset/Bone Controllers
	//	AIAimOffsetViewLocation = Brumak.GetAIAimOffsetViewLocation();
		if( Brumak != None )
		{
			if( Brumak.LeftGunAimController != None && Brumak.LeftGunAimController.LookAtAlpha > 0.f )
			{
				if( Brumak.IsWithinLeftGunFOV( Brumak.GetGunLookTargetLocation( Brumak.LeftGunTarget, Brumak.LeftGunAimController ) ) )
				{
					DrawDebugLine(LeftGunAI.Pawn.GetPawnViewLocation(), Brumak.LeftGunAimController.TargetLocation, 0, 255, 0 );
				}
				else
				{
					DrawDebugLine(LeftGunAI.Pawn.GetPawnViewLocation(), Brumak.LeftGunAimController.TargetLocation, 255, 0, 0 );
				}
			}
			if( Brumak.RightGunAimController != None && Brumak.RightGunAimController.LookAtAlpha > 0.f )
			{
				if( Brumak.IsWithinRightGunFOV( Brumak.GetGunLookTargetLocation( Brumak.RightGunTarget, Brumak.RightGunAimController ) ) )
				{
					DrawDebugLine(RightGunAI.Pawn.GetPawnViewLocation(), Brumak.RightGunAimController.TargetLocation, 0, 255, 0 );
				}
				else
				{
					DrawDebugLine(RightGunAI.Pawn.GetPawnViewLocation(), Brumak.RightGunAimController.TargetLocation, 255, 0, 0 );
				}				
			}
			if( Brumak.MainGunAimController1 != None && Brumak.MainGunAimController1.LookAtAlpha > 0.f )
			{
				if( Brumak.IsWithinMainGunFOV( Brumak.GetGunLookTargetLocation( Brumak.MainGunTarget, Brumak.MainGunAimController1 ) ) )
				{
					DrawDebugLine(DriverAI.Pawn.GetPawnViewLocation(), Brumak.MainGunAimController1.TargetLocation, 0, 255, 0 );
				}
				else
				{
					DrawDebugLine(DriverAI.Pawn.GetPawnViewLocation(), Brumak.MainGunAimController1.TargetLocation, 255, 0, 0 );
				}
			}

			// Debug FOV Cone
			// Right side gun
	//		DrawDebugGunCone(Brumak.SideGunOriginOffset, Brumak.SideGunConeAngle);
			// Left Side gun
	//		DrawDebugGunCone(Brumak.SideGunOriginOffset, Brumak.SideGunConeAngle, TRUE);
			// Main gun
	//		DrawDebugGunCone(Brumak.MainGunOriginOffset, Brumak.MainGunConeAngle);
		}

		if( Pawn != None )
		{
			GroundLocation = Pawn.Location - (Pawn.CylinderComponent.CollisionHeight - 5) * vect(0,0,1);

			DrawDebugLine( Pawn.Location, Pawn.Location + vector(Pawn.DesiredRotation)*1024, 255, 128, 0);		// orange
			DrawDebugLine( Pawn.Location, Pawn.Location + vector(DesiredRotation)*1024, 0, 255, 255 );			// teal
			DrawDebugCoordinateSystem(GroundLocation, Pawn.Rotation, 768);

			// Draw enemy distance thresholds
			/*
			DrawDebugSphere(Pawn.Location, EnemyDistance_Melee,		8, 255, 000, 000);
			DrawDebugSphere(Pawn.Location, EnemyDistance_Short,		8, 000, 255, 000);
			DrawDebugSphere(Pawn.Location, EnemyDistance_Medium,	8, 000, 000, 255);
			DrawDebugSphere(Pawn.Location, EnemyDistance_Long,		8, 000, 255, 255);
			*/
		}
	}
}

// Gun FOV Debugging
final function DrawDebugGunCone(Vector InOffset, float InDegAngle, optional bool bMirrorYOffset)
{
	local Vector	OutOrigin, OutOrientation;
	local float		OutRadAngle;

	// Get FOV parameters.
	Brumak.GetGunFOVParameters(InOffset, InDegAngle, OutOrigin, OutOrientation, OutRadAngle, bMirrorYOffset);
	// Draw Cone
	DrawDebugCone(OutOrigin, OutOrientation, EnemyDistance_Medium, OutRadAngle, OutRadAngle, 24, MakeColor(0, 255, 0));
}

function Possess( Pawn NewPawn, bool bVehicleTransition )
{
	super.Possess( NewPawn, bVehicleTransition );

	Brumak		= GearPawn_LocustBrumakBase(Pawn);
	DriverAI	= GearAI_BrumakDriver(Brumak.Driver.Controller);
	LeftGunAI	= GearAI_Brumak_SideGun(Brumak.LeftGunPawn.Controller);
	RightGunAI	= GearAI_Brumak_SideGun(Brumak.RightGunPawn.Controller);

	MainGunFireOffset	= (DriverAI.Pawn.GetWeaponStartTraceLocation()   - Brumak.Location) << Brumak.Rotation;
	LeftGunFireOffset	= (LeftGunAI.Pawn.GetWeaponStartTraceLocation()  - Brumak.Location) << Brumak.Rotation;
	RightGunFireOffset	= (RightGunAI.Pawn.GetWeaponStartTraceLocation() - Brumak.Location) << Brumak.Rotation;
}

final function OnBrumakControl( SeqAct_BrumakControl inAction )
{
	local int	Idx;
	local Actor Targ;
	local bool	bClearLatent, bAborted;

	//debug
	`AILog( GetFuncName()@inAction@inAction.bFire@"0"@inAction.InputLinks[0].bHasImpulse@"1"@inAction.InputLinks[1].bHasImpulse@"2"@inAction.InputLinks[2].bHasImpulse@"3"@inAction.InputLinks[3].bHasImpulse@"4"@inAction.InputLinks[4].bHasImpulse  );
	`AILog( "SpecialMove:"@Brumak.SpecialMove );

	SetFirePattern( -1 );
	bHijackedByLD = TRUE;
	ReactionManager.SuppressAll();

	// Increase turn rate for hijacked brumak
	RotationRate = rot(10000,10000,10000);
	Brumak.TurnPlayer.Rate = 1.5f;

	for( Idx = 0; Idx < inAction.TargetList.Length; Idx++ )
	{
		Targ = GetFinalTarget( inAction.TargetList[Idx] );

		//debug
		//`AILog( "Get Targ"@Idx@inAction.TargetList[Idx]@Targ );

		if( Targ != None )
		{
			break;
		}
	}

	// Side Guns ...
	if( inAction.InputLinks[0].bHasImpulse )
	{
		//debug
		`AILog( "SIDE GUN"@Targ@Brumak.IsDoingASpecialMove() );

		if( Targ != None )
		{
			if( inAction.bSetFocus )
			{
				Focus = Targ;
			}			
			LeftGunAI.SetScriptedFireTarget( Targ, inAction.bFire, inAction.FireDuration );
			RightGunAI.SetScriptedFireTarget( Targ, inAction.bFire, inAction.FireDuration );
			bClearLatent = (inAction.FireDuration <= 0.f);
		}
		else
		{
			FireTarget = None;
			Focus = None;
			LeftGunAI.SetScriptedFireTarget( None );
			RightGunAI.SetScriptedFireTarget( None );
			bClearLatent = TRUE;
		}
	}
	else
	// Main Gun ...
	if( inAction.InputLinks[1].bHasImpulse )
	{
		//debug
		`AILog( "MAINGUN"@Targ@Brumak.IsDoingASpecialMove() );

		if( Targ != None )
		{
			if( inAction.bSetFocus )
			{
				Focus = Targ;
			}
			DriverAI.SetScriptedFireTarget( Targ, inAction.bFire, inAction.FireDuration );
			bClearLatent = (inAction.FireDuration <= 0.f);
		}
		else
		{
			FireTarget = None;
			Focus = None;
			DriverAI.SetScriptedFireTarget( None );
			bClearLatent = TRUE;
		}		
	}
	else
	// Rockets ...
	if( inAction.InputLinks[2].bHasImpulse )
	{
		//debug
		`AILog( "ROCKETS"@Targ@Brumak.IsDoingASpecialMove() );

		if( Targ != None && !Brumak.IsDoingASpecialMove() )
		{
			Focus = Targ;
			DriverAI.SetScriptedFireTarget( Targ, FALSE );
			DriverAI.FireRockets( self );
		}
		else
		{
			bClearLatent = TRUE;
			bAborted	 = TRUE;
		}
	}
	else
	// Roar ...
	if( inAction.InputLinks[3].bHasImpulse )
	{
		//debug
		`AILog( "ROAR"@Targ@Brumak.IsDoingASpecialMove() );
	
		if( Targ != None && !Brumak.IsDoingASpecialMove() )
		{
			Focus = Targ;
			FireTarget = Targ;
			Brumak.PlayROAR();
		}
		else
		{
			bClearLatent = TRUE;
			bAborted	 = TRUE;
		}
	}
	else
	// Flinch ...
	if( inAction.InputLinks[4].bHasImpulse )
	{
		//debug
		`AILog( "FLINCH" );

		bClearLatent = !Brumak.PlayStumbleAnim( TRUE );
		bAborted	 = bClearLatent;
	}
	else
	// Release ...
	if( inAction.InputLinks[5].bHasImpulse )
	{
		//debug
		`AILog( "RELEASE" );

		bHijackedByLD = FALSE;
		ReactionManager.UnSuppressAll();

		// Clear all slave AI targets
		LeftGunAI.SetScriptedFireTarget( None );
		RightGunAI.SetScriptedFireTarget( None );
		DriverAI.SetScriptedFireTarget( None );
		FireTarget = None;
		Focus = None;

		// Reset turn rate when released
		RotationRate = default.RotationRate;
		Brumak.TurnPlayer.Rate = 1.0f;
		bClearLatent = TRUE;
	}

	if( bHijackedByLD && Enemy != None )
	{
		SetEnemy( None );
	}

	if( bClearLatent )
	{
		ClearLatentAction( class'SeqAct_BrumakControl', bAborted );
	}
}

function NotifyEndOfSpecialMove( GearPawn.ESpecialMove LastMove )
{
	Super.NotifyEndOfSpecialMove( LastMove );

	if( LastMove == SM_StumbleGoDown && Brumak.bFlinching )
	{
		ClearLatentAction( class'SeqAct_BrumakControl', FALSE );
	}
}

function bool SelectEnemy()
{
	if( bHijackedByLD )
	{
		return FALSE;
	}

	return Super.SelectEnemy();
}

function SetFirePattern( int PatternIdx )
{
	if( LeftGunAI != None )
	{
		LeftGunAI.bAbleToFire	= FALSE;
	}
	if( RightGunAI != None )
	{
		RightGunAI.bAbleToFire	= FALSE;
	}
	if( DriverAI != None )
	{
		DriverAI.bAbleToFire	= FALSE;
	}

	FirePatternIdx = PatternIdx;
	if( FirePatternIdx >= 0 )
	{
		FirePattern[FirePatternIdx].LeftGunTimer	= (LeftGunAI != None)	? FirePattern[FirePatternIdx].LeftGunFireTime  : -1.f;
		FirePattern[FirePatternIdx].RightGunTimer	= (RightGunAI != None)	? FirePattern[FirePatternIdx].RightGunFireTime : -1.f;
		FirePattern[FirePatternIdx].MainGunTimer	= (DriverAI != None)	? FirePattern[FirePatternIdx].MainGunFireTime  : -1.f;
		FirePattern[FirePatternIdx].SuperGunTimer	= (DriverAI != None)	? FirePattern[FirePatternIdx].SuperGunFireTime : -1.f;
		FirePattern[FirePatternIdx].DelayEndTimer   =						  FirePattern[FirePatternIdx].DelayEndTime;
	
		// Clear next selection of fire pattern until this one is over
		ClearTimer( 'ChooseNewFirePattern' );


		//debug
		`AILog( GetFuncName()@FirePatternIdx@FirePattern[FirePatternIdx].PatternName@IsFiringRockets()  );
		//debug
		MessagePlayer( "NEW FIRE PATTERN:"@FirePattern[FirePatternIdx].PatternName@IsFiringRockets()  );
	}
}

function ChooseNewFirePattern()
{
	local int Idx;
	local array<int> ValidPatterns;

	//debug
	`AILog( GetFuncName()@bHijackedByLD );

	if( bHijackedByLD || !bAllowCombatTransitions )
		return;

	for( Idx = 0; Idx < FirePattern.Length; Idx++ )
	{
		if( !CanDoFirePattern( Idx ) )
		{
			continue;
		}

		ValidPatterns[ValidPatterns.Length] = Idx;
	}

	Idx = ValidPatterns[Rand(ValidPatterns.Length)];

`if(`notdefined(FINAL_RELEASE))
	if( bDebugCyclePatterns )
	{
		//debug - cycle patterns
		Idx = 0;
		if( FirePatternIdx >= 0 )
			Idx = (FirePatternIdx + 1) % FirePattern.Length;
	}
`endif

	SetFirePattern( Idx );
}

function bool CanDoFirePattern( int Idx )
{
	if( (FirePattern[Idx].LeftGunFireTime  > 0.f &&  Brumak.IsLeftGunDestroyed())  ||
		(FirePattern[Idx].RightGunFireTime > 0.f &&  Brumak.IsRightGunDestroyed()) ||
		(FirePattern[Idx].MainGunFireTime  > 0.f &&  Brumak.IsMainGunDestroyed())  ||
		(FirePattern[Idx].SuperGunFireTime > 0.f && (Brumak.IsMainGunDestroyed() || !IsEnemyVisible(DriverAI.Enemy))) )
	{
		return FALSE;
	}

	return TRUE;
}

function bool IsFirePatternComplete( optional bool bEnd )
{
	if( FirePatternIdx < 0 )
	{
		return TRUE;
	}

	if( FindCommandOfClass( class'AICmd_Attack_Brumak_Rockets' ) != None )
	{
		return FALSE;
	}

	if( (!Brumak.IsLeftGunDestroyed()  && (FirePattern[FirePatternIdx].LeftGunTimer  >= 0.f || (LeftGunAI  != None && LeftGunAI.IsFiringWeapon())))	||
		(!Brumak.IsRightGunDestroyed() && (FirePattern[FirePatternIdx].RightGunTimer >= 0.f || (RightGunAI != None && RightGunAI.IsFiringWeapon()))) ||
		(!Brumak.IsMainGunDestroyed()  && (FirePattern[FirePatternIdx].MainGunTimer  >= 0.f || (DriverAI   != None && DriverAI.IsFiringWeapon())))	||
		(!Brumak.IsMainGunDestroyed()  && (FirePattern[FirePatternIdx].SuperGunTimer >= 0.f || (DriverAI   != None && DriverAI.IsFiringWeapon())))   )
	{
		return FALSE;
	}

	if( bEnd && FirePattern[FirePatternIdx].DelayEndTimer >= 0.f )
	{
		return FALSE;
	}

	return TRUE;
}

function ClearFirePattern()
{
	if( FirePatternIdx < 0 )
		return;

	//debug
	`AILog( GetFuncName() );

	FirePattern[FirePatternIdx].LeftGunTimer	= -1.f;
	FirePattern[FirePatternIdx].RightGunTimer	= -1.f;
	FirePattern[FirePatternIdx].MainGunTimer	= -1.f;
	FirePattern[FirePatternIdx].SuperGunTimer	= -1.f;
}

function bool IsFiringRockets()
{
	return (FindCommandOfClass( class'AICmd_Attack_Brumak_Rockets' ) != None);
}

function Tick( float DeltaTime )
{
	Super.Tick( DeltaTime );

	if( IsFirePatternComplete() && FirePatternIdx >= 0 )
	{
		FirePattern[FirePatternIdx].DelayEndTimer -= DeltaTime;
	}

	// Set timer to pick a new fire pattern
	if( HasAnyEnemies() && IsFirePatternComplete(TRUE) && !IsTimerActive( 'ChooseNewFirePattern' ) && Brumak != None && !Brumak.IsDoingASpecialMove() && !bHijackedByLD && bAllowCombatTransitions )
	{
		SetTimer( RandRange( FirePatternDelayRange.X, FirePatternDelayRange.Y ), FALSE, nameof(ChooseNewFirePattern) );
	}
}

function WarnEnemiesOfAttack();

function NotifyKilled( Controller Killer, Controller Killed, Pawn KilledPawn )
{
	if( KilledPawn == Brumak.Driver )
	{
		Brumak.NotifyDriverDied( KilledPawn, Killer );
	}

	Super.NotifyKilled( Killer, Killed, KilledPawn );

	if( DriverAI != None )
	{
		DriverAI.NotifyKilled( Killer, Killed, KilledPawn );
	}
	if( LeftGunAI != None )
	{
		LeftGunAI.NotifyKilled( Killer, Killed, KilledPawn );
	}
	if( RightGunAI != None )
	{
		RightGunAI.NotifyKilled( Killer, Killed, KilledPawn );
	}
}

event Actor GeneratePathTo( Actor Goal, optional float Distance, optional bool bAllowPartialPath )
{
	class'Path_TowardGoal'.static.TowardGoal( Pawn, Goal );
	class'Goal_AtActor'.static.AtActor( Pawn, Goal, Distance, bAllowPartialPath );

	return FindPathToward( Goal );
}

/** Find best fire position on the path network */
function bool PickBestFirePosition( EBrumakSearchType Type, out NavigationPoint out_Nav )
{
	if( Enemy != None )
	{
		out_Nav = NavigationPoint(GeneratePathTo( Enemy,, TRUE ));
		return (out_Nav != None);
	}

	return FALSE;
}

function GotoBestFirePosition( EBrumakSearchType Type )
{
	local NavigationPoint Nav, BestNav;
	local array<NavigationPoint> NavList;
	local float	Weight, BestWeight;
	local int Idx;
	local Cylinder MinSize;

	bReachedMoveGoal = FALSE;

	if( Type == BrumakSearchType_Direct && ActorReachable( Enemy ) )
	{
		//debug
		`AILog( GetFuncName()@Type@"reachable enemy" );

		SetMoveGoal( Enemy, Enemy, TRUE );
		return;
	}

	if( PickBestFirePosition( Type, Nav ) )
	{
		SetMoveGoal( Nav, Enemy, TRUE,, TRUE );
	}
	else
	if( TimeSince( LastMoveTime ) > 8.f )
	{
		//debug
		`AILog( GetFuncName()@"Failed to move for too long"@TimeSince(LastMoveTime)@"try to get unstuck" );

		MinSize.Height = 120.f;
		MinSize.Radius = 100.f;
		if( class'NavigationPoint'.static.GetAllNavInRadius( Pawn, Pawn.Location, 512.f, NavList, FALSE,, MinSize ) )
		{
			for( Idx = 0; Idx < NavList.Length; Idx++ )
			{
				if( !ActorReachable( NavList[Idx] ) )
					continue;

				Weight = VSize(NavList[Idx].Location - Pawn.Location);
				if( BestWeight == 0.f || (Weight > BestWeight) )
				{
					BestWeight = Weight;
					BestNav = NavList[Idx];
				}
			}
		}

		//debug
		`AILog( "Get unstuck by moving to..."@BestNav );

		if( BestNav != None )
		{
			SetMoveGoal( BestNav, Enemy, TRUE );
		}
	}
}

/**
 * Pick melee attack for brumak to use.
 * doesn't play twice the same attack in a row, and pick a random one based on a chance %
 */
function BrumakMeleeAttackDef PickBrumakMeleeAttack()
{
	local Array<INT>	IndexList;
	local Array<FLOAT>	WeightList;
	local float			TotalWeight, RandomWeight;
	local INT			Idx, DesiredIdx;

	// Build a list of valid indices to choose from
	TotalWeight = 0.f;
	for(Idx=0; Idx<BrumakMeleeAttackList.Length; Idx++)
	{
		if( BrumakMeleeAttackList[Idx].AttackSpecialMove != CurrentBrumakMeleeAttack.AttackSpecialMove )
		{
			IndexList[IndexList.Length] = Idx;
			TotalWeight += BrumakMeleeAttackList[Idx].Chance;
		}
	}

	// if we have a list, pick from it
	if( IndexList.Length > 0 )
	{
		/** Value used to normalize weights so all childs add up to 1.f */
		for(Idx=0; Idx<IndexList.Length; Idx++)
		{
			WeightList[Idx] = BrumakMeleeAttackList[IndexList[Idx]].Chance / TotalWeight;
		}

		RandomWeight	= FRand();
		Idx				= 0;
		DesiredIdx		= IndexList[0];

		// This child has too much weight, so skip to next.
		while( Idx < IndexList.Length - 1 && RandomWeight > WeightList[Idx] )
		{
			RandomWeight -= WeightList[Idx];
			Idx++;
			DesiredIdx = IndexList[Idx];
		}

		return BrumakMeleeAttackList[DesiredIdx];
	}

	// Otherwise, just return a random one
	return BrumakMeleeAttackList[Rand(BrumakMeleeAttackList.Length)];
}

function DoExposeDriver()
{
	class'AICmd_React_Brumak_ExposeDriver'.static.InitCommand( self );
}

function DoRoar()
{
	class'AICmd_React_Brumak_Roar'.static.InitCommand( self );
}

function float GetInaccuracyScale()
{
	if( Brumak != None )
	{
		return Brumak.GetHeadDamageInaccuracyScale();
	}

	return 1.f;
}

function Rotator GetAccuracyRotationModifier( GearWeapon Wpn, float InaccuracyPct )
{
	local float TargetDist;

	TargetDist = VSize(FireTarget.Location - Pawn.Location);
	InaccuracyPct *= GetRangeValueByPct( vect2d(1.f,0.2f), FMin( (TargetDist - EnemyDistance_Short) / (EnemyDistance_Long - EnemyDistance_Short), 1.f ) );

	return Super.GetAccuracyRotationModifier( Wpn, InaccuracyPct );
}

defaultproperties
{
	RotationRate=(Pitch=10000,Yaw=7500,Roll=10000)

	FirePattern(0)=(PatternName="ArmsLeftFirst",LeftGunFireTime=0.5f,RightGunFireTime=3.f,DelayEndTime=1.f)
	FirePattern(1)=(PatternName="ArmsRightFirst",RightGunFireTime=0.5f,LeftGunFireTime=3.f,DelayEndTime=1.f)
	FirePattern(2)=(PatternName="MainGunOnly",MainGunFireTime=0.5f,DelayEndTime=1.f)
	FirePattern(3)=(PatternName="Haymaker",LeftGunFireTime=0.5f,RightGunFireTime=0.5f,MainGunFireTime=4.f,DelayEndTime=1.f)
	FirePattern(4)=(PatternName="BoomBoom",SuperGunFireTime=0.5f,DelayEndTime=1.f)
	FirePattern(5)=(PatternName="Finisher",SuperGunFireTime=0.5f,MainGunFireTime=2.f,DelayEndTime=1.f)

	FirePatternDelayRange=(X=1.5f,Y=3.f)

	// List of melee attacks
	BrumakMeleeAttackList(0)=(Chance=0.25f,AttackSpecialMove=GSM_Brumak_MeleeAttack,bRequiresBrumakToStop=TRUE)
	BrumakMeleeAttackList(1)=(Chance=0.5f,AttackSpecialMove=GSM_Brumak_OverlayLftArmSwing,bRequiresBrumakToStop=FALSE)
	BrumakMeleeAttackList(2)=(Chance=0.5f,AttackSpecialMove=GSM_Brumak_OverlayRtArmSwing,bRequiresBrumakToStop=FALSE)
	BrumakMeleeAttackList(3)=(Chance=2.0f,AttackSpecialMove=GSM_Brumak_OverlayBite,bRequiresBrumakToStop=FALSE)

	DefaultCommand=class'AICmd_Base_Brumak'

	// ignore reactions
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	// execute enemy
	Begin Object Name=ExecutEnemyReaction0
		OutputFunction=HandleExecuteReactionAndPushExecuteCommand
	End Object
	DefaultReactConditions.Add(ExecutEnemyReaction0)

	bIgnoreStepAside=TRUE
	bUseFireTickets=FALSE
	bCanRevive=FALSE
}
