class AICmd_Move_JumpToCeiling_Skorge extends AICommand
	within GearAI_Skorge;
	
/** Location for skorge to teleport to when he finishes his leap */	
var()	Actor	Destination;
	
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Jump( GearAI_Skorge AI, Actor Dest )
{
	local AICmd_Move_JumpToCeiling_Skorge Cmd;

	if( AI != None && Dest != None )
	{
		Cmd = new(AI) class'AICmd_Move_JumpToCeiling_Skorge';
		if( Cmd != None )
		{
			Cmd.Destination	= Dest;

			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

	
function Pushed()
{
	Super.Pushed();

	Focus = None;

	// Immediately jump back to ceiling
	class'AICmd_Move_JumpToCeiling'.static.Jump( AIOwner, None, None );
}

function Popped()
{
	Super.Popped();
	
	TeleportToLocation( Destination.Location, Destination.Rotation, FALSE );
	Pawn.SetPhysics( PHYS_None );

	ClearLatentAction( class'SeqAct_Skorge_LeapToCeiling', (Status!='Success') );
	StopFiring();
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( ChildStatus != 'Success' )
	{
		GotoState( 'DelayFailure' );
		return;
	}

	if( OldCommandName == 'AICmd_Move_JumpToCeiling' )
	{
		Status = 'Success';
		PopCommand( self );
	}
	else
	{
		`AILog( "ERROR: RESUMED"@self@"after"@OldCommandName@"?!?!?!?!" );	
	}
}
	
defaultproperties
{
	bIgnoreNotifies=TRUE
}