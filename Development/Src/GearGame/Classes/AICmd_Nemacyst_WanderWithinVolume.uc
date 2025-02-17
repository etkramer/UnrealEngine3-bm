class AICmd_Nemacyst_WanderWithinVolume extends AICommand
	within GearAI_Nemacyst;

/** GoW global macros */

var vector MovePt;
var() float RandomCoef;

function Pushed()
{
	Super.Pushed();
	GotoState('Wander');
}

function Vector GetMoveDir()
{
	return ((Vector(Pawn.Rotation)/RandomCoef)+(RandomCoef*VRand()));
}

state Wander `DEBUGSTATE
{

Begin:
	if(!SuggestNewMovePoint(MovePt, GetMoveDir()))
	{
		`AILog("COULDN'T FIND NEW MOVE POINT!");
		Stop;
	}
	MoveTo(MovePt);
	Sleep(1.0);
	Goto('Begin');
};


DefaultProperties
{
	RandomCoef=1.05f
}