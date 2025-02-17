/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_Berserker extends GearAI
	native(AI)
	config(AI);

/** GoW global macros */

cpptext
{
	void AdjustHearingLocation(FVector &out_Location);
}

//// CHARGE VARIABLES ////
/** Pawn is preparing the charge - rotating towards target **/
var bool	bPreparingCharge;
/** Should charge directly at target -- set when only bustable objects in the way */
var bool	bDirectCharge;
/** Start charge again after finishing current special move */
var bool	bChargeAgain;
/** Actor we are charging at */
var Actor ChargeActor;
/** Location we are charging at */
var Vector	ChargeLocation;
/** Location of charge actor when info was gathered */
var Vector	ChargePivot;
/** Last time charge actor/location was updated */
var float ChargeUpdateTime;
/** Num objects we can bust through with this charge */
var config Byte ChargeBreakCount;
/** List of objects we already bust through so we don't collide more than once */
var array<Actor> ChargeHitList;
/** Max amount of time to search before sniffing and charging a target */
var config float MaxSearchTime;
/** FOV angle needed to readjust to target while charging */
var config float ChargeAdjustFOV;
/** Max distance to readjust to target while charging */
var config float ChargeAdjustDist;
/** Time buffer added to hear a sound after a slide attack starts */
var config float NextHearTime;
/** Min/Max time for sniffing before charging */
var config Vector2D SniffTime;

/** Num foot steps before reacting */
var config int NumFootSteps;

function StopFiring();
function bool IsTooLongSinceEnemySeen( Pawn EnemyPawn );
function HandleIdleBoredom();
function NotifyProjLanded( Projectile Proj );
function NotifyHODThreat( Pawn Shooter, Vector TargetingLocation );

simulated function DrawDebug(GearHUD_Base HUD, name Category)
{
	super.DrawDebug( HUD, Category );

	if( bDrawAIDebug && Category=='Default')
	{
		if( ChargeLocation != vect(0,0,0) )
		{
			// REDish
			DrawDebugLine( Pawn.Location, ChargeLocation, 150, 0, 0 );
			// BLUEish
			DrawDebugLine( Pawn.Location, GetDestinationPosition(), 0, 0, 150 );
		}
	}
}

simulated function DrawIconOverhead(GearHUD_Base HUD, Texture2D Icon)
{
	local Canvas	Canvas;

	super.DrawIconOverhead(HUD, Icon);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255,255,255);

	if( IsAlert() )
	{
		Canvas.DrawText("Alert",TRUE);
	}
	else
	{
		Canvas.DrawText("Normal",TRUE);
	}
}

function bool SelectTarget()
{
	ClearTimer( 'SelectTarget' );

	return FALSE;
}

