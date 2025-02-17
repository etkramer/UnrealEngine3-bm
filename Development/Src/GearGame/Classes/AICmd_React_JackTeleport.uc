class AICmd_React_JackTeleport extends AICommand
	within GearAI_Jack;

var Vector	Destination;
var bool	bGotStuck;
	
static function bool Teleport( GearAI_Jack AI, Vector inDestination )
{
	local AICmd_React_JackTeleport Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_React_JackTeleport';
		if( Cmd != None )
		{
			Cmd.Destination = inDestination;
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}
	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Dest"@Destination;
}

function Pushed()
{
	Super.Pushed();
	
	bTeleporting = TRUE;
	GotoState( 'TeleportJack' );
}

function Popped()
{
	Super.Popped();

	bTeleporting = FALSE;
	Pawn.SetAnchor( None );
}

function CloakStuckCheck()
{
	//debug
	`AILog( "Jack got stuck cloaking?! hmmmmmm...." );

	bGotStuck = TRUE;

	Jack.CloakedAnimName = Jack.JackFoldAnimName;
	Jack.bCloaking = FALSE;
}

state TeleportJack `DEBUGSTATE
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@Jack.CloakedAnimName );

	Jack.Throttle = 0;
	Jack.Steering = 0;
	Jack.Rise	  = 0;

	Sleep( 1.f );

	// If not cloaked... Cloak jack
	if( !Jack.IsCloaked() )
	{
		//debug
		`AILog( "Tell jack to cloak..." );

		SetTimer( 5.f, FALSE, nameof(self.CloakStuckCheck), self );

		Jack.SetCloakState( Jack.JackFoldAnimName );
		do
		{
			Sleep( 0.1f );
		} until( !Jack.bCloaking );

		ClearTimer( 'CloakStuckCheck', self );

		//debug
		`AILog( "Jack finished cloaking"@Jack.bCloaking );
	}

	if( FindGoodTeleportSpot( Pawn.GetCollisionExtent() * vect(1.2,1.2,1.2), Destination ) &&
		TeleportToLocation( Destination, Pawn.Rotation, FALSE, FALSE ) )
	{
		//debug
		`AILog( "Jack got to a good spot - uncloak him" );

		Status = 'Success';
	}
	else
	{
		//debug
		`AILog( "Failed to find spot for jack to teleport to" );

		Status = 'Failure';
	}

	Sleep( 0.25f );

	// Uncloak jack
	Jack.SetCloakState( '' );
	do
	{
		Sleep( 0.1f );
	} until( !Jack.bCloaking );

	
	PopCommand( self );
}

