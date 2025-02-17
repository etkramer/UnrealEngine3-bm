class AICmd_Nemacyst_Launch extends AICommand
	within GearAI_Nemacyst;
	
/** Simple constructor that pushes a new instance of the command for the AI */
static function bool Launch( GearAI_Nemacyst AI )
{
	local AICmd_Nemacyst_Launch Cmd;

	if( AI != None && AI.Pawn != None )
	{
		Cmd = new(AI) class'AICmd_Nemacyst_Launch';
		if( Cmd != None )
		{
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();

	// don't allow interruptions!
	ReactionManager.SuppressAll();
	bLaunching = TRUE;
	GotoState( 'Lauching' );
}

function Popped()
{
	Super.Popped();

	// restore
	ReactionManager.UnSuppressAll();
	bLaunching = FALSE;
	FinishedLaunch();
}

function vector GetLaunchDestination()
{
	local vector X, Y, Z;

	GetAxes(Pawn.Rotation,X,Y,Z);
	return Pawn.Location + X * 768;
}

function vector GetHoverDestination()
{
	return Pawn.Location + vect(0,0,1) * RandRange(128.f,256.f) + vect(0,1,0) * RandRange(-256,256) + vect(1,0,0) * RandRange(-256,256);
}

function PrepareForLaunch()
{
	//debug
	`AILog( GetFuncName() );

	Pawn.SetCollision(FALSE,FALSE);
	Pawn.bCollideWorld = FALSE;
	Pawn.SetPhysics(PHYS_Flying);
	Pawn.AirSpeed = 2048.f;
	bRestoreAirSpeed = TRUE;

	// Start at full speed
	Pawn.Velocity = Vector(Pawn.Rotation) * Pawn.AirSpeed;
}

function FinishedLaunch()
{
	//debug
	`AILog( GetFuncName() );

	Pawn.SetCollision(TRUE,TRUE);
	Pawn.bCollideWorld = TRUE;
}

state Lauching
{
Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	// set them to flying and add an initial velocity
	PrepareForLaunch();

	// and transition after a slight delay
	MoveTo(GetLaunchDestination());
	FinishedLaunch();

	MoveTo(GetHoverDestination());

	BeginCombatCommand( None, "Lauching complete" );
}