function HearNoise( float Loudness, Actor NoiseMaker, optional Name NoiseType )
{
	local Projectile Proj;

	//debug
	`AILog( GetFuncName()@Loudness@NoiseMaker@NoiseType@CanDoSpecialMove( FALSE ) );

	Proj = Projectile(NoiseMaker);
	if( Proj != None )
	{
		if( CanDoSpecialMove( FALSE ) )
		{
			ChargeAt( Proj, Proj.Location, "Heard enemy w/ LOS" );
		}
	}
	else
	{
		super.HearNoise( Loudness, NoiseMaker, NoiseType );
	}
}

event NotifyEnemySeen( Pawn SeenEnemy )
{
	BecomeAlert();
	CheckInterruptCombatTransitions();
}

event NotifyEnemyHeard( Pawn HeardEnemy, Name NoiseType )
{
	local Vector HitLocation, HitNormal, EnemyLocation, ViewLoc, Dir;
	local Actor	 HitActor;
	local int EnemyIdx;

	if( NoiseType == 'NOISETYPE_FootStep' && !IsAlert() )
	{
		EnemyIdx = LocalEnemyList.Find( 'Pawn', HeardEnemy );
		if( EnemyIdx >= 0 )
		{
			if( TimeSince(LocalEnemyList[EnemyIdx].LastFootStepTime) > 5.f )
			{
				//debug
				`AILog( "Reset footstep counter for"@LocalEnemyList[EnemyIdx].Pawn );

				LocalEnemyList[EnemyIdx].FootStepCount = 0;
			}

			LocalEnemyList[EnemyIdx].FootStepCount++;
			LocalEnemyList[EnemyIdx].LastFootStepTime = WorldInfo.TimeSeconds;

			if( LocalEnemyList[EnemyIdx].FootStepCount < NumFootSteps )
			{
				//debug
				`AILog( GetFuncName()@"Ignore footstep" );

				return;
			}
		}
	}

	//debug
	`AILog( GetFuncName()@HeardEnemy@MyGearPawn.SpecialMove@CanDoSpecialMove( TRUE ) );

	// If not already doing an action
	if( CanDoSpecialMove( TRUE ) )
	{
		BecomeAlert();

		ViewLoc = Pawn.GetPawnViewLocation();
		EnemyLocation = GetEnemyLocation(HeardEnemy);

		// Trace world geometry only... skip pillars/doors/actors
		HitActor = Pawn.Trace( HitLocation, HitNormal, EnemyLocation, ViewLoc, FALSE, Pawn.GetCollisionExtent(),, TRACEFLAG_SkipMovers );
		bDirectCharge = (HitActor == None);

		// Trace a few units from the back of the
		// player to see if there is a pillar or door behind them
		Dir = Normal(EnemyLocation - ViewLoc);
		HitActor = HeardEnemy.Trace( HitLocation, HitNormal, EnemyLocation+Dir*128.f, EnemyLocation, TRUE, Pawn.GetCollisionExtent() );

		//debug
		if( HitActor != None )
			`AILog( "Check behind enemy"@HitActor@HitActor.Tag );
		else
			`AILog( "Check behind enemy"@HitActor );

		// If did not hit a bustable door/pillar
		if( !ActorIsBustable( HitActor ) )
		{
			// Keep the enemy pawn as the charge target
			HitActor	= HeardEnemy;
			HitLocation = EnemyLocation;
		}
		// Otherwise, we hit something ... align to center
		else
		{
			//debug
			`AILog( "Adjust to center of"@HitActor );

			// Adjust to the center of the object we hit
			HitLocation = HitActor.Location;
		}

		ChargeAt( HitActor, HitLocation, "Heard enemy w/ LOS" );
	}
	else
	// Otherwise, if heard sound while already charging
	// (Allowed from CanHearPawn - requires same heard ChargeActor and was far enough away to adjust)
	if( MyGearPawn.IsDoingSpecialMove(SM_Berserker_Charge) )
	{
		// Adjust charge location
		SetChargeLocation( ChargeActor, ChargeActor.Location );
	}
}

function bool CanDoSpecialMove( bool bIncludeOthers )
{
	if( MyGearPawn != None )
	{
		if( !MyGearPawn.IsDoingASpecialMove() ||
			 MyGearPawn.IsDoingSpecialMove(SM_Berserker_Alert) )
		{
			return TRUE;
		}

		if (bIncludeOthers && MyGearPawn.IsDoingSpecialMove(SM_Berserker_Slide))
		{
			return TRUE;
		}
	}

	return FALSE;
}

function BecomeAlert()
{
	//debug
	`AILog( GetFuncName() );

	SetPerceptionMood( AIPM_Alert );
	SetTimer( 20.f, FALSE, nameof(NoLongerAlert) );
}
function NoLongerAlert()
{
	//debug
	`AILog( GetFuncName() );

	SetPerceptionMood( AIPM_Normal );
}

function bool CanHearPawn( Pawn Heard, float Loudness, Name NoiseType )
{
	local float HearRadius, Dist;

	// I hate this...
	if( !Heard.IsHumanControlled() )
	{
		return FALSE;
	}

	// Don't respond to weapon switches
	if( NoiseType == 'ChangedWeapon' || MyGearPawn == None )
	{
		return FALSE;
	}

	// If is charging ...
	if( MyGearPawn.IsDoingSpecialMove(SM_Berserker_Charge) )
	{
		// If sound is not from our victim - Ignore
		if( Heard != ChargeActor )
		{
			return FALSE;
		}

		// If...
		// Already charging AND
		// (sound is too far to the side to adjust OR
		//  sound is too close to adjust)
		if( !bPreparingCharge &&
			((Normal(Heard.Location-MyGearPawn.Location) DOT Normal(MyGearPawn.Velocity) < ChargeAdjustFOV ) ||
			(VSizeSq(Heard.Location-Pawn.Location) < ChargeAdjustDist*ChargeAdjustDist)) )
		{
			return FALSE;
		}

		// CHANGE TO TRACE!
		if( !ChargeLocationReachable( Heard.Location ) )
		{
			return FALSE;
		}
	}
	else
	// Otherwise not sliding OR
	// enough time has passed since slide started
	if( !CanDoSpecialMove( TRUE ) ||
		 WorldInfo.TimeSeconds < NextHearTime )
	{
		// Wait until we go back to search state
		return FALSE;
	}

	if( NoiseType == 'NOISETYPE_FootStep' )
	{
		// Hear footsteps at max volume at 512 units
		HearRadius = Pawn.HearingThreshold * Loudness;
		Dist = VSize(Heard.Location-Pawn.Location);

		//debug
		`AILog( "Heard footstep"@(Dist <= HearRadius)@Loudness@Pawn.HearingThreshold@HearRadius@Dist );

		return (Dist <= HearRadius);
	}

	return TRUE;
}

/**
 *	Don't call super Notify b/c it will interrupt transition
 *	into stunned/collide states if enemy is close
 */
function NotifyEndOfSpecialMove( GearPawn.ESpecialMove LastMove )
{
	`AILog( class@GetFuncName()@LastMove );
}

/** Takes longer for her to time out her move */
function float GetMoveTimeOutDuration(vector dest, bool bDontCare)
{
	return 10.f;
}

// ignore this since Berserker can't fire
function FireFromOpen(optional int InBursts = 1, optional bool bAllowFireWhileMoving=true);

state Action_Idle
{
	function BeginState(Name PreviousStateName)
	{
		Pawn.ZeroMovementVariables();

		super.BeginState( PreviousStateName );
	}
}

state Action_Combat_Search
{
	function BeginState( Name PreviousStateName )
	{
//		local class<GearCombatAction> Action;

		super.BeginState( PreviousStateName );

		if( bChargeAgain )
		{
			//debug
			`AILog( "CHARGE AGAIN!!!!!" );

			bChargeAgain = FALSE;

			SetChargeLocation(ChargeActor, ChargeLocation);
			BeginCombatCommand(class'AICmd_Berserker_Kamikaze', "Stored charge from before");
		}
	}

	function float GetSearchRadius()
	{
		if( PerceptionMood == AIPM_Alert )
		{
			return EnemyDistance_Short;
		}

		return EnemyDistance_Medium;
	}

	function float GetPostAdjustSleep()
	{
		return 0.25+0.5f*FRand();
	}

	function bool PickMoveLocation()
	{
		local bool	bResult;
		local int	Attempts, RandIdx;
		local Vector Dest, Center, EnemyLocation, Edge, Tangent, TestLocation;
		local float Radius;
		local array<NavigationPoint> NavList;
		local Cylinder Cyl;

		SearchTarget = None;
		SearchPoint = vect(0,0,0);
		if( Enemy != None )
		{
			if( ActorReachable( Enemy ) )
			{
				//debug
				`AILog( GetFuncName()@"Enemy reachable", 'Combat' );

				EnemyLocation = GetEnemyLocation();
				Center  = EnemyLocation;
				Edge	= Normal(Pawn.Location-EnemyLocation) * GetSearchRadius();
				Tangent = Normal((Edge-Center) CROSS vect(0,0,1));
				Radius  = GetSearchRadius();

				//debug
				DebugSearchRadius	= Radius;
				DebugSearchLocation = Center;

				Attempts = 4;
				while( Attempts-- >= 0 )
				{
					// Pick test destination so that it tends to cross through the middle area of the circle
					Dest	= VRand();
					Dest.Z	= 0;
					Dest	= Normal(Dest);
					Dest	= Dest * Radius * FMax( FRand(), 0.1f );

					if( VSizeSq((Dest+Center)-Edge) <= Radius * Radius )
					{
						Dest = MirrorVectorByNormal( Dest, Tangent );
					}

					Dest += Center;

					if( PointReachable( Dest ) )
					{
						SearchPoint = Dest;
						bResult = TRUE;
						break;
					}
				}
			}
			else
			{
				//debug
				`AILog( GetFuncName()@"Enemy NOT reachable", 'Combat' );
			}
		}


		if( !bResult )
		{
			SearchTarget = None;
			SearchPoint = vect(0,0,0);
			if( Enemy != None )
			{
				EnemyLocation = Enemy.Location;
				TestLocation = EnemyLocation;

				//debug
				DebugSearchRadius = GetSearchRadius();
				DebugSearchLocation = TestLocation;

				Cyl.Radius = Pawn.GetCollisionRadius();
				Cyl.Height = Pawn.GetCollisionHeight();

				if( class'NavigationPoint'.static.GetAllNavInRadius( Pawn, TestLocation, GetSearchRadius(), NavList, TRUE, (Pawn.Anchor != None) ? Pawn.Anchor.NetworkID : -1, Cyl ) )
				{
					while( SearchTarget == None && NavList.Length > 0 )
					{
						RandIdx = Rand(NavList.Length);
						if( CoverSlotMarker(NavList[RandIdx]) == None )
						{
							SearchTarget = GeneratePathTo( NavList[RandIdx] );
							break;
						}
						else
						{
							NavList.Remove(RandIdx, 1);
						}
					}

					bResult = TRUE;
				}
			}

			//debug
			`AILog( GetFuncName()@NavList.Length@SearchTarget@Enemy@FireTarget@"RESULT"@bResult, 'Combat' );
		}

		return bResult;
	}

	function BeginningSearch()
	{
		local GearPawn_LocustBerserkerBase Berz;

		//debug
		`AILog( GetFuncName()@GearPawn_LocustBerserkerBase(Pawn).bVulnerableToDamage );

		Berz = GearPawn_LocustBerserkerBase(Pawn);
		if( Berz != None && Berz.bVulnerableToDamage )
		{
			ForceUpdateOfEnemies();
			NotifyEnemyHeard( Enemy, 'SuperAgro' );
		}
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();

	// Search for the player
	SelectEnemy();
	FireTarget = Enemy;

	if( Enemy != None )
	{
		BeginningSearch();
	}

	if( PickMoveLocation() )
	{
		// Move toward search target
		PushState( 'SubAction_ReadjustToTarget' );
		CheckInterruptCombatTransitions();
		Sleep( GetPostAdjustSleep() );
	}
	else
	{
		//debug
		`AILog( "Delay search", 'Combat' );

		Sleep( 1.f );
	}

Finished:
		// Check combat transitions
		CheckCombatTransition();
		Goto('Begin');
}

state SubAction_ReadjustToTarget `DEBUGSTATE
{
Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if( SearchTarget != None )
	{
		//`warn("!!!!! #1");
		SetMoveGoal( SearchTarget,, TRUE, Pawn(SearchTarget)==None?0.f:256.f );
	}
	else
		if( SearchPoint != vect(0,0,0) )
		{
			SetMovePoint( SearchPoint,, TRUE );
		}
		else
		{
			Sleep( 1.f );
		}

		PopState();
}

state Action_Berserker_Sniff extends Combat
{
	function StartSniff()
	{
		Pawn.ZeroMovementVariables();
		// @fixme laurent -- we shouldn't have to set that here, it should be local to the charge state.
		// find out why it's not unset when reaching this point!
		Pawn.bForceMaxAccel	= FALSE;
		bPreparingMove		= TRUE;
		MoveTimer			= -1;
		GearPawn_LocustBerserkerBase(Pawn).Sniff();
	}

	function StopSniff()
	{
		bPreparingMove = FALSE;
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// Check for any interrupt transitions
	CheckInterruptCombatTransitions();

	// Hold still for a few seconds
	// (Idle animation is the sniff animation)
	StartSniff();
	Sleep( RandRange( SniffTime.X, SniffTime.Y ) );
	StopSniff();

	// Force update of all enemy locations
	ForceUpdateOfEnemies();

	// Select the nearest enemy
	if( SelectEnemy() )
	{
		FireTarget = Enemy;
		NotifyEnemyHeard( Enemy, 'Sniff' );
	}

	// Check combat transitions
	CheckCombatTransition();
}

function DoMeleeAttack( optional Pawn NewEnemy )
{
	if (NewEnemy != None)
	{
		SetEnemy(NewEnemy);
	}

	class'AICmd_Berserker_SmashAttack'.static.InitCommand(self);
}

/**
 * notification from animation that Berserker started charging.
 * This is because animation transitions delay this. It's not instantaneous.
 */
function ChargeStartedNotify()
{
	bPreciseDestination = TRUE;

	// Pawn is aligned towards target, we can charge now
	bPreparingCharge	= FALSE;
	bDirectCharge		= FALSE;

	SetTimer( 0.25, FALSE, nameof(WarnEnemiesOfAttack) );
}

function SetChargeLocation( Actor Victim, Vector VictimLocation )
{
	local float Dist, TravelTime;

	//debug
	`AILog( GetFuncName()@Victim@VictimLocation );

	Dist = VSize(VictimLocation-Pawn.Location);
	TravelTime = Dist / 600.f; // MAGIC NUMBER: Charge speed roughly 600

	// Lead charge target a little bit (assume half velocity b/c they are likely to change direction)
	ChargeLocation	 = VictimLocation + TravelTime * Victim.Velocity * 0.5f;
	ChargeLocation.Z = Pawn.Location.Z;
	ChargePivot		 = ChargeLocation;

	ChargeActor	  	 = Victim;
	ChargeUpdateTime = WorldInfo.TimeSeconds;
}

function ChargeAt( Actor A, Vector Loc, coerce String Reason, optional bool bDirect )
{
	//debug
	`AILog( GetFuncName()@Reason@"---"@A@Loc@bDirect@CanDoSpecialMove( FALSE ) );

	// check for a smash first
	CheckInterruptCombatTransitions();

	bDirectCharge = bDirect;

	// If not doing sliding OR colliding special move
	if( CanDoSpecialMove( FALSE ) )
	{
		//debug
		`AILog( GetFuncName()@"How to respond?"@(FindCommandOfClass(class'AICmd_Berserker_Kamikaze') != None)@A@ChargeActor );

		// If not already in Kamikaze
		if (FindCommandOfClass(class'AICmd_Berserker_Kamikaze') == None)
		{
			// Set charge info
			SetChargeLocation( A, Loc );

			// Make the transition to charge
			BeginCombatCommand(class'AICmd_Berserker_Kamikaze', Reason);
		}
		// Otherwise, we are pathing toward some enemy
		else
		{
			// Check to see if new enemy is closer than old enemy
			if( A == ChargeActor ||
				VSizeSq(Pawn.Location-Loc) < VSizeSq(Pawn.Location-ChargeLocation) )
			{
				//debug
				`AILog( "New noise maker is closer than old one" );

				// Set charge info
				SetChargeLocation( A, Loc );

				// Reset state
				BeginCombatCommand(class'AICmd_Berserker_Kamikaze', Reason, true);
			}
			// Otherwise, ignore noise
			else
			{
				//debug
				`AILog( "Ignore noise" );
			}
		}
	}
	else
	{
		// Store charge info for later
		bChargeAgain	= TRUE;
		ChargeActor		= A;
		ChargeLocation	= Loc;

		//debug
		`AILog( "Will charge again... set flag and store info" );
	}
}

function bool ChargeLocationReachable( Vector TestLocation )
{
	local Vector HitLocation, HitNormal, Extent, VectToLoc, VectToHit;
	local Actor HitActor;
	local float DotP;

	Extent = Pawn.GetCollisionExtent();
	Extent.Z *= 0.5;

	HitActor = Pawn.Trace( HitLocation, HitNormal, TestLocation, Pawn.Location, TRUE, Extent,, TRACEFLAG_SkipMovers );
	if( HitActor == None || Pawn(HitActor) != None )
	{
		return TRUE;
	}

	VectToLoc = Normal(TestLocation - Pawn.Location);
	VectToHit = Normal(HitLocation - TestLocation);
	DotP = VectToHit DOT VectToLoc;

	if( DotP > 0.f )
	{
		return TRUE;
	}


	//debug
//	DrawDebugBox( HitLocation, Extent, 255, 0, 0, TRUE );

	return FALSE;
}

state SubAction_MoveToGoal
{
	function bool HandlePathObstruction( Actor BlockedBy )
	{
		return TRUE;
	}
}

function bool NotifyHitByHOD()
{
	PushState( 'SubAction_HODHitReaction' );
	return TRUE;
}

state SubAction_HODHitReaction extends SubAction_SpecialMove
{
	function bool NotifyHitByHOD();
	function bool CanSeePawn( Pawn Seen );
	event SeePlayer( Pawn Seen );
	function bool CanHearPawn( Pawn Heard, float Loudness, Name NoiseType );

	function ESpecialMove GetSpecialMove()
	{
		return SM_Berserker_HODHitReaction;
	}

	function FinishedSpecialMove()
	{
		// Check combat transitions
		CheckCombatTransition();
	}
}

function bool ActorIsBustable( Actor TestActor )
{
	return TestActor != None && (TestActor.Tag == 'BustablePillar' || TestActor.Tag == 'BustableDoor');
}

function bool NotifyCollideWithActor( Vector HitNormal, Actor Other )
{
	local int Idx;
	local Vector Dir;
	local Pawn OtherPawn;
	local GearPawn OtherWP;

	OtherPawn = Pawn(Other);

	//debug
	`AILog( GetFuncName()@Other@Other.Tag@Other.bWorldGeometry@MyGearPawn.SpecialMove@ActorIsBustable( Other )@ChargeBreakCount );

	if( KActor(Other) != None )
		return FALSE;

	if( MyGearPawn.IsDoingSpecialMove( SM_Berserker_Charge ) ||
		MyGearPawn.IsDoingSpecialMove( SM_Berserker_Slide  ) )
	{
		// Only break pillars when hitting directly
		Dir = MyGearPawn.Velocity;
		Dir.Z = 0;
		Dir = Normal(Dir);
		HitNormal.Z = 0;
		HitNormal = Normal(HitNormal);

		//debug
		`AILog( Dir@"Dot"@HitNormal@"is"@(Dir DOT HitNormal)@"vs"@MinHitWall@OtherPawn@ChargeHitList.Find( Other ) );

		if( MinHitWall > (Dir DOT HitNormal) ||
			(OtherPawn == None && !PickWallAdjust( HitNormal )) )
		{
			Idx = ChargeHitList.Find( Other );

			if( Idx < 0 )
			{
				ChargeHitList[ChargeHitList.Length] = Other;

				Other.TakeDamage( 1000.f, self, Other.Location, vect(0,0,0), class'GDT_Berserker_Charge' );

				if( GearPawn(Other) != none )
				{
					GearPawn_LocustBerserkerBase(Pawn).PlayChargeHitPlayerEffect( Pawn.Location, HitNormal );
				}
				else
				{
					GearPawn_LocustBerserkerBase(Pawn).PlayChargeHitWallEffect( Pawn.Location, HitNormal );
				}

				if( !ActorIsBustable( Other ) || ChargeBreakCount-- == 0 )
				{
					if( MyGearPawn.IsDoingSpecialMove( SM_Berserker_Charge ) )
					{
						if( Other.bWorldGeometry )
						{
							class'AICmd_Berserker_Stunned'.static.InitCommand(self);
						}
						else
						{
							class'AICmd_Berserker_Collide'.static.InitCommand(self);
						}
					}
				}
				else if( Other.Tag == 'BustableDoor' )
				{
					class'AICmd_Berserker_Collide'.static.InitCommand(self);
				}
			}

			return TRUE;
		}
	}
	else
	if( CanDoSpecialMove( FALSE ) )
	{
		OtherWP = GearPawn(Other);
		if( OtherWP != None &&
			OtherWP.IsDBNO() )
		{
			ChargeAt( OtherWP, OtherWP.Location, "Bumped into down enemy", TRUE );
		}
	}

	return FALSE;
}


function bool NotifyHitWall( Vector HitNormal, Actor Wall )
{
	if( NotifyCollideWithActor( HitNormal, Wall ) )
	{
		return TRUE;
	}

	return super.NotifyHitWall( HitNormal, Wall );
}

function bool NotifyBump( Actor Other, Vector HitNormal )
{
	if( Pawn(Other) != None &&
		NotifyCollideWithActor( HitNormal, Other ) )
	{
		return TRUE;
	}

	return super.NotifyBump( Other, HitNormal );
}

function NotifyBerserkerSmashCollision( Actor Other, Vector HitLocation )
{
	local FracturedStaticMeshActor FracActor;

	//debug
	`AILog( GetFuncName()@Other@MyGearPawn.SpecialMove );

	if( MyGearPawn.IsDoingSpecialMove( SM_Berserker_Smash ) )
	{
		`AILog("doing damage to"@Other);
		Other.TakeDamage( 1000.f, self, Other.Location, vect(0,0,0), class'GDT_Berserker_Smash' );

		FracActor = FracturedStaticMeshActor(Other);
		if( FracActor != None && FracActor.Physics == PHYS_None && FracActor.IsFracturedByDamageType(class'GDT_Berserker_Smash') )
		{
			FracActor.BreakOffPartsInRadius( HitLocation, 300.f, 512.f, true );
		}

		MyGearPawn.PlaySound(SoundCue'Foley_BodyMoves.BodyMoves.BerserkerArmImpactFlesh_Cue');
	}
}

function WarnEnemiesOfAttack()
{
	local GearAI_Dom AI;
	local float DistSq;

	if( MyGearPawn == None )
	{
		return;
	}
	if( MyGearPawn.IsDoingSpecialMove(SM_Berserker_Charge) ||
		MyGearPawn.IsDoingSpecialMove(SM_Berserker_Slide) )
	{
		SetTimer( 0.25f, FALSE, nameof(WarnEnemiesOfAttack) );
	}
	else
	if( !MyGearPawn.IsDoingSpecialMove(SM_Berserker_Smash) )
	{
		return;
	}

	foreach WorldInfo.AllControllers( class'GearAI_Dom', AI )
	{
		DistSq = VSizeSq(Pawn.Location-AI.Pawn.Location);
		if( DistSq < 65536 )
		{
			AI.EvadeAwayFromPoint( Pawn.Location );
		}
	}
}

defaultproperties
{
	bAlwaysHear=TRUE

	MinHitWall=-0.5f
	RotationRate=(Pitch=20000,Yaw=20000,Roll=20000)

	ExtraPathCost=10000

	DefaultCommand=class'AICmd_Base_Berserker'

	// don't want reactions for this guy
	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty
	// have to re-init the delegates here so they get the right function object
	// melee reaction
	Begin Object Name=EnemyInMeleeRangeReaction0
		OutputFunction=BeginMeleeCommandWrapper
	End Object
	DefaultReactConditions.Add(EnemyInMeleeRangeReaction0)

	bCanRevive=FALSE
}
