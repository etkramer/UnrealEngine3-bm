/**
* Class to handle the AI controlling of Jack
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearAI_Jack extends GearAI
	native(AI)
	config(AI);

/** GoW global macros */

var() Vehicle_Jack_Base Jack;

var Vector	LastIdleLocation;
var Vector	LastMoveLocation;
var float	NextPOITime;
var float	NextIdleMoveTime;
var bool	bScriptedCloaking;
var float	CloakByEnemyTime;

/** Jack is teleporting to a good spot... suspend auto cloak/decloak */
var bool	bTeleporting;

/** Last time successfully reached idle move goal */
var float	LastReachedMoveGoalTime;

var array<JackPointOfInterest>	JackInterestList;
/** Jack interest POIs were assigned by LD */
var bool						bLDSetPOI;

simulated function DrawDebug( GearHUD_Base HUD, name Category )
{
	Super.DrawDebug( HUD, Category );
	
	DrawDebugBox( GetDestinationPosition(), vect(5,5,5), 0, 255, 0 );
	DrawDebugBox( LastIdleLocation, vect(5,5,5), 255, 0, 0 );
	DrawDebugBox( LastMoveLocation, vect(5,5,5), 0, 0, 255 );
}

function Possess( Pawn NewPawn, bool bVehicleTransition )
{
	super.Possess( NewPawn, bVehicleTransition );

	Jack = Vehicle_Jack_Base(Pawn);
	// Make Jack the driver of himself
	Jack.Driver		= Jack;
	Jack.bDriving	= TRUE;

	bForceDesiredRotation = TRUE;
	DesiredRotation = Jack.Rotation;

//	SetTimer( 10.f, TRUE, nameof(CheckAreaForPOI) );
}

function float GetMoveTimeOutDuration(vector dest, bool bDontCare)
{
	local float Dist;

	if( JackPointOfInterest(MoveGoal) != None || JackPointOfInterest(TetherActor) != None )
	{
		return 10.f;
	}
	if( TetherActor != None && MoveGoal == TetherActor )
	{
		return 10.f;
	}

	Dist = VSize(Pawn.Location - Dest);
	return FMax(0.5f,4.0f * (Dist / Jack.GroundSpeed));
}

function ClearMovementInfo( optional bool bSkipCombatZones = TRUE )
{
	Super.ClearMovementInfo( bSkipCombatZones );

	LastIdleLocation = vect(0,0,0);
	LastMoveLocation = vect(0,0,0);
	LastReachedMoveGoalTime = 0.f;
}

protected function AbortMovementCommands()
{
	Super.AbortMovementCommands();

	AbortCommand( None, class'AICmd_React_JackInvestigate' );
}

final function bool LDSetPOI()
{
	return (bLDSetPOI || JackPointOfInterest(TetherActor) != None);
}

function bool SetTether( Actor NewTetherActor, optional float NewTetherDistance, optional bool NewbDynamicTether, optional float NewTetherPersistDuration, optional bool bInteruptable, optional bool bIsValidCache )
{
	local JackPointOfInterest POI;

	POI = JackPointOfInterest(NewTetherActor);
	if( POI != None )
	{
		return InvestigatePOI( POI, TRUE );
	}
	
	return Super.SetTether( NewTetherActor, NewTetherDistance, NewbDynamicTether, NewTetherPersistDuration, bInteruptable, bIsValidCache );
}

function bool ShouldMoveToSquad( optional bool bSkipLastPositionCheck )
{
	local Vector LeaderPos;
	local float	 DistToLeader;

	// Don't move to squad if already given an action or no squad leader to move to
	if( MoveAction != None || TetherActor != None || !HasSquadLeader() || ShouldBeStationary() )
	{
		//debug
		`AILog("Shouldn't move to squad, move action:"@MoveAction@"tether actor:"@TetherActor@"squad formation:"@GetSquadName()@GetSquadLeader()@ShouldBeStationary() );

		return FALSE;
	}

	LeaderPos = GetSquadLeaderLocation();
	DistToLeader = VSize(LeaderPos-Pawn.Location);

	//debug
	`AILog("Too far from squad leader?"@LeaderPos@DistToLeader );

	if( DistToLeader > 768.f )
	{
		return TRUE;
	}
	return FALSE;
}

function bool ShouldBeStationary()
{
	if( IsTimerActive('DelayJackCloak') || 
		IsTimerActive('DelayJackDecloak') )
	{
		return TRUE;
	}

	return Jack.ShouldBeStationary();
}

function PickIdleMoveLocation()
{
	local Vector MovePt, CamLoc, AvoidDir, HitLocation, HitNormal, LeaderLoc;
	local Rotator CamRot;
	local float	 AnchorDist, DistOffset, DistTest;
	local bool	 bMove, bSkipHeightAdjust, bValidCache;
	local NavigationPoint DestNav;
	local int Idx;
	local JackPointOfInterest POI;
	local Actor FocusActor, HitActor;
	local bool	bForceTeleport;

	//debug
	`AILog( GetFuncName()@TetherActor@JackInterestList.Length@NextPOITime-WorldInfo.TimeSeconds@MoveAction@TetherActor@ShouldBeStationary() );

	bSkipHeightAdjust = FALSE;
	bReachedMoveGoal = FALSE;
	RouteCache_Empty();

	if( ShouldBeStationary() )
	{
		return;
	}

	// If have some places to go an inspect
	if( TetherActor == None && JackInterestList.Length > 0 && WorldInfo.TimeSeconds > NextPOITime )
	{
		Idx	= Rand(JackInterestList.Length);
		POI	= JackInterestList[Idx];
		JackInterestList.Remove( Idx, 1 );
		bLDSetPOI = (bLDSetPOI && JackInterestList.Length > 0);
		if( POI != None )
		{
			InvestigatePOI( POI );
			return;
		}		
	}
	
	// Check if Jack should move to the squad leader...
	if( ShouldMoveToSquad() )
	{
		// If leader is reachable...
		LeaderLoc = GetSquadLeaderLocation();

		//debug
		`AILog( GetFuncName()@"should move to squad"@PointReachable( LeaderLoc ) );

		if( PointReachable( LeaderLoc ) )
		{
			// Pick a spot a good distance from him
			MovePt = LeaderLoc + Normal(Pawn.Location-LeaderLoc) * 512.f;
		}
		else
		{
			// Otherwise, move directly to leader
			LastIdleLocation = vect(0,0,0);
			MovePt = LeaderLoc;
			DistOffset = 512.f;
		}

		bMove = TRUE;
		Focus = None;
	}
	else
	if( Jack.IsTooCloseToPlayerCamera( CamLoc, CamRot ) && 
		GetObstructionAdjustDir( CamLoc, AvoidDir ) )
	{
		//debug
		`AILog( "Too close to player camera!"@AvoidDir );

		LastIdleLocation = vect(0,0,0);
		MovePt = Pawn.Location + AvoidDir * 256.f;
		bMove = TRUE;		 
	}
	else
	// Otherwise, if we have been idle long enough...
	if( WorldInfo.TimeSeconds > NextIdleMoveTime )
	{
		// Twitch in a random direction
		if( IsZero(LastIdleLocation) )
		{
			MovePt = Pawn.Location + VRand() * 128.f;
		}
		else
		{
			MovePt = LastIdleLocation + VRand() * 128.f;
		}
		bMove = FastTrace( MovePt, Pawn.Location );

		//debug
		`AILog( "Idle twitch..."@LastIdleLocation@bMove );
	}

	// If should move...
	if( bMove )
	{
		bForceTeleport = ShouldForceTeleport( MovePt );

		//debug
		`AILog( "Moving..."@MovePt@PointReachable( MovePt )@bForceTeleport );

		if( bForceTeleport )
		{
			//debug
			`AILog( ".... Force teleport to good spot"@MovePt );

			TeleportToGoodSpot( MovePt );
			return;
		}
		else
		// If point is not directly reachable...
		if( !PointReachable( MovePt ) )
		{
			bMove = FALSE;
			// Find the best anchor at the destination
			DestNav = Pawn.GetBestAnchor( Pawn, MovePt, FALSE, FALSE, AnchorDist );

			//debug
			`AILog( "Find path to end anchor"@DestNav );

			if( DestNav != None )
			{
				// If we can generate at least a partial path to the end anchor...
				if( GeneratePathTo( DestNav,, TRUE ) != None )
				{
					// Limit the cache distance and move there using our current route cache
					LimitRouteCacheDistance( 256.f );
					bValidCache = TRUE;
					bMove		= TRUE;

					//debug
					DebugLogRoute();
				}
			}
			
			if( !bMove )
			{
				//debug
				`AILog( "failed to find path to dest nav..." );

				// Try lowering to ground
				HitActor = Trace( HitLocation, HitNormal, Location - vect(0,0,700), Location );
				if( HitActor != None )
				{
					DistTest = VSize(HitLocation-Location);

					//debug
					`AILog( "Try to lower to ground"@DistTest );

					if( DistTest > 80.f )
					{
						LastIdleLocation = vect(0,0,0);
						MovePt = HitLocation + vect(0,0,50);
						bMove = TRUE;
						bSkipHeightAdjust = TRUE;
					}					
				}

				if( !bMove )
				{
					//debug
					`AILog( ".... Try teleport to good spot"@MovePt );

					TeleportToGoodSpot( MovePt );
					return;
				}
			}
		}
		else
		{
			//debug
			`AILog( "..... point was reachable!" );

			if( TetherActor != None )
			{
				FocusActor = TetherActor;
			}
			else
			{
				FocusActor = Focus;
			}	
		}

		// If should complete the move
		if( bMove )
		{
			// Try to adjust the height of the move point to be over the leaders head
			if( !bSkipHeightAdjust && TetherActor == None )
			{
				MovePt += Jack.AdjustDestination( None, MovePt );

				//debug
				`AILog( "... Adjusted destination to"@MovePt );
			}

			if( !Pawn.ReachedPoint( MovePt, Pawn.Anchor ) )
			{
				if( IsZero(LastIdleLocation) )
				{
					LastIdleLocation = MovePt;
				}

				// Update next idle twitch time
				NextIdleMoveTime = WorldInfo.TimeSeconds + (1.f + FRand() * 5.f);
				// Execute the move
				LastMoveLocation = MovePt;
				SetMovePoint( MovePt, FocusActor, TRUE, DistOffset, bValidCache );
			}
			else
			{
				`AILog( "Error: ... No move?" );
			}
		}
	}
	else
	{
		bReachedMoveGoal = TRUE;
	}
	
}

function PlayIdleSound()
{
	SetTimer( 15.0f + 45.f * FRand(), FALSE, nameof(PlayIdleSound) );

	if( Jack.IsCloaked() || Pawn == None )
	{
		return;
	}

	Jack.PlayIdleSound();
}

/**
* Overridden to not call BeginCombatCommand as it can cause infinite recursion
*/
simulated function OnTeleport(SeqAct_Teleport Action)
{
	//debug
	`AILog( GetFuncName()@Action );

	// Clear cover
	InvalidateCover();

	ClearMovementInfo(FALSE);

	// Clear route list
	RouteCache_Empty();

	Pawn.ZeroMovementVariables();

	if( Action != None )
	{
		// Actually do the teleport
		super(AIController).OnTeleport( Action );
	}
}

state Action_Idle 
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		SetTimer( 1.5f + 5.f * FRand(), FALSE, nameof(PlayIdleSound) );
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@HasAnyEnemies()@bTeleporting@Jack.ShouldBeStationary(), 'State' );

	if( !bTeleporting )
	{
		if( !Jack.ShouldBeStationary() )
		{
			SetCloaking( HasAnyEnemies(),,, 5.f );
		}		

		if( (Pawn != None) && !IsWithinTether( Pawn.Location ) )
		{
			//debug
			`AILog( "Not within tether, moving..."@TetherActor );

			if( JackPointOfInterest(TetherActor) != None )
			{
				InvestigatePOI( JackPointOfInterest(TetherActor), FALSE );
			}
			else
			{
				// Move to tether
				class'AICmd_MoveToGoal'.static.MoveToGoal( self, TetherActor );
			}
		}
		else
		{
			PickIdleMoveLocation();
		}

		//debug
		`AILog( "Finished idle move"@bReachedMoveGoal@Focus@TetherActor );

		UpdateFocalPoint();
	}

	Jack.Throttle = 0;
	Jack.Steering = 0;
	Jack.Rise	  = 0;

	Sleep(0.1f+FRand()*0.5f);
	
	Goto('Begin');
}

function UpdateFocalPoint()
{
	if( TetherActor != None )
	{
		if( NavigationPoint(TetherActor) == None )
		{
			// Look at the actor we were told to move to
			SetFocalPoint( TetherActor.Location, TRUE );
			return;
		}
	}
	
	if( Focus == None )
	{
		Focus = TetherActor;
	}

	if( Focus == None )
	{
		SetFocalPoint( Pawn.Location + Vector(Pawn.Rotation) * 512.f, TRUE );
	}	
}

function bool ShouldForceTeleport( Vector MovePt )
{
	if( TimeSince( LastReachedMoveGoalTime ) > 10.f )
	{
		if( LastReachedMoveGoalTime == 0.f || PointReachable( MovePt ) )
		{
			LastReachedMoveGoalTime = WorldInfo.TimeSeconds;
			return FALSE;
		}

		return TRUE;
	}
	return FALSE;
}

function ReachedMoveGoal()
{
	LastIdleLocation = vect(0,0,0);

	if( bReachedMoveGoal )
	{
		LastReachedMoveGoalTime = WorldInfo.TimeSeconds;
	}	
}

final function bool GetObstructionAdjustDir( Vector AvoidLoc, out Vector out_Dir )
{
	local Vector X, Y, Z, VectToDanger;
	local bool bLeftOpen, bRightOpen, bFrontOpen, bBackOpen;
	local float DotX, DotY;

	// Figure out which directions we can evade
	GetAxes( Pawn.Rotation, X, Y, Z );
	bRightOpen = FastTrace( Pawn.Location + Y * 256.f, Pawn.Location );
	bLeftOpen  = FastTrace( Pawn.Location - Y * 256.f, Pawn.Location );
	bFrontOpen = FastTrace( Pawn.Location + X * 256.f, Pawn.Location );
	bBackOpen  = FastTrace( Pawn.Location - X * 256.f, Pawn.Location );

	// Get Dots to the point
	VectToDanger = Normal(AvoidLoc-Pawn.Location);
	DotX = X DOT VectToDanger;
	DotY = Y DOT VectToDanger;

	// If dot is mostly forward or backward
	if( DotX >= 0.7071 ||
		DotX <= -0.7071 )
	{
		// If can evade back and should
		if( bBackOpen && DotX >= 0.7071 )
		{
			// Evade back
			out_Dir = -X;
			return TRUE;
		}
		else
		// Otherwise, if can evade forward and should
		if( bFrontOpen && DotX <= -0.7071 )
		{
			out_Dir = X;
			return TRUE;
		}
		else
		// Otherwise, if can evade right and should OR
		// Wants to evade left but can't
		if( (bRightOpen && DotY < 0) ||
			(DotY >= 0 && !bLeftOpen) )
		{
			// Evade right
			out_Dir = Y;
			return TRUE;
		}
		else
		// Otherwise, if can evade left and should OR
		// Wants to evade right but can't
		if( (bLeftOpen && DotY >= 0) ||
			(DotY < 0 && !bRightOpen) )
		{
			out_Dir = -Y;
			return TRUE;
		}
	}
	else
	{
		// If can evade left and should
		if( bLeftOpen && DotY >= 0.7071 )
		{
			// Evade left
			out_Dir = -Y;
			return TRUE;
		}
		else
		// Otherwise, if can evade right and should
		if( bRightOpen && DotY <= -0.7071 )
		{
			// Evade right
			out_Dir = Y;
			return TRUE;
		}
		else
		// Otherwise, if can evade forward and should OR
		// Wants to evade back but can't
		if( (bFrontOpen && DotX < 0) ||
			(DotX >= 0 && !bBackOpen) )
		{
			// Evade forward
			out_Dir = X;
			return TRUE;
		}
		else
		// Otherwise, if can evade back and should OR
		// Wants to evade forward but can't
		if( (bBackOpen && DotX >= 0) ||
			(DotX < 0 && !bFrontOpen) )
		{
			out_Dir = -X;
			return TRUE;
		}
	}

	return FALSE;
}

function bool DoRecoil()
{
	//debug
	`AILog( GetFuncName() );

	if( Jack.Recoil() )
	{
		return class'AICmd_React_JackRecoil'.static.Recoil( self );
	}
}

simulated function OnToggleCinematicMode( SeqAct_ToggleCinematicMode Action )
{
	Super.OnToggleCinematicMode( Action );

	ClearTimer( 'DelayJackCloak' );
	ClearTimer( 'DelayJackDecloak' );
}

function bool TeleportToGoodSpot( Vector DesiredLoc, optional bool bClearMovement = TRUE; )
{
	//debug
	`AILog( GetFuncName()@DesiredLoc@bClearMovement );

	ClearTimer( 'DelayJackCloak' );
	ClearTimer( 'DelayJackDecloak' );

	if( bClearMovement )
	{
		ClearMovementInfo();
	}	
	return class'AICmd_React_JackTeleport'.static.Teleport( self, DesiredLoc );
}

/** 
 * Accessor function for toggling Jack's cloaking device.
 * Allows chance for guds sounds, delays, etc.
 */
function bool SetCloaking( bool inbCloak, optional bool bScripted, optional bool bForce, optional float Delay )
{
//	`log( "A"@bScriptedCloaking@bScripted@"--"@inbCloak@bForce );

	if( !bForce && bScriptedCloaking && !bScripted )
	{
		return FALSE;
	}

	//debug
	`AILog( GetFuncName()@inbCloak@bScripted@bForce@Delay@Jack.IsCloaked()@ShouldMoveToSquad()@"Timers"@IsTimerActive('DelayJackCloak')@IsTimerActive('DelayJackDecloak') );

	if( ( inbCloak &&  Jack.IsCloaked()) ||
		(!inbCloak && !Jack.IsCloaked()) )
	{
		//debug
		`AILog( "Already cloaking..."@Jack.bCloaking@bScriptedCloaking );

		return Jack.bCloaking && bScriptedCloaking;
	}

	if( Delay <= 0.f )
	{
		ClearTimer( 'DelayJackCloak' );
		ClearTimer( 'DelayJackDecloak' );

		// If this is a non-scripted decloak and Jack is too far from squad
		if( !inbCloak && Jack.IsCloaked() )
		{
			if( !bForce && !bScripted && ShouldMoveToSquad() )
			{
				TeleportToGoodSpot( GetSquadLeaderLocation() );
				return FALSE;
			}
		}

		return Jack.SetCloakState( inbCloak ? Jack.JackFoldAnimName : '', bForce );
	}

	if( inbCloak && !IsTimerActive('DelayJackCloak') )
	{
		//debug
		`AILog( "Set Delay Cloak" );

		ClearTimer( 'DelayJackDecloak' );
		SetTimer( Delay, FALSE, nameof(DelayJackCloak) );
	}
	else
	if( !inbCloak && !IsTimerActive('DelayJackDecloak') )
	{
		//debug
		`AILog( "Set Delay DeCloak" );

		ClearTimer( 'DelayJackCloak' );
		SetTimer( Delay, FALSE, nameof(DelayJackDecloak) );
	}
	return TRUE;
}

function DelayJackCloak()
{
	ClearTimer( 'DelayJackCloak' );
	ClearTimer( 'DelayJackDecloak' );

	//debug
	`AILog( GetFuncName()@Jack.IsCloaked() );

	if( !Jack.IsCloaked() )
	{
		// Play recoil anim half the time
		Jack.SetCloakState( FRand() < 0.5f ? Jack.JackRecoilAnimName : Jack.JackFoldAnimName );
	}
}

function DelayJackDecloak()
{
	ClearTimer( 'DelayJackCloak' );
	ClearTimer( 'DelayJackDecloak' );

	NextPOITime = WorldInfo.TimeSeconds + 20.f + FRand() * 20.f;

	//debug
	`AILog( GetFuncName()@Jack.IsCloaked()@ShouldMoveToSquad() );

	// If this is a non-scripted decloak and Jack is too far from squad
	if( Jack.IsCloaked() )
	{
		if( ShouldMoveToSquad() )
		{
			// Teleport to better location before decloaking
			TeleportToGoodSpot( GetSquadLeaderLocation() );
		}
		else
		{
			Jack.SetCloakState( '' );
		}
	}
}

function OnJackControl( SeqAct_JackControl inAction )
{
	local bool bClearLatent, bAborted;

	ClearTimer( 'DelayJackCloak' );
	ClearTimer( 'DelayJackDecloak' );

	// Cloak/Decloak
	if( inAction.InputLinks[0].bHasImpulse ||
		inAction.InputLinks[1].bHasImpulse )
	{
		//debug
		`AILog( "..." );
		
		bScriptedCloaking = TRUE;
		bClearLatent = !SetCloaking( inAction.InputLinks[0].bHasImpulse, TRUE );
		bAborted	 = bClearLatent;
		
		//debug
		`AILog( GetFuncName()@"Cloak"@inAction.InputLinks[0].bHasImpulse@bClearLatent@inAction );
	}
	else
	// Lights on/off
	if( inAction.InputLinks[2].bHasImpulse ||
		inAction.InputLinks[3].bHasImpulse )
	{
		Jack.SetSpotLightState( inAction.InputLinks[2].bHasImpulse );
		bClearLatent = TRUE;

		//debug
		`AILog( GetFuncName()@"Lights"@inAction.InputLinks[2].bHasImpulse@bClearLatent@inAction );
	}
	else
	// Unfold/Fold monitor
	if( inAction.InputLinks[4].bHasImpulse ||
		inAction.InputLinks[5].bHasImpulse )
	{
		bClearLatent = !Jack.SetMonitorState( inAction.InputLinks[4].bHasImpulse );
		bAborted	 = bClearLatent;

		//debug
		`AILog( GetFuncName()@"Monitors"@inAction.InputLinks[4].bHasImpulse@bClearLatent@inAction );
	}
	else
	// Start/Stop welding
	if( inAction.InputLinks[6].bHasImpulse ||
		inAction.InputLinks[7].bHasImpulse )
	{
		bClearLatent = !Jack.SetWeldingState( inAction.InputLinks[6].bHasImpulse );
		bAborted	 = bClearLatent;

		//debug
		`AILog( GetFuncName()@"Welding"@inAction.InputLinks[6].bHasImpulse@bClearLatent@inAction );
	}
	else
	// Recoil
	if( inAction.InputLinks[8].bHasImpulse )
	{
		bClearLatent = !Jack.Recoil();
		bAborted	 = bClearLatent;

		//debug
		`AILog( GetFuncName()@"Recoil"@bClearLatent@inAction );
	}
	else
	// Start/Stop scan
	if( inAction.InputLinks[9].bHasImpulse ||
		inAction.InputLinks[10].bHasImpulse )
	{
		bClearLatent = !Jack.SetScanningState( inAction.InputLinks[9].bHasImpulse );
		bAborted	 = bClearLatent;
		bClearLatent = TRUE;

		//debug
		`AILog( GetFuncName()@"Scanning"@inAction.InputLinks[9].bHasImpulse@bAborted@inAction );
	}
	else
	// Pointing
	if( inAction.InputLinks[11].bHasImpulse )
	{
		bClearLatent = !Jack.SetPointingState( TRUE );
		bAborted	 = bClearLatent;

		//debug
		`AILog( GetFuncName()@"Pointing"@inAction );
	}
	else
	// Reset LD control of Jack
	if( inAction.InputLinks[12].bHasImpulse )
	{
		bScriptedCloaking = FALSE;
		bClearLatent = TRUE;

		//debug
		`AILog( GetFuncName()@"Release"@inAction );
	}

	if( bClearLatent )
	{
		ClearLatentAction( class'SeqAct_JackControl', bAborted );
	}
}

function OnAISetTarget( SeqAct_AISetTarget InAction )
{
	local JackPointOfInterest POI;
	local Actor NewTarget;

	//debug
	`AILog( GetFuncName()@"override?"@InAction.bOverwriteExisting );

	// clear the existing list if specified
	if( InAction.bOverwriteExisting )
	{
		JackInterestList.Length = 0;
	}

	// copy the targets
	foreach InAction.FocusTargets( NewTarget )
	{
		POI = JackPointOfInterest(NewTarget);
		if( POI != None )
		{
			POI.bInvestigated = FALSE;
			JackInterestList.AddItem( POI );
		}		
	}

	bLDSetPOI = (JackInterestList.Length > 0);
}

function CheckAreaForPOI()
{
	local JackPointOfInterest JPOI;

	if( LDSetPOI() )
		return;

	JackInterestList.Length = 0;
	foreach Pawn.AllActors( class'JackPointOfInterest', JPOI )
	{
		if( !JPOI.bInvestigated && VSize(Pawn.Location-JPOI.Location) < 512.f )
		{
			JackInterestList[JackInterestList.Length] = JPOI;
		}
	}

	//debug
	`AILog( GetFuncName()@"POI List"@JackInterestList.Length );
}

function bool InvestigatePOI( JackPointOfInterest POI, optional bool bSetTether )
{
	//debug
	`AILog( GetFuncName()@POI );

	if( bSetTether )
	{
		TetherActor = POI;
	}

	return class'AICmd_React_JackInvestigate'.static.Investigate( self, POI );
}

function bool ShouldPrintStaleWarning()
{
	return FALSE;
}

function bool NotifyBump( Actor Other, vector HitNormal )
{
	local Pawn bumpPawn;
	local AICmd_MoveToGoal ActiveMoveCmd;

	//debug
	`AILog(GetFuncName()@Other);

	bumpPawn = Pawn(Other);
	if( bumpPawn != None )
	{
		ActiveMoveCmd = AICmd_MoveToGoal(GetActiveCommand());
		if (ActiveMoveCmd != None && MoveGoal == bumpPawn)
		{
			`AILog("Popping Move To Goal command early because bumped into MoveGoal");
			ActiveMoveCmd.Status = 'Success';
			PopCommand(ActiveMoveCmd);
		}
		if( IsFriendlyPawn( bumpPawn ) )
		{
			if( Pawn.Base != bumpPawn &&
				bumpPawn.Base != Pawn )
			{
				//debug
				`AILog(GetFuncName()@Other);

				// try to step aside for them
				StepAsideFor( bumpPawn );
			}
		}
	}

	return FALSE;
}

function bool StepAsideFor( Pawn ChkPawn )
{
	// Don't let jack be bumped out of the way if he's welding scanning or pointing
	if( Jack == None	|| 
		Jack.bWelding	||
		Jack.bScanning	||
		Jack.bPointing	)
	{
		return FALSE;
	}

	return Super.StepAsideFor( ChkPawn );
}

native function bool FindGoodTeleportSpot( Vector ChkExtent, out Vector out_Dest );

defaultproperties
{
	DefaultCommand=class'AICmd_Base_Jack'

	DefaultReactConditions.Empty
	DefaultReactConditionClasses.Empty

	bAllowCombatTransitions=FALSE
	bIgnoreSquadPosition=TRUE
	bCanRevive=false

	MaxStepAsideDist=128.f
}
