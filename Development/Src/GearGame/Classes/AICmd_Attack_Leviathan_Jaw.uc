class AICmd_Attack_Leviathan_Jaw extends AICommand_Base_Combat
	within GearAI_Leviathan;

var GearAI_Leviathan			 LeviAI;
var GearPawn_LocustLeviathanBase LeviPawn;
	
function Pushed()
{
	LeviAI	 = GearAI_Leviathan(AIOwner);
	LeviPawn = GearPawn_LocustLeviathanBase(LeviAI.Pawn);
	ValidMouthTargets.Length = 0;

	Super.Pushed();

	GotoState( 'WaitForJaw' );
}

state WaitForJaw `DEBUGSTATE
{
	function BeginState( Name PreviousStateName )
	{
		Super.BeginState( PreviousStateName );

		LeviPawn.OpenJaw( CurrentJawSeq.Duration );
	}

	function CheckSuccess()
	{
		local int VictimIdx;
		local Actor A;
		local bool bSuccess;

		if( CurrentJawSeq != None && CurrentJawSeq.InnerTrigger != None )
		{
			foreach CurrentJawSeq.InnerTrigger.TouchingActors( class'Actor', A )
			{
				for( VictimIdx = 0; VictimIdx < ValidTargets.Length; VictimIdx++ )
				{
					if( A == ValidTargets[VictimIdx] && ValidMouthTargets.Find(A) < 0 )
					{
						bSuccess = TRUE;
						ValidMouthTargets[ValidMouthTargets.Length] = A;
						
						//debug
						`AILog( "Valid Mouth Target Found"@A );
					}
				}
			}
		}

		ClearLatentAction( class'SeqAct_Leviathan_Jaw', !bSuccess );
	}

Begin:
	do
	{
		Sleep( 0.05f );
	} until( LeviPawn.bJawAnimNotifyClosed );

	CheckSuccess();

	Status = 'Success';
	PopCommand( self );
}

function bool AllowTransitionTo( class<AICommand> AttemptCommand )
{
	return FALSE;
}


defaultproperties
{
}