class AICmd_Move_DropFromCeiling_Skorge extends AICommand
	within GearAI_Skorge;
	
/** Location for skorge to teleport to when he finishes his leap */	
var()	Actor	Destination;
	
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Jump( GearAI_Skorge AI, Actor Dest )
{
	local AICmd_Move_DropFromCeiling_Skorge Cmd;

	if( AI != None && Dest != None )
	{
		Cmd = new(AI) class'AICmd_Move_DropFromCeiling_Skorge';
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
	
	// Teleport Skorge
	if( !TeleportToLocation( Destination.Location, Destination.Rotation, FALSE ) )
	{
		//debug
		`AILog( "Failed to teleport to start" );

		GotoState( 'DelayFailure' );
		return;
	}

	Focus = None;

	class'AICmd_Move_DropFromCeiling'.static.Jump( AIOwner, None, None );
}

function Popped()
{
	Super.Popped();

	ClearLatentAction( class'SeqAct_Skorge_LeapToCeiling', (Status!='Success') );
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