class AICmd_React_JackInvestigate extends AICommand
	within GearAI_Jack;

/** POI Jack is investigating */
var() JackPointOfInterest	POI;
/** Time that jack should stop investigating this point */
var() float					InvestigateEndTime;
/** Next idle POI interest sound */
var() float					IdleInterestSoundTime;
/** Duration of time to investigate this actor */
var() float					InvestigateDuration;
	
static function bool Investigate( GearAI_Jack AI, JackPointOfInterest inPOI, optional float inInvestigateDuration = -1.f )
{
	local AICmd_React_JackInvestigate Cmd;

	if( AI != None && inPOI != None )
	{
		Cmd = new(AI) class'AICmd_React_JackInvestigate';
		if( Cmd != None )
		{
			Cmd.POI	= inPOI;
			Cmd.InvestigateDuration = inInvestigateDuration;
			AI.PushCommand( Cmd );
			return TRUE;
		}
	}
	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"POI"@POI;
}

function Pushed()
{
	Super.Pushed();

	// Remove the used POI
	POI.bInvestigated = TRUE;

	// Play sound/effects, etc when Jack spots something interesting
	Jack.AcquiredPOI( POI );

	MoveToPOI();
}

function Popped()
{
	Super.Popped();

	NextPOITime = WorldInfo.TimeSeconds + 20.f + FRand() * 20.f;
	LastReachedMoveGoalTime = 0.f;
	Focus = None;
}

function Resumed( Name OldCommandName )
{
	Super.Resumed( OldCommandName );

	if( ChildStatus == 'Success' )
	{
		if( OldCommandName == 'AICmd_MoveToGoal' )
		{
			if( MoveAction != None )
			{
				MoveAction.ReachedGoal( AIOwner );
			}
			GotoState( 'Investigating', 'Begin' );
		}
		else
		{
			// If teleported and had a move action, let kismet know we got there!
			if( OldCommandName == 'AICmd_React_JackTeleport' )
			{
				if( MoveAction != None )
				{
					MoveAction.ReachedGoal( AIOwner );
				}
			}

			MoveToPOI();
		}		
	}
	else
	{
		if( ChildStatus != 'Aborted' &&
			OldCommandName == 'AICmd_MoveToGoal' &&
			TeleportToGoodSpot( POI.Location, FALSE ) )
		{
			return;
		}
			
		Status = 'Failure';
		PopCommand( self );
	}
}

function MoveToPOI()
{
	if( !Pawn.ReachedDestination( POI ) )
	{
		SetMovePoint( POI.Location,, MoveIsInterruptable() );
	}
	else
	{
		GotoState( 'Investigating', 'Begin' );
	}	
}

function UpdateFocus()
{
	if( POI.bDirectional )
	{
		Focus = None;
		SetFocalPoint( POI.FocalPoint );
	}
	else
	{
		Focus = POI;
	}		
}

state Investigating `DEBUGSTATE
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		LastIdleLocation = POI.Location;

		if( InvestigateEndTime == 0.f )
		{
			InvestigateEndTime	  = WorldInfo.TimeSeconds + ((InvestigateDuration > 0.f) ? InvestigateDuration : (10.f + FRand()*10.f));
			IdleInterestSoundTime = WorldInfo.TimeSeconds + 5.f + FRand()*5.f;
		}
	}

Begin:
	//debug
	`AILog( "BEGIN TAG"@TetherActor@POI );

	UpdateFocus();

	Jack.Throttle = 0;
	Jack.Steering = 0;
	Jack.Rise	  = 0;

	while( (WorldInfo.TimeSeconds < InvestigateEndTime) || (TetherActor == POI) )
	{
		if( WorldInfo.TimeSeconds > IdleInterestSoundTime )
		{
			IdleInterestSoundTime = WorldInfo.TimeSeconds + 5.f + FRand()*5.f;
			Jack.InspectingPOI( POI );
		}

		Sleep( 0.5f );
	}

	Status = 'Success';
	PopCommand( self );
}

defaultproperties
{
	bReplaceActiveSameClassInstance=TRUE
}