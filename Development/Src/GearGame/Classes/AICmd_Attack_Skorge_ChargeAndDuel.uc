class AICmd_Attack_Skorge_ChargeAndDuel extends AICommand_Base_Combat
	within GearAI_Skorge;

var GearAI_Skorge				Skorge;
var SeqAct_Skorge_ChargeAndDuel	SeqAct;

/** Pawn that we will duel */
var GearPawn	DuelTarget;
/** Last target we charged at */
var GearPawn	LastChargeTarget;

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Duel:"@DuelTarget;
}
	
function Pushed()
{
	local GearPawn E;

	Skorge = GearAI_Skorge(AIOwner);
	SeqAct = SeqAct_Skorge_ChargeAndDuel(Skorge.CurrentAttackSeq);

	// Select the Stun/Duel targets
	foreach AIOwner.WorldInfo.AllPawns( class'GearPawn', E )
	{
		// (ONLY DUEL HUMAN ENEMIES)
		// Skip pawns on same team or controlled by AI
		if( E.GetTeam() == None || Pawn.IsSameTeam( E ) || !E.IsHumanControlled() )
			continue;

		// Skip if enemy is DBNO (or not a valid target for some other reason)
		if( !E.IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
			continue;

		if( DuelTarget == None || VSize(E.Location-Pawn.Location) < VSize(DuelTarget.Location-Pawn.Location) )
		{
			DuelTarget = E;
		}
	}

	Super.Pushed();

	// Teleport to start location
	if( SeqAct.StartPosition != None && 
		!TeleportToLocation( SeqAct.StartPosition.Location, SeqAct.StartPosition.Rotation, FALSE ) )
	{
		//debug
		`AILog( "Failed to teleport to start position"@SeqAct.StartPosition );

		GotoState( 'DelayFailure' );
		return;
	}

	// Failed to get a duel target
	if( DuelTarget == None )
	{
		//debug
		`AILog( "Failed to get a duel target" );

		// Force draw
		SeqAct.DuelResult = 2;

		// Jump back to ceiling immediately
		class'AICmd_Move_JumpToCeiling'.static.Jump( AIOwner, None, None );
		return;
	}

	SkorgePawn.HideGun( TRUE );

	SkorgePawn.GroundSpeed = 400;

	ForceUpdateOfEnemies();
	SetTimer( 0.25f, FALSE, nameof(ForceEnemiesToUpdateTarget) );

	// Charge over to duel target
	ChargeAt( DuelTarget );
}

function Popped()
{
	Super.Popped();

	TeleportToLocation( SeqAct.Destination.Location, SeqAct.Destination.Rotation, FALSE );
	Pawn.SetPhysics( PHYS_None );
	SkorgePawn.GroundSpeed = SkorgePawn.DefaultGroundSpeed;

	ClearLatentAction( class'SeqAct_Skorge_ChargeAndDuel', Status!='Success' || DuelTarget == None );
	CurrentAttackSeq = None;

	SetTimer( 0.25f, FALSE, nameof(ForceEnemiesToUpdateTarget) );
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( OldCommandName == 'AICmd_Attack_Skorge_Charge' )
	{
		// Abort if enemy is DBNO (or not a valid target for some other reason)
		if( !DuelTarget.IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
		{
			//debug
			`AILog( "Duel target was put dbno during charge" );

			// Force draw
			SeqAct.DuelResult = 2;

			// Jump back to ceiling immediately
			class'AICmd_Move_JumpToCeiling'.static.Jump( AIOwner, None, None );
			return;
		}

		if( DuelTarget.IsA( 'GearPawn_COGMarcus' ) )
		{
			SeqAct.ActivateOutputLink( 3 );
		}
		else
		{
			SeqAct.ActivateOutputLink( 4 );
		}

		MyGearPawn.ServerDoSpecialMove(SM_ChainsawDuel_Leader, TRUE, DuelTarget);
		SetTimer( 0.25f, FALSE, nameof(ForceEnemiesToUpdateTarget) );
	}
	else
	if( OldCommandName == 'AICmd_Move_JumpToCeiling' )
	{
		// Done with charge attack
		// Pop command - Success if player won duel, fail if they lost
		Status = 'Success';
		PopCommand( self );
	}
	// Otherwise, duel is complete...
	else
	{
		// Jump back to ceiling
		class'AICmd_Move_JumpToCeiling'.static.Jump( AIOwner, None, None );
	}
}

function ChargeAt( GearPawn CT )
{
	LastChargeTarget = CT;
	class'AICmd_Attack_Skorge_Charge'.static.Charge( Skorge, CT, FALSE, TRUE );
}
	
defaultproperties
{
	InitialTransitionCheckTime=(X=0.f,Y=0.f)
	bIgnoreNotifies=TRUE
}