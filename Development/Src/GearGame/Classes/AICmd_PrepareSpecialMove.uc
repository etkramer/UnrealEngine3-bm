class AICmd_PrepareSpecialMove extends AICommand
	within GearAI;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();
	bIgnoreStepAside=true;
	ReactionManager.SuppressAllChannels();
	GotoState('Prepare');
}

function Popped()
{
	Super.Pushed();
	ReactionManager.UnSuppressAllChannels();
	bIgnoreStepAside=false;
}

function bool ShouldIgnoreTimeTransitions()
{
	return TRUE;
}


state Prepare 
{
Begin:
	`AILog(self@GetStatename()@"preparing special move for "@SpecialMoveAlignActor);
	InvalidateCover();
	StopFiring();
	if (SpecialMoveAlignActor != None)
	{
		SetFocalPoint( SpecialMoveAlignActor.Location + vector(SpecialMoveAlignActor.Rotation) * 1024.f, TRUE );
		if (VSize(SpecialMoveAlignActor.Location - Pawn.Location) > 512.f)
		{
			bPreciseDestination = TRUE;
			SetMoveGoal(SpecialMoveAlignActor,,,32.f);
			`AILog("Reached align destination");
		}
		else
		{
			Pawn.SetLocation(SpecialMoveAlignActor.Location);
			`AILog("Teleported to align destination "@SpecialMoveAlignActor@SpecialMoveAlignActor.Location);
		}
		Focus = None;
		SetFocalPoint( SpecialMoveAlignActor.Location + vector(SpecialMoveAlignActor.Rotation) * 1024.f, TRUE );
		// set desiredrotation since that's what finish rotation requires
		DesiredRotation = rotator(GetFocalPoint() - Pawn.Location);
		FinishRotation();
		`AILog("Finished rotating to align destination");
	}
	if (QueuedSpecialMove == SM_Engage_Start)
	{
		MyGearPawn.EngageTrigger = Trigger_Engage(SpecialMoveAlignActor);
		
		// special hacky handling for wheel turns!
		`AILog("Doing SM_Engage_Start"@MyGearPawn.EngageTrigger);
		MyGearPawn.DoSpecialMove(SM_Engage_Start,TRUE);
		GSM_EngageStart(MyGearPawn.SpecialMoves[SM_Engage_Start]).StartGrabAnim();
		do
		{
			Sleep(0.1f);
		}
		until (MyGearPawn.SpecialMove == SM_Engage_Idle||MyGearPawn.SpecialMove == SM_None);
		do
		{
			MyGearPawn.EngageTrigger.LastAttemptTurnTime = WorldInfo.TimeSeconds;
			// check to see if this is an unlinked trigger, or if the linked trigger is ready to be turned
			if (MyGearPawn.EngageTrigger.LinkedCoopTrigger == None || (MyGearPawn.EngageTrigger.LinkedCoopTrigger.EngagedPawn != None && Abs(MyGearPawn.EngageTrigger.LastAttemptTurnTime - MyGearPawn.EngageTrigger.LinkedCoopTrigger.LastAttemptTurnTime) < 1.f))
			{
				`log("turning wheel..");
				//`AILog("Doing SM_Engage_Loop");
				MyGearPawn.DoSpecialMove(SM_Engage_Loop,TRUE);
				do
				{
					Sleep(0.1f);
				}
				until (MyGearPawn.SpecialMove == SM_Engage_Idle||MyGearPawn.SpecialMove == SM_None);
			}
			else
			{
				Sleep(0.25f);
			}
		}
		until (MyGearPawn.SpecialMove == SM_None);

		MyGearPawn.DoSpecialMove(SM_Engage_End,TRUE);
	}
	else
	{
		MyGearPawn.DoSpecialMove(QueuedSpecialMove,TRUE);
		// wait for the sm to finish
		do
		{
			Sleep(0.1f);
		} until (MyGearPawn.SpecialMove == SM_None);
	}
	GotoState('DelaySuccess');
}

DefaultProperties
{
	bIgnoreNotifies=true
}
