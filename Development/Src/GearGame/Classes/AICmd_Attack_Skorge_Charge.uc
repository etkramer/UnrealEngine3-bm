
class AICmd_Attack_Skorge_Charge extends AICommand_SpecialMove
	within GearAI_Skorge;

var GearPawn	ChargeTarget;
var bool		bDBNO;
var bool		bPrepareForJump;
var	float		MinJumpDistance;

/** 
 * Simple constructor that pushes a new instance of the command for the AI 
 * @param	bPrepareForJump	if TRUE, Skorge will stop charging when he's at a given distance from target in order to do a jump to chain a jump.
 */
static function bool Charge( GearAI_Skorge AI, GearPawn inTarget, bool inbDBNO, bool inbPrepareForJump )
{
	local AICmd_Attack_Skorge_Charge Cmd;

	if( AI != None && inTarget != None )
	{
		Cmd = new(AI) class'AICmd_Attack_Skorge_Charge';
		if( Cmd != None )
		{
			Cmd.ChargeTarget	= inTarget;
			Cmd.bDBNO			= inbDBNO;
			Cmd.bPrepareForJump	= inbPrepareForJump;
			
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Target:"@ChargeTarget@bDBNO;
}

function Pushed()
{
	Super.Pushed();

	bMovingToGoal = TRUE;
	bMoveGoalInterruptable = FALSE;
	
	GotoState( 'Command_SpecialMove' );
}

function Popped()
{
	Super.Popped();

	bMovingToGoal = FALSE;
	bMoveGoalInterruptable = TRUE;
}

function LockdownAI();
function UnlockAI();

final function float GetThreshold()
{
	if( bPrepareForJump )
	{
		// Make sure there is no geometry obstruction to charge target.
		if( Pawn.FastTrace(ChargeTarget.Location) )
		{
			return MinJumpDistance;
		}
	}

	return (Pawn.GetCollisionRadius() + ChargeTarget.GetCollisionRadius()) * 1.2f;
}

state Command_SpecialMove
{
Begin:
	//debug
	`AILog( "--"@GetStateName()$":"$class@"-- BEGIN TAG", 'State' );

	if( bPrepareForJump )
	{
		SkorgePawn.bBlockingBullets = TRUE;
		SkorgePawn.AC_StaffTwirl.Play();
	}

	while( VSize(ChargeTarget.Location-Pawn.Location) > GetThreshold() )
	{
		SetTimer( 0.25f, FALSE, nameof(self.FailMove), self );
		MoveToward( ChargeTarget, ChargeTarget );
	}
	
	if( bPrepareForJump )
	{
		SkorgePawn.bBlockingBullets = FALSE;
		SkorgePawn.AC_StaffTwirl.Stop();
	}

	if( bDBNO )
	{
		ChargeTarget.bCanRecoverFromDBNO = FALSE;
		if( !ChargeTarget.IsDBNO() )
		{
			ChargeTarget.EnterDBNO(Pawn.Controller, class'GearDamageType');
		}
	}

	GotoState( 'DelaySuccess' );
}

simulated function FailMove()
{
	MoveTimer = -1.f;
}

defaultproperties
{
	MinJumpDistance=700
	bIgnoreNotifies=TRUE
}