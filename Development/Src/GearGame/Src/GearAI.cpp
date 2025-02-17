/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "GearGame.h"
#include "DemoRecording.h"

#include "GearGameSpecialMovesClasses.h"
#include "GearGameWeaponClasses.h"
#include "EngineMaterialClasses.h"
#include "OpCode.h"

IMPLEMENT_CLASS(AGearAI_Berserker)
IMPLEMENT_CLASS(AGearAI)
IMPLEMENT_CLASS(AGearAI_Cover)
IMPLEMENT_CLASS(AGearAI_COGGear)
IMPLEMENT_CLASS(AGearAI_TDM);
IMPLEMENT_CLASS(AGearAI_Jack)
IMPLEMENT_CLASS(AGearAI_Wretch)
IMPLEMENT_CLASS(AGearAI_Brumak)
IMPLEMENT_CLASS(AGearAI_Brumak_SideGun)
IMPLEMENT_CLASS(AGearAI_Brumak_Slave)
IMPLEMENT_CLASS(AGearAI_BrumakDriver)
IMPLEMENT_CLASS(AGearAI_Nemacyst)
IMPLEMENT_CLASS(AGearAI_SecurityNemacyst)
IMPLEMENT_CLASS(AGearAI_Ticker)
IMPLEMENT_CLASS(AGearAI_Bloodmount)
IMPLEMENT_CLASS(AGearAI_SecurityBotStationary)
IMPLEMENT_CLASS(AGearSquadFormation)
IMPLEMENT_CLASS(AGearSquad)

IMPLEMENT_CLASS(AGearSpawner)
IMPLEMENT_CLASS(AGearSpawner_EmergenceHoleBase)

IMPLEMENT_CLASS(UEmergenceHoleRenderingComponent)

IMPLEMENT_CLASS(UAICommand)

IMPLEMENT_CLASS(UAIReactCondition_Base)
IMPLEMENT_CLASS(UAIReactionManager)
IMPLEMENT_CLASS(UAIReactChannel)
IMPLEMENT_CLASS(UAIReactCond_EnemyMoved)
IMPLEMENT_CLASS(UAIReactCond_Conduit_Base)
IMPLEMENT_CLASS(UAIReactCond_NewEnemy)
IMPLEMENT_CLASS(UAIReactCond_SurpriseEnemyLoc)
IMPLEMENT_CLASS(UAIReactCond_Targeted)
IMPLEMENT_CLASS(UAIReactChan_Timer)

IMPLEMENT_CLASS(UAIVisibilityManager)

IMPLEMENT_CLASS(UAICmd_MoveToGoal)
IMPLEMENT_CLASS(UAICmd_Attack_PlantGrenade)

IMPLEMENT_CLASS(AAIAvoidanceCylinder)
IMPLEMENT_CLASS(UAIAvoidanceCylinderComponent)

IMPLEMENT_CLASS(AGearAI_RockWorm)
IMPLEMENT_CLASS(UMantleReachSpec_Rockworm)

IMPLEMENT_CLASS(AGameplayRoute)

IMPLEMENT_CLASS(ULadderMeshComponent)



#if DO_AI_LOGGING
VARARG_BODY(void,AGearAI::AILog,const TCHAR*,VARARG_NONE)
{
	// We need to use malloc here directly as GMalloc might not be safe.	
	if( !FName::SafeSuppressed(NAME_AILog) )
	{
		INT		BufferSize	= 1024;
		TCHAR*	Buffer		= NULL;
		INT		Result		= -1;

		while(Result == -1)
		{
			appSystemFree(Buffer);
			Buffer = (TCHAR*) appSystemMalloc( BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, Fmt, Fmt, Result );
			BufferSize *= 2;
		};
		Buffer[Result] = 0;

		eventAILog_Internal(Buffer);

		appSystemFree( Buffer );
	}
}
VARARG_BODY(void,AGearAI::AILog,const TCHAR*,VARARG_EXTRA(enum EName E))
{
	if( !FName::SafeSuppressed(NAME_AILog) )
	{
		INT		BufferSize	= 1024;
		TCHAR*	Buffer		= NULL;
		INT		Result		= -1;

		while(Result == -1)
		{
			appSystemFree(Buffer);
			Buffer = (TCHAR*) appSystemMalloc( BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT( Buffer, BufferSize, BufferSize-1, Fmt, Fmt, Result );
			BufferSize *= 2;
		};
		Buffer[Result] = 0;

		eventAILog_Internal(Buffer,FName(E));

		appSystemFree( Buffer );
	}
}
#endif

UBOOL AGearAI::HasValidTarget(AActor *TestTarget)
{
	// If no actor given use current fire target
	if( TestTarget == NULL )
	{
		TestTarget = FireTarget;
	}

	// If we don't have a target - invalid
	if( TestTarget == NULL )
	{
		return FALSE;
	}

	// If target is a dead pawn - invalid
	APawn *TestPawn = Cast<APawn>(TestTarget);
	if(  TestPawn != NULL &&
		!TestPawn->IsValidTargetFor( this ) )
	{
		return FALSE;
	}

	return TRUE;
}

void AGearAI::PreBeginPlay()
{
	MyGuid = appCreateGuid();

	Super::PreBeginPlay();
}

/** Overridden to clear the AI latent action on any state transition. */
EGotoState UAICommand::GotoState( FName State, UBOOL bForceEvents, UBOOL bKeepStack )
{
	AGearAI *AI = GetOuterAGearAI();
	if (AI != NULL && AI->GetStateFrame() != NULL)
	{
		AI->GetStateFrame()->LatentAction = 0;
	}
	return Super::GotoState(State,bForceEvents,bKeepStack);
}

/**
 * Overridden to provide normal state ticking for AICommand
 * 
 * @note - AICommand abuses the stateframe of the owning AI in order to provide "within" latent function support
 *         such that MoveToward() can be called from within a state for AICommand, but still operate correctly for
 *         the AIController.
 */
void UAICommand::ProcessState(FLOAT DeltaSeconds)
{
	AGearAI *AI = GetOuterAGearAI();
	if( GetStateFrame() &&	
		GetStateFrame()->Code &&	
		!IsPendingKill() &&	
		AI != NULL &&	
		AI->Pawn != NULL &&
		!AI->ActorIsPendingKill()&&	
		AI->GetStateFrame() && 
		!bAborted )
	{
		//debug
		//debugf(TEXT("%s ProcessState %s %d"), *GetName(), *AI->GetName(), AI->GetStateFrame()->LatentAction );
		// If a latent action is in progress, update it.
		if (AI->GetStateFrame()->LatentAction != 0)
		{
			(AI->*GNatives[AI->GetStateFrame()->LatentAction])(*GetStateFrame(), (BYTE*)&DeltaSeconds);
		}

		if (AI->GetStateFrame()->LatentAction == 0)
		{
			//debug
//			debugf(TEXT("%s PROCESSSTATE - EXECUTE STATE CODE"), *GetName() );


			// Execute code.
			INT NumStates = 0;
			BYTE Buffer[MAX_SIMPLE_RETURN_VALUE_SIZE];
			// create a copy of the state frame to execute state code from so that if the state is changed from within the code, the executing frame's code pointer isn't modified while it's being used
			FStateFrame ExecStateFrame(*GetStateFrame());
			while(  !IsPendingKill() && 
					 ExecStateFrame.Code != NULL && 
					 AI != NULL && 
					 AI->Pawn != NULL &&
					!AI->ActorIsPendingKill() && 
					 AI->GetStateFrame()->LatentAction == 0 &&
					 AI->GetActiveCommand() == this && // If we pushed a new command, we need to be proccessing the new one instead of this one
					 !bAborted  // Stop processing state once this command is aborted
				) 
			{
				// if we are continuing interrupted state code, we need to manually push the frame onto the script debugger's stack
				if (GetStateFrame()->bContinuedState)
				{
#if !FINAL_RELEASE
					if (GDebugger != NULL)
					{
						GDebugger->DebugInfo(this, &ExecStateFrame, DI_NewStack, 0, 0);
					}
#endif
					GetStateFrame()->bContinuedState = FALSE;
				}

				// remember old starting point (+1 for the about-to-be-executed byte so we can detect a state/label jump back to the same byte we're at now)
				BYTE* OldCode = ++GetStateFrame()->Code;

				ExecStateFrame.Step( this, Buffer ); 

				//debug
//				debugf(TEXT("%s has command changed? %s %s"), *GetName(), AI->GetActiveCommand()?*AI->GetActiveCommand()->GetName():TEXT("NULL"), (this!=AI->GetActiveCommand())?TEXT("!!!!!!!!!!!!!!!!!"):TEXT(""));

				// if a state was pushed onto the stack, we need to correct the originally executing state's code pointer to reflect the code *after* the last state command was executed
				if (GetStateFrame()->StateStack.Num() > ExecStateFrame.StateStack.Num())
				{
					GetStateFrame()->StateStack(ExecStateFrame.StateStack.Num()).Code = ExecStateFrame.Code;
				}
				// if the state frame's code pointer was directly modified by a state or label change, we need to update our copy
				if (GetStateFrame()->Node != ExecStateFrame.Node)
				{
					// we have changed states
					if( ++NumStates > 4 )
					{
						//debugf(TEXT("%s pause going from state %s to %s"), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						// shouldn't do any copying as the StateFrame was modified for the new state/label
						break;
					}
					else
					{
						//debugf(TEXT("%s went from state %s to %s"), *GetName(), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						ExecStateFrame = *GetStateFrame();
					}
				}
				else if (GetStateFrame()->Code != OldCode)
				{
					// transitioned to a new label
					//debugf(TEXT("%s went to new label in state %s"), *GetName(), *GetStateFrame()->StateNode->GetName());
					ExecStateFrame = *GetStateFrame();
				}
				else
				{
					// otherwise, copy the new code pointer back to the original state frame
					GetStateFrame()->Code = ExecStateFrame.Code;
				}
			}

#if !FINAL_RELEASE
			// notify the debugger if state code ended prematurely due to this Actor being destroyed
			if ((AI->ActorIsPendingKill() || IsPendingKill()) && GDebugger != NULL)
			{
				GDebugger->DebugInfo(this, &ExecStateFrame, DI_PrevStackState, 0, 0);
			}
#endif
		}
	}
}

void UAICommand::TickCommand(FLOAT DeltaTime)
{
	//debug
//	debugf(TEXT("%s TickCommand %s"), *GetName(), ChildCommand?*ChildCommand->GetName():TEXT("NULL"));

	// pass tick through to child
	if( ChildCommand != NULL )
	{
		ChildCommand->TickCommand(DeltaTime);
	}
	if( ChildCommand == NULL )
	{
		eventInternalTick( DeltaTime );
		// by default only tick if there is no active child
		ProcessState( DeltaTime );
	}
}

void UAICommand::PopChildCommand()
{
	//debug
#if DO_AI_LOGGING
	AIOwner->AILog( NAME_AICommand, TEXT("%s PopChildCommand: %s"), *GetName(), ChildCommand?*ChildCommand->GetName():TEXT("NULL") );
#endif

	if ( ChildCommand != NULL )
	{
		FName ChildName = ChildCommand->GetClass()->GetFName();

		// recursively pop any children
		ChildCommand->bPendingPop = TRUE;

		// if the command we're about to pop has children, throw a warning because it probably should be aborted
#if DO_AI_LOGGING
		if(ChildCommand->ChildCommand != NULL && !ChildCommand->bAborted)
		{
			AIOwner->AILog( NAME_AICommand, TEXT("WARNING!! I'm about to pop %s but it has children!  You should use abort instead of pop for this case!!"),*ChildCommand->ChildCommand->GetName());
			debugf(NAME_Warning,TEXT("WARNING!! [%2.3f] (%s) %s is about to pop command %s but it has children!  You should use abort instead of pop for this case!!"),GWorld->GetTimeSeconds(),*AIOwner->GetName(),*GetName(),*ChildCommand->ChildCommand->GetName());
		}
#endif
		ChildCommand->PopChildCommand();

		// need to check ChildCommand again because it could have popped itself on resume
		if (ChildCommand != NULL)
		{
			// notify the child is being popped
			ChildCommand->eventInternalPopped();
			// and again
			if (ChildCommand != NULL)
			{
				// clear out refs and record the exit status
				ChildCommand->AIOwner	 = NULL;
				ChildCommand->CoverOwner = NULL;
				ChildStatus  = ChildCommand->Status;
				if (GDebugger != NULL && ChildCommand->GetStateFrame() != NULL)
				{
					GDebugger->DebugInfo(this, ChildCommand->GetStateFrame(), DI_PrevStackState, 0, 0);
				}
				GetStateFrame()->bContinuedState = TRUE;
				ChildCommand->SetFlags(RF_PendingKill);
				ChildCommand = NULL;

				if( Status != NAME_Aborted  && !bPendingPop)
				{
#if DO_AI_LOGGING
					//debugf(TEXT("RESUMED on %s ChildStatus: %s %2.3f"), *GetName(), *ChildStatus.ToString(),GWorld->GetTimeSeconds());
					AIOwner->AILog( NAME_AICommand, TEXT("RESUMED on %s ChildStatus: %s"), *GetName(), *ChildStatus.ToString());
#endif
					// send notification to this command
					eventInternalResumed( ChildName );
				}
			}
		}
	}
}

UBOOL UAICommand::ShouldIgnoreNotifies() const
{
	if(ChildCommand != NULL)
	{
		return ChildCommand->ShouldIgnoreNotifies();
	}

	return bIgnoreNotifies;
}

/** 
 * Overridden to prevent state transitions when operating AICommands, since any state transition for the base controller
 * would break the "within" state functionality for the current command.
 */
EGotoState AGearAI::GotoState( FName State, UBOOL bForceEvents, UBOOL bKeepStack )
{
	if (CommandList != NULL)
	{
		return GOTOSTATE_Preempted;
	}
	else
	{
		return Super::GotoState(State,bForceEvents,bKeepStack);
	}
}

/** 
 *	Copy of AActor::ProcessState that stops state processing when a AICommand is pushed onto the stack 
 *	Didn't want to add a virtual function right now because UT guys complained that they are slow on the PS3
 */
void AGearAI::ProcessState( FLOAT DeltaSeconds )
{
	//debug
//	debugf(TEXT("%s ProcessState %s"), *GetName(), CommandList?*CommandList->GetName():TEXT("NULL") );

	if(	GetStateFrame() && 
		GetStateFrame()->Code && 
		(Role>=ROLE_Authority || 
			(GetStateFrame()->StateNode->StateFlags & STATE_Simulated)) &&
		!IsPendingKill() &&
		CommandList == NULL )
	{
		if (GetStateFrame()->LatentAction != 0)
		{
			(this->*GNatives[GetStateFrame()->LatentAction])(*GetStateFrame(), (BYTE*)&DeltaSeconds);
		}

		if (GetStateFrame()->LatentAction == 0)
		{
			// Execute code.
			INT NumStates = 0;
			BYTE Buffer[MAX_SIMPLE_RETURN_VALUE_SIZE];
			// create a copy of the state frame to execute state code from so that if the state is changed from within the code, the executing frame's code pointer isn't modified while it's being used
			FStateFrame ExecStateFrame(*GetStateFrame());
			while( !bDeleteMe && 
					ExecStateFrame.Code != NULL && 
					GetStateFrame()->LatentAction == 0 &&
					CommandList == NULL )	// Stop executing state code once an AICommand is pushed
			{
				// if we are continuing interrupted state code, we need to manually push the frame onto the script debugger's stack
				if (GetStateFrame()->bContinuedState)
				{
#if !FINAL_RELEASE
					if (GDebugger != NULL)
					{
						GDebugger->DebugInfo(this, &ExecStateFrame, DI_NewStack, 0, 0);
					}
#endif
					GetStateFrame()->bContinuedState = FALSE;
				}

				// remember old starting point (+1 for the about-to-be-executed byte so we can detect a state/label jump back to the same byte we're at now)
				BYTE* OldCode = ++GetStateFrame()->Code;

				ExecStateFrame.Step( this, Buffer ); 
				// if a state was pushed onto the stack, we need to correct the originally executing state's code pointer to reflect the code *after* the last state command was executed
				if (GetStateFrame()->StateStack.Num() > ExecStateFrame.StateStack.Num())
				{
					GetStateFrame()->StateStack(ExecStateFrame.StateStack.Num()).Code = ExecStateFrame.Code;
				}
				// if the state frame's code pointer was directly modified by a state or label change, we need to update our copy
				if (GetStateFrame()->Node != ExecStateFrame.Node)
				{
					// we have changed states
					if( ++NumStates > 4 )
					{
						//debugf(TEXT("%s pause going from state %s to %s"), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						// shouldn't do any copying as the StateFrame was modified for the new state/label
						break;
					}
					else
					{
						//debugf(TEXT("%s went from state %s to %s"), *GetName(), *ExecStateFrame.StateNode->GetName(), *GetStateFrame()->StateNode->GetName());
						ExecStateFrame = *GetStateFrame();
					}
				}
				else if (GetStateFrame()->Code != OldCode)
				{
					// transitioned to a new label
					//debugf(TEXT("%s went to new label in state %s"), *GetName(), *GetStateFrame()->StateNode->GetName());
					ExecStateFrame = *GetStateFrame();
				}
				else
				{
					// otherwise, copy the new code pointer back to the original state frame
					GetStateFrame()->Code = ExecStateFrame.Code;
				}
			}

#if !FINAL_RELEASE
			// notify the debugger if state code ended prematurely due to this Actor being destroyed
			if (bDeleteMe && GDebugger != NULL)
			{
				GDebugger->DebugInfo(this, &ExecStateFrame, DI_PrevStackState, 0, 0);
			}
#endif
		}
	}
}

void AGearAI::PushCommand( UAICommand *NewCommand )
{
	if( NewCommand != NULL )
	{
		//debug
		AILog( NAME_AICommand, TEXT("PushCommand: %s"), *NewCommand->GetName() );

		// if the active command has the same class as the one we're pushing, give it special care 		
		UAICommand* ActiveCommand = GetActiveCommand();
		if( ActiveCommand != NULL && ActiveCommand->GetClass() == NewCommand->GetClass() )
		{
			// check bReplaceActiveSameClassInstance first, it trumps bAllowNewSameClassInstance
			if(NewCommand->bReplaceActiveSameClassInstance)
			{
				AbortCommand(ActiveCommand);
				AILog( NAME_AICommand, TEXT("PushCommand ABORTING %s because Incoming command %s has same class, bReplaceActiveSameClassInstance:%i bAllowNewSameClassInstance:%i"), *ActiveCommand->GetName(), *NewCommand->GetName(), NewCommand->bReplaceActiveSameClassInstance, NewCommand->bAllowNewSameClassInstance);
			}
			else if(!NewCommand->bAllowNewSameClassInstance)
			{
				AILog( NAME_AICommand, TEXT("PushCommand IGNORED for : %s because ActiveCommand %s has same class, bReplaceActiveSameClassInstance:%i bAllowNewSameClassInstance:%i"), *NewCommand->GetName(), *ActiveCommand->GetName(), NewCommand->bReplaceActiveSameClassInstance, NewCommand->bAllowNewSameClassInstance);
				return;
			}

		}

		NewCommand->AIOwner		= this;
		NewCommand->CoverOwner	= Cast<AGearAI_Cover>(this);
		if( CommandList == NULL )
		{
#if !FINAL_RELEASE
			if (GDebugger != NULL && GetStateFrame() != NULL)
			{
				// manually pop the debugger stack node for this state...we'll restore it later
				GDebugger->DebugInfo(this, GetStateFrame(), DI_PrevStackState, 0, 0);
			}
#endif
			CommandList = NewCommand;
		}
		else
		{
			UAICommand *LastCommand = GetActiveCommand();		
#if !FINAL_RELEASE
			if (GDebugger != NULL && LastCommand->GetStateFrame() != NULL)
			{
				// manually pop the debugger stack node for this state...we'll restore it later
				GDebugger->DebugInfo(LastCommand, LastCommand->GetStateFrame(), DI_PrevStackState, 0, 0);
			}
#endif
			LastCommand->ChildCommand = NewCommand;
			// notify the current command it is being paused
			LastCommand->eventInternalPaused( NewCommand );
		}

		//debug
		//DumpCommandStack();

		// clear any latent action currently being used by the last command on the stack
		GetStateFrame()->LatentAction = 0;
		// initial the command
		NewCommand->InitExecution();
		// and notify the command
		NewCommand->eventInternalPushed();
	}
}

void AGearAI::PopCommand( UAICommand *NewCommand )
{
	//debugf(TEXT("%s pop command; Child: %s"),*NewCommand->GetName(),(NewCommand->ChildCommand) ? *NewCommand->ChildCommand->GetName() : TEXT("NONE"));
	if( NewCommand != NULL )
	{
		//debug
#if DO_AI_LOGGING
		AILog( NAME_AICommand, TEXT("PopCommand: %s -- Status %s"), *NewCommand->GetName(),*NewCommand->Status.ToString() );
#endif

		// if it is the head of the command list
		if( NewCommand == CommandList )
		{
			// Force it to pop children
			NewCommand->PopChildCommand();
			// Notify the child is being popped
			NewCommand->eventInternalPopped();
			if (GDebugger != NULL && NewCommand->GetStateFrame() != NULL)
			{
				GDebugger->DebugInfo(this, NewCommand->GetStateFrame(), DI_PrevStackState, 0, 0);
			}
			GetStateFrame()->bContinuedState = TRUE;
			NewCommand->SetFlags(RF_PendingKill);
			// Then just clear the ref
			CommandList = NULL;
		}
		else
		{
			// otherwise find the parent command
			UAICommand *ParentCommand = CommandList;
			while (ParentCommand != NULL && ParentCommand->ChildCommand != NewCommand)
			{
				ParentCommand = ParentCommand->ChildCommand;
			}
			if (ParentCommand != NULL)
			{
				// and tell it to begin popping children
				ParentCommand->PopChildCommand();
			}
		}

		//debug
		//DumpCommandStack();

		// ensure we don't leave a latent action lying around when this command is popped
		GetStateFrame()->LatentAction = 0;
	}
}

UBOOL AGearAI::AbortCommand( UAICommand* AbortCmd, UClass* AbortClass )
{
	UBOOL bResult = FALSE;

	//debug
//	AILog(TEXT("AbortCommand %s"), *AbortClass->GetName() );
//	DumpCommandStack();

	UAICommand* Cmd = CommandList;
	while( Cmd != NULL )
	{
		// If command isn't already aborted and it is a class we want to abort
		if( !Cmd->bAborted && 
			((AbortCmd != NULL && Cmd == AbortCmd) || 
			 (AbortClass != NULL && Cmd->GetClass()->IsChildOf( AbortClass ))) )
		{
			//debug
#if DO_AI_LOGGING
			AILog(TEXT("ABORTING... %s"), *Cmd->eventGetDumpString() );
#endif

			// Set aborted status/flag
			Cmd->Status		= NAME_Aborted;
			Cmd->bAborted	= TRUE;

			// Abort all the children too
			UAICommand* ChildCmd = Cmd->ChildCommand;
			while( ChildCmd != NULL )
			{
				ChildCmd->Status	= NAME_Aborted;
				ChildCmd->bAborted	= TRUE;

				ChildCmd = ChildCmd->ChildCommand;
			}
			
			bResult = TRUE;
		}

		Cmd = Cmd->ChildCommand;
	}

	if( bResult )
	{
		// Handle any aborted commands
		Cmd = CommandList;
		while( Cmd != NULL )
		{
			// If this command was aborted, pop the command
			if( Cmd->bAborted )
			{
				PopCommand( Cmd );
				Cmd = CommandList;	// Start looking from the beginning of the list again
			}
			else
			{
				Cmd = Cmd->ChildCommand;
			}
		}
	}

	return bResult;
}

UAICommand* AGearAI::GetActiveCommand()
{
	if( CommandList == NULL )
	{
		return NULL;
	}

	UAICommand* Cmd = CommandList;
	while( Cmd->ChildCommand )
	{
		Cmd = Cmd->ChildCommand;
	}

	return Cmd;
}

void AGearAI::CheckCommandCount()
{
	INT Cnt = 0;
	UAICommand* Cmd = CommandList;
	while( Cmd )
	{
		Cnt++;
		if (Cnt >= 50)
		{
			warnf(TEXT("Runaway Loop in AICommand list detected (more than 50 commands)... %s"), *GetName());
			DumpCommandStack();
			if( AILogFile != NULL && AILogFile->ArchivePtr != NULL )
			{
				AILogFile->ArchivePtr->Flush();
			}

			bHasRunawayCommandList = TRUE;
// this appErrorf insures that any AI errors are immediately caught and reported.  We don't do this in LTCG nor if we are not ai logging as all of the pertinent info is outputted to the AILogs
#if !FINAL_RELEASE && DO_AI_LOGGING
			appErrorf(TEXT("Runaway Loop in AICommand list detected (more than 50 commands)... %s"), *GetName());
#endif
			break;
		}
		Cmd = Cmd->ChildCommand;
	}
}

void AGearAI::DumpPathConstraints()
{
	AILog(TEXT("DUMP PATH CONSTRAINTS: %s"), *Pawn->GetName() );

	AILog(TEXT("Path constraints..."));
	if( Pawn->PathConstraintList == NULL )
	{
		AILog(TEXT("\t\t [PATH EMPTY]"));
	}
	else
	{
		UPathConstraint* PCon = Pawn->PathConstraintList;
		INT Count = 0;
		while( PCon )
		{
			if( ++Count > 50 )
			{
				AILog(TEXT("\t\t [Truncated due to depth]"));
				break;
			}
			AILog(TEXT("\t\t %s"), *PCon->eventGetDumpString());
			PCon = PCon->NextConstraint;
		}
	}

	AILog(TEXT("Goal evaluators..."));
	if( Pawn->PathGoalList == NULL )
	{
		AILog(TEXT("\t\t [GOAL EMPTY]"));
	}
	else
	{
		UPathGoalEvaluator* GEval = Pawn->PathGoalList;
		INT Count = 0;
		while( GEval )
		{
			if( ++Count > 50 )
			{
				AILog(TEXT("\t\t [Truncated due to depth]"));
				break;
			}
			AILog( TEXT("\t\t %s"), *GEval->eventGetDumpString());
			GEval = GEval->NextEvaluator;
		}
	}
}

void AGearAI::DumpCommandStack()
{
	AILog(TEXT("DUMP COMMAND STACK: %s"), *Pawn->GetName());
	if( CommandList == NULL )
	{
		AILog(TEXT("\t\t [Empty]"));
	}
	else
	{
		UAICommand* Cmd = CommandList;
		INT Count = 0;
		while (Cmd)
		{
			if (++Count > 50)
			{
				AILog(TEXT("\t\t [Truncated due to stack depth]"));
				break;
			}
			AILog(TEXT("\t\t %s"), *Cmd->eventGetDumpString());
			Cmd = Cmd->ChildCommand;
		}
	}
}

void AGearAI::execFindCommandOfClass(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, SearchClass);
	P_FINISH;

	if (SearchClass != NULL)
	{
		UAICommand* CurrCommand = CommandList;
		while (CurrCommand != NULL && !CurrCommand->IsA(SearchClass))
		{
			CurrCommand = CurrCommand->ChildCommand;
		}
		*(UAICommand**)Result = CurrCommand;
	}
	else
	{
		*(UAICommand**)Result = NULL;
	}
}


/**
 * Calculates the total distance in the current route cache.
 */
FLOAT AGearAI::GetRouteCacheDistance()
{
	FLOAT Distance = 0.f;
	AActor* LastAnchor = Pawn;

	//@fixme - i'm pretty sure this is horribly inaccurate, but this is just to get a rough idea for now
	for( INT Idx = 0; Idx < RouteCache.Num(); Idx++)
	{
		if( RouteCache(Idx) != NULL )
		{
			Distance += (LastAnchor->Location - RouteCache(Idx)->Location).Size();
			LastAnchor = RouteCache(Idx);
		}
	}
	return Distance;
}


UBOOL AGearAI::IsCoverWithinCombatZone( FCoverInfo TestCover )
{
	if( TestCover.Link == NULL )
		return FALSE;

	if( CombatZoneList.Num() > 0 )
	{
		ACoverSlotMarker*	Marker		= TestCover.Link->Slots(TestCover.SlotIdx).SlotMarker;
		ACombatZone*		MarkerZone	= MyGearPawn->GetCombatZoneForNav( Marker );
		for( INT ZoneIdx = 0; ZoneIdx < CombatZoneList.Num(); ZoneIdx++ )
		{
			ACombatZone* CZ = CombatZoneList(ZoneIdx);
			if( CZ != NULL && CZ == MarkerZone )
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	return TRUE;
}

UBOOL AGearAI::IsWithinCombatZone( FVector TestLocation )
{
	if( CombatZoneList.Num() > 0 )
	{
		for( INT ZoneIdx = 0; ZoneIdx < CombatZoneList.Num(); ZoneIdx++ )
		{
			ACombatZone* CZ = CombatZoneList(ZoneIdx);			
			if( CZ != NULL && CZ->Encompasses( TestLocation ) )
			{
				return TRUE;
			}
		}
		return FALSE;
	}

	return TRUE;
}

void AGearAI::AdjustHearingLocation(FVector &out_Location)
{
	out_Location.X += -32.f + (64.f * appFrand());
	out_Location.Y += -32.f + (64.f * appFrand());
}

void AGearAI_Berserker::AdjustHearingLocation(FVector &out_Location)
{
	FLOAT Dist = (out_Location - Pawn->Location).Size();
	FLOAT Max = 128.f * (Dist/(Pawn->HearingThreshold*0.25f));

	FVector Loc = out_Location;
	Loc.X += -Max + ((Max - -Max) * appFrand());
	Loc.Y += -Max + ((Max - -Max) * appFrand());

	// Check for a wall between actual location and adjusted location
	FCheckResult Hit;
	//@fixme - gearsetup
	//GWorld->SingleLineCheck(Hit,this,Loc,out_Location,TRACE_World|TRACE_IgnoreMovers);
	GWorld->SingleLineCheck(Hit,this,Loc,out_Location,TRACE_World);
	if (Hit.Actor != NULL)
	{
		// If there is a wall, make adjusted location the impact on the wall
		Loc.X = Hit.Location.X;
		Loc.Y = Hit.Location.Y;
	}
	else
	{
		// Otherwise, if there is no wall...
		// Check to make sure there is a floor below the adjusted location
		GWorld->SingleLineCheck(Hit,this,Loc - FVector(0,0,100), Loc, TRACE_World);
		// If there is no floor
		if( Hit.Actor == NULL )
		{
			// Don't adjust the location
			return;
		}
	}

	out_Location = Loc;
}

UBOOL AGearAI::IsMeleeRange(FVector TestLocation)
{
	return Pawn != NULL && (TestLocation - Pawn->Location).SizeSquared() <= (EnemyDistance_Melee * EnemyDistance_Melee);
}

UBOOL AGearAI::Tick(FLOAT DeltaTime, ELevelTick TickType)
{
	if (!WorldInfo->bPlayersOnly)
	{
		//debug
//		debugf(TEXT("%s Tick -- %s"), *GetName(), CommandList?*CommandList->GetName():TEXT("NULL"));

		// tick the active AICommand if available
		if( CommandList != NULL )
		{
			CheckCommandCount();

			CommandList->TickCommand(DeltaTime);

			// so we have a run away commandlist.  So we will Abort it and then try try again
			if( bHasRunawayCommandList == TRUE )
			{
				AbortCommand( CommandList );  // abort his command he'll pop his entire command stack abort everythign he's doing and go back to default behavior might cause some weirdness but usually will be ok
				bHasRunawayCommandList = FALSE;
			}
		}

		if( FearActor != NULL )
		{
			FearLocation = FearActor->Location;
		}

		ReactionManager->Tick(DeltaTime);
	}

	if (GWorld->DemoRecDriver != NULL && GWorld->DemoRecDriver->ServerConnection == NULL)
	{
		// record debug action string into demo
		DemoActionString = eventGetActionString();
	}

	// Register enter/exit of combat zone
	// Only registers changes in occupancy
	if( CurrentCombatZone != PendingCombatZone )
	{
		if( CurrentCombatZone != NULL )
		{
			CurrentCombatZone->eventRemoveOccupant( Pawn );
		}
		if( PendingCombatZone != NULL )
		{
			PendingCombatZone->eventAddOccupant( Pawn );
		}

		bInValidCombatZone = (CombatZoneList.Num()==0);
		for( INT ZoneIdx = 0; ZoneIdx < CombatZoneList.Num(); ZoneIdx++ )
		{
			if( PendingCombatZone == CombatZoneList(ZoneIdx) )
			{
				bInValidCombatZone = TRUE;
				break;
			}
		}

		// Update current zone
		eventSetCurrentCombatZone( PendingCombatZone );
	}		

	return Super::Tick(DeltaTime,TickType);
}

FVector AGearAI::GetFireTargetLocation(BYTE LT)
{
	// If fire target is our enemy and we aren't being forced to fire on it
	if( FireTarget == Enemy && TargetList.Num() == 0 )
	{
		// Get assumed location
		return GetEnemyLocation( NULL, LT );
	}
	else
	if (FireTarget != NULL)
	{
		// Otherwise, get exact location
		return FireTarget->Location;
	}
	return FVector(0.f);
}

FVector AGearAI::GetEnemyLocation(APawn *TestPawn, BYTE LT)
{
	// default to enemy
	if( TestPawn == NULL )
	{
		TestPawn = Enemy;
	}
	// Early out if invalid
	if( TestPawn == NULL )
	{
		return FVector(0.f);
	}

	// Get location info from squad
	if( Squad )
	{
		return Squad->GetEnemyLocation( TestPawn, LT );
	}

	// default to actual pawn location
	return TestPawn->Location;
}

/** Hack to get around const Anchor var */
void AGearAI::AssignAnchor( APawn* P, ANavigationPoint* NewAnchor )
{
	if( P != NULL )
	{
		P->Anchor = NewAnchor;
	}
}

void AGearAI::ClearCrossLevelPaths(ULevel *Level)
{
	if (Pawn != NULL)
	{
		if (Pawn->Anchor != NULL && Pawn->Anchor->IsInLevel(Level))
		{
			eventNotifyAnchorBeingStreamedOut();
		}
	}

	Super::ClearCrossLevelPaths(Level);

	if (MoveGoal != NULL && MoveGoal->IsInLevel(Level))
	{
#if DO_AI_LOGGING
		AILog(TEXT("Clearing move goal in a level that is streaming out: %s"),*MoveGoal->GetPathName());
#endif
		MoveGoal = NULL;
	}
	for( INT Idx = 0; Idx < CombatZoneList.Num(); Idx++ )
	{
		ACombatZone* CZ = CombatZoneList(Idx);
		if( CZ == NULL || CZ->IsInLevel(Level) )
		{
			CombatZoneList.Remove(Idx--,1);
		}
	}
}

UBOOL AGearAI_Cover::IsLookingAtWall()
{
	FCheckResult Hit(1.f);
	if(Pawn != NULL)
	{
		FVector Start = Pawn->eventGetPawnViewLocation();
		FVector Dir = FVector(0.f);
		GetLookDir(Dir);
		FVector End = Start + Dir * 768.0f;
		return !GWorld->SingleLineCheck(Hit,Pawn,End,Start,TRACE_World,FVector(0.f));
	}

	return FALSE;
}

void AGearAI_Cover::ForceLog(const FString& s)
{
	debugf(TEXT("%s-> %s"),*GetName(),*s);
}

void AGearAI_Cover::ClearCrossLevelPaths(ULevel *Level)
{
	Super::ClearCrossLevelPaths(Level);
	if (CoverGoal.Link != NULL && CoverGoal.Link->IsInLevel(Level))
	{
#if DO_AI_LOGGING
		AILog(TEXT("Clearing covergoal in a level that is streaming out: %s"),*CoverGoal.Link->GetPathName());
#endif
		CoverGoal.Link = NULL;
		CoverGoal.SlotIdx = -1;
	}
	if (Cover.Link != NULL && Cover.Link->IsInLevel(Level))
	{
#if DO_AI_LOGGING
		AILog(TEXT("Clearing covergoal in a level that is streaming out: %s"),*Cover.Link->GetPathName());
#endif
		Cover.Link = NULL;
		Cover.SlotIdx = -1;
	}
}


UBOOL AGearAI::GetPlayerCover(AGearPawn* ChkPlayer, FCoverInfo& out_Cover, UBOOL bCanGuess)
{
	out_Cover.Link = NULL;
	out_Cover.SlotIdx = -1;
	UBOOL bResult = FALSE;
	if( Squad != NULL && ChkPlayer != NULL && ChkPlayer->DrivenVehicle == NULL)
	{
		// search the cached version in the enemy list first
		// use it only if we've seen the enemy recently or the enemy is still there
		// (assume we could tell if they moved, but not where they went, as usually you can see the edge of a player in cover)
		INT EnemyIndex = Squad->GetEnemyIndex(ChkPlayer);
		if ( EnemyIndex != INDEX_NONE &&
			(WorldInfo->TimeSeconds - Squad->EnemyList(EnemyIndex).LastUpdateTime < 10.0f || Squad->EnemyList(EnemyIndex).Cover.Link == ChkPlayer->CurrentLink) )
		{
			out_Cover = Squad->EnemyList(EnemyIndex).Cover;
			bResult	= (out_Cover.Link != NULL);
		}

		if (!bResult && (EnemyIndex == INDEX_NONE || WorldInfo->TimeSeconds - Squad->EnemyList(EnemyIndex).LastUpdateTime < 2.0f))
		{
			// if they're AI then look at current cover, or their covergoal
			AGearAI_Cover *AI = Cast<AGearAI_Cover>(ChkPlayer->Controller);
			if (AI != NULL)
			{
				if (AI->HasValidCover())
				{
					out_Cover = AI->Cover;
					bResult = TRUE;
				}
				else if (AI->IsValidCover(AI->CoverGoal))
				{
					out_Cover = AI->CoverGoal;
					bResult = TRUE;
				}
			}
		}
		if (!bResult && bCanGuess)
		{
			//@FIXME: shouldn't this use the location we think the enemy is at instead of their real location?
			ChkPlayer->GuessAtCover(out_Cover);
			bResult = out_Cover.Link != NULL;
		}
	}
	return bResult;
}

/**
 * Returns TRUE if any known enemies are within the specified distance.
 */
UBOOL AGearAI::HasEnemyWithinDistance(FLOAT Distance, class APawn** out_EnemyPawn/*=NULL*/, UBOOL bExact )
{
	if( Pawn != NULL && Squad != NULL )
	{
		const FLOAT DistSq = Distance * Distance;

		for( INT Idx = 0; Idx < Squad->EnemyList.Num(); Idx++)
		{
			// if enemy is an invalid target
			APawn *EnemyPawn = Squad->EnemyList(Idx).Pawn;
			if( EnemyPawn == NULL ||
				!EnemyPawn->IsValidEnemyTargetFor( PlayerReplicationInfo, TRUE ) )
			{
				// skip
				continue;
			}

			// figure out the distance
			FVector EnemyLocation = bExact ? EnemyPawn->Location : GetEnemyLocation( EnemyPawn );
			const FLOAT EnemyDistSq = (EnemyLocation - Pawn->Location).SizeSquared();
			if( EnemyDistSq <= DistSq )
			{
				// is within range, return
				if ( out_EnemyPawn )
				{
					*out_EnemyPawn = EnemyPawn;
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

void AGearAI::RouteCache_AddItem( ANavigationPoint* Nav )
{
	if( bSkipRouteCacheUpdates )
		return;

	if( Nav )
	{
		// Give extra cost so AI won't all take same route
		Nav->ExtraCost += appTrunc(ExtraPathCost);

		RouteCache.AddItem( Nav );
	}
}
void AGearAI::RouteCache_InsertItem( ANavigationPoint* Nav, INT Idx )
{
	if( bSkipRouteCacheUpdates )
		return;

	if( Nav )
	{
		// Give extra cost so AI won't all take same route
		Nav->ExtraCost += appTrunc(ExtraPathCost);

		RouteCache.InsertItem( Nav, Idx );
	}
}
void AGearAI::RouteCache_RemoveItem( ANavigationPoint* Nav )
{
	if( bSkipRouteCacheUpdates )
		return;

	if( Nav )
	{
		Nav->ExtraCost -= appTrunc(ExtraPathCost);
		RouteCache.RemoveItem( Nav );
	}
}
void AGearAI::RouteCache_RemoveIndex( INT Index, INT Count )
{
	if( bSkipRouteCacheUpdates )
		return;

	//AILog(TEXT("RouteCache_RemoveIndex %i %i"),Index,Count);
	if( Index >= 0 && Index < RouteCache.Num() )
	{
		for( INT Idx = Index; Idx < Index+Count && Idx < RouteCache.Num(); Idx++ )
		{
			ANavigationPoint* Nav = RouteCache(Idx);
			if( Nav )
			{
				//AILog(TEXT("...removing %s(%i)"),*Nav->GetName(),Idx);
				Nav->ExtraCost -= appTrunc(ExtraPathCost);
			}			
		}

		RouteCache.Remove( Index, Count );
	}
}

void AGearAI::RouteCache_Empty()
{
	if( bSkipRouteCacheUpdates )
		return;

	for( INT Idx = 0; Idx < RouteCache.Num(); Idx++ )
	{
		ANavigationPoint* Nav = RouteCache(Idx);
		if( Nav )
		{
			Nav->ExtraCost -= appTrunc(ExtraPathCost);
		}
	}

	RouteCache.Empty();
}

void AGearAI_Cover::execPollMoveToward(FFrame &Stack,RESULT_DECL)
{
	// check to see if we can run2cover
	ACoverSlotMarker *Marker = Cast<ACoverSlotMarker>(MoveTarget);

	UBOOL bCanRun2Cover = !bPreparingMove && Marker != NULL && eventCanRun2Cover();
	if( bCanRun2Cover )
	{
		// set the flag for the ai to trigger the move
		bShouldRun2Cover = TRUE;
		// and stop the move toward
		GetStateFrame()->LatentAction = 0;
	}
	else
	{
		// otherwise move normally
		Super::execPollMoveToward(Stack,Result);
	}
}

UBOOL AGearAI::WantsLedgeCheck()
{
	// see if we should force ledge check on/off (never just after mantle as a fallback for some rare cases where mantle doesn't work correctly and they get stuck on top of cover)
	if ( GetStateFrame()->LatentAction == AI_PollMoveToward && MoveTarget != NULL &&
		(MyGearPawn == NULL || (WorldInfo->TimeSeconds - MyGearPawn->LastmantleTime > 4.0f && WorldInfo->TimeSeconds - MyGearPawn->CreationTime > 4.0)) )
	{
		if (CurrentPath != NULL && Cast<ANavigationPoint>(MoveTarget) != NULL)
		{
			// check if we're on a path that requires falling off a ledge
			if (CurrentPath->IsA(ULeapReachSpec::StaticClass()) || CurrentPath->IsA(UMantleReachSpec::StaticClass()))
			{
				// turn bCanJump on because checkforledges turns it off and never turns it back on :(
				if(MyGearPawn != NULL)
				{
					MyGearPawn->bCanJump = TRUE;
				}
				return FALSE;
			}
			// check if still on path
			if (CurrentPath->Start != NULL && CurrentPath->End == MoveTarget)
			{
				FVector LineDir = Pawn->Location - (CurrentPath->Start->Location + (CurrentPathDir | (Pawn->Location - CurrentPath->Start->Location)) * CurrentPathDir);
				if (LineDir.SizeSquared2D() < 0.5f * Pawn->CylinderComponent->CollisionRadius * Pawn->CylinderComponent->CollisionRadius)
				{
					//debugf(TEXT("%s skip ledge check because on path"), *Pawn->GetName());
					// turn bCanJump on because checkforledges turns it off and never turns it back on :(
					if(MyGearPawn != NULL)
					{
						MyGearPawn->bCanJump = TRUE;
					}
					return FALSE;
				}
			}

			return TRUE;
		}
		else
		{
			// turn bCanJump on because checkforledges turns it off and never turns it back on :(
			if(!bWantsLedgeCheck && MyGearPawn != NULL)
			{
				MyGearPawn->bCanJump = TRUE;
			}
			return bWantsLedgeCheck;
		}
	}
	else
	{
		// turn bCanJump on because checkforledges turns it off and never turns it back on :(
		if(!bWantsLedgeCheck && MyGearPawn != NULL)
		{
			MyGearPawn->bCanJump = TRUE;
		}
		return bWantsLedgeCheck;
	}
}

void AGearAI::UpdatePawnRotation()
{
	if (MyGearPawn == NULL)
	{
		if (FireTarget != NULL && bWeaponCanFire)
		{
			Focus = FireTarget;
			SetFocalPoint( FireTarget->Location );
		}
		Super::UpdatePawnRotation();
	}
	else if (MyGearPawn->bLockRotation || (MyGearPawn->SpecialMove != SM_None && MyGearPawn->SpecialMoves(MyGearPawn->SpecialMove) && MyGearPawn->SpecialMoves(MyGearPawn->SpecialMove)->bPawnRotationLocked))
	{
		// keep pawn rotation
		Rotation = Pawn->Rotation;
	}
	else if (MyGearPawn->IsInCover())
	{
		FVector LookFocalPoint = Pawn->Location + FRotationMatrix(MyGearPawn->CurrentLink->GetSlotRotation(MyGearPawn->CurrentSlotIdx)).TransformFVector(FVector(256.f,0.f,0.f));
		Pawn->rotateToward( LookFocalPoint );
		Rotation = Pawn->Rotation;
	}
	else if (MyGearPawn->SpecialMove == SM_RoadieRun && MoveTarget != NULL)
	{
		Pawn->rotateToward(!MyGearPawn->Acceleration.IsNearlyZero() ? (MyGearPawn->Location + MyGearPawn->Acceleration) : MoveTarget->Location);
		Rotation = Pawn->Rotation;
	}
	else if (FireTarget && (MyGearPawn->SpecialMove == SM_None || MyGearPawn->SpecialMove == SM_Kidnapper) && (bWeaponCanFire || MyGearPawn->bWantsToMelee))
	{
		// face fire target if attacking with our weapon
		Pawn->rotateToward(FireTarget->Location);
		Rotation = Pawn->Rotation;
	}
	else if( MyGearPawn && MyGearPawn->bAllowTurnSmoothing && Focus && Focus == MoveTarget && !GetFocalPoint().IsNearlyZero())
	{
		// if we would normally rotate toward our move target, instead rotate toward focalpoint
		Pawn->rotateToward(GetFocalPoint());
		Rotation = Pawn->Rotation;
	}
	else
	{
		Super::UpdatePawnRotation();
	}
}


UBOOL AGearAI::IsEnemyVisible(APawn* EnemyPawn)
{
	for(INT Idx=0;Idx<LocalEnemyList.Num();Idx++)
	{
		if(LocalEnemyList(Idx).Pawn == EnemyPawn)
		{
			return LocalEnemyList(Idx).bVisible;
		}		
	}

	return FALSE;
}

/** Called to force controller to abort current move
  * -overidden to reset turning radius
  */
void AGearAI::FailMove()
{
	//debugf(TEXT("(%4.2f) FAILMOVE for %s %s "),GWorld->GetTimeSeconds(),*GetName(),*Pawn->GetName());
	if(MyGearPawn != NULL)
	{
		MyGearPawn->EffectiveTurningRadius = MyGearPawn->TurningRadius;
	}
	Super::FailMove();	
}


UBOOL AGearAI::ProcessStimulus( APawn* E, BYTE Type, FName EventName )
{
	AGearPC* PC = (E) ? Cast<AGearPC>(E->Controller) : NULL;
	if((PC && PC->bInvisible) || (E && (!E->IsPlayerPawn() || IsFriendlyPawn(E))))
	{
		return FALSE;
	}


	//debugf(TEXT("%2.3f: %s process %s type %d"),GWorld->GetTimeSeconds(),*GetName(),*E->GetName(),Type);
	
	// if we just got a stimulus, that's interesting!
	NotifyDidSomethingInteresting();

	// nudge the special 'player sight' channel
	if(Type == PT_Sight)
	{
		if(E->IsHumanControlled())
		{
			ReactionManager->NudgePerceptionChannel(E, PT_SightPlayer);
		}
	}
	ReactionManager->NudgePerceptionChannel(E, Type);

	if( Squad )
	{
		return Squad->ProcessStimulus( this, E, Type, EventName );
	}
	return FALSE;
}

void AGearAI::NotifyDidSomethingInteresting(FLOAT TimeOfInterestingThing)
{
	FLOAT recheckTime = StaleTimeout;
	FLOAT TimeSeconds = GWorld->GetTimeSeconds();
	if(TimeOfInterestingThing > -1.f &&(TimeSeconds - TimeOfInterestingThing) < StaleTimeout)
	{
		recheckTime = StaleTimeout-(TimeSeconds-TimeOfInterestingThing);
	}
	SetTimer(recheckTime,FALSE,FName(TEXT("ConditionalDeleteWhenStale")));
}

void AGearAI::SeePlayer( APawn* Seen )
{
	if( Seen &&
		Seen != Pawn )
	{
		if( !IsFriendlyPawn( Seen ) )
		{
			ProcessStimulus( Seen, PT_Sight, FName(TEXT("SeePlayer")) );
		}
	}
}

UBOOL AGearAI::IsFriendlyPawn(APawn* TestPlayer)
{
	if (TestPlayer)
	{
		if (TestPlayer->Controller)
		{
			return IsFriendly(TestPlayer->Controller);
		}
		else
			if (TestPlayer->DrivenVehicle && TestPlayer->DrivenVehicle->Controller)
			{
				return IsFriendly(TestPlayer->DrivenVehicle->Controller);
			}
	}
	// assume they're friendly otherwise
	return TRUE;
}

UBOOL AGearAI::CanHear(const FVector& NoiseLoc, FLOAT Loudness, AActor *Other)
{
	// check to see if we've seen this dude recently.. if we have don't bother doing a LOS check
	AActor* InstActor = Other;
	if(Other && Other->Instigator)
	{
		InstActor = Other->Instigator;
	}

	APawn* OtherPawn = Cast<APawn>(InstActor);
	
	UBOOL bReturnVal = FALSE;
	if(Pawn && OtherPawn && IsEnemyVisible(OtherPawn))
	{
		UBOOL OldLOSHearing = Pawn->bLOSHearing;
		Pawn->bLOSHearing = FALSE;
		bReturnVal = Super::CanHear(NoiseLoc,Loudness,Other);
		Pawn->bLOSHearing = OldLOSHearing;
		return bReturnVal;
	}
	else
	{
		return Super::CanHear(NoiseLoc,Loudness,Other);
	}
}

void AGearAI_Wretch::UpdatePawnRotation()
{
	// If wretch is climbing a wall
	if( Pawn && Pawn->Physics == PHYS_Spider )
	{
		// Compute the desired axes
		// Up vector is the floor normal
		FVector Z = Pawn->Floor;
		// Forward vector is the direction the pawn is moving
		FVector X = Pawn->Velocity.SafeNormal();
		// Right vector is cross product of the two we know
		FVector Y = X ^ Z;

		// Build a matrix out of the known axes
		FMatrix R;
		R.SetAxes( &X, &Y, &Z, &Pawn->Location );

		// Grab desired rotation from the matrix
		// (This approace avoid gimbal lock that occurs when using straight rotators)
		Pawn->DesiredRotation = R.Rotator();
	}
	else
	{
		// only rotate toward our focus if it's close to us, and not too far away from the direction we're going
		if (MyGearPawn != NULL && Focus && Focus != MoveTarget && !MyGearPawn->Acceleration.IsNearlyZero())
		{
			FVector MeToFocus = Focus->Location - MyGearPawn->Location;
			FLOAT DistToFocus = MeToFocus.Size();
			//DrawDebugCylinder(MyGearPawn->Location,MyGearPawn->Location,EnemyDistance_Short,16,255,0,0);
			if(DistToFocus > EnemyDistance_Short || (MyGearPawn->Acceleration.SafeNormal() | (MeToFocus/DistToFocus)) < 0.f)
			{
				MyGearPawn->rotateToward(MyGearPawn->Location + MyGearPawn->Velocity);
				return;
			}
		}
		Super::UpdatePawnRotation();
	}
}

UBOOL AGearAI::GetNavigationPointsNear( FVector chkLocation, FLOAT ChkDistance, TArray<class ANavigationPoint*>& Nodes )
{
	ANavigationPoint*	Nav = GWorld->GetWorldInfo()->NavigationPointList;

	Nodes.Empty();

	while( Nav )
	{
		if( !Nav->bBlocked &&
			(Nav->Location - chkLocation).Size() <= ChkDistance )
//			&&
//			!IsInvalidatedMoveGoal( Nav ) )
		{
			Nodes.AddItem( Nav );
		}
		Nav = Nav->nextNavigationPoint;
	}

	return Nodes.Num() > 0;
}

UBOOL AGearAI::IgnoreNotifies() const
{
	// no pawn yet - wait for possession
	if (Pawn == NULL)
	{
		return TRUE;
	}

	if(CommandList != NULL && CommandList->ShouldIgnoreNotifies())
	{
		return TRUE;
	}

	// If move to goal is not interruptable - skip notification
	if( (bMovingToGoal || bMovingToCover) && !bMoveGoalInterruptable )
	{
		return TRUE;
	}

	// If pawn is doing a special move that mutes AI - skip notification
	if( MyGearPawn != NULL &&
		(MyGearPawn->IsDBNO() ||
		(MyGearPawn->IsDoingASpecialMove() ))
	   )
	{
		const UGearSpecialMove* DefGSM = MyGearPawn->SpecialMoveClasses(MyGearPawn->SpecialMove)->GetDefaultObject<UGearSpecialMove>();
		if(DefGSM != NULL && DefGSM->bDisableAI)
		{
			return TRUE;
		}
	}

	if( !bAllowCombatTransitions )
	{
		return TRUE;
	}

	return FALSE;
}

UBOOL AGearAI::IsFriendly(AController* TestPlayer)
{
	if (TestPlayer != NULL && TestPlayer->PlayerReplicationInfo != NULL && PlayerReplicationInfo != NULL)
	{
		// 255 == always friendly in SP, always enemy in MP
		if (TestPlayer->GetTeamNum() == 255)
		{
			return GWorld->GetGameInfo()->IsA(AGearGameSP_Base::StaticClass());
		}
		// 254 == always friendly
		else if (TestPlayer->GetTeamNum() == 254)
		{
			return TRUE;
		}
		else
		{
			return (TestPlayer->PlayerReplicationInfo->Team == PlayerReplicationInfo->Team);
		}
	}
	else
	{
		return FALSE;
	}
}

UBOOL AGearAI::CanEvade( UBOOL bCheckChanceToEvade, FLOAT InChanceToEvade, FLOAT ChanceToEvadeScale )
{
	if(InChanceToEvade < 0.f)
	{
		InChanceToEvade = ChanceToEvade;
	}

	if( MyGearPawn == NULL ||
		MyGearPawn->IsDoingASpecialMove() ||
		MyGearPawn->Physics != PHYS_Walking ||
		MyGearPawn->IsDBNO() ||
		!MyGearPawn->IsAliveAndWell() ||
		CurrentTurret != NULL ||
		Cast<APawn>(MyGearPawn->GetBase()) != NULL ||
		(WorldInfo->TimeSeconds - MyGearPawn->LastEvadeTime) < MyGearPawn->MinTimeBetweenEvades ||
		(MyGearPawn->MyGearWeapon != NULL && MyGearPawn->MyGearWeapon->WeaponType == WT_Heavy) )
	{
		return FALSE;
	}

	// hardcode Campaign Locust AI to never evade on Casual
	if (Cast<AGearGameSP_Base>(WorldInfo->Game) != NULL && GetTeamNum() == 1)
	{
		AGearPRI* PRI = Cast<AGearPRI>(PlayerReplicationInfo);
		if (PRI != NULL && PRI->Difficulty != NULL && PRI->Difficulty->GetDefaultObject<UDifficultySettings>()->DifficultyLevel == DL_Casual)
		{
			return FALSE;
		}
	}

	// If checking AI chance to evade
	if( bCheckChanceToEvade )
	{
		FLOAT Chance = InChanceToEvade * ChanceToEvadeScale;
		FLOAT R = appFrand();
		// if this is locust AI then scale chance based on difficulty
		if (GetTeamNum() == 1)
		{
			Chance *= eventGetEvadeChanceScale();
		} 
		if( R > Chance )
		{
			return FALSE;
		}
	}

	return TRUE;
}

UBOOL AGearAI::ShouldIgnoreNavigationBlockingFor(const AActor* Other)
{
	AGearPawn* GP = const_cast<AGearPawn*>(ConstCast<const AGearPawn>(Other));

	if(GP != NULL)
	{
		// ignore if on enemy team, since we can kill it
		BYTE OtherTeam = GP->GetTeamNum();
		if (OtherTeam != 254 && OtherTeam != GetTeamNum())
		{
			return TRUE;
		}
		else if (MoveTarget != NULL)
		{
			// if it's a gearpawn and it's moving, and it's moving in roughly the same direction we are.. ignore it
			FLOAT Speed = GP->Velocity.Size2D();		
			if(Speed > 5.0f && // moving
			   GP->MyGearAI != NULL && GP->MyGearAI->MoveTarget != NULL  &&  // is an AI
			   ((MoveTarget->Location - Pawn->Location).SafeNormal() | (GP->Velocity/Speed)) >= 0.5f // moving in same dir
			   )
			{
				return TRUE;
			}
		}
	}

	return Super::ShouldIgnoreNavigationBlockingFor(Other);
}

UBOOL AGearAI_Cover::HasValidCover()
{
	return (IsValidCover(Cover) && Cover.Link->IsValidClaim(MyGearPawn,Cover.SlotIdx));
}

UBOOL AGearAI_Cover::IsValidCover(const FCoverInfo& TestCover)
{
	return (TestCover.Link != NULL &&
		TestCover.SlotIdx >= 0 && TestCover.SlotIdx < TestCover.Link->Slots.Num() &&
		!TestCover.Link->bPlayerOnly &&
		!TestCover.Link->bCircular &&
		TestCover.Link->Slots(TestCover.SlotIdx).bEnabled &&
		!TestCover.Link->Slots(TestCover.SlotIdx).bPlayerOnly);
}


#define DEBUG_ADJUSTTOSLOT	(0)
void AGearAI_Cover::execAdjustToSlot(FFrame &Stack,RESULT_DECL)
{
	P_GET_OBJECT(ACoverSlotMarker,TargetMarker);
	P_FINISH;

	bReachedCover = FALSE;
	if( TargetMarker == NULL )
	{
		return;
	}

	AGearPawn* WP = Cast<AGearPawn>(Pawn);
	if( WP && 
		WP->RightSlotIdx >= 0 &&
		WP->LeftSlotIdx  >= 0 )
	{
//debug
#if DEBUG_ADJUSTTOSLOT
		AILog(TEXT("AdjustToSlot C %i T %i R %i L %i Pct %f Link %s"), WP->CurrentSlotIdx, WP->RightSlotIdx, WP->LeftSlotIdx, WP->CurrentSlotPct, *WP->CurrentLink->GetName());
#endif

		WP->TargetSlotMarker	= TargetMarker;

		// Set poll function
		AdjustToSlotTime = 0.f;		
		GetStateFrame()->LatentAction = AI_PollAdjustToSlot + 100;
	}
	else
	{
		// If we don't have valid slots, assume we reached the cover (ie cover link only has one slot)
		bReachedCover = TRUE;
	}
}

void AGearAI_Cover::execPollAdjustToSlot( FFrame &Stack,RESULT_DECL )
{
	AGearPawn* WP = Cast<AGearPawn>(Pawn);
	if( WP != NULL )
	{
		//debug
		if( WP->CurrentLink )
		{
			if( WP->RightSlotIdx == -1 ||
				WP->LeftSlotIdx == -1 ||
				WP->RightSlotIdx >= WP->CurrentLink->Slots.Num() ||
				WP->LeftSlotIdx >= WP->CurrentLink->Slots.Num() )
			{
				debugf(TEXT("ERROR: %s [%s] Link %s Slot Index out of bounds!!!!! C %i L %i R %i Num %i"),
					Pawn ? *Pawn->GetName() : TEXT("None"),
					(GetStateFrame() && GetStateFrame()->StateNode) ? *GetStateFrame()->StateNode->GetName() : TEXT("None"),
					*WP->CurrentLink->GetName(),
					WP->CurrentSlotIdx,
					WP->LeftSlotIdx,
					WP->RightSlotIdx,
					WP->CurrentLink->Slots.Num() );
				debugf(TEXT("%s"), *Stack.GetStackTrace() );
				return;
			}
		}
		else
		{
			debugf(TEXT("ERROR %s execPollAdjustToSlot w/ no current link!"), *WP->GetName());
			debugf(TEXT("%s"), *Stack.GetStackTrace() );
			GetStateFrame()->LatentAction = 0;
			return;
		}


		// Update adjustment time
		AdjustToSlotTime += Pawn->AvgPhysicsTime;
		if (AdjustToSlotTime > UCONST_TIMER_ADJUSTTOSLOT_LIMIT)
		{
			debugf(TEXT("exceeded adjust to slot time. Pawn: %s"), *Pawn->GetName() );
#if DO_AI_LOGGING && !FINAL_RELEASE
			AILog(TEXT("AdjustToSlot failed due to timeout! (Took more than %.2f seconds) Target: %s LeftIdx: %i RightIdx: %i CurrentIdx: %i CurrentPct:%.2f SlotDir:%i"),
				UCONST_TIMER_ADJUSTTOSLOT_LIMIT,
				*WP->TargetSlotMarker->OwningSlot.Link->GetName(),
				WP->LeftSlotIdx,
				WP->RightSlotIdx,
				WP->CurrentSlotIdx,
				WP->CurrentSlotPct,
				WP->CurrentSlotDirection);
#endif


			WP->CurrentSlotDirection = CD_Default;
			WP->Velocity = FVector(0.f);
			GetStateFrame()->LatentAction = 0;
			return;
		}

		// Wait until mirror transition is done before moving
		UBOOL bInMirrorTransition = (WP->bWantsToBeMirrored != WP->bIsMirrored) || WP->bDoingMirrorTransition;
		if( bInMirrorTransition )
		{
			return;
		}

		// Once AGearPawn::CalcVelocity has updated our anchor to match the target marker, we are done adjusting
		if( WP->Anchor == WP->TargetSlotMarker )
		{
			// Notify AI that we reached the slot and exit latent function
			bReachedCover = TRUE;
			WP->CurrentSlotDirection = CD_Default;
			WP->Velocity = FVector(0.f);
			GetStateFrame()->LatentAction = 0;
			return;
		}

		if( RouteCache.Num() == 0 )
		{
			debugf(TEXT("ERROR %s execPollAdjustToSlot w/ empty routecahce?! %s %s"), *WP->GetName(), *CurrentPath->GetName(), *MoveTarget->GetName() );
			debugf(TEXT("%s"), *Stack.GetStackTrace() );
			GetStateFrame()->LatentAction = 0;
			return;
		}

		ANavigationPoint* CurrGoal = RouteCache(0);
		USlotToSlotReachSpec* Spec = Cast<USlotToSlotReachSpec>(WP->Anchor->GetReachSpecTo( CurrGoal ));
		if( Spec != NULL )
		{
			WP->CurrentSlotDirection = Spec->SpecDirection;

			if( !WP->bDoing360Aiming )
			{
				// Turn off animation mirroring when moving right
				if(  WP->CurrentSlotDirection == CD_Right && 
					 WP->CurrentSlotPct < 0.9 &&	
					 WP->bWantsToBeMirrored )
				{
					WP->eventSetMirroredSide( FALSE );
				}
				else
				// Turn on animation mirroring when moving left
				if(  WP->CurrentSlotDirection == CD_Left && 
					 WP->CurrentSlotPct > 0.1 &&	
					!WP->bWantsToBeMirrored )
				{
					WP->eventSetMirroredSide( TRUE );
				}
			}
		}
		else
		{
			// MT->Note: this can happen when our anchor is swapped out for a dynamicanchor to avoid something in our way
			//debug
			debugf(TEXT("failed to reach slot? %d/%d %d %2.1f, Pawn: %s"),WP->RightSlotIdx,WP->LeftSlotIdx,WP->TargetSlotMarker->OwningSlot.SlotIdx,WP->CurrentSlotPct,*Pawn->GetName());

			// We should never get here! - but just in case
			// exit latent function
			WP->CurrentSlotDirection = CD_Default;
			WP->Velocity = FVector(0.f);
			GetStateFrame()->LatentAction = 0;
		}
	}
}

IMPLEMENT_FUNCTION( AGearAI_Cover, AI_PollAdjustToSlot + 100, execPollAdjustToSlot );

UBOOL AGearAI_Cover::GetCoverAction( FCoverInfo& ChkCover, AActor* ChkTarget, UBOOL bAnyAction, UBOOL bTest, FName DebugTag )
{
	if( ChkCover.Link == NULL || ChkCover.SlotIdx < 0 )
	{
		//debug
#if DO_AI_LOGGING
		AILog(TEXT("GetCoverAction - failed because of invalid cover: %s %d"), *ChkCover.Link->GetName(),ChkCover.SlotIdx);
#endif

		return FALSE;
	}

	// default to the current fire target
	if( ChkTarget == NULL )
	{
		ChkTarget = FireTarget != NULL ? FireTarget : Enemy;
	}
	if( Cast<AController>(ChkTarget) != NULL )
	{
		ChkTarget = Cast<AController>(ChkTarget)->Pawn;
	}
	// if still no target then abort
	if( ChkTarget == NULL )
	{
		//debug
#if DO_AI_LOGGING
		AILog(TEXT("GetCoverAction - failed because of no target %s %s"),*FireTarget->GetName(),*Enemy->GetName());
#endif

		return FALSE;
	}

	FCoverSlot&	Slot = ChkCover.Link->Slots(ChkCover.SlotIdx);

	INT FireLinkIdx = -1;
	INT	ItemIdx		= -1;
	UBOOL bFound	= FALSE;
	UBOOL bTestFallbacks = FALSE;
	FFireLinkItem Item;
	appMemzero( &Item, sizeof(Item) );

	// see if it's another warpawn
	AGearPawn* GP = Cast<AGearPawn>(ChkTarget);

	// if checking for any action
	if( bAnyAction )
	{
		if( Slot.FireLinks.Num() > 0 )
		{
			INT FireLinkIdx = RandHelper(Slot.FireLinks.Num());
			if( Slot.FireLinks(FireLinkIdx).Items.Num() > 0 )
			{
				ItemIdx = RandHelper(Slot.FireLinks(FireLinkIdx).Items.Num());
				Item	= Slot.FireLinks(FireLinkIdx).Items(ItemIdx);
				bFound	= TRUE;
			}
		}
	}
	else
	{
		// if it's a pawn and they have cover
		FCoverInfo ChkTargetCover;
		if( GP != NULL && GetPlayerCover( GP, ChkTargetCover ) )
		{
			//debug
#if DO_AI_LOGGING
			AILog(EName(DebugTag.GetIndex()), TEXT("Getting fire links to target: %s %s %s"), *ChkCover.Link->eventGetDebugString(ChkCover.SlotIdx), *GP->GetName(), *ChkTargetCover.Link->eventGetDebugString(ChkTargetCover.SlotIdx));
#endif

			// then use existing firelinks
			TArray<INT>	Items;
			if( ChkCover.Link->GetFireLinkTo( ChkCover.SlotIdx, ChkTargetCover, GP->CoverAction, GP->CoverType, FireLinkIdx, Items ) )
			{
				bFound = (Items.Num() > 0);
				if( bFound )
				{
					ItemIdx = Items(RandHelper(Items.Num()));
					Item	= Slot.FireLinks(FireLinkIdx).Items(ItemIdx);
				}
			}
		}
		else
		{
			bTestFallbacks = TRUE;
		}

		if( bTestFallbacks )
		{
			TArray<BYTE> Actions;

			//debug
#if DO_AI_LOGGING
			AILog(EName(DebugTag.GetIndex()), TEXT("Testing fallbacks..."));
#endif

			// Fallback to direct traces
			// check lean left
			if( Slot.bLeanLeft &&
				eventCanFireAt( ChkTarget, ChkCover.Link->GetSlotViewPoint( ChkCover.SlotIdx, CT_None, CA_LeanLeft ) ) )
			{
				Actions.AddItem( CA_LeanLeft );
			}

			// check lean right
			if( Slot.bLeanRight &&
				eventCanFireAt( ChkTarget, ChkCover.Link->GetSlotViewPoint( ChkCover.SlotIdx, CT_None, CA_LeanRight ) ) )
			{
				Actions.AddItem( CA_LeanRight );
			}

			// check pop up
			if( Slot.CoverType == CT_MidLevel &&
				Slot.bCanPopUp &&
				eventCanFireAt( ChkTarget, ChkCover.Link->GetSlotViewPoint( ChkCover.SlotIdx, CT_None, CA_PopUp ) ) )
			{
				Actions.AddItem( CA_PopUp );
			}
			
			//debug
#if DO_AI_LOGGING
			AILog(EName(DebugTag.GetIndex()), TEXT("FALLBACK ACTIONS %d %s"),Actions.Num(),*ChkTarget->GetName());
			for( INT Idx = 0; Idx < Actions.Num(); Idx++ )
			{
				AILog(EName(DebugTag.GetIndex()), TEXT("Action %d %d"), Idx, Actions(Idx));
			}
#endif

			bFound = (Actions.Num() > 0);
			if( bFound )
			{
				ItemIdx = RandHelper(Actions.Num());
				Item.SrcAction	= Actions(ItemIdx);
				Item.SrcType	= CT_None;
				Item.DestAction = CA_Default;
				Item.DestType	= CT_None;
			}
		}
	}

	UBOOL b360Aiming = (Cast<APawn>(ChkTarget) != NULL) && 
							eventShouldDo360Aiming( GetEnemyLocation( Cast<APawn>(ChkTarget) )) && 
							eventCanFireAt( ChkTarget, ChkCover.Link->GetSlotViewPoint( ChkCover.SlotIdx, Slot.CoverType == CT_Standing ? CT_Standing : CT_MidLevel, CA_Default ) );
	if( bFound || b360Aiming )
	{
		// If just testing, retrun true w/o altering pending action
		if( !bTest )
		{
			// If doing 360 aiming to hit target
			if( b360Aiming )
			{
				//debug
#if DO_AI_LOGGING
				AILog(EName(DebugTag.GetIndex()), TEXT("Do 360 aiming instead of cover action"));
#endif

				// Don't actually step out, just stay default and let
				// animation code do aiming
				if( Item.SrcAction == CA_LeanLeft ||
					Item.SrcAction == CA_LeanRight ||
					Item.SrcAction == CA_PopUp )
				{
					//debug
#if DO_AI_LOGGING
					AILog(EName(DebugTag.GetIndex()), TEXT("- forcing default action for 360 aiming"));
#endif

					Item.SrcAction = CA_Default;
				}
			}
			else
			if( !bAnyAction )
			{
				// If allowed to blind fire AND we
				// passed chance of blind fire test
				if( eventShouldBlindFire() )
				{
					// Substitued blind fire actions
					switch( Item.SrcAction )
					{
						case CA_PopUp:		Item.SrcAction = CA_BlindUp;	break;
						case CA_LeanLeft:	Item.SrcAction = CA_BlindLeft;	break;
						case CA_LeanRight:	Item.SrcAction = CA_BlindRight;	break;
					}
				}
				else if (MyGearPawn != NULL && Cast<AGearWeap_GrenadeBase>(MyGearPawn->Weapon) != NULL)
				{
					// Force AI to stand up to throw a grenade
					if( (Item.SrcAction == CA_LeanLeft ||
							Item.SrcAction == CA_LeanRight) &&
						Slot.CoverType == CT_MidLevel &&
						Slot.bAllowPopup )
					{
						Item.SrcAction = CA_PopUp;
					}
				}
			}

			//debug

#if DO_AI_LOGGING
			AILog(EName(DebugTag.GetIndex()), TEXT("New cover action is: %d %d"), Item.SrcAction, Item.SrcType);
#endif

			PendingFireLinkItem = Item;
		}

		return TRUE;
	}

	if( !bTest )
	{
		//debug
#if DO_AI_LOGGING
		AILog(EName(DebugTag.GetIndex()), TEXT("Failed to set cover action"));
#endif

		PendingFireLinkItem.SrcAction = CA_Default;
		PendingFireLinkItem.SrcType	  = CT_None;
	}

	return FALSE;
}



/**
 *	Override to take into account GearPawns in cover and how/when our AI 
 *	can get updates from the world
 */
DWORD AGearAI::SeePawn( APawn *Other, UBOOL bMaySkipChecks )
{
	AGearPawn* OtherWP = Cast<AGearPawn>(Other);
	if(OtherWP != NULL)
	{
		// return false here because we are relying in the visibility man to update us
		return FALSE;
	}

	return Super::SeePawn(Other,bMaySkipChecks);
}

void AGearAI::ProcessSightCheckResult( APawn* Other, UBOOL bSeen, FVector& VisLoc )
{
	UBOOL bResult = FALSE;
	
	// Look through enemy list for this pawn
	FLocalEnemyInfo* Info = NULL;
	INT VisIdx = -1;
	for( VisIdx = 0; VisIdx < LocalEnemyList.Num(); VisIdx++ )
	{
		if( LocalEnemyList(VisIdx).Pawn == Other )
		{
			Info = &LocalEnemyList(VisIdx);
			break;
		}		
	}
	// If not already in local enemy list add an item
	if( !Info && bSeen )
	{
		VisIdx = LocalEnemyList.AddZeroed();
		Info = &LocalEnemyList(VisIdx);
		Info->Pawn = Other;
	}

	bResult = bSeen;
	if( Info )
	{
		FLOAT TimeSinceLastVisible = WorldInfo->TimeSeconds - Info->LastVisibleTime;

		// If enemy seen
		if( bSeen )
		{
			// If enemy just became visible
			if( !Info->bVisible )
			{
				// Mark initial time
				Info->InitialVisibleTime = WorldInfo->TimeSeconds;
			}
			FLOAT NotifyBecomeVisibleTime = Info->InitialVisibleTime + Response_MinEnemySeenTime;

			// If enemy has just become visible long enough
			UBOOL bNotify = FALSE;
			if( Info->bSeenLastFrame )
			{
				if( Info->LastVisibleTime  <  NotifyBecomeVisibleTime && 
					WorldInfo->TimeSeconds >= NotifyBecomeVisibleTime )
				{
					// We should notify AI of initial sighting
					bNotify = TRUE;
				}				
			}
			
			// Update visibility vars
			Info->bVisible = TRUE;
			Info->LastVisibleTime = WorldInfo->TimeSeconds;
			Info->AsyncVisibleLocation = VisLoc;
		
			// Actually notify the AI
			if( bNotify )
			{
				eventNotifyEnemyBecameVisible( Other, TimeSinceLastVisible );
			}

			// AI should respond to seeing enemy if they have been visible long enough
			bResult = (WorldInfo->TimeSeconds >= NotifyBecomeVisibleTime);
		}
		// Otherwise, enemy is blocked from view
		else
		{
			// Only if wasn't visible for a while, mark as not visible
			if( TimeSinceLastVisible > 2.f )
			{
				// If losing visibility, notify the AI
				if( Info->bVisible )
				{
					eventNotifyLostEnemyVisibility( Other );
				}

				Info->bVisible = FALSE;
			}			
		}

		// Update last frame sighting
		Info->bSeenLastFrame = bSeen;
	}


	// we just got a result, so call an event if we saw something
	if(bResult)
	{
		if (Other->Controller != NULL && Other->Controller->bIsPlayer)
		{
			eventSeePlayer( Other );
		}
		else
		{
			eventSeeMonster( Other );
		}
	}
}

UBOOL AGearAI::ShouldCheckToSeeEnemy( APawn *InPawn, FVector& LineChkStart, FVector& LineChkEnd )
{
	// if it's not a gear pawn then rely on ray cast only
	if(InPawn == NULL)
	{
		return TRUE;
	}

	if(IsFriendlyPawn(InPawn) || (!InPawn->IsPlayerPawn()))
	{
		return FALSE;
	}

	FVector EnemyViewLoc, ViewLoc, VectToEnemy, LookDir, X, Y, Z;
	FLOAT DotP = 0.f;

	//debug
	Debug_ViewLoc		= FVector(0.f);
	Debug_EnemyViewLoc	= FVector(0.f);
	Debug_LookDir		= FVector(0.f);
	
	// If pawns are invalid
	if(  Pawn == NULL || 
		!InPawn->IsValidEnemyTargetFor( PlayerReplicationInfo, TRUE ) )
	{
		return FALSE;
	}

	// If this isn't our current enemy, don't check beyond fog
	if( InPawn != Enemy &&
		BeyondFogDistance( Pawn->Location, InPawn->Location ) )
	{
		return FALSE;
	}

	FLOAT DistSq = (InPawn->Location - Pawn->Location).SizeSquared();
	// If target is beyond sight distance
	if( DistSq > Pawn->SightRadius * Pawn->SightRadius )
	{
		return FALSE;
	}

	AGearAI* AI = Cast<AGearAI>(InPawn->Controller);
	if( AI != NULL && AI->CombatMood == AICM_Ambush )
	{
		if( DistSq > 512.f && 
			((Pawn->Location - InPawn->Location).SafeNormal() | InPawn->Rotation.Vector()) > 0  )
		{
			return FALSE;
		}
	}

	// If doing an action in cover that has the gun stick out
	AGearPawn* E = Cast<AGearPawn>(InPawn);
	if( E != NULL &&
		E->IsInCover() && 
		E->CoverAction > CA_Default &&
		E->CoverAction < CA_PeekLeft )
	{
		// Try to see the gun
		EnemyViewLoc = E->eventGetPhysicalFireStartLoc( FVector(0.f) ) + FVector(0,0,10);
	}
	else
	{
		// Otherwise, look at their head location
		EnemyViewLoc = InPawn->eventGetPawnViewLocation();
	}
	
	ViewLoc		 = Pawn->eventGetPawnViewLocation() + FVector(0,0,10);
	VectToEnemy  = (EnemyViewLoc - ViewLoc).SafeNormal();
	
	// Get our facing direction	
	GetLookDir(LookDir);

	//debug
	Debug_ViewLoc		= ViewLoc;
	Debug_EnemyViewLoc	= EnemyViewLoc;
	Debug_LookDir		= LookDir;

	// If enemy is in front of where we are looking
	DotP = (VectToEnemy | LookDir);
	if( DotP > Pawn->PeripheralVision )
	{
		// Do line check to see
		LineChkStart = ViewLoc;
		LineChkEnd = EnemyViewLoc;
		return TRUE;
	}

	return FALSE;
}

void AGearAI::GetLookDir(FVector& LookDir)
{
	LookDir	= Pawn->Rotation.Vector();	
	
	AGearPawn *P = Cast<AGearPawn>(Pawn);
	if( P )
	{
		// If Pawn has a sight bone - use it's orientation
		if( P->Mesh && P->Mesh->SkeletalMesh && P->SightBoneName != NAME_None )
		{
			USkeletalMeshSocket* Socket = P->Mesh->SkeletalMesh->FindSocket(P->SightBoneName);
			FMatrix SocketMatrix;
			if( Socket != NULL && Socket->GetSocketMatrix( SocketMatrix, P->Mesh ) )
			{
				LookDir = -SocketMatrix.GetAxis( 1 );
			}
			else
			{
				// If bone exists, use that, otherwise, use Pawn rotation.
				if( P->Mesh->MatchRefBone(P->SightBoneName) != INDEX_NONE )
				{
					LookDir = -P->Mesh->GetBoneAxis( P->SightBoneName, AXIS_Y );
				}
				else
				{
					LookDir = P->Rotation.Vector();
				}
			}
		}
		else
			// Otherwise, if pawn is in default cover, use mirror flag to determine sight direction
			if( P->IsInCover() && 
				P->CoverAction == CA_Default )
			{
				FRotationMatrix R(Pawn->Rotation);
				LookDir = P->bIsMirrored ? -R.GetAxis(1) : R.GetAxis(1);
			}
	}
	else
	{
		LookDir = Pawn->eventGetViewRotation().Vector();
	}
	LookDir = LookDir.SafeNormal2D();

}

#define AI_SCALEBYXPOSURE (0)

UBOOL AGearAI::IsCoverExposedToAnEnemy( FCoverInfo TestCover, APawn *TestEnemy, FLOAT* out_ExposedScale )
{
	if (Squad == NULL || 
		TestCover.Link == NULL ||
		TestCover.SlotIdx < 0 ||
		TestCover.SlotIdx >= TestCover.Link->Slots.Num())
	{
		return FALSE;
	}
   
	UBOOL bResult = FALSE;

	FVector CoverLocation  = TestCover.Link->GetSlotLocation(TestCover.SlotIdx);
	FRotationMatrix RotMatrix(TestCover.Link->GetSlotRotation(TestCover.SlotIdx));
	FVector X = RotMatrix.GetAxis(0);
	FVector Y = RotMatrix.GetAxis(1);
	FVector Z = RotMatrix.GetAxis(2);

	FCoverSlot& Slot = TestCover.Link->Slots(TestCover.SlotIdx);
    FVector EnemyLocation(0.f);

	if (out_ExposedScale != NULL)
	{
		*out_ExposedScale = 0.f;
	}

	for( INT Idx = 0; Idx < Squad->EnemyList.Num(); Idx++ )
	{
		// If entry is invalid
		APawn* CheckEnemy = Squad->EnemyList(Idx).Pawn;
		if( !CheckEnemy )
		{
			// Remove it and move on
			Squad->EnemyList.Remove( Idx--, 1 );
			continue;
		}

		// If this isn't the enemy we want to check OR
		// it's an invalid target - SKIP
		if( (TestEnemy && TestEnemy != CheckEnemy) ||
			!CheckEnemy->IsValidTargetFor( this ) )
		{
			continue;
		}

		UBOOL bExposedToEnemy = FALSE;

		// Get enemy location - actual or perceived
		FCoverInfo EnemyCover;
		EnemyCover.Link		= NULL;
		EnemyCover.SlotIdx	= -1;
		
		EnemyLocation = Squad->GetEnemyLocationByIndex( Idx );
		AGearPawn* WP = Squad->EnemyList(Idx).GearPawn;
		if( WP )
		{
			GetPlayerCover( WP, EnemyCover, FALSE );
		}

		// If they have cover and they were in range to have a fire link to expose us
		if( EnemyCover.Link != NULL && (EnemyLocation-CoverLocation).SizeSquared() <= (EnemyCover.Link->MaxFireLinkDist*EnemyCover.Link->MaxFireLinkDist) )
		{
			// then check for an exposed fire link
			FLOAT ExposedScale = 1.f;
			bExposedToEnemy = TestCover.Link->IsExposedTo( TestCover.SlotIdx, EnemyCover, ExposedScale );
			if( bExposedToEnemy )
			{
#if DO_AI_LOGGING
				//debug
				AILog(NAME_Cover,TEXT("%s exposed to enemy cover: %s [%s %d]"),*TestCover.Link->GetName(),*WP->GetName(),*EnemyCover.Link->GetName(),EnemyCover.SlotIdx);
#endif

				if( out_ExposedScale != NULL )
				{
					*out_ExposedScale += ExposedScale;
				}
			}
		}
		else
		{
			// then check the dot
			FVector CoverToEnemy = (EnemyLocation-CoverLocation);
			FLOAT	DistSq = CoverToEnemy.SizeSquared();
			CoverToEnemy.Normalize();
			
			FLOAT YDotEnemy = (Y | CoverToEnemy);

			// Determine the angle we want to use
			FLOAT TestDot = UCONST_COVERLINK_ExposureDot;
			if( (Slot.bLeanLeft  && YDotEnemy  < -UCONST_COVERLINK_EdgeCheckDot) ||
				(Slot.bLeanRight && YDotEnemy  >  UCONST_COVERLINK_EdgeCheckDot) )
			{
				TestDot = UCONST_COVERLINK_EdgeExposureDot;
			}
			// Increase tolerance if they are a fair distance away
			
			AGearWeapon *EnemyWeap = NULL;			
			AGearPawn* GP = Squad->EnemyList(Idx).GearPawn;
			if(GP != NULL && GP->MyGearWeapon != NULL)
			{
				EnemyWeap = GP->MyGearWeapon;
			}
			else
			{
				EnemyWeap = Cast<AGearWeapon>(CheckEnemy->Weapon);
			}
			
			if( EnemyWeap != NULL && 
				DistSq >= (EnemyWeap->Range_Medium*EnemyWeap->Range_Medium)  )
			{
				TestDot *= 0.75f;
			}

			// Get dot to forward of cover
			FLOAT XDotEnemy = (X | CoverToEnemy);
			if( XDotEnemy <= TestDot )
			{
#if DO_AI_LOGGING
				AILog(NAME_Cover,TEXT("%s exposed to enemy: %s, testdot: %2.3f, xdotenemy: %2.3f, ydotenemy: %2.3f"),*TestCover.Link->GetName(),*WP->GetName(),TestDot,XDotEnemy,YDotEnemy);
#endif
				bExposedToEnemy = TRUE;
				if (out_ExposedScale != NULL)
				{
					*out_ExposedScale += 1.0f;
				}
			}
		}

		bResult = bResult || bExposedToEnemy;
	}

	return bResult;
}

UBOOL AGearAI::IsWithinTether(FVector TestPoint)
{
	return  (TetherActor == NULL || 
			(bReachedTether && Cast<ANavigationPoint>(TetherActor)) || 
			((TestPoint - TetherActor->Location).SizeSquared2D() <= TetherDistance*TetherDistance &&
			 Abs<FLOAT>(TestPoint.Z - TetherActor->Location.Z) <= TetherDistance * 2.0f));
}

FRotator AGearAI::SetRotationRate(FLOAT DeltaTime)
{
	FRotator Result = Super::SetRotationRate(DeltaTime);
	if( MyGearPawn != NULL )
	{
		if( MyGearPawn->SpecialMove == SM_RoadieRun )
		{
			Result *= 0.33f;
		}
		else if( MyGearPawn->SpecialMove == SM_DBNO )
		{
			// If just fell down, don't rotate just now, looks bad.
			if( (GWorld->GetWorldInfo()->TimeSeconds - MyGearPawn->TimeOfDBNO) < 2.f )
			{
				Result.Yaw = 0;
				//return FRotator(0,0,0);
			}
			else
			{
				Result.Yaw *= 0.15f;
			}
		}
		else
		{
			Result *= RotationRateMultiplier;
		}
	}

	return Result;
}

#include "DebugRenderSceneProxy.h"

class FEmergenceHoleRenderingSceneProxy : public FDebugRenderSceneProxy
{
public:

	FEmergenceHoleRenderingSceneProxy(const UEmergenceHoleRenderingComponent* InComponent):
	  FDebugRenderSceneProxy(InComponent)
	  {
		  AGearSpawner_EmergenceHoleBase *Hole = Cast<AGearSpawner_EmergenceHoleBase>(InComponent->GetOwner());
		  if( Hole && 
			  Hole->HasAnyFlags( RF_EdSelected ) )
		  {
			  FVector X, Y, Z;
			  FRotationMatrix R(Hole->Rotation);
			  R.GetAxes( X, Y, Z );

			  FLOAT	SpawnHeight = 78.f;
			  FLOAT	SpawnRadius = 36.f;
			  for (INT Idx = 0; Idx < Hole->SpawnSlots.Num(); Idx++)
			  {
				  FSpawnerSlot &Slot = Hole->SpawnSlots(Idx);
				  
				  // Get spawn location
				  FVector SpawnLoc = Hole->Location + 
										  X * Slot.LocationOffset.X + 
										  Y * Slot.LocationOffset.Y + 
										  Z * Slot.LocationOffset.Z;
				  if (Slot.bEnabled)
				  {
					  new(Cylinders) FWireCylinder(SpawnLoc,SpawnRadius,SpawnHeight,FColor(0,0,255));

					  // Draw a hatch mark for LDs to see which slots are enabled
					  new(Stars) FWireStar( SpawnLoc + FVector(0,0,200), FColor(0,0,255), 16.f );
				  }
				  else
				  {
					  // Draw a hatch mark for LDs to see which slots are disabled
					  new(Stars) FWireStar( SpawnLoc + FVector(0,0,200), FColor(255,0,0), 16.f );
				  }
			  }
		  }
	  }

	  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	  {
		  FPrimitiveViewRelevance Result;
		  Result.bDynamicRelevance = IsShown(View);
		  Result.SetDPG(SDPG_World,TRUE);
		  return Result;
	  }

	  virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	  virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	  DWORD GetAllocatedSize( void ) const { return( FDebugRenderSceneProxy::GetAllocatedSize() ); }
};

FPrimitiveSceneProxy* UEmergenceHoleRenderingComponent::CreateSceneProxy()
{
	return new FEmergenceHoleRenderingSceneProxy(this);
}


void AGearSpawner_EmergenceHoleBase::PostLoad()
{
	Super::PostLoad();
	ApplySourceMaterial();
}

void AGearSpawner_EmergenceHoleBase::PostEditChange(UProperty *PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
	ApplySourceMaterial();
	for (INT Idx = 0; Idx < Components.Num(); Idx++)
	{
		UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Components(Idx));
		if (MeshComp != NULL)
		{
			MeshComp->SetLightingChannels(LightingChannels);
			MeshComp->bForceDirectLightMap = bForceDirectLightMap;
			UStaticMeshComponent *SMComp = Cast<UStaticMeshComponent>(MeshComp);
			if (SMComp != NULL)
			{
				SMComp->bOverrideLightMapResolution = bOverrideLightMapResolution;
				SMComp->OverriddenLightMapResolution = OverriddenLightMapResolution;
			}
		}
	}
}


void AGearSpawner_EmergenceHoleBase::ApplySourceMaterial()
{
	if (MatInst != NULL)
	{
		MatInst->SetTextureParameterValue(FName(TEXT("SourceTexture")),SourceTexture);
		CoverMesh->SetMaterial(0,MatInst);
	}
	if (SourceMaterial != NULL)
	{
		BaseMesh->SetMaterial(0,SourceMaterial);
		BrokenMesh->SetMaterial(0,SourceMaterial);
		Mesh->SetMaterial(0,SourceMaterial);
	}
}





UBOOL AGearAI_Brumak::GetActiveEnemyList( TArray<APawn*>& ActiveList )
{
	// Get a list of visible enemies
	// Add side guns first to allow bias towards side guns
	if( LeftGunAI != NULL && 
		LeftGunAI->Enemy != NULL && 
		LeftGunAI->Enemy->IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
	{
		ActiveList.AddItem( LeftGunAI->Enemy ); 
	}
	if( RightGunAI != NULL && 
		RightGunAI->Enemy != NULL && 
		RightGunAI->Enemy->IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
	{
		ActiveList.AddItem( RightGunAI->Enemy );
	}
	if( DriverAI != NULL && 
		DriverAI->Enemy != NULL && 
		DriverAI->Enemy->IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
	{
		ActiveList.AddUniqueItem( DriverAI->Enemy );
	}
	if( Enemy != NULL &&
		Enemy->IsValidEnemyTargetFor( PlayerReplicationInfo, FALSE ) )
	{
		ActiveList.AddUniqueItem( Enemy );
	}

	return (ActiveList.Num()>0);
}

UBOOL AGearAI_Brumak::CanFireAtTarget( class AGearAI_Brumak_Slave* GunAI, class APawn* TestEnemy, FVector TestBaseLocation, FVector GunOffset, FRotator PawnRot )
{
	if( TestEnemy == NULL )
	{
		TestEnemy = GunAI->Enemy;
	}
	if( GunAI == NULL || GunAI->Pawn == NULL || TestEnemy == NULL ) 
	{	
		return FALSE; 
	}

	FVector	 TestLoc = GetEnemyLocation( TestEnemy, LT_Known );
	if( PawnRot.IsZero() )
	{
		PawnRot = (TestLoc-TestBaseLocation).Rotation();
	}
	FVector  ViewPt;
	if( !TestBaseLocation.IsNearlyZero() )
	{
		ViewPt = TestBaseLocation + FRotationMatrix(PawnRot).Transpose().TransformNormal( GunOffset );
	}
	else
	{
		ViewPt = GunAI->Pawn->eventGetPawnViewLocation();
	}
	return CanSeeByPoints( ViewPt, TestLoc, PawnRot );
}

UBOOL AGearAI_Brumak::CanFireAtAnyTarget( class APawn* TestEnemy, FVector TestBaseLocation, FRotator PawnRot )
{
	return (DriverAI   != NULL && CanFireAtTarget( DriverAI,   (TestEnemy != NULL) ? TestEnemy : DriverAI->Enemy,   TestBaseLocation, MainGunFireOffset,  PawnRot  )) ||
		(LeftGunAI  != NULL && CanFireAtTarget( LeftGunAI,  (TestEnemy != NULL) ? TestEnemy : LeftGunAI->Enemy,  TestBaseLocation, LeftGunFireOffset,  PawnRot  )) || 
		(RightGunAI != NULL && CanFireAtTarget( RightGunAI, (TestEnemy != NULL) ? TestEnemy : RightGunAI->Enemy, TestBaseLocation, RightGunFireOffset, PawnRot  ));		   
}

FVector AGearAI_Brumak::GetVectorAvg( const TArray<class APawn*>& PawnList, FVector TestBaseLocation )
{
	// Figure out the average vector to those enemies
	FVector AvgVect(0,0,0);
	for( INT EnemyIdx = 0; EnemyIdx < PawnList.Num(); EnemyIdx++ )
	{
		APawn* EnemyPawn = PawnList(EnemyIdx);
		if( EnemyPawn == NULL || !EnemyPawn->IsValidEnemyTargetFor( PlayerReplicationInfo, TRUE ) )
		{
			continue;
		}

		FVector VectToEnemy = (GetEnemyLocation(PawnList(EnemyIdx)) - TestBaseLocation);
		AvgVect += VectToEnemy;
	}
	AvgVect.Z = 0.f;
	AvgVect /= PawnList.Num();
	return AvgVect;
}

/** 
 *	Searches EnemyList for the given pawn and returns the index 
 */
INT AGearSquad::GetEnemyIndex( APawn* Enemy )
{
	for( INT EnemyIdx = 0; EnemyIdx < EnemyList.Num(); EnemyIdx++ )
	{
		// Remove dead enemies
		if( EnemyList(EnemyIdx).Pawn == NULL )
		{
			EnemyList.Remove(EnemyIdx--,1);
			continue;
		}

		// If this is the enemy we are looking for - return index
		if( EnemyList(EnemyIdx).Pawn == Enemy )
		{
			return EnemyIdx;
		}
	}

	// Enemy not found
	return -1;
}

/** 
 *	Adds an enemy to the EnemyList if they aren't already there
 *	Returns the index into the EnemyList for given enemy
 */
INT AGearSquad::AddEnemy(const AGearAI* AI, APawn* NewEnemy)
{
	// Ignore invisible players
	if( NewEnemy != NULL )
	{
		AGearPC* PC = Cast<AGearPC>(NewEnemy->Controller);
		if( PC && PC->bInvisible )
		{
			return -1;
		}
	}
	
	// If enemy is a turret
	ATurret* TurretEnemy = Cast<ATurret>(NewEnemy);
	if( TurretEnemy )
	{
		// Keep driver as enemy
		NewEnemy = TurretEnemy->Driver;

	}

	if( NewEnemy == NULL )
	{
		return -1;
	}

	// Find enemy in our list of current foes
	INT EnemyIdx = GetEnemyIndex( NewEnemy );
	// If given enemy isn't known yet
	if( EnemyIdx == -1 )
	{
		//debug
		//debugf( TEXT("%s Add New Enemy %s"), *GetName(), *NewEnemy->GetName() );

		// Add to the enemy list
		EnemyIdx = EnemyList.AddZeroed();
		EnemyList(EnemyIdx).Pawn = NewEnemy;
		EnemyList(EnemyIdx).GearPawn = Cast<AGearPawn>(NewEnemy);
		EnemyList(EnemyIdx).InitialSeenTime = GWorld->GetTimeSeconds();
		EnemyList(EnemyIdx).LastSeenTime = GWorld->GetTimeSeconds();
		ProcessStimulus( AI, NewEnemy, PT_Force, FName(TEXT("AddEnemy")) );
		// return -1 here since we're already processing this enemy addition (keep process stimulus from executing twice)
		return -1;
	}

	return EnemyIdx;
}

/**
 *	Removes enmey from the EnemyList
 */
UBOOL AGearSquad::RemoveEnemy( APawn* DeadEnemy )
{
	UBOOL bResult = FALSE;
	for( INT EnemyIdx = 0; EnemyIdx < EnemyList.Num(); EnemyIdx++ )
	{
		if( EnemyList(EnemyIdx).Pawn == DeadEnemy )
		{
			EnemyList.Remove( EnemyIdx-- );
			bResult = TRUE;
			break;
		}
	}

	if( bResult )
	{
		for( INT MemberIdx = 0; MemberIdx < SquadMembers.Num(); MemberIdx++ )
		{
			AGearAI* AI = Cast<AGearAI>(SquadMembers(MemberIdx).Member);
			if( AI != NULL )
			{
				for( INT VisIdx = 0; VisIdx < AI->LocalEnemyList.Num(); VisIdx++ )
				{
					if( AI->LocalEnemyList(VisIdx).Pawn == DeadEnemy || AI->LocalEnemyList(VisIdx).Pawn == NULL )
					{
						AI->LocalEnemyList.Remove(VisIdx--);
					}
				}
			}
		}
	}
	
	return bResult;
}

UBOOL AGearSquad::ProcessStimulus(const AGearAI* AI, APawn* Enemy, BYTE Type, FName EventName)
{
	// If couldn't find enemy - exit
	INT EnemyIdx = AddEnemy( AI, Enemy );
	if( EnemyIdx < 0 && EnemyIdx < EnemyList.Num())
	{
		return FALSE;
	}
	
	// Check if the player was already updated this frame
	FPlayerInfo& Info = EnemyList(EnemyIdx);
	if( Info.LastUpdateTime == WorldInfo->TimeSeconds )
	{
		return TRUE;
	}
	FLOAT TimeSinceLastUpdate = WorldInfo->TimeSeconds - Info.LastUpdateTime;

	//debug
	//debugf(TEXT("ProcessStimulus: %s, %d, %s from %s"),*Info.Pawn->GetName(),Type,*EventName.ToString(),*AI->GetName());

	AGearPawn *E = Cast<AGearPawn>(Info.Pawn);
	if( E != NULL )
	{
		// Grab enemy location
		FVector EnemyViewLoc = E->eventGetPawnViewLocation();
		if( Type == PT_Force && E->IsInCover() )
		{
			TArray<BYTE> ActionList;

			INT EnemySlotIdx = appTrunc(E->CurrentSlotPct);
			E->CurrentLink->GetSlotActions( EnemySlotIdx, ActionList );
			if( ActionList.Num() > 0 )
			{
				EnemyViewLoc = E->CurrentLink->GetSlotViewPoint( EnemySlotIdx, CT_None, ActionList(0) );
			}
		}
		else
		if( Type == PT_Sight && AI != NULL )
		{
			for( INT LocalIdx = 0; LocalIdx < AI->LocalEnemyList.Num(); LocalIdx++ )
			{
				const FLocalEnemyInfo& Local = AI->LocalEnemyList(LocalIdx);
				if( Local.Pawn == Enemy )
				{
					EnemyViewLoc = Local.AsyncVisibleLocation;
				}
			}			
		}

		// If saw them, shot by them, or notified about them, make them visible
		if( Type == PT_Sight	||
			Type == PT_HurtBy	||
			Type == PT_Force	)
		{
			// Update known location/cover/base
			SetKnownEnemyInfo( EnemyIdx, E, EnemyViewLoc );
		}
		else
		// Otherwise, if we heard them and they haven't been updated recently
		if( Type == PT_Heard && TimeSinceLastUpdate > 1.f )
		{
			// If enemy has moved much from the last known position
			FLOAT Dist = (GetEnemyLocationByIndex(EnemyIdx)-EnemyViewLoc).SizeSquared();
			if( Dist > 4096 ) // 64^2
			{
				// Update known location/cover/base
				SetKnownEnemyInfo( EnemyIdx, E, EnemyViewLoc );
			}
		}
	}
	else 
	if( Info.Pawn != NULL )
	{
		Info.KnownLocation	= Info.Pawn->Location;
		Info.Base			= Info.Pawn->Base;
		Info.LastUpdateTime = WorldInfo->TimeSeconds;
	}

	BroadcastStimulus( Info.Pawn, Type, EventName );

	// if supported, send this stimulus to any other squads
	if (bInterSquadCommunication && Team != NULL)
	{
		for (INT i = 0; i < Team->Squads.Num(); i++)
		{
			if (Team->Squads(i) != NULL && Team->Squads(i) != this && Team->Squads(i)->bInterSquadCommunication)
			{
				Team->Squads(i)->bInterSquadCommunication = FALSE;
				Team->Squads(i)->ProcessStimulus(AI, Enemy, Type, EventName);
				Team->Squads(i)->bInterSquadCommunication = TRUE;
			}
		}
	}
	
	return TRUE;
}

UBOOL AGearSquad::BroadcastStimulus( APawn* Enemy, BYTE Type, FName EventName )
{
	UBOOL bResult = FALSE;

	for( INT Idx = 0; Idx < SquadMembers.Num(); Idx++ )
	{
		AGearAI* AI = Cast<AGearAI>(SquadMembers(Idx).Member);
		if( AI != NULL && !AI->IgnoreNotifies() )
		{
			bResult = TRUE;

			if( Type == PT_Heard )
			{
				AI->eventNotifyEnemyHeard( Enemy, EventName );
			}
			else
			{
				AI->eventNotifyEnemySeen( Enemy );
			}

			if( Type == PT_Force )
			{
				AI->eventNoticedEnemy( Enemy );
			}
		}
	}

	return bResult;
}

/**
 *	Update known enemy location AND cover
 *	Properly handles setting location as an offset for Enemies
 *		- In Cover
 *		- On movable objects (Base==InterpActor)
 */
void AGearSquad::SetKnownEnemyInfo( INT EnemyIdx, AGearPawn *Enemy, FVector EnemyLoc )
{
	FPlayerInfo& Info = EnemyList(EnemyIdx);

	// Update known location
	Info.KnownLocation	= EnemyLoc;
	Info.Base			= Enemy->Base;
	Info.LastSeenTime	= WorldInfo->TimeSeconds;

	// Update known cover
	if( Enemy->IsInCover() )
	{
		Info.Cover.Link		= Enemy->CurrentLink;
		Info.Cover.SlotIdx	= Enemy->CurrentSlotIdx;

		// Update location as an OFFSET from the cover slot (used to track in dynamic cover)
		Info.KnownLocation	= EnemyLoc - Enemy->CurrentLink->GetSlotLocation(Info.Cover.SlotIdx);
	}
	else
	{
		// Clear known cover
		Info.Cover.Link		= NULL;
		Info.Cover.SlotIdx	= -1;

		// If enemy is on a base that will move
		if( Cast<AInterpActor>(Enemy->Base) != NULL )
		{
			// Update location as an OFFSET from the based actor
			Info.KnownLocation = EnemyLoc - Enemy->Base->Location;
		}
	}

	Info.LastUpdateTime = WorldInfo->TimeSeconds;
}

FVector AGearSquad::GetEnemyLocationByIndex( INT EnemyIdx, BYTE LT )
{
	FPlayerInfo& Info = EnemyList(EnemyIdx);

	if( LT == LT_Exact )
	{
		// If doing an action in cover that has the gun stick out
		AGearPawn* GP = Cast<AGearPawn>(Info.Pawn);
		if( GP != NULL &&
			GP->IsInCover() && 
			GP->CoverAction > CA_Default &&
			GP->CoverAction < CA_PeekLeft )
		{
			// Try to see the gun
			return GP->eventGetPhysicalFireStartLoc( FVector(0.f) ) + FVector(0,0,10);
		}

		// Otherwise, look at their head location
		return Info.Pawn->eventGetPawnViewLocation();
	}

	FVector Offset(0.f);
	// If enemy is in cover
	// Use location as an offset to the cover slot (used to keep tracking dynamic cover)
	if( Info.Cover.Link != NULL )
	{
		Offset = Info.Cover.Link->GetSlotLocation(Info.Cover.SlotIdx);
	}
	else
	// If enemy is in on a moving actor
	// Use location as an offset to the base
	if( Cast<AInterpActor>(Info.Base) != NULL )
	{
		Offset = Info.Base->Location;
	}

	if(LT == LT_InterpVisibility)
	{	
		// if we've seen them within a half second, just use exact location
		if(WorldInfo->TimeSeconds - Info.LastSeenTime < 0.5f)
		{
			// If doing an action in cover that has the gun stick out
			AGearPawn* GP = Cast<AGearPawn>(Info.Pawn);
			if( GP != NULL &&
				GP->IsInCover() && 
				GP->CoverAction > CA_Default &&
				GP->CoverAction < CA_PeekLeft )
			{
				// Try to see the gun
				return GP->eventGetPhysicalFireStartLoc( FVector(0.f) ) + FVector(0,0,10);
			}

			// Otherwise, look at their head location
			return Info.Pawn->eventGetPawnViewLocation();
		}
		// otherwise fall through and use last known loc
	}

	return Info.KnownLocation + Offset;

}

FVector AGearSquad::GetEnemyLocation( APawn* TestPawn, BYTE LT )
{
	if( !TestPawn )
	{
		return FVector(0.f);
	}

	if( LT != LT_Exact )
	{
		INT EnemyIdx = GetEnemyIndex( TestPawn );
		if( EnemyIdx >= 0 )
		{
			return GetEnemyLocationByIndex( EnemyIdx, LT );
		}
	}

	return TestPawn->Location;
}

FLOAT AGearSquad::GetEnemyLastSeenTime( APawn* TestPawn )
{
	INT EnemyIdx = GetEnemyIndex( TestPawn );
	if(EnemyIdx >= 0)
	{
		return EnemyList(EnemyIdx).LastSeenTime;
	}
	return 0.f;
}

/**
 * Find the actual location of the squad leader.
 */
FVector AGearSquad::GetSquadLeaderLocation()
{
	AActor* LeaderPos = GetSquadLeaderPosition();
	if( LeaderPos == NULL && (Leader == NULL || Leader->Pawn == NULL) )
	{
		return FVector(0,0,0);
	}
	return ((LeaderPos == NULL) ? Leader->Pawn->Location : LeaderPos->Location);
}

/**
 * Find the position the squad leader is at, or in the case of the AI, where they are attempting to go.
 */
AActor* AGearSquad::GetSquadLeaderPosition()
{
	if( Leader != NULL && 
		Leader->Pawn != NULL )
	{
		AGearAI* AI = Cast<AGearAI>(Leader);
		if( AI != NULL && !bPlayerSquad)
		{
			if( AI->bMovingToRoute && 
				AI->ActiveRoute)
			{
				INT Idx = AI->ActiveRoute->MoveOntoRoutePath(Leader->Pawn,ERD_Forward,0.2f);
				if(Idx >=0 && Idx < AI->ActiveRoute->RouteList.Num())
				{
					return AI->ActiveRoute->RouteList(Idx);
				}
				else
				{
					return AI->MoveGoal;
				}
			}
			else
			if( AI->MoveAction != NULL &&
				AI->TetherActor != NULL )
			{
				return AI->TetherActor;
			}
			else
			if( AI->bMovingToGoal &&
				AI->MoveGoal != NULL )
			{
				return AI->MoveGoal;
			}
			else
			if( Leader->Pawn->Anchor != NULL )
			{
				return Leader->Pawn->Anchor;
			}
		}
		else
		{
			// If there is a gameplay route
			if( SquadRoute != NULL )
			{
				// If leader pawn is DBNO don't update squad route index because the AI will then
				// run off and leave the leader... want them to stick around to revive/protect
				AGearPawn* LeaderGP = Cast<AGearPawn>(Leader->Pawn);
				if( LeaderGP != NULL && LeaderGP->IsDBNO() && SquadRouteIndex >= 0 )
				{
					return SquadRoute->RouteList(SquadRouteIndex);
				}

				INT BestIdx = -1;
				if(bPlayerSquad)
				{
					// search for the furthest ahead player in the squad
					for(INT Idx=0;Idx<SquadMembers.Num();Idx++)
					{
						AController* Member = SquadMembers(Idx).Member;
						if(Member && Member->GetAPlayerController() && Member->Pawn)
						{
							BestIdx = Max<INT>(BestIdx,SquadRoute->MoveOntoRoutePath( Member->Pawn, ERD_Forward, SquadRoute->FudgeFactor ));
						}
					}

					// if there is a player in this squad, and the leader is not a player then we need to fake the squad position so that the ai's "lead" the player
					if(!Leader->GetAPlayerController())
					{
						if(BestIdx >=0 && BestIdx+1 < SquadRoute->RouteList.Num())
						{
							// we want the squad to lead the player
							BestIdx += 1;
						}
					}
				}
				else
				{
					BestIdx = SquadRoute->MoveOntoRoutePath( Leader->Pawn, ERD_Forward, SquadRoute->FudgeFactor );
				}

				// if the new index is less than the current one, and we're not allowed to go backward stick with the current one
				if(!SquadRoute->bAllowBackwardProgression && BestIdx < SquadRouteIndex)
				{
					return SquadRoute->RouteList(SquadRouteIndex);
				}

				if( BestIdx >= 0 )
				{
					if( SquadRouteIndex != BestIdx )
					{
						eventSetSquadRouteIndex( BestIdx );
					}
					return SquadRoute->RouteList(BestIdx);
				}

				eventSetSquadRoute( NULL );
			}
		}

		if (Leader->Pawn->Anchor != NULL && (Leader->Pawn->LastValidAnchorTime == WorldInfo->TimeSeconds || Leader->Pawn->ValidAnchor()))
		{
			return Leader->Pawn->Anchor;
		}
		else if (Leader->Pawn->FindAnchorFailedTime < WorldInfo->TimeSeconds && Leader->Pawn->Physics == PHYS_Walking)
		{
			FLOAT BestDist = 0.f;
			Leader->Pawn->SetAnchor(Leader->Pawn->GetBestAnchor( Leader->Pawn, Leader->Pawn->Location, TRUE, FALSE, BestDist ));
			if( Leader->Pawn->Anchor != NULL )
			{
				return Leader->Pawn->Anchor;
			}
			else
			{
				Leader->Pawn->FindAnchorFailedTime = WorldInfo->TimeSeconds;
			}
		}

		return Leader->Pawn;
	}

	return NULL;
}


/**
 * Retrieves enemy cover info
 */
FCoverInfo AGearSquad::GetEnemyCover( APawn* TestPawn )
{
	FCoverInfo Result;
	Result.Link		= NULL;
	Result.SlotIdx	= -1;

	for( INT EnemyIdx = 0; EnemyIdx < EnemyList.Num(); EnemyIdx++ )
	{
		FPlayerInfo& Info = EnemyList(EnemyIdx);
		if( Info.Pawn == TestPawn )
		{
			Result = Info.Cover;
			break;
		}
	}

	return Result;
}

void AGearSquad::execAllMembers(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(AController, OutC);
	P_FINISH;

	// if we have a valid subclass of NavigationPoint
	if( BaseClass )
	{
		INT MemberIdx = 0;

		PRE_ITERATOR;
		// get the next Controller in the iteration
		OutC = NULL;

		while( MemberIdx < SquadMembers.Num() && OutC == NULL )
		{
			AController* C = SquadMembers(MemberIdx).Member;
			if( C == NULL )
			{
				SquadMembers.Remove( MemberIdx );
				continue;
			}

			if( C->IsA( BaseClass ) )
			{
				OutC = C;
			}
			MemberIdx++;
		}

		if( OutC == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
		POST_ITERATOR;
	}
}

void AGearSquad::execAllEnemies(FFrame& Stack, RESULT_DECL)
{
	P_GET_OBJECT(UClass, BaseClass);
	P_GET_OBJECT_REF(APawn, OutP);
	P_GET_OBJECT_OPTX( AController, Asker, NULL );
	P_FINISH;

	// if we have a valid subclass of NavigationPoint
	if( BaseClass )
	{
		INT EnemyIdx = 0;

		PRE_ITERATOR;
		// get the next Controller in the iteration
		OutP = NULL;

		while( EnemyIdx < EnemyList.Num() && OutP == NULL )
		{
			APawn* P = EnemyList(EnemyIdx).Pawn;
			if( P == NULL )
			{
				EnemyList.Remove( EnemyIdx );
				continue;
			}

			if( P->IsA( BaseClass ) && 
				(Asker == NULL || P->IsValidEnemyTargetFor( Asker->PlayerReplicationInfo, TRUE )) )
			{
				OutP = P;
			}
			EnemyIdx++;
		}

		if( OutP == NULL )
		{
			Stack.Code = &Stack.Node->Script(wEndOffset + 1);
			break;
		}
		POST_ITERATOR;
	}
}

FLOAT AGearSquad::GetChanceToDuckBackIntoCover( AGearAI* AI )
{
	if(AI == NULL || AI->MyGearPawn == NULL || AI->MyGearPawn->Weapon == NULL)
	{
		return 0.f;
	}

	// Can't duck back into cover if Aggressive or too recently popped out or trying to throw a grenade
	if( AI->CombatMood == AICM_Aggressive ||
		(GWorld->GetTimeSeconds() - AI->MyGearPawn->LastCoverActionTime) < 0.5f ||
		AI->MyGearPawn->Weapon->IsA( AGearWeap_GrenadeBase::StaticClass() )  ||
		AI->MyGearPawn->Weapon->IsInState( FName(TEXT("Charge")) )
	  )
	{
		return -1.f;
	}

	FLOAT EnemyFriendlyRatio = 1.f - (FLOAT(EnemyList.Num()) / FLOAT(SquadMembers.Num()));
	return AI->ChanceToCoverUnderFire * EnemyFriendlyRatio;
}

inline void AddCoverCostToMap(TMap<ACoverSlotMarker*, INT>& DecayedCoverMap, ACoverSlotMarker* CoverMarker, INT AddedCost)
{
	INT* CurrentValue = DecayedCoverMap.Find(CoverMarker);
	if (CurrentValue != NULL)
	{
		*CurrentValue += AddedCost;
	}
	else
	{
		DecayedCoverMap.Set(CoverMarker, AddedCost);
	}
}

void AGearSquad::UpdateDecayedCoverMap()
{
	DecayedCoverMap.Reset();

	for (INT i = 0; i < DecayedCoverList.Num(); i++)
	{
		if (DecayedCoverList(i).CoverMarker == NULL || DecayedCoverList(i).CoverMarker->ActorIsPendingKill())
		{
			DecayedCoverList.Remove(i--);
		}
		else
		{
			AddCoverCostToMap(DecayedCoverMap, DecayedCoverList(i).CoverMarker, DecayedCoverList(i).ExtraCoverCost);
			INT AdjacentCost = appTrunc(DecayedCoverList(i).ExtraCoverCost * AdjacentDecayMult);
			if (AdjacentCost > 0)
			{
				for (INT j = 0; j < DecayedCoverList(i).CoverMarker->PathList.Num(); j++)
				{
					if (DecayedCoverList(i).CoverMarker->PathList(j) != NULL)
					{
						ACoverSlotMarker* CoverMarker = Cast<ACoverSlotMarker>(DecayedCoverList(i).CoverMarker->PathList(j)->End.Nav());
						if (CoverMarker != NULL)
						{
							AddCoverCostToMap(DecayedCoverMap, CoverMarker, AdjacentCost);
						}
					}
				}
			}
		}
	}
}

void AGearSquad::AddCoverDecay(ACoverSlotMarker* CoverMarker, INT Amount)
{
	for (INT i = 0; i < DecayedCoverList.Num(); i++)
	{
		if (DecayedCoverList(i).CoverMarker == CoverMarker)
		{
			DecayedCoverList(i).ExtraCoverCost += Amount;
			// remove if cost was reduced to zero
			if (DecayedCoverList(i).ExtraCoverCost <= 0)
			{
				DecayedCoverList.Remove(i);
			}
			return;
		}
	}

	// add element
	if (Amount > 0)
	{
		INT Index = DecayedCoverList.Add();
		DecayedCoverList(Index).CoverMarker = CoverMarker;
		DecayedCoverList(Index).ExtraCoverCost = Amount;
	}
}

void AGearSquad::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	DecayRecoveryIntervalRemaining -= DeltaTime;
	while (DecayRecoveryIntervalRemaining <= 0.0f)
	{
		for (INT i = 0; i < DecayedCoverList.Num(); i++)
		{
			DecayedCoverList(i).ExtraCoverCost -= DecayRecoveryPerSecond;
			if (DecayedCoverList(i).ExtraCoverCost <= 0)
			{
				DecayedCoverList.Remove(i--);
			}
		}

		UpdateDecayedCoverMap();
		DecayRecoveryIntervalRemaining += 1.0f;
	}
}

FVector AGearSquad::GetSquadCentroid()
{
	INT Count =0;
	FVector BaryCenter=FVector(0.f);
	for(INT Idx=0;Idx<SquadMembers.Num();Idx++)
	{
		if(SquadMembers(Idx).Member && SquadMembers(Idx).Member->Pawn)
		{
			BaryCenter += SquadMembers(Idx).Member->Pawn->Location;
			Count++;
		}
	}

	return BaryCenter / Count;
}

INT AGearSquadFormation::GetNumSquadMembersThatUsePositions()
{
	INT Ret = 0;
	if( Squad != NULL )
	{
		for( INT Idx = 0; Idx < Squad->SquadMembers.Num(); Idx++ )
		{
			AController* SM = Squad->SquadMembers(Idx).Member;
			if( SM != NULL )
			{
				AGearAI* AI = Cast<AGearAI>(SM);
				if( SM != Squad->Leader && (AI == NULL || !AI->bIgnoreSquadPosition) )
				{
					Ret++;
				}
			}
		}
	}
	return Ret;
}

/************************************************************************/
/* AI Reactions                                                        */
/************************************************************************/

/** ---->> UAIReactChannel **/
void UAIReactChannel::NudgeChannel(AActor* Instigator)
{
	if(bChannelSuppressed==FALSE)
	{
		UAIReactCondition_Base* pReaction = NULL;
		for(INT Idx=0; Idx<Reactions.Num(); Idx++)
		{

			pReaction = Reactions(Idx);
			if(pReaction != NULL)
			{
				pReaction->ConditionalCheckActivate(Instigator,this);
			}
		}
	}
}

void UAIReactChannel::Subscribe(UAIReactCondition_Base* Condition)
{
	check(!Reactions.ContainsItem(Condition));
	#if DO_AI_LOGGING	
		Condition->GetOuterAGearAI()->AILog(TEXT("%s Successfuly subscribed to channel %s"),*Condition->GetName(),*ChannelName.ToString());
	#endif
	Reactions.AddItem(Condition);
	Condition->SubscribedChannels.AddItem(ChannelName);
}

UBOOL UAIReactChannel::UnSubscribe(UAIReactCondition_Base* Condition)
{
	if( ! Reactions.ContainsItem(Condition))
	{
		return FALSE;
	}
	#if DO_AI_LOGGING	
		Condition->GetOuterAGearAI()->AILog(TEXT("%s Successfuly unsubscribed from channel %s"),*Condition->GetName(),*ChannelName.ToString());
	#endif
	Reactions.RemoveItem(Condition);
	Condition->SubscribedChannels.RemoveItem(ChannelName);
	return TRUE;
}

void UAIReactChannel::Init(FName NewChannelName)
{
	ChannelName = NewChannelName;
}

UAIReactCondition_Base* UAIReactChannel::SuppressReactionByType(UClass* Type)
{
	UAIReactCondition_Base* Cond = FindReactionByType(Type);	
	if(Cond != NULL)
	{
		Cond->eventSuppress();
		return Cond;
	}

	return NULL;
}

UAIReactCondition_Base* UAIReactChannel::UnSuppressReactionByType(UClass* Type)
{
	UAIReactCondition_Base* Cond = FindReactionByType(Type);	
	if(Cond != NULL)
	{
		Cond->eventUnSuppress();
		return Cond;
	}

	return NULL;
}

UAIReactCondition_Base* UAIReactChannel::FindReactionByType(UClass* Type)
{
	UAIReactCondition_Base* CurCond = NULL;
	for(INT Idx = 0; Idx < Reactions.Num(); Idx++)
	{
		CurCond = Reactions(Idx);
		if(CurCond != NULL && CurCond->GetClass() == Type)
		{
			return CurCond;
		}
	}

	return NULL;
}

void UAIReactChan_Timer::Poll(FLOAT DeltaTime)
{
	if( !bChannelSuppressed && Reactions.Num() > 0)
	{
		FLOAT WorldTime = GWorld->GetTimeSeconds();

		for(INT Idx=0;Idx < Reactions.Num(); Idx++)
		{
			UAIReactCondition_Base* Cond = Reactions(Idx);
			if(Cond && (WorldTime - Cond->TimerLastActivationTime > Cond->TimerInterval))
			{
				Cond->ConditionalCheckActivate(NULL,this);
			}
		}
	}
	
}

void UAIReactChan_Timer::Subscribe(class UAIReactCondition_Base* Condition)
{
	if(Condition != NULL)
	{
		Condition->TimerLastActivationTime = GWorld->GetTimeSeconds();
		Super::Subscribe(Condition);
	}	
}

/** ---->> UAIReactionManager **/
UClass* UAIReactionManager::GetClassForChannelName(FName Name)
{
	// check the intrinsic channel list to see if we have a class override
	for(INT Idx=0;Idx<IntrinsicChannels.Num();Idx++)
	{
		if(IntrinsicChannels(Idx).ChannelName == Name)
		{
			return IntrinsicChannels(Idx).ChannelClass;
		}
	}

	return DefaultChannelClass;
}
void UAIReactionManager::Subscribe(UAIReactCondition_Base* Condition, FName ChannelName)
{
	UAIReactChannel* FoundChan = GetChannelFor(ChannelName);
	// if we don't have any subscriptions for this channel yet, add a new channel
	if(FoundChan == NULL)
	{
		//debugf(TEXT("%s: Creating new channel for %s of class %s"),*GetOuterAGearAI()->GetName(),*ChannelName.ToString(),*GetClassForChannelName(ChannelName)->GetName());
		FoundChan = Cast<UAIReactChannel>(StaticConstructObject(GetClassForChannelName(ChannelName),this));
		InitializeChannel(ChannelName,FoundChan);
	}
	
	FoundChan->Subscribe(Condition);
}

/** returns true if successful remove **/
UBOOL UAIReactionManager::UnSubscribe(UAIReactCondition_Base* Condition, FName ChannelName)
{
	UAIReactChannel* FoundChan = GetChannelFor(ChannelName);
	check(FoundChan!=NULL);
	return FoundChan->UnSubscribe(Condition);
}

void UAIReactionManager::SuppressChannel(FName ChannelName)
{
	#if DO_AI_LOGGING
	GetOuterAGearAI()->AILog(TEXT("Suppressing channel %s.."),*ChannelName.ToString());
	#endif
	UAIReactChannel* FoundChan = GetChannelFor(ChannelName);
	if(FoundChan != NULL)
	{
		FoundChan->eventSuppress();
	}
}

void UAIReactionManager::UnSuppressChannel(FName ChannelName)
{
	#if DO_AI_LOGGING	
	GetOuterAGearAI()->AILog(TEXT("UnSuppressing channel %s.."),*ChannelName.ToString());
	#endif
	UAIReactChannel* FoundChan = GetChannelFor(ChannelName);
	if(FoundChan != NULL)
	{
		FoundChan->eventUnSuppress();
	}
}
void UAIReactionManager::Tick(FLOAT DeltaTime)
{
	// update the channels that need to be polled every frame
	for(INT Idx=0;Idx<PollChannels.Num();Idx++)
	{
		PollChannels(Idx)->Poll(DeltaTime);
	}
}

FName GetNameForPerception(BYTE InType)
{
	switch (InType)
	{
	case PT_SightPlayer:
		return FName(TEXT("SightPlayer"));
	case PT_Sight:
		return FName(TEXT("Sight"));
	case PT_Heard:
		return FName(TEXT("Hearing"));
	case PT_HurtBy:
	case PT_Force:
		return FName(TEXT("Force"));
	default:
		return FName(TEXT("Unknown"));
	}
}

/** Notification that a perception channel just had an impulse **/
void UAIReactionManager::NudgePerceptionChannel(AActor* Instigator, BYTE PT)
{
	if(bAllReactionsSuppressed==FALSE)
	{
		UAIReactChannel* FoundChan = NULL;
		// see if we have a cached channel
		if(PT < BasicPerceptionChannels.Num() && BasicPerceptionChannels(PT))
		{
			FoundChan = BasicPerceptionChannels(PT);
		}
		else
		{
			FoundChan = GetChannelFor(GetNameForPerception(PT));

			// cache off this chanel
			if(BasicPerceptionChannels.Num() <= PT)
			{
				BasicPerceptionChannels.AddZeroed((PT+1) - BasicPerceptionChannels.Num());
			}
			BasicPerceptionChannels.InsertItem(FoundChan,PT);
		}

		// if we have no subscribers, nobody cares
		if(FoundChan == NULL)
		{
			return;
		}

		//DEBUG
		//GLog->Logf(TEXT("AIReactionManager: Nudging channel (%s)"),*ChannelName.ToString());
		FoundChan->NudgeChannel(Instigator);
	}
}

/** Notification that a named stimulus just had an impulse **/
void UAIReactionManager::NudgeChannel(AActor* Instigator, FName ChannelName)
{
	if(bAllReactionsSuppressed==FALSE)
	{
		UAIReactChannel* FoundChan = GetChannelFor(ChannelName);
		// if we have no subscribers, nobody cares
		if(FoundChan == NULL)
		{
			return;
		}

		//DEBUG
		//GLog->Logf(TEXT("AIReactionManager: Nudging channel (%s)"),*ChannelName.ToString());
		FoundChan->NudgeChannel(Instigator);
	}
}
void UAIReactionManager::InitializeChannel( FName ChannelName, UAIReactChannel* ReactChannel )
{
	ReactChannel->Init(ChannelName);
	ChannelMap.Set(ChannelName,ReactChannel);
	if(ReactChannel->bNeedsPoll == TRUE)
	{
		PollChannels.AddItem(ReactChannel);
	}
}

void UAIReactionManager::Initialize()
{
	check(GetOuterAGearAI() != NULL);
	// loop through our default conditions and set them up
	for(INT Idx=0; Idx<GetOuterAGearAI()->DefaultReactConditions.Num();Idx++)
	{
		GetOuterAGearAI()->DefaultReactConditions(Idx)->eventInitialize();
	}

	// loop through default condition classes and set them up
	for(INT Idx=0; Idx<GetOuterAGearAI()->DefaultReactConditionClasses.Num();Idx++)
	{
		UClass* CurClass = GetOuterAGearAI()->DefaultReactConditionClasses(Idx);
		if(CurClass != NULL)
		{
			UAIReactCondition_Base* NewCondition = Cast<UAIReactCondition_Base>(StaticConstructObject(CurClass,GetOuterAGearAI()));
			NewCondition->eventInitialize();
		}
		else
		{
			debugf(TEXT("AIReactionManager WARNING! - default reaction class at index %i was NULL, could not initialize reaction."),Idx);
		}
	}
}

UAIReactChannel* UAIReactionManager::GetChannelFor(FName ChannelName)
{
	UAIReactChannel** FoundChan = ChannelMap.Find(ChannelName);
	if(FoundChan == NULL)
	{
		return NULL;
	}

	return *FoundChan;
}

void UAIReactionManager::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	if( Ar.IsObjectReferenceCollector() )
	{
		Ar << ChannelMap;
	}
}

void UAIReactionManager::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );

	for( TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Value() );
	}
}

void UAIReactionManager::Wipe()
{
	UAIReactChannel* Chan = NULL;
	UAIReactCondition_Base* Cond = NULL;
	for( TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap); It; ++It )
	{
		Chan = It.Value();
		for(INT Idx = Chan->Reactions.Num() -1 ; Idx >= 0; Idx--)
		{
			Cond = Chan->Reactions(Idx);
			if(Cond != NULL)
			{
				Chan->UnSubscribe(Cond);
			}
		}
	}

	PollChannels.Empty();
	ChannelMap.Empty();
}
/** Find the _FIRST_ ReactCondition by a given type.
 bDeepSearch indicates whether we should search the whole list of reactions if we don't find it in the auto subscribe channels **/
UAIReactCondition_Base* UAIReactionManager::FindReactionByType(UClass* Type, UBOOL bDeepSearch)
{
	// first search the channels it auto subscribes to
	UAIReactCondition_Base* DefaultObj = Cast<UAIReactCondition_Base>(Type->GetDefaultObject());
	check(DefaultObj!=NULL);
	UAIReactChannel* ReactChannel = NULL;
	UAIReactCondition_Base* RetVal = NULL;
	for(INT Idx = 0; Idx < DefaultObj->AutoSubscribeChannels.Num(); Idx++)
	{
		ReactChannel = GetChannelFor(DefaultObj->AutoSubscribeChannels(Idx));
		if(ReactChannel != NULL)
		{
			RetVal = ReactChannel->FindReactionByType(Type);
			if(RetVal != NULL)
			{
				return RetVal;
			}
		}
	}

	// if we got here it means it's not in any of its autosubscribe channels, but it could have been subscribed into other channels later
	if(bDeepSearch == TRUE)
	{
		// search EVERYTHING!
		for(TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap);It;++It)
		{
			ReactChannel = It.Value();
			if(ReactChannel != NULL)
			{
				RetVal = ReactChannel->FindReactionByType(Type);
				if(RetVal != NULL)
				{
					return RetVal;
				}
			}
		}
	}

	return NULL;
}

/** calls the passed function on every reaction of the passed type **/
void UAIReactionManager::ForEachReactionOfType(UClass* Type, ForEachFunction CallThisFunction)
{
	UAIReactChannel* ReactChannel = NULL;

	UAIReactCondition_Base* ReactCond = NULL;
	for(TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap);It;++It)
	{
		ReactChannel = It.Value();
		if(ReactChannel != NULL)
		{
			for(INT Idx=0; Idx < ReactChannel->Reactions.Num(); Idx++)
			{
				ReactCond = ReactChannel->Reactions(Idx);
				if(ReactCond != NULL && ReactCond->GetClass() == Type)
				{
					((*ReactCond).*CallThisFunction)();
				}
			}
		}
	}
}

/** disables (but does not remove) react condition of the passed type, returns the reaction that was suppressed if one is found, none otherwise **/
void UAIReactionManager::SuppressReactionsByType(UClass* Type, UBOOL bDeepSearch)
{
	ForEachReactionOfType(Type,&UAIReactCondition_Base::eventSuppress);
}

/** disables the given reaction **/
void UAIReactionManager::SuppressReaction(UAIReactCondition_Base* ReactCondition)
{
	if(ReactCondition != NULL)
	{
		ReactCondition->eventSuppress();
	}	
}

/** re-enables a ReactCondition of the given type, returns FALSE if one was not found **/
void UAIReactionManager::UnSuppressReactionsByType(UClass* Type, UBOOL bDeepSearch)
{
	ForEachReactionOfType(Type,&UAIReactCondition_Base::eventUnSuppress);
}

/** Re-Enables a given reactcondition **/
void UAIReactionManager::UnSuppressReaction(UAIReactCondition_Base* ReactCondition)
{
	if(ReactCondition != NULL)
	{
		ReactCondition->eventUnSuppress();
	}
}

void UAIReactionManager::SuppressAllChannels()
{
	#if DO_AI_LOGGING
	GetOuterAGearAI()->AILog(TEXT("Suppressing all channels individually.."));
	#endif
	for( TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap); It; ++It )
	{
		It.Value()->eventSuppress();
	}
}

void UAIReactionManager::UnSuppressAllChannels()
{
	#if DO_AI_LOGGING
	GetOuterAGearAI()->AILog(TEXT("UnSuppressing all channels individually.."));
	#endif
	for( TMap<FName,UAIReactChannel*>::TIterator It(ChannelMap); It; ++It )
	{
		It.Value()->eventUnSuppress();
	}
}

/** ---->> UAIReactCondition **/

/** automatically subscribes this condition to all the default channels it should listen for **/
void UAIReactCondition_Base::SubscribeToMyChannels()
{
	check(GetOuterAGearAI() != NULL);
	check(GetOuterAGearAI()->ReactionManager != NULL);

	UAIReactionManager* pAIReactionMgr = GetOuterAGearAI()->ReactionManager;
	// loop through our list and subscribe ourselves to them all
	for(INT Idx = 0; Idx<AutoSubscribeChannels.Num();Idx++)
	{
		pAIReactionMgr->Subscribe(this,AutoSubscribeChannels(Idx));
	}
}

/** unsubscribe from all our current channels **/
void UAIReactCondition_Base::UnsubscribeAll()
{
	check(GetOuterAGearAI() != NULL);
	check(GetOuterAGearAI()->ReactionManager != NULL);

	UAIReactionManager* AIReactionMgr = GetOuterAGearAI()->ReactionManager;
	// counts backwards because UnSubScribe removes entries from SubscribedChannels
	for(INT Idx=SubscribedChannels.Num()-1; Idx>=0; Idx--)
	{
		AIReactionMgr->UnSubscribe(this,SubscribedChannels(Idx));		
	}
}
UBOOL UAIReactCondition_Base::ShouldActivateNative( AActor* Instigator, UAIReactChannel* OriginatingChannel)
{
	if(GetOuterAGearAI() == NULL || (!bAlwaysNotify && GetOuterAGearAI()->IgnoreNotifies()))
	{
		return FALSE;
	}

	AGearPawn* MyGearPawn = GetOuterAGearAI()->MyGearPawn;
	if(	GetOuterAGearAI()->Pawn == NULL ||
		MyGearPawn != NULL && 
		(
		MyGearPawn->Health <= 0 ||
		MyGearPawn->IsDBNO() ||
		(!bActivateWhenBasedOnInterpActor && Cast<AInterpActor>(MyGearPawn->GetBase()) != NULL && !MyGearPawn->GetBase()->Velocity.IsZero() ) ||	// Don't allow reactions on interp actor since we could fall off
		(!bActivateWhenBasedOnPawn && Cast<APawn>(MyGearPawn->GetBase()) != NULL) )			// Don't allow reactions on vehicle pawn since we could fall off
		)
	{
		return FALSE;
	}
	return TRUE;
}

void UAIReactCondition_Base::ConditionalCheckActivate( AActor* Instigator, UAIReactChannel* OriginatingChannel )
{
	if(bSuppressed == FALSE && ShouldActivateNative(Instigator,OriginatingChannel))
	{
		if( eventShouldActivate(Instigator,OriginatingChannel) == TRUE)
		{
			eventActivate(Instigator,OriginatingChannel);
		}
	}
}


/** ---->> UAIReactCond_EnemyMoved **/
#define MAX_MAP_SIZE 30
UBOOL UAIReactCond_EnemyMoved::IsEnemyInNewLocation( APawn* Enemy )
{
	FVector Vect;

	INT Idx = FindRec(Enemy,Vect);
	if( Idx < 0)
	{
		AddNewRec(Enemy);
		return TRUE;
	}

	if((Vect-Enemy->Location).SizeSquared() > (DistanceThreshold * DistanceThreshold))
	{
		EnemyThreshList(Idx).Position = Enemy->Location;
		return TRUE;
	}

	return FALSE;
}

void UAIReactCond_EnemyMoved::AddNewRec(APawn* Pawn)
{
	FLastThreshPosPair NewPair;
	NewPair.Enemy = Pawn;
	NewPair.Position = Pawn->Location;
	
	if(RingBufIndex < EnemyThreshList.Num())
	{
		EnemyThreshList(RingBufIndex) = NewPair;
	}
	else
	{
		EnemyThreshList.AddItem(NewPair);
	}

	// increment index
	if(++RingBufIndex >= UCONST_ThreshPosBufferSize)
	{
		RingBufIndex = 0;
	}
}

INT UAIReactCond_EnemyMoved::FindRec(APawn* Pawn, FVector& outLoc)
{
	for(INT Idx=0; Idx < EnemyThreshList.Num(); Idx++)
	{
		if(EnemyThreshList(Idx).Enemy == Pawn)
		{
			outLoc = EnemyThreshList(Idx).Position;
			return Idx;
		}
	}

	return -1;
}

/** ---->>> UAIReactCond_Conduit_Base **/
UBOOL UAIReactCond_Conduit_Base::ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel)
{
	if(Super::ShouldActivateNative(Instigator,OriginatingChannel) == FALSE)
	{
		return FALSE;
	}

	if(MinTimeBetweenActivations > 0.f && LastActivationTime > 0.f && (GWorld->GetTimeSeconds() - LastActivationTime) < MinTimeBetweenActivations)
	{
		return FALSE;
	}
	return TRUE;
}

/** ---->>> UAIReactCond_NewEnemy **/
UBOOL UAIReactCond_NewEnemy::ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel)
{
	FLOAT LastSeenTime = 0.f;

	if(Super::ShouldActivateNative(Instigator,OriginatingChannel)==FALSE)
	{
		return FALSE;
	}

	AGearSquad* Squad = GetOuterAGearAI()->Squad;
	if( Squad != NULL )
	{
		LastSeenTime = Squad->GetEnemyLastSeenTime(Cast<APawn>(Instigator));
	}

	if( (LastSeenTime==0 || (GWorld->GetTimeSeconds() - LastSeenTime) > TimeSinceSeenThresholdSeconds) )
	{
		return TRUE;
	}

	return FALSE;
}

/** ---->>> UAIReactCond_SurpriseEnemyLoc **/
/** Activates if we have not seen the incoming dude in a long time (or haven't ever seen) **/
UBOOL UAIReactCond_SurpriseEnemyLoc::ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel)
{
	if (Instigator == NULL || !Super::ShouldActivateNative(Instigator,OriginatingChannel))
	{
		return FALSE;
	}

	APawn* P = Cast<APawn>(Instigator);

	AGearAI* MyGearAI = GetOuterAGearAI();
	if(!MyGearAI || !MyGearAI->Pawn)
	{
		return FALSE;
	}
	FVector VecToEnemy = (Instigator->Location - MyGearAI->Pawn->Location).SafeNormal();
	FVector VecToKnown = (MyGearAI->GetEnemyLocation(P) - MyGearAI->Pawn->Location).SafeNormal();
	FLOAT DotP	   = VecToEnemy | VecToKnown;

	if( P != NULL )
	{
		if( MyGearAI->PerceptionMood == AIPM_Unaware	 ||
			MyGearAI->PerceptionMood == AIPM_Oblivious ||
			DotP < 0.6f )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** ---->>> UAIReactCond_Targeted **/
UBOOL UAIReactCond_Targeted::ShouldActivateNative(AActor* Instigator, UAIReactChannel* OriginatingChannel)
{
	AGearAI* MyGearAI = GetOuterAGearAI();
	if(MyGearAI == NULL)
	{
		return FALSE;
	}
	AGearPawn* MyGearPawn = MyGearAI->MyGearPawn;

	if( MyGearPawn == NULL || !Super::ShouldActivateNative( Instigator, OriginatingChannel ) )
	{
		return FALSE;
	}

	AGearSquad* Squad = MyGearAI->Squad;

	// Don't respond if can't evade or if based on another pawn (riding something)
	FLOAT ChanceToRespond = (MyGearPawn->IsInCover() && Squad != NULL) ? Squad->GetChanceToDuckBackIntoCover( MyGearAI ) : MyGearAI->ChanceToEvade;
	FLOAT ChanceToRespondScale = 1.f;

	// If targetted by heavy weapon, increase chance of evading
	APawn* EnemyPawn = Instigator->GetAPawn();
	if( EnemyPawn != NULL )
	{
		AGearWeapon* EnemyWeap = Cast<AGearWeapon>(EnemyPawn->Weapon);
		if( EnemyWeap != NULL && EnemyWeap->WeaponType == WT_Heavy )
		{
			ChanceToRespondScale = 1.1f;
		}
	}

	if( !MyGearAI->CanEvade( TRUE, ChanceToRespond, ChanceToRespondScale ) )		
	{
		return FALSE;
	}

	if( Squad != NULL )
	{
		// For each enemy I know about
		for( INT Idx = 0; Idx < Squad->EnemyList.Num(); Idx++ )
		{
			APawn *P = Squad->EnemyList(Idx).Pawn;
			if(P == NULL)
			{
				continue;
			}

			// If enemy is in front of me
			FLOAT DotP = (MyGearPawn->Rotation.Vector()) | (P->Location - MyGearPawn->Location).SafeNormal();
			if( DotP > 0.7 )
			{
				// If enemy is aiming at me
				AGearPC* PC = Cast<AGearPC>(P->Controller);
				if( PC != NULL && PC->bIsTargeting )
				{
					FVector FireStart = P->eventGetWeaponStartTraceLocation();
					FVector VectToTarget = MyGearPawn->Location - FireStart;
					FVector CamLoc(0.0f);
					FRotator CamRot(0,0,0);
					PC->eventGetPlayerViewPoint( CamLoc, CamRot );

					FVector CamRotDir = CamRot.Vector();
					FVector ClosestPt = ((VectToTarget | CamRotDir) * CamRotDir) + FireStart;
					if( (ClosestPt - FireStart).SizeSquared() < ShooterRangeThreshold * ShooterRangeThreshold &&			// 1024^2
						(ClosestPt - MyGearPawn->Location).SizeSquared() < 4096.f )	// 64^2
					{
						shooter = P;
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

/************************************************************************
  AIVisibilitymanager
 ************************************************************************/
void UAIVisibilityManager::Init()
{
	// set up the initial record first
	FreeLineCheckResList = new LineCheckResult();
	LineCheckResult* TempRes = FreeLineCheckResList;
	for(INT Idx=0; Idx < UCONST_PoolSize-1; Idx++)
	{
		FreeLineCheckResList = new LineCheckResult();
		FreeLineCheckResList->Next = TempRes;
		TempRes = FreeLineCheckResList;
	}

}

UBOOL UAIVisibilityManager::RequestSightCheck(AGearAI* InTestingAI, APawn* PawnToTest, LineCheckResult::ShouldLineCheckCb ShouldCheckCb, LineCheckResult::FinishedCallback FinishedCb)
{
	check(InTestingAI != NULL);
	check(PawnToTest != NULL);

	// of we have no free results then bail
	if(FreeLineCheckResList == NULL) 
	{
		return FALSE;
	}

	// pull first element from free list
	LineCheckResult* LineCheckResToUse = FreeLineCheckResList;	
	FreeLineCheckResList = LineCheckResToUse->Next;

	// put it on the back of the pending list
	LineCheckResToUse->Next = NULL;
	// if the list is empty, gotta set the head as well
	if(PendingLineCheckHead == NULL)
	{
		PendingLineCheckHead = LineCheckResToUse;
	}

	if(PendingLineCheckTail != NULL)
	{
		PendingLineCheckTail->Next = LineCheckResToUse;
	}	
	PendingLineCheckTail = LineCheckResToUse;

	// set up the result
#if !FINAL_RELEASE
	check(InTestingAI != NULL);
	check(PawnToTest != NULL);
	check(ShouldCheckCb != NULL);
	check(FinishedCb != NULL);
#endif
	LineCheckResToUse->InitLineCheck(InTestingAI,PawnToTest,ShouldCheckCb,FinishedCb);

	return TRUE;
}

void UAIVisibilityManager::Tick(FLOAT DeltaTime)
{
	// iterate through all controllers and queue requests from them
	QueueUpVisRequests();
	
	// push however many ray casts we're going to push this frame off to physics
	DoPhysRequestsForPendingLineChecks();

	// finish up any requests that physics is done with
	UpdateBusyLineChecks();
}

/** return val of FALSE means we couldn't complete all the requests for this controller, so start here next frame **/
UBOOL UAIVisibilityManager::SuccessfullyShowedController(AController* Controller)
{
	// if something is wrong, skip it
	if ( Controller == NULL || Controller->Pawn == NULL)
	{
		return TRUE;
	}	

	// queue up line checks where appropriate
	for ( AController *C=GWorld->GetFirstController(); C!=NULL; C=C->NextController )
	{
		if( C!=Controller && C->ShouldCheckVisibilityOf(Controller) )
		{
			AGearAI* GAI = Cast<AGearAI>(C);
			if(GAI == NULL)
			{
				continue;
			}

			FVector Dummy;
			// call ShouldCheckToSeeEnemy for a quick early out
			if(GAI->ShouldCheckToSeeEnemy(Controller->Pawn,Dummy,Dummy)==TRUE)
			{				 
				 // queue up a sight check
				 if(RequestSightCheck(GAI,Controller->Pawn,&AGearAI::ShouldCheckToSeeEnemy,&AGearAI::ProcessSightCheckResult)==FALSE)
				 {
					 // if the sight check failed to get queued, notify the calling function so we start here next frame
					 return FALSE;
				 }
			}		
		}
	}

	return TRUE;
}
/**
* (replaces ShowSelf() iterates over all controllers and queues up line check requests when necessary
*  This replaces ShowSelf so that we can rotate through the controller list round-robin style and make sure everyone gets
*  a sight result 
*/
void UAIVisibilityManager::QueueUpVisRequests()
{

	// we need to jump to the place we stopped last frame (Denoted by ControllerIttStartPoint) and iterate from there
	
	//   first find the controller that matches that position
	AController* FirstUpdated = NULL;
	INT LoopCounter = 0;
	
	// ** Iterate from start to finish, calling ShowController once we've reached the start point
	for ( AController* C=GWorld->GetFirstController(); C!=NULL; C=C->NextController,LoopCounter++ )
	{
		// if this is where we should end the other loop, mark it
		if(LoopCounter == ControllerIttStartPoint)
		{
			FirstUpdated = C;
		}

		// if we've passed the starting point, queue them up!
		if(LoopCounter >= ControllerIttStartPoint)
		{
						
			if(SuccessfullyShowedController(C)==FALSE)
			{
				// if ShowController returned false that means we ran out of linechecks.. so we should start here next frame!
				ControllerIttStartPoint = LoopCounter;
				return;
			}
		}
	}

	LoopCounter = 0;

	// ** Iterate from the starting controller to the first one we called ShowController on in the earlier loop 
	for ( AController *C=GWorld->GetFirstController(); C!=NULL && C != FirstUpdated; C=C->NextController, LoopCounter++ )
	{		
		if(SuccessfullyShowedController(C)==FALSE)
		{
			// if ShowController returned false that means we ran out of linechecks.. so we should start here next frame!
			ControllerIttStartPoint = LoopCounter;
			return;
		}
	}

}

void UAIVisibilityManager::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects(ObjectArray);

	for (LineCheckResult* Check = PendingLineCheckHead; Check != NULL; Check = Check->Next)
	{
		AddReferencedObject(ObjectArray, Check->TestingAI);
		AddReferencedObject(ObjectArray, Check->PawnToTestAgainst);
	}
}

/**
* initiates requests to physics for pending line checks until we hit the max per frame
*/
void UAIVisibilityManager::DoPhysRequestsForPendingLineChecks()
{
	LineCheckResult* CurrentPendingCheck = PendingLineCheckHead;
	LineCheckResult* Next = NULL;
	
	INT NumRequestedThisFrame = 0;
	while(CurrentPendingCheck != NULL && NumRequestedThisFrame++ <= UCONST_MaxLineChecksPerFrame)
	{
		Next = CurrentPendingCheck->Next;

		// call to physics and start the ball rolling
		UBOOL bPhysReqQueued = CurrentPendingCheck->TriggerLineCheck();

		// pull it off 'pending' and put it on 'busy'
		//	mind the tail
		if(CurrentPendingCheck == PendingLineCheckTail)
		{
			// then we're at the end of the list, and we should set the tail to NULL!
			PendingLineCheckTail = NULL;
		}

		//	if TriggerLineCheck returned TRUE that means a physics request was queued, so add it to busy list
		if(bPhysReqQueued)	
		{
			// we know prev was null cuz we're clipping everything off the front
			AddBackToPool(CurrentPendingCheck,NULL,BusyLineCheckResList,PendingLineCheckHead);
		}
		else // otherwise no physics request was needed, so back to free list
		{
			AddBackToPool(CurrentPendingCheck,NULL,FreeLineCheckResList,PendingLineCheckHead);
		}
		
		// go to next
		CurrentPendingCheck = Next;
	}
}

/*
void UAIVisibilityManager::LineCheckFinished(LineCheckResult* Res)
{
	if(bDrawVisTests == TRUE)
	{
		INT R,G,B;
		R= (Res->bHit) ? 0 : 255;
		G= (Res->bHit) ? 255 : 0;
		B=0;
		GWorld->LineBatcher->DrawLine(Res->CheckStart,Res->CheckEnd,FColor(R,G,B),SDPG_World);
	}

	// notify the client
	Res->TestFinished();

	// move the result back to the free list
	AddBackToPool(CurrentLineCheckRes,PrevLineCheckRes,FreeLineCheckResList,BusyLineCheckResList);
}*/

/** 
* iterates over busy line check results checking to see if they've gotten a result back yet, if so move them back to free list
* TODO: change this to use FAsyncLineCheckResult callbacks so we don't have to iterate over them all
*/
void UAIVisibilityManager::UpdateBusyLineChecks()
{
	//iterate through the results we're waiting to finish and check them
	LineCheckResult* CurrentLineCheckRes = BusyLineCheckResList;
	LineCheckResult* PrevLineCheckRes = NULL;
	while(CurrentLineCheckRes != NULL)
	{
		if(CurrentLineCheckRes->IsReady() == TRUE)
		{
			if(bDrawVisTests == TRUE)
			{
				INT R,G,B;
				R= (CurrentLineCheckRes->bHit) ? 0 : 255;
				G= (CurrentLineCheckRes->bHit) ? 255 : 0;
				B=0;
				GWorld->LineBatcher->DrawLine(CurrentLineCheckRes->CheckStart,CurrentLineCheckRes->CheckEnd,FColor(R,G,B),SDPG_World);
			}

			// notify the client
			CurrentLineCheckRes->TestFinished();

			LineCheckResult* Next = CurrentLineCheckRes->Next;
			// move the result back to the free list
			AddBackToPool(CurrentLineCheckRes,PrevLineCheckRes,FreeLineCheckResList,BusyLineCheckResList);

			//advance from the prev pointer since we clipped one off
			CurrentLineCheckRes = Next;
		}
		else // it wasn't ready so advance both
		{
			PrevLineCheckRes = CurrentLineCheckRes;
			CurrentLineCheckRes = CurrentLineCheckRes->Next;
		}

	}
}

void UAIVisibilityManager::Flush()
{
	if(HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}
	
	// try to clean up any busy line check results that physics is done with
	LineCheckResult* LineCheckRes = BusyLineCheckResList;
	INT iBusyCount = 0;
	LineCheckResult* NextLineCheckRes = NULL;
	while(LineCheckRes != NULL)
	{
		NextLineCheckRes = LineCheckRes->Next;
		if(LineCheckRes->IsReady())
		{
			delete LineCheckRes;
		}
		else
		{
			iBusyCount++;
		}
		LineCheckRes = NextLineCheckRes;
	}
	BusyLineCheckResList = NULL;

	// if there are still line check results which physics is still processing we can't safely delete them!  So spit out a warning and leak them.
#if !FINAL_RELEASE	
	if(iBusyCount > 0)
	{
		// assert if this happens anywhere outside of exitpurge
		check(GExitPurge);
		debugf(TEXT("WARNING! GExitPurge:%i AIVisibilityManager was not able to delete %i LineCheckResults because they are still being referenced by Novodex! (in other words we just leaked %i bytes.)"),GExitPurge,iBusyCount,sizeof(LineCheckResult)*iBusyCount);
	}
#endif


	LineCheckRes = FreeLineCheckResList;
	LineCheckResult* NextCheckRes = NULL;
	if(FreeLineCheckResList != NULL)
	{
		NextCheckRes = FreeLineCheckResList->Next;
	}

	for(;LineCheckRes != NULL; LineCheckRes = NextCheckRes)
	{
		NextCheckRes = LineCheckRes->Next;
		delete LineCheckRes;	
	}
	FreeLineCheckResList = NULL;

	// wipe the pending list
	LineCheckRes = PendingLineCheckHead;
	if(PendingLineCheckHead != NULL)
	{
		NextCheckRes = PendingLineCheckHead->Next;
	}

	for(;LineCheckRes != NULL; LineCheckRes = NextCheckRes)
	{
		NextCheckRes = LineCheckRes->Next;
		delete LineCheckRes;	
	}
	PendingLineCheckHead = NULL;
	PendingLineCheckTail = NULL;

}

void UAIVisibilityManager::NotifyPawnDestroy(APawn* Pawn)
{
	// iterate over busy list and mark anything matching as stale
	LineCheckResult* CurrentRes = BusyLineCheckResList;
	for(;CurrentRes != NULL;CurrentRes = CurrentRes->Next)
	{
		// if this line check result is waiting on the passed pawn, mark it as stale
		if (!CurrentRes->bStale && (CurrentRes->TestingAI->Pawn == Pawn || CurrentRes->PawnToTestAgainst == Pawn))
		{
			CurrentRes->bStale = TRUE;
		}		
	}

	// now check the pending list, here just remove it from the list
	CurrentRes = PendingLineCheckHead;
	LineCheckResult* PrevRes = NULL;	
	while(CurrentRes != NULL)
	{
		// if this line check result is waiting on the passed pawn, remove it from the pending list
		if(CurrentRes->TestingAI->Pawn == Pawn || CurrentRes->PawnToTestAgainst == Pawn)
		{
			// watch the tail! .. many welps!
			if(CurrentRes == PendingLineCheckTail)
			{
				PendingLineCheckTail = PrevRes;
			}
			LineCheckResult* Next = CurrentRes->Next;
			AddBackToPool(CurrentRes,PrevRes,FreeLineCheckResList,PendingLineCheckHead);
			CurrentRes = Next;
		}
		else
		{
			PrevRes = CurrentRes;
			CurrentRes = CurrentRes->Next;
		}

	}
}
/** 
	Removes a line check from its current list and adds it to 'DestPool'
	@param LineCheckResToMove - the line check to move 
	@param PrevLineCheckRes - the line check result in the list just before the one we're moving
	@param DestPool - pool to add the line check back to
*/	 
void UAIVisibilityManager::AddBackToPool(LineCheckResult* LineCheckResToMove, LineCheckResult* PrevLineCheckRes, LineCheckResult*& DestPool, LineCheckResult*& SrcPool)
{
	// remove from src
	if(PrevLineCheckRes != NULL)
	{
		PrevLineCheckRes->Next = LineCheckResToMove->Next;
	}
	else
	{
		SrcPool = LineCheckResToMove->Next;
	}

	// add to dest
	LineCheckResToMove->Next = DestPool;
	DestPool = LineCheckResToMove;
}

/** 
 * calls ShouldCheckCallback to determine endpoints for linecheck, and see if we should early out
 * @return returns TRUE if a sight check was queued
 */
UBOOL UAIVisibilityManager::LineCheckResult::TriggerLineCheck()
{
	FVector Start,End;

	UBOOL bValidData = 	(TestingAI != NULL && !TestingAI->bDeleteMe && !TestingAI->HasAnyFlags(RF_PendingKill) &&
						PawnToTestAgainst != NULL && !PawnToTestAgainst->bDeleteMe && !PawnToTestAgainst->HasAnyFlags(RF_PendingKill));

	if( ! bValidData )
	{
		return FALSE;
	}

	// if the data callback function thinks we should, start a sigh trace
	if(((*TestingAI).*ShouldCheckFunction)(PawnToTestAgainst,Start,End) == TRUE)
	{
		CheckStart = Start;
		CheckEnd = End;
#if UCONST_bUseAsyncLineChecksForVisibility
		{
			GWorld->RBPhysScene->AsyncLineCheck(Start,End,FALSE,this);
			return TRUE;
		}
#else
		{
			FCheckResult Hit;
			bHit = !GWorld->SingleLineCheck(Hit, TestingAI, End, Start, TRACE_World | TRACE_StopAtAnyHit | TRACE_GearAIVisibility | TRACE_ComplexCollision, FVector(0.f), NULL);
			TestFinished();
			return FALSE;
		}
#endif
	}
	else // otherwise just call back saying it's not visible
	{
		bHit = TRUE;
		TestFinished();
		return FALSE;
	}

}

void UAIVisibilityManager::Reset()
{
	// iterate through all pending checks and put them back on the free list
	LineCheckResult* CurrentPendingCheck = PendingLineCheckHead;
	LineCheckResult* Next = NULL;

	while(CurrentPendingCheck != NULL)
	{
		Next = CurrentPendingCheck->Next;

		// pull it off 'pending' and put it on 'Free'
		//	mind the tail
		if(CurrentPendingCheck == PendingLineCheckTail)
		{
			// then we're at the end of the list, and we should set the tail to NULL!
			PendingLineCheckTail = NULL;
		}

		AddBackToPool(CurrentPendingCheck,NULL,FreeLineCheckResList,PendingLineCheckHead);

		// go to next
		CurrentPendingCheck = Next;
	}

	// iterate through all busy checks and mark them stale
	CurrentPendingCheck = BusyLineCheckResList;
	while(CurrentPendingCheck != NULL)
	{
		CurrentPendingCheck->bStale = TRUE;
		CurrentPendingCheck = CurrentPendingCheck->Next;
	}
	ControllerIttStartPoint = 0;
}


void AGearAI_SecurityNemacyst::GetLookDir(FVector& LookDir)
{
	LookDir	= Pawn->Rotation.Vector();	

	AGearPawn *P = Cast<AGearPawn>(Pawn);
	if( P )
	{
		// If Pawn has a sight bone - use it's orientation
		if( P->Mesh && P->SightBoneName != NAME_None )
		{
			USkeletalMeshSocket* Socket = P->Mesh->SkeletalMesh->FindSocket(P->SightBoneName);
			FMatrix SocketMatrix;
			if( Socket != NULL && Socket->GetSocketMatrix( SocketMatrix, P->Mesh ) )
			{
				LookDir = SocketMatrix.GetAxis( 0 );
				return;
			}
		}
	}

	Super::GetLookDir(LookDir);

}

AVolume_Nemacyst* AGearAI_Nemacyst::GetActiveNemacystVolume()
{
	if(CurrentVolume != NULL)
	{
		return CurrentVolume;
	}

	if(Pawn == NULL)
	{
		return NULL;
	}

	AVolume_Nemacyst* Vol = NULL;
	for(INT Idx=0;Idx<Pawn->Touching.Num();Idx++)
	{
		Vol = Cast<AVolume_Nemacyst>(Pawn->Touching(Idx));
		if(Vol != NULL)
		{
			CurrentVolume = Vol;
			return Vol;
		}
	}

	return NULL;
}

#define IDEAL_MOVE_DIST InMoveDist
UBOOL AGearAI_Nemacyst::SuggestNewMovePoint(FVector& out_NewMovePt, FVector TryThisDirFirst, FLOAT InMoveDist)
{
	AVolume_Nemacyst* Vol = GetActiveNemacystVolume();

	if(Vol == NULL)
	{
		debugf(TEXT("WARNING! SuggestNewMovePoint - %s wandering, but is not assigned to any volumes!  Choosing random direction"),*GetName());
		out_NewMovePt = IDEAL_MOVE_DIST * VRand();
		return TRUE;
	}


	TryThisDirFirst = TryThisDirFirst.SafeNormal2D();
	// first try and go striaght ahead
	FLOAT MoveDist = IDEAL_MOVE_DIST;
	FVector MoveDir = TryThisDirFirst;
	FVector TryPoint = Pawn->Location + MoveDir * MoveDist;
	UBOOL bFoundPoint = FALSE;
	
	// first try shortening the distance
	while(MoveDist > 32.f)
	{
		if(Vol->FormationContainsPoint(TryPoint))
		{
			bFoundPoint = TRUE;
			break;
		}
		MoveDist -= 250.f;
		TryPoint = Pawn->Location + MoveDir * MoveDist;
	}

	// now try rotating to find a good point
	
	INT YawRotAdd = 0;
	FRotator CurRot = TryThisDirFirst.Rotation();
	CurRot.Pitch = 0;
	CurRot.Roll = 0;

	while(!bFoundPoint && YawRotAdd < 65535)
	{
		if(Vol->FormationContainsPoint(TryPoint))
		{
			bFoundPoint = TRUE;
			break;
		}
		CurRot.Yaw += YawRotAdd;
		MoveDir = CurRot.Vector();
		YawRotAdd += 1800;
		
		MoveDist = IDEAL_MOVE_DIST;
		TryPoint = Pawn->Location + MoveDir * MoveDist;
		while(MoveDist > 32.f)
		{
			if(Vol->FormationContainsPoint(TryPoint))
			{
				bFoundPoint = TRUE;
				break;
			}
			MoveDist -= 250.f;
			TryPoint = Pawn->Location + MoveDir * MoveDist;
		}
	}


	if(!bFoundPoint)
	{
		debugf(TEXT("WARNING!!! SuggestNewMovePoint could not find a valid point for %s"),*GetName());
		return FALSE;
	}

	out_NewMovePt = TryPoint;
	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
// UAICmd_MoveToGoal
//////////////////////////////////////////////////////////////////////////
void UAICmd_MoveToGoal::TickCommand(FLOAT DeltaTime)
{
	Super::TickCommand(DeltaTime);
	// don't do this when we have a child command running
	if(ChildCommand == NULL && bEnableSkipAheadChecks)
	{
		UpdateSubGoal(DeltaTime);
	}	
}
void UAICmd_MoveToGoal::TestFinished(SkipAheadCheck* Check)
{
	check(Check->TestingCommand == this);
	
	// if we're testing for a pit and didn't hit anything, FAIL!
	if(Check->bPitTest && !Check->bHit)
	{
		bSkipAheadFail=TRUE;
	}
	else
	// if we're testing horizontally for clearness and we hit something, FAIL
	if(!Check->bPitTest && Check->bHit)
	{
		bSkipAheadFail=TRUE;
	}

	//if(bSkipAheadFail)
	//{
	//	GetOuterAGearAI()->DrawDebugLine(Check->TestingForNav->Location+FVector(3.f),GetOuterAGearAI()->Pawn->Location+FVector(3.f),255,0,0,TRUE);
	//}
	//else
	//{
	//	GetOuterAGearAI()->AILog(NAME_AICommand,TEXT(">>>>>>>>>>>>>>>>>>>>>>PASS! CheckNav: %s bPitTest: %i"),*Check->TestingForNav->GetName(),Check->bPitTest);
	//	if(!Check->bPitTest)
	//	{
	//		GetOuterAGearAI()->DrawDebugLine(Check->TestingForNav->Location,GetOuterAGearAI()->Pawn->Location,0,255,0,TRUE);
	//	}
	//	
	//}

	SkipAheadNumActiveTests--;
	delete Check;
}


UBOOL UAICmd_MoveToGoal::IsClearOfAvoidanceZones(APawn* Pawn, ANavigationPoint* PotentialNodeToSkipTo)
{
	FVector Start = Pawn->Location;
	FVector End = PotentialNodeToSkipTo->Location;
	FVector PawnExtent = Pawn->GetDefaultCollisionSize();
	PawnExtent.Z /= 2.f;

	FCheckResult Hit(1.f);

	// check to see if we'd run into avoidance zones
	TDoubleLinkedList<UAIAvoidanceCylinderComponent*>& AvoidanceCylinders = UAIAvoidanceCylinderComponent::GetList();
	for(TDoubleLinkedList<UAIAvoidanceCylinderComponent*>::TIterator Itt(AvoidanceCylinders.GetHead());Itt;++Itt)
	{
		if(!(*Itt)->LineCheck(Hit,End,Start,PawnExtent,0))
		{
			return FALSE;
		}
	}

	return TRUE;
}

#define STEP_SIZE SkipAheadPitCheckInterval
#define DOWN_CHECK_SIZE SkipAheadPitCheckHeight
void UAICmd_MoveToGoal::QueueTests(const FVector& Start, ANavigationPoint* NodeToTest, FLOAT CollisionRadius)
{

	FVector End = NodeToTest->Location;
	// do three horizontal checks to approximate an extent check
	FVector LeftOffset = ((End-Start).SafeNormal() ^ FVector(0.f,0.f,1.f)) * CollisionRadius;

#if UCONST_bUseAsyncLineChecksForVisibility 
	// left
	SkipAheadCheck* LineCheck = new SkipAheadCheck(NodeToTest,this,FALSE,&StaticTestFinished);
	GWorld->RBPhysScene->AsyncLineCheck(Start+LeftOffset,End+LeftOffset,FALSE,LineCheck);
	//AIOwner->DrawDebugLine(Start+LeftOffset,End+LeftOffset,255,255,255,TRUE);
	SkipAheadNumActiveTests++;

	// right
	LineCheck = new SkipAheadCheck(NodeToTest,this,FALSE,&StaticTestFinished);
	GWorld->RBPhysScene->AsyncLineCheck(Start-LeftOffset,End-LeftOffset,FALSE,LineCheck);
	//AIOwner->DrawDebugLine(Start-LeftOffset,End-LeftOffset,255,255,255,TRUE);
	SkipAheadNumActiveTests++;

	// down the center
	LineCheck = new SkipAheadCheck(NodeToTest,this,FALSE,&StaticTestFinished);
	GWorld->RBPhysScene->AsyncLineCheck(Start,End,FALSE,LineCheck);
	//AIOwner->DrawDebugLine(Start,End,255,255,255,TRUE);
	SkipAheadNumActiveTests++;

	// check downward periodically
	FVector Dir = (End - Start).SafeNormal();
	FVector CurPos = Start + (Dir * STEP_SIZE * 0.5f);
	while(((End-CurPos) | Dir) > 0.f)
	{
		LineCheck = new SkipAheadCheck(NodeToTest,this,TRUE,&StaticTestFinished);
		GWorld->RBPhysScene->AsyncLineCheck(CurPos,CurPos+FVector(0.f,0.f,-DOWN_CHECK_SIZE),FALSE,LineCheck);
		SkipAheadNumActiveTests++;
		//AIOwner->DrawDebugLine(CurPos,CurPos+FVector(0.f,0.f,-DOWN_CHECK_SIZE),255,255,255,TRUE);

		CurPos += Dir * STEP_SIZE;			
	}
#else // UCONST_bUseAsyncLineChecksForVisibility

	
	FCheckResult Hit(1.0f);
	
	if(AIOwner == NULL || AIOwner->Pawn == NULL)
	{
		return;
	}

	UBOOL bDone = FALSE;
	DWORD TraceFlags = (TRACE_AllBlocking|TRACE_PhysicsVolumes) &~TRACE_Pawns;

	// Left
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* MultiHit = GWorld->MultiLineCheck
	(
		GMainThreadMemStack,
		End+LeftOffset,
		Start+LeftOffset,
		FVector(1.f),
		TraceFlags,
		AIOwner->Pawn,
		NULL
	);
	// If we hit something solid or a physics volume that causes damage
	for( FCheckResult* Hit = MultiHit; Hit; Hit = Hit->GetNext() )
	{
		APhysicsVolume* PV = Cast<APhysicsVolume>(Hit->Actor);
		if( PV == NULL || (PV->bPainCausing && PV->DamagePerSec > 0) )
		{
			bSkipAheadFail = TRUE;
			Mark.Pop();
			return;		
		}
	}

	// Right
	MultiHit = GWorld->MultiLineCheck
		(
		GMainThreadMemStack,
		End-LeftOffset,
		Start-LeftOffset,
		FVector(1.f),
		TraceFlags,
		AIOwner->Pawn,
		NULL
		);
	// If we hit something solid or a physics volume that causes damage
	for( FCheckResult* Hit = MultiHit; Hit; Hit = Hit->GetNext() )
	{
		APhysicsVolume* PV = Cast<APhysicsVolume>(Hit->Actor);
		if( PV == NULL || (PV->bPainCausing && PV->DamagePerSec > 0) )
		{
			bSkipAheadFail = TRUE;
			Mark.Pop();
			return;		
		}
	}

	// Center
	MultiHit = GWorld->MultiLineCheck
		(
		GMainThreadMemStack,
		End,
		Start,
		FVector(1.f),
		TraceFlags,
		AIOwner->Pawn,
		NULL
		);
	// If we hit something solid or a physics volume that causes damage
	for( FCheckResult* Hit = MultiHit; Hit; Hit = Hit->GetNext() )
	{
		APhysicsVolume* PV = Cast<APhysicsVolume>(Hit->Actor);
		if( PV == NULL || (PV->bPainCausing && PV->DamagePerSec > 0) )
		{
			bSkipAheadFail = TRUE;
			Mark.Pop();
			return;		
		}
	}

	Mark.Pop();

	// check downward periodically
	FVector Dir = (End - Start).SafeNormal();
	FVector CurPos = Start + (Dir * STEP_SIZE * 0.5f);
	while(((End-CurPos) | Dir) > 0.f)
	{
		if(GWorld->SingleLineCheck(Hit,AIOwner->Pawn,CurPos+FVector(0.f,0.f,-DOWN_CHECK_SIZE),CurPos,TraceFlags,FVector(1.f)))
		{
			bSkipAheadFail=TRUE;
			return;
		}
		CurPos += Dir * STEP_SIZE;			
	}
#endif// UCONST_bUseAsyncLineChecksForVisibility

}
void UAICmd_MoveToGoal::SkipToSubGoal(AGearAI* GAI, INT Index)
{
	GAI->SetBasedPosition( SkipAheadLastUpdatePos, GAI->Pawn->Location );
	SkipAheadCurrentTestingIndex=0;
	bSkipAheadFail=FALSE;
	if(Index > 0 && Index < GAI->RouteCache.Num())
	{
#if DO_AI_LOGGING && !FINAL_RELEASE && !CONSOLE
		GAI->AILog(NAME_AICommand,TEXT("Found clear path to %s (Idx:%i).. removing intermediate routecache entries and resetting move timer to -1!"),(GAI->RouteCache(Index)!=NULL) ? *GAI->RouteCache(Index)->GetName() : TEXT("NULL"),Index);
#endif
		bGoalChangedDueToSkipAhead=TRUE;
		GAI->CurrentPath = NULL; // we're off the path baby!  LIVE FREE!
		GAI->RouteCache_RemoveIndex(0, Index);
		GAI->FailMove();
	}
}

UBOOL LineBlockedByActor(const FVector& Start,const FVector& End, UReachSpec* Spec, AActor* Actor)
{
	FCheckResult Hit(1.f);
	// first do a quick check to see if the current segment is even close to the ray
	FLOAT DistFromSpec = PointDistToLine(Actor->Location,(End-Start),Start);
	if(DistFromSpec < Spec->CollisionRadius*1.25f)
	{
		if(!Actor->ActorLineCheck(Hit,End,Start,FVector(Spec->CollisionRadius,Spec->CollisionRadius,Spec->CollisionHeight*0.5f),TRACE_Pawns | TRACE_Others | TRACE_Blocking |TRACE_AllBlocking))
		{
			return TRUE;
		}		
	}

	return FALSE;
}

UBOOL IsForbiddenByRockWorm(APawn* P, UReachSpec* Spec)
{
	if(Spec == NULL || Spec->Start == NULL || Spec->End.Nav() == NULL)
	{
		return FALSE;
	}

	if(Spec->BlockedBy != NULL && Spec->BlockedBy->IsA(AGearPawn_RockWormBase::StaticClass()))
	{
		return TRUE;
	}

	AGearPawn_RockWormBase* RW = NULL;
	for(APawn* CurPawn = GWorld->GetWorldInfo()->PawnList; CurPawn != NULL; CurPawn = CurPawn->NextPawn)
	{
		RW = Cast<AGearPawn_RockWormBase>(CurPawn);
		if(RW == NULL)
		{
			continue;
		}

		if(LineBlockedByActor(P->Location,Spec->End->Location,Spec,RW))
		{
			return TRUE;
		}
		
		ARockWorm_TailSegment* CurSeg = RW->TailSegmentList;
		while(CurSeg != NULL)
		{
			if(LineBlockedByActor(P->Location,Spec->End->Location,Spec,CurSeg))
			{
				return TRUE;
			}
			CurSeg = CurSeg->NextSegment;
		}
	}

	return FALSE;
}

UBOOL IsForbiddenByCombatZone(AGearAI* AI, ANavigationPoint* Nav)
{
	if(!AI || !AI->MyGearPawn || !Nav)
	{
		return FALSE;
	}
	ACombatZone* CZ = AI->MyGearPawn->GetCombatZoneForNav(Nav);
	
	// even if there is room right now it's still not safe to skipahead through combat zones because the state may change while we're running to the combat zone.  Never skip ahead through combat zones
	// which might make you wait
	return ( ((CZ && CZ->bDelayMovesAtMaxOccupancy) || (AI->CurrentCombatZone && AI->CurrentCombatZone->bDelayMovesAtMaxOccupancy)) && CZ != AI->CurrentCombatZone);
}
void UAICmd_MoveToGoal::ClearNonSkippableWayPoints()
{
	NonSkippableWaypoints.Empty();
}

void UAICmd_MoveToGoal::AddNonSkippableWayPoint(ANavigationPoint* Point)
{
	NonSkippableWaypoints.Set(Point,TRUE);
}

void UAICmd_MoveToGoal::UpdateSubGoal(FLOAT DeltaTime)
{
	AGearAI* GAI = GetOuterAGearAI();
	if(GAI && GAI->Pawn && GAI->RouteCache.Num() > 1 && (GAI->Pawn->Location - GAI->GetBasedPosition( SkipAheadLastUpdatePos )).SizeSquared() > SkipAheadUpdateThreshold * SkipAheadUpdateThreshold)
	{
		
		// if we're waiting for results, do nothing!
		if( AreTestsPending() )
		{
			return;
		}

		
		// if the last batch of tests failed, skip to the last successful node, and finish
		if(bSkipAheadFail)
		{
			if(SkipAheadCurrentTestingIndex > 1)
			{
				SkipToSubGoal(GAI,SkipAheadCurrentTestingIndex-1);
			}
			else
			{
				SkipToSubGoal(GAI,-1);
			}
		}
		else if (SkipAheadCurrentTestingIndex + 1 >= GAI->RouteCache.Num() || GAI->RouteCache(SkipAheadCurrentTestingIndex + 1) == NULL) // if we're out of routecache, use the last one
		{
			SkipToSubGoal(GAI,SkipAheadCurrentTestingIndex);
			
		}
		else // otherwise the last batch passed, increment the counter and queue up the next set
		{
			SkipAheadCurrentTestingIndex++;

			UReachSpec* Spec = GAI->RouteCache(SkipAheadCurrentTestingIndex-1)->GetReachSpecTo(GAI->RouteCache(SkipAheadCurrentTestingIndex));

			// see if this goal is on the 'don't skip' list
			ANavigationPoint* LastPt=GAI->RouteCache(SkipAheadCurrentTestingIndex-1);
			if(LastPt != NULL && NonSkippableWaypoints.Find(LastPt) != NULL)
			{
				bSkipAheadFail=TRUE;
				
			}
			else			
			// if the spec forbids skipahead, or there is an avoidance zone in the way, stop here
			if( (Spec != NULL && !Spec->CanBeSkipped(GAI->Pawn)) ||
				!IsClearOfAvoidanceZones(GAI->Pawn,GAI->RouteCache(SkipAheadCurrentTestingIndex)) ||
				IsForbiddenByRockWorm(GAI->Pawn,Spec) ||
				IsForbiddenByCombatZone(GAI,GAI->RouteCache(SkipAheadCurrentTestingIndex))
			  )
			{
				//GAI->DrawDebugLine(Spec->Start->Location,Spec->End.Nav()->Location,255,0,0,TRUE);
				bSkipAheadFail = TRUE;
			}
			else
			{				
				//GAI->DrawDebugLine(Spec->Start->Location,Spec->End.Nav()->Location,0,255,0,TRUE);
				FLOAT Radius = 0.f;
				FLOAT HeightOffset = 0.f;
				if(GAI->Pawn->CylinderComponent)
				{
					Radius = GAI->Pawn->CylinderComponent->CollisionRadius;
					// want to test from maxstepheight off the ground
					HeightOffset =  - ((GAI->Pawn->CylinderComponent->CollisionHeight*0.5f) - GAI->Pawn->MaxStepHeight);
				}
				QueueTests(GAI->Pawn->Location + FVector(0.f,0.f,HeightOffset),GAI->RouteCache(SkipAheadCurrentTestingIndex),Radius);
			}
			//GAI->DrawDebugCoordinateSystem(Spec->End.Nav()->Location,FRotator(0,0,0),10.f,TRUE);
		}

	}
}

//////////////////////////////////////////////////////////////////////////
// UAIAvoidanceCylinder
//////////////////////////////////////////////////////////////////////////
TDoubleLinkedList<UAIAvoidanceCylinderComponent*> UAIAvoidanceCylinderComponent::GAvoidanceCylinders;
UBOOL UAIAvoidanceCylinderComponent::IsNavPointWithin(ANavigationPoint* Pt)
{
	if(!bEnabled)
	{
		return FALSE;
	}

	ConditionalReLinkToSpecsAndNavPts();
	return (LinkedNavigationPoints.Find(Pt) != NULL);
}
UBOOL UAIAvoidanceCylinderComponent::DoesSpecIntersect(UReachSpec* Spec)
{
	if(!bEnabled)
	{
		return FALSE;
	}

	ConditionalReLinkToSpecsAndNavPts();
	return (LinkedReachSpecs.Find(Spec) != NULL);
}


void UAIAvoidanceCylinderComponent::ConditionalReLinkToSpecsAndNavPts()
{
	if((LocalToWorld.GetOrigin() - LastReLinkLocation).SizeSquared() < UpdateThreshold* UpdateThreshold)
	{
		return;
	}

	LastReLinkLocation = LocalToWorld.GetOrigin();

	//otherwise, re-link to nav points and reach specs
	LinkedNavigationPoints.Reset();
	LinkedReachSpecs.Reset();

	TArray<FNavigationOctreeObject*> NavObjects;  
	//GWorld->GetWorldInfo()->DrawDebugCoordinateSystem(LocalToWorld.GetOrigin(),FRotator(0,0,0),100.f,TRUE);
	GWorld->NavigationOctree->RadiusCheck(LocalToWorld.GetOrigin(), Max<FLOAT>(CollisionRadius,CollisionHeight), NavObjects);

	ANavigationPoint* CurNav = NULL;
	UReachSpec* CurSpec = NULL;
	for(INT Idx = 0; Idx < NavObjects.Num(); Idx++)
	{
		FCheckResult Hit;
		CurNav = NavObjects(Idx)->GetOwner<ANavigationPoint>();		
		if(CurNav != NULL)
		{
			LinkedNavigationPoints.Set(CurNav,TRUE);
		}
		else
		{
			CurSpec = NavObjects(Idx)->GetOwner<UReachSpec>();
			if(CurSpec != NULL && CurSpec->Start != NULL && CurSpec->End.Nav() != NULL)
			{
				if(!LineCheck(Hit,CurSpec->End->Location,CurSpec->Start->Location,FVector(CurSpec->CollisionRadius,CurSpec->CollisionRadius,CurSpec->CollisionHeight),0))
				{
					LinkedReachSpecs.Set(CurSpec,TRUE);

					// add the one in the other direction
					UReachSpec* OtherDirSpec = CurSpec->End.Nav()->GetReachSpecTo(CurSpec->Start);
					if(OtherDirSpec != NULL)
					{
						LinkedReachSpecs.Set(OtherDirSpec,TRUE);
					}
					
				}
			}
		}
	}

}

UBOOL UAIAvoidanceCylinderComponent::BuildListOfAffectingCylinders(class AGearAI* AskingAI,TArray<class UAIAvoidanceCylinderComponent*>& InAffectingCylinders)
{
	TDoubleLinkedList<UAIAvoidanceCylinderComponent*>& AvoidanceCylinders = UAIAvoidanceCylinderComponent::GetList();
	for (TDoubleLinkedList<UAIAvoidanceCylinderComponent*>::TIterator Itt(AvoidanceCylinders.GetHead()); Itt; ++Itt)
	{
		UAIAvoidanceCylinderComponent* Comp = *Itt;
		if (Comp != NULL && Comp->ShouldAIAvoidMe(AskingAI))
		{
			InAffectingCylinders.AddItem(Comp);
		}
	}

	return (InAffectingCylinders.Num() > 0);
}

UBOOL UAIAvoidanceCylinderComponent::ShouldAIAvoidMe(AGearAI* AskingAI)
{
	// if combat transitions are off, bail
	return (  AskingAI != NULL && 
			  AskingAI->bAllowCombatTransitions &&
			  bEnabled && 
			  (
			    TeamThatShouldFleeMe == TEAM_EVERYONE || 
			    TeamThatShouldFleeMe == AskingAI->GetTeamNum()
			  ) &&
			  eventInternalShouldAIAvoidMe(AskingAI)
		   );

}

static inline UBOOL AllComponentsEqual(const FVector& Vec, FLOAT Tolerance=KINDA_SMALL_NUMBER)
{
	return Abs( Vec.X - Vec.Y ) < Tolerance && Abs( Vec.X - Vec.Z ) < Tolerance && Abs( Vec.Y - Vec.Z ) < Tolerance;
}
void AAIAvoidanceCylinder::EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown)
{
	const FVector ModifiedScale = DeltaScale * 500.0f;

	if ( bCtrlDown )
	{
		// CTRL+Scaling modifies trigger collision height.  This is for convenience, so that height
		// can be changed without having to use the non-uniform scaling widget (which is
		// inaccessable with spacebar widget cycling).
		AvoidanceComp->CollisionHeight += ModifiedScale.X;
		AvoidanceComp->CollisionHeight = Max( 0.0f, AvoidanceComp->CollisionHeight );
	}
	else
	{
		AvoidanceComp->CollisionRadius += ModifiedScale.X;
		AvoidanceComp->CollisionRadius = Max( 0.0f, AvoidanceComp->CollisionRadius );

		// If non-uniformly scaling, Z scale affects height and Y can affect radius too.
		if ( !AllComponentsEqual(ModifiedScale) )
		{
			AvoidanceComp->CollisionHeight += -ModifiedScale.Z;
			AvoidanceComp->CollisionHeight = Max( 0.0f, AvoidanceComp->CollisionHeight );

			AvoidanceComp->CollisionRadius += ModifiedScale.Y;
			AvoidanceComp->CollisionRadius = Max( 0.0f, AvoidanceComp->CollisionRadius );
		}
	}
}

void AAIAvoidanceCylinder::ForceReTouch()
{
	// clear only primitive components so we don't needlessly reattach components that never collide
	for (INT ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Components(ComponentIndex));
		if (Primitive != NULL && Primitive->CollideActors)
		{
			Primitive->ConditionalDetach();
		}
	}

	ConditionalUpdateComponents();
	FindTouchingActors();
}

UBOOL AGearAI_RockWorm::ShouldIgnoreNavigationBlockingFor(const AActor* Other)
{ 
	
	// rockworms don't care about other pawns.. just walk through them! bwahahhaa
	if(Other->IsA(AGearPawn::StaticClass()))
	{
		return TRUE;
	}

	return Super::ShouldIgnoreNavigationBlockingFor(Other);
}

UBOOL AGearAI_RockWorm::IsFriendly( AController* TestPlayer )
{
	return TRUE;
}

void AGearAI_RockWorm::ClearCrossLevelPaths(ULevel* Level)
{
	Super::ClearCrossLevelPaths(Level);

	AGearPawn_RockWormBase* RockWorm = Cast<AGearPawn_RockWormBase>(Pawn);
	if (RockWorm != NULL)
	{
		INT i = 0;
		while (i < RockWorm->ReachSpecsImBlocking.Num())
		{
			if (RockWorm->ReachSpecsImBlocking(i) == NULL || RockWorm->ReachSpecsImBlocking(i)->IsIn(Level))
			{
				RockWorm->ReachSpecsImBlocking.Remove(i);
			}
			else
			{
				i++;
			}
		}
	}
}

void AGearAI_RockWorm::UpdatePawnRotation()
{
	if(!Pawn->Acceleration.IsNearlyZero())
	{
		Super::UpdatePawnRotation();
	}
	else
	{
		Pawn->DesiredRotation = Pawn->Rotation;
	}
}

UBOOL UMantleReachSpec_Rockworm::CanBeSkipped( APawn* P )
{
	if(P == NULL || Start == NULL || End.Nav() == NULL)
	{
		return Super::CanBeSkipped(P);
	}

	FVector PToEnd = End.Nav()->Location - P->Location;
	
	if((GetDirection() | PToEnd) > 0.f )
	{
		return FALSE;
	}

	return Super::CanBeSkipped(P);

}

void AGearAI_Ticker::UpdatePawnRotation()
{
	if (MyGearPawn == NULL)
	{		
		Super::UpdatePawnRotation();
	}
	// only rotate toward our focus if it's close to us, and not too far away from the direction we're going
	else if (Focus && Focus != MoveTarget && !MyGearPawn->Acceleration.IsNearlyZero())
	{
		FVector MeToFocus = Focus->Location - MyGearPawn->Location;
		FLOAT DistToFocus = MeToFocus.Size();
		//DrawDebugCylinder(MyGearPawn->Location,MyGearPawn->Location,EnemyDistance_Short,16,255,0,0);
		if(DistToFocus > EnemyDistance_Short || (MyGearPawn->Acceleration.SafeNormal() | (MeToFocus/DistToFocus)) < 0.f)
		{
			MyGearPawn->rotateToward(MyGearPawn->Location + MyGearPawn->Velocity);
			return;
		}
	}

	Super::UpdatePawnRotation();
}

void AGearAI_Bloodmount::UpdatePawnRotation()
{
	if (MyGearPawn == NULL)
	{		
		Super::UpdatePawnRotation();
	}
	// only rotate toward our focus if it's close to us, and not too far away from the direction we're going
	else if (FireTarget && !MyGearPawn->Acceleration.IsNearlyZero())
	{
		MyGearPawn->rotateToward(MyGearPawn->Location + MyGearPawn->Velocity);
		return;
	}

	Super::UpdatePawnRotation();
}

/** Represents a CoverGroupRenderingComponent to the scene manager. */
class FLadderMeshSceneProxy : public FDebugRenderSceneProxy
{
	UBOOL bCreatedInGame;
public:

	/** Initialization constructor. */
	FLadderMeshSceneProxy(const ULadderMeshComponent* InComponent, UBOOL bInGame):
	  FDebugRenderSceneProxy(InComponent)
	  {	
		  bCreatedInGame = bInGame;
		  
		  ATrigger_LadderInteraction* Ladder = Cast<ATrigger_LadderInteraction>(InComponent->GetOwner());
		  
		  OwnerTrig = Ladder; // Keep a pointer to the Actor that owns this

		  // only draw if selected in the editor
		  UBOOL bDraw = Ladder && !Ladder->bIsTopOfLadder && (Ladder->IsSelected() || bCreatedInGame);

		  if( bDraw ) 
		  {
			  FVector  BottomEntryLoc, TopEntryLoc;
			  FRotator BottomEntryRot, TopEntryRot;
			  Ladder->GetBottomEntryPoint( BottomEntryLoc, BottomEntryRot );
			  Ladder->GetTopEntryPoint( TopEntryLoc, TopEntryRot );

			  const FLadderMeshes &MeshSet = InComponent->Meshes(0);

			  // Draw Top Marker
			  {
				  // update the translation
				  const FRotationTranslationMatrix MarkerLocalToWorld( FRotator(TopEntryRot.Quaternion() * FRotator(0, 16384, 0).Quaternion()), TopEntryLoc + InComponent->LocationOffset );
				  CreateLadderMesh( MeshSet.Base, MarkerLocalToWorld, FALSE );
			  }

			  // Draw Bottom Marker
			  {
				  // update the translation
				  const FRotationTranslationMatrix MarkerLocalToWorld( FRotator(BottomEntryRot.Quaternion() * FRotator(0, 16384, 0).Quaternion()), BottomEntryLoc + InComponent->LocationOffset );
				  CreateLadderMesh( MeshSet.Base, MarkerLocalToWorld, FALSE );
			  }
		  }
	  }

	  // FPrimitiveSceneProxy interface.
	  virtual HHitProxy* CreateHitProxies(const UPrimitiveComponent* Component,TArray<TRefCountPtr<HHitProxy> >& OutHitProxies)
	  {
		  // Create hit proxies for the ladder meshes.
		  for( INT MeshIndex = 0; MeshIndex < LadderMeshes.Num(); MeshIndex++ )
		  {
			  FLadderStaticMeshInfo& LadderInfo = LadderMeshes(MeshIndex);
			  LadderInfo.HitProxy = new HActorComplex(OwnerTrig,TEXT("Ladder"),-1);
			  OutHitProxies.AddItem(LadderInfo.HitProxy);
		  }
		  return FPrimitiveSceneProxy::CreateHitProxies(Component,OutHitProxies);
	  }

	  /** 
	  * Draw the scene proxy as a dynamic element
	  *
	  * @param	PDI - draw interface to render to
	  * @param	View - current view
	  * @param	DPGIndex - current depth priority 
	  * @param	Flags - optional set of flags from EDrawDynamicElementFlags
	  */
	  virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags)
	  {
		  PDI->SetHitProxy(NULL);

		  if( !bCreatedInGame )
		  {
			  // Draw all the ladder meshes we want.
			  for( INT i = 0; i< LadderMeshes.Num(); i++ )
			  {
				  FLadderStaticMeshInfo& LadderInfo = LadderMeshes(i);
				  const UStaticMesh* StaticMesh = LadderInfo.Mesh;
				  const FStaticMeshRenderData& LODModel = StaticMesh->LODModels(0);

				  PDI->SetHitProxy( LadderInfo.HitProxy );

				  // Draw the static mesh elements.
				  for(INT ElementIndex = 0;ElementIndex < LODModel.Elements.Num();ElementIndex++)
				  {
					  const FStaticMeshElement& Element = LODModel.Elements(ElementIndex);

					  FMeshElement MeshElement;
					  MeshElement.IndexBuffer = &LODModel.IndexBuffer;
					  MeshElement.VertexFactory = &LODModel.VertexFactory;
					  MeshElement.DynamicVertexData = NULL;
					  MeshElement.MaterialRenderProxy = Element.Material->GetRenderProxy(LadderInfo.bSelected);
					  MeshElement.LCI = NULL;
					  MeshElement.LocalToWorld = LadderInfo.LocalToWorld;
					  MeshElement.WorldToLocal = LadderInfo.LocalToWorld.Inverse();
					  MeshElement.FirstIndex = Element.FirstIndex;
					  MeshElement.NumPrimitives = Element.NumTriangles;
					  MeshElement.MinVertexIndex = Element.MinVertexIndex;
					  MeshElement.MaxVertexIndex = Element.MaxVertexIndex;
					  MeshElement.UseDynamicData = FALSE;
					  MeshElement.ReverseCulling = LadderInfo.LocalToWorld.Determinant() < 0.0f ? TRUE : FALSE;
					  MeshElement.CastShadow = FALSE;
					  MeshElement.Type = PT_TriangleList;
					  MeshElement.DepthPriorityGroup = SDPG_World;

					  PDI->DrawMesh( MeshElement );
				  }
			  }
		  }
		  PDI->SetHitProxy(NULL);
	  }


	  virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	  {
		  const UBOOL bVisible = OwnerTrig && !OwnerTrig->bIsTopOfLadder && OwnerTrig->IsSelected();
		  FPrimitiveViewRelevance Result;
		  Result.bDynamicRelevance = IsShown(View) && bVisible;
		  Result.SetDPG(SDPG_World,TRUE);
		  if (IsShadowCast(View))
		  {
			  Result.bShadowRelevance = TRUE;
		  }
		  return Result;
	  }

	  virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocOther ); }
	  virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	  DWORD GetAllocatedSize( void ) const { return( FDebugRenderSceneProxy::GetAllocatedSize() + LadderMeshes.GetAllocatedSize() ); }

private:

	/** One sub-mesh to render as part of this ladder setup. */
	struct FLadderStaticMeshInfo
	{
		FLadderStaticMeshInfo( UStaticMesh* InMesh, const FMatrix &InLocalToWorld, const UBOOL bInSelected ) :
		Mesh(InMesh),
		LocalToWorld(InLocalToWorld),
		bSelected(bInSelected),
		HitProxy(NULL)
	{}

	UStaticMesh* Mesh;
	FMatrix LocalToWorld;
	UBOOL bSelected;
	HHitProxy* HitProxy;
	};

	// Owning Trigger_LadderInteraction Actor.
	ATrigger_LadderInteraction* OwnerTrig;

	/** Cached array of information about each ladder mesh. */
	TArray<FLadderStaticMeshInfo>	LadderMeshes;

	/** Creates a FLadderStaticMeshInfo with the given settings. */
	void CreateLadderMesh(UStaticMesh* InMesh, const FMatrix &InLocalToWorld, const UBOOL bInSelected)
	{
		if(InMesh)
		{
			new(LadderMeshes) FLadderStaticMeshInfo(InMesh,InLocalToWorld,bInSelected);
		}
	}
};

void ULadderMeshComponent::UpdateBounds()
{
	Super::UpdateBounds();
	ATrigger_LadderInteraction *Ladder= Cast<ATrigger_LadderInteraction>(Owner);
	if( Ladder != NULL )
	{
		FBox BoundingBox = FBox(Ladder->Location,Ladder->Location).ExpandBy(32.f);
		
		FVector  BottomEntryLoc, TopEntryLoc;
		FRotator BottomEntryRot, TopEntryRot;
		Ladder->GetBottomEntryPoint( BottomEntryLoc, BottomEntryRot );
		Ladder->GetTopEntryPoint( TopEntryLoc, TopEntryRot );

		{
			FBox MarkerBoundingBox = FBox(TopEntryLoc,TopEntryLoc).ExpandBy(64.f);
			// add to the component bounds
			BoundingBox += MarkerBoundingBox;
		}
		{
			FBox MarkerBoundingBox = FBox(BottomEntryLoc,BottomEntryLoc).ExpandBy(64.f);
			// add to the component bounds
			BoundingBox += MarkerBoundingBox;
		}
		Bounds = Bounds + FBoxSphereBounds(BoundingBox);
	}
}

FPrimitiveSceneProxy* ULadderMeshComponent::CreateSceneProxy()
{
	UpdateMeshes();
	return new FLadderMeshSceneProxy(this,GIsGame);
}

UBOOL ULadderMeshComponent::ShouldRecreateProxyOnUpdateTransform() const
{
	// The cover mesh scene proxy caches a lot of transform dependent info, so it's easier to just recreate it when the transform changes.
	return TRUE;
}


// statics for FindNodesInRange()/GetBestPlantPoint()
static TArray<ANavigationPoint*> FoundNodes;
static UAICmd_Attack_PlantGrenade* QueryingCmd;

static FLOAT FindNodesInRange(ANavigationPoint* CurrentNode, APawn* seeker, FLOAT bestWeight)
{
	if (CurrentNode->visitedWeight > QueryingCmd->MaxDistance)
	{
		return 1.0f;
	}
	else
	{
		if (CurrentNode->visitedWeight >= QueryingCmd->MinDistance)
		{
			FoundNodes.AddItem(CurrentNode);
		}
		return 0.0f;
	}
}

ANavigationPoint* UAICmd_Attack_PlantGrenade::GetBestPlantPoint()
{
	APawn* Pawn = CastChecked<AGearAI>(GetOuter())->Pawn;
	if (Pawn == NULL || Pawn->Controller == NULL)
	{
		return NULL;
	}
	else
	{
		if (!Pawn->ValidAnchor())
		{
			FLOAT Unused = 0.f;
			Pawn->SetAnchor(Pawn->FindAnchor(Pawn, Pawn->Location, TRUE, FALSE, Unused));
		}
		if (Pawn->Anchor == NULL)
		{
			return NULL;
		}
		else if ((MinDistance <= 0 && MaxDistance <= 0) || MinDistance > MaxDistance)
		{
			return Pawn->Anchor;
		}
		else
		{
			// set up statics
			FoundNodes.Empty();
			QueryingCmd = this;

			BYTE OldSearchType = Pawn->PathSearchType;
			Pawn->PathSearchType = PST_Breadth;
			Pawn->findPathToward(NULL, FVector(0,0,0), &FindNodesInRange, 0.0f, FALSE, MaxDistance, FALSE);
			Pawn->PathSearchType = OldSearchType;
			
			// look through the list and find the best one
			ANavigationPoint* Best = NULL;
			INT BestRating = 1000000;
			for (INT i = 0; i < FoundNodes.Num(); i++)
			{
				INT Rating = RandHelper(100); // initial random weight so the AI doesn't choose the same point every time
				for (INT j = 0; j < FoundNodes(i)->PathList.Num(); j++)
				{
					Rating += FoundNodes(i)->PathList(j)->CollisionRadius;
				}
				if (Rating < BestRating)
				{
					Best = FoundNodes(i);
					BestRating = Rating;
				}
			}

			// clear statics
			FoundNodes.Empty();
			QueryingCmd = NULL;

			return (Best != NULL) ? Best : Pawn->Anchor;
		}
	}
}

UBOOL AGearAI_Jack::FindGoodTeleportSpot( FVector ChkExtent, FVector& out_Dest )
{
	FVector GoalDest = out_Dest;

	//debug
//	FlushPersistentDebugLines();
//	DrawDebugBox( GoalDest, ChkExtent, 0, 0, 255, TRUE );

	TArray<FNavigationOctreeObject*> NavObjects;
	GWorld->NavigationOctree->RadiusCheck(GoalDest, 512.f, NavObjects);
	for(INT Idx = 0; Idx < NavObjects.Num(); Idx++)
	{
		// look for valid nav points
		FNavigationOctreeObject* NavObj = NavObjects(Idx);
		if( NavObj == NULL )
			continue;

		ANavigationPoint *Nav = NavObj->GetOwner<ANavigationPoint>();
		if( Nav == NULL )
			continue;

		FCheckResult Hit1, Hit2;
		FVector ChkLoc = Nav->Location;
		ChkLoc.Z += ChkExtent.Z;
		if(  GWorld->SingleLineCheck( Hit1, Pawn, GoalDest, ChkLoc, TRACE_World|TRACE_StopAtAnyHit ) && 
			(GWorld->SinglePointCheck( Hit2, ChkLoc, ChkExtent, TRACE_Pawns|TRACE_World) || Hit2.Actor == Pawn) )
		{
			//debug
//			DrawDebugBox( ChkLoc, ChkExtent, 0, 255, 0, TRUE );

			out_Dest = ChkLoc;
			return TRUE;
		}
		//debug
//		else
//		{
//			DrawDebugBox( ChkLoc, ChkExtent, 255, 0, 0, TRUE );
//		}
	}

	return FALSE;
}

UBOOL AGearAI_COGGear::BumperSomewhereToGo()
{
	FVector EndPos;
	FLOAT  MoveDist;
	FVector ClosestPt;

	if(lastbumper == NULL || MyGearPawn == NULL)
	{
		return FALSE;
	}

	PointDistToSegment(MyGearPawn->Location,lastbumper->Location,lastbumper->Location + lastbumper->Acceleration*1000.f,ClosestPt);

	MoveDist = 2.f*(ClosestPt-lastbumper->Location).Size();
	EndPos = lastbumper->Location + (lastbumper->Acceleration).SafeNormal() * MoveDist*1.05f;
	
	FlushPersistentDebugLines();
	
	FLOAT Rad,Height;
	lastbumper->GetBoundingCylinder(Rad,Height);
	FVector Extent = FVector(Rad,Rad,Height) * 0.4f;
	
	FMemMark Mark(GMainThreadMemStack);
	FCheckResult* MultiHit = GWorld->MultiPointCheck(
		GMainThreadMemStack,
		EndPos,
		Extent,
		TRACE_World|TRACE_AllBlocking
		);
	for( FCheckResult* Hit = MultiHit; Hit; Hit = Hit->GetNext() )
	{
		if(Hit->Actor != NULL && Hit->Actor != MyGearPawn && Hit->Actor->bBlockActors && !Hit->Actor->bTearOff)
		{
			Mark.Pop();
			return FALSE;
		}
	}

	Mark.Pop();
	return TRUE;
}


void AGearAI_SecurityBotStationary::PostScriptDestroyed()
{
	Super::PostScriptDestroyed();
	// if we're being destroyed during GC make sure our squad is cleaned up as well (it's our own squad, and destroyed isn't called otherwise so we need to do this here
	if(GIsGarbageCollecting && Squad != NULL && !Squad->bPendingDelete && ! Squad->HasAnyFlags(RF_Unreachable|RF_PendingKill) && GWorld != NULL)
	{
		if(Squad->Formation != NULL)
		{
			GWorld->DestroyActor(Squad->Formation);
		}

		GWorld->DestroyActor(Squad);
		Squad=NULL;
	}
}

