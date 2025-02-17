class AICmd_Nemacyst_Investigate extends AICommand
	within GearAI_Nemacyst;

/** GoW global macros */

var vector MovePt;
var vector InvestigatePt;
var int loopIts;
var float OldAirSpeed;

static function bool InitCommandUserActor( GearAI AI, Actor UserActor )
{
	local AICmd_Nemacyst_Investigate Cmd;
	`log(GetFuncName()@AI);

	if( AI != None)
	{
		Cmd = new(GearAI_Nemacyst(AI)) Default.Class;
		if( Cmd != None )
		{
			Cmd.InvestigatePt = UserActor.Location;
			AI.PushCommand(Cmd);
			return TRUE;
		}
	}

	return FALSE;
}

function Pushed()
{
	Super.Pushed();
	//MessagePlayer(self@"investigating noise at "@InvestigatePt );
	GotoState('Investigate');
}


state Investigate `DEBUGSTATE
{

Begin:
	`AILog("Investigating noise at "$InvestigatePt$" made by "$Enemy);

Loop:
	`AILog("Not close enough to "$Enemy$" moving closer!");
	if(!SuggestNewMovePoint(MovePt, Normal(InvestigatePt - Pawn.Location)))
	{
		`AILog("Couldn't get move point near target, moving directly to target");
		MovePt = InvestigatePt;
		MovePt.Z = Pawn.Location.Z;
		GotoState('DelayFailure');
	}
	OldAirSpeed = pawn.AirSpeed;
	pawn.AirSpeed=pawn.AirSpeed*1.25f;
	MoveTo(MovePt);
	//Sleep(1.0);
	if(loopIts++ < 50 && VSizeSq2D(pawn.Location - InvestigatePt) > 512.f*512.f)
	{
		Goto('Begin');
	}
	else
	{
		`AILog("Was close enough, or hit max itts.. stopping and spinning around!");
		pawn.Acceleration = vect(0.f,0.f,0.f);
		Sleep(7.0f);
	}
	Pawn.AirSpeed = OldAirSpeed;
	GotoState('DelaySuccess');
};