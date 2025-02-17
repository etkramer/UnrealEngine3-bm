class AICmd_Attack_Skorge_PistolStrafe extends AICommand_Base_Combat
	within GearAI_Skorge;

var GearAI_Skorge				Skorge;
var GearPawn_LocustSkorgeBase	SkorgePawn;
var SeqAct_Skorge_PistolStrafe	SeqAct;

function Pushed()
{
	Super.Pushed();

	// Store Skorge and Kismet action pointers
	Skorge = GearAI_Skorge(AIOwner);
	SkorgePawn = GearPawn_LocustSkorgeBase(Skorge.Pawn);
	SeqAct = SeqAct_Skorge_PistolStrafe(Skorge.CurrentAttackSeq);

	// Get location Skorge should jump to
	if( SeqAct.Start == None )
	{
		//debug
		`AILog( "Failed to get start actor to teleport to" );

		GotoState( 'DelayFailure' );
		return;
	}

	// Teleport Skorge
	if( !TeleportToLocation( SeqAct.Start.Location, SeqAct.Start.Rotation, FALSE ) )
	{
		//debug
		`AILog( "Failed to teleport to start of route" );

		GotoState( 'DelayFailure' );
		return;
	}

	Skorge.MyGearPawn.bAllowTurnSmoothing  = FALSE;
	Skorge.MyGearPawn.bAllowAccelSmoothing = FALSE;
	Skorge.bPistolStrafeAttack = TRUE;

	SkorgePawn.HideGun( FALSE );

	SetTimer( 0.25f, FALSE, nameof(ForceEnemiesToUpdateTarget) );

	class'AICmd_Move_DropFromCeiling'.static.Jump( AIOwner, None, None );
}

function Popped()
{
	Super.Popped();

	TeleportToLocation( SeqAct.Destination.Location, SeqAct.Destination.Rotation, FALSE );
	Pawn.SetPhysics( PHYS_None );

	Skorge.MyGearPawn.bAllowTurnSmoothing  = Skorge.MyGearPawn.default.bAllowTurnSmoothing;
	Skorge.MyGearPawn.bAllowAccelSmoothing = Skorge.MyGearPawn.default.bAllowAccelSmoothing;
	Skorge.bPistolStrafeAttack = FALSE;

	ClearLatentAction( class'SeqAct_Skorge_PistolStrafe', FALSE );
	CurrentAttackSeq = None;
	StopFiring();

	SkorgePawn.HideGun( TRUE );

	SetTimer( 0.25f, FALSE, nameof(ForceEnemiesToUpdateTarget) );
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( ChildStatus != 'Success' )
	{
		GotoState( 'DelayFailure' );
		return;
	}

	if( OldCommandName == 'AICmd_Move_DropFromCeiling' )
	{
		// Start moving along the route
		MoveAlongRoute();
	}
	else
	if( OldCommandName == 'AICmd_Move_JumpToCeiling' )
	{
		// Done with pistol strafe attack
		Status = 'Success';
		PopCommand( self );
	}
	else
	{
		`AILog( "ERROR: RESUMED"@self@"after"@OldCommandName@"?!?!?!?!" );	
	}
}

function MoveAlongRoute()
{
	// Set timer for the duration of the route
	SkorgePawn.CompleteStrafeAttackTime = SkorgePawn.WorldInfo.TimeSeconds + SeqAct.RouteDuration;
	SetTimer( 0.25f, TRUE, nameof(self.CheckRouteDurationExpired), self );

	StartFiring();
	SetRouteMoveGoal( SeqAct.RouteObj, ERD_Forward, FALSE );
}

function CheckRouteDurationExpired()
{
	// Route running complete
	if( WorldInfo.TimeSeconds > SkorgePawn.CompleteStrafeAttackTime )
	{
		// Clear route and timer
		AbortCommand( None, class'AICmd_MoveToRoute' );
		ClearTimer( 'CheckRouteDurationExpired', self );
		StopFiring();

		// Jump back to ceiling
		class'AICmd_Move_JumpToCeiling'.static.Jump( AIOwner, None, None );
	}
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return FALSE;
}

	
defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
	bIgnoreNotifies=TRUE
}