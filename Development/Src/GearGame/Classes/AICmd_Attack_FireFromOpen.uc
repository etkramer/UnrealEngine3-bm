/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_FireFromOpen extends AICommand
	within GearAI;

/** number of bursts to fire (-1 for infinite) */
var int BurstsToFire;
var bool bFireWhileMoving;

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool FireFromOpen(GearAI AI, optional int InBursts = 1,optional bool bAllowFireWhileMoving=true)
{
	local AICmd_Attack_FireFromOpen Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Attack_FireFromOpen';
		Cmd.BurstsToFire = InBursts;
		Cmd.bFireWhileMoving = bAllowFireWhileMoving;
		AI.PushCommand( Cmd );
		return TRUE;
	}

	return FALSE;
}

/** Build debug string */
event String GetDumpString()
{
	return Super.GetDumpString()@"Bursts"@BurstsToFire;
}

function Pushed()
{
	Super.Pushed();

	if (GearAI_Cover(Outer) != None)
	{
		GearAI_Cover(Outer).ResetCoverType();
	}

	bFailedToFireFromOpen = FALSE;

	TriggerAttackGUDS();
	AIOwner.SetTimer( 15.f, TRUE, nameof(AIOwner.TriggerAttackGUDS) );

	GotoState( 'Firing' );
}

function Popped()
{
	Super.Popped();

	AIOwner.ClearTimer('TriggerAttackGUDS');

	if (MyGearPawn != None)
	{
		MyGearPawn.SetTargetingMode(false);
	}

	bFailedToFireFromOpen = (Status != 'Success');
}

state Firing `DEBUGSTATE
{
Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if( !bTurtle )
	{
		if( HasValidTarget() )
		{
			if( !CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()) )
			{
				//debug
				`AILog("No clear shot at fire target:"@FireTarget);

				if( FireTarget == Enemy && AllowedToMove() && IsEnemyWithinCombatZone(Enemy))
				{
					SetEnemyMoveGoal(,,,bFireWhileMoving);
					if( bFailedToMoveToEnemy ||
						!CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()))
					{
						//debug
						`AILog("Still unable to fire at target");

						Sleep(0.25f);

						Status = 'Failure';
						PopCommand( self );
					}
				}
				else
				{
					Sleep(0.25f);

					Status = 'Failure';
					PopCommand( self );
				}
			}

			Focus = FireTarget;
			Sleep(0.5f);

			if (MyGearPawn != None)
			{
				if(MyGearPawn.IsCarryingAHeavyWeapon())
				{
					Sleep(0.5f); // more time to recenter toward enemy
				}

				// set fire mode for all (not just heavy)
				MyGearPawn.SetTargetingMode(true);

				if(MyGearPawn.IsCarryingAHeavyWeapon())
				{
					do
					{
						Sleep(0.25f);
						//debug
						`AIlog("Waiting for weapon to be done mounting", 'Loop');
					} until (!IsMountingWeapon());

					`AILog("Checking weapon switch..."@IsSwitchingWeapons());
					if( !IsSwitchingWeapons() )
					{
						if( SelectWeapon() )
						{
							Sleep( 0.25f );
						}
					}
					while( IsSwitchingWeapons() )
					{
						//debug
						`AILog( "Waiting for weapon switch", 'Loop' );

						Sleep(0.25f);
					}
				}


			}

			//@FIXME: probably should be done generally but I'd rather not shake things up at this point
			//	just to make the Casual bots a little easier
			if (GetDifficultyLevel() == DL_Casual)
			{
				FinishRotation();
			}

			if( ClaimFireTicket() )
			{
				Sleep(AboutToFireFromOpen());

				StartFiring(BurstsToFire);

				// wait for possible reload
				if(IsReloading())
				{
					if (CoverOwner != None && GetDifficultyLevel() > DL_Normal)
					{
						`AILog("Aborting because we'd have to stand here in the open waiting for reload");
						Status = 'Success'; // sort of lying, but don't want commands under this to think something broke
						PopCommand(self);
					}

					do
					{
						Sleep(0.25f);
						//debug
						`AILog("Waiting for reload!",'loop');
					} until( !IsReloading() );
				}

				do
				{
					Sleep(0.25f);

					//debug
					`AILog( "Firing weapon...", 'Loop' );
				} until (!IsFiringWeapon());

				Sleep(0.1f);

				`AILog("Checking weapon switch..."@IsSwitchingWeapons());
				if( !IsSwitchingWeapons() )
				{
					if( SelectWeapon() )
					{
						Sleep( 0.25f );
					}
				}
				while( IsSwitchingWeapons() )
				{
					//debug
					`AILog( "Waiting for weapon switch", 'Loop' );

					Sleep(0.25f);
				}
			}
			else
			{
				//debug
				`AILog( "Failed to get fire ticket", 'Combat' );

/*
				// Give combat action a chance to evaluate
				if( CombatAction != None )
				{
					CombatAction.static.NotifyFireTicketUnavailable( self );
				}
*/
				// Just rest for a moment
				Sleep( 0.5f );
			}
		}
		else
		{
			//debug
			`AILog( "Not able to fire at enemy", 'Combat' );

			Sleep(0.5f);
//			CombatAction.static.NotifyUnableToFireFromCover(self);
		}
	}
	else
	{
		`AILog("Turtling...", 'Combat');
		Sleep(0.5);
	}

	Status = bFailedToFireFromOpen ? 'Failure' : 'Success';
	PopCommand( self );
}


defaultproperties
{
	bIsStationaryFiringCommand=true
}
