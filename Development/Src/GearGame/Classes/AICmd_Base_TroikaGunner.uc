/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Base_TroikaGunner extends AICmd_Base_UseTurret
	within GearAI_Cover;

var AIReactCond_GenericCallDelegate TurretClamped;

function Pushed()
{
	if( EnsureWereOnATurret() )
	{
		// Don't let people go DBNO off a turret
		MyGearPawn.bCanDBNO = FALSE;

		Super.Pushed();
	}	

	// ensure this isn't suppressed from an earlier subscription
	ReactionManager.UnSuppressChannel('TurretRotationClamped');
	TurretClamped = new(outer) class'AIReactCond_GenericCallDelegate';
	TurretClamped.AutosubscribeChannels.AddItem('TurretRotationClamped');
	TurretClamped.OutputFunction = NotifyTurretRotationClamped;
	TurretClamped.Initialize();
}

function Popped()
{
	Super.Popped();
	TurretClamped.UnsubscribeAll();
	TurretClamped=none;
	LeaveTurret();
	RegisterReGunReaction();
}

// registers a reaction which will check periodically to see if it's ok to come back to this troika
function RegisterReGunReaction()
{
	local AIReactCond_CanReturnToTroika ReturnReact;
	ReturnReact = new(outer) class'AIReactCond_CanReturnToTroika';
	ReturnReact.Initialize();
}

function NotifyTurretRotationClamped(Actor Inst, AIreactChannel OrigChan)
{
	`AILog(GetFuncName()@self@Outer@Enemy@bIgnoreFlank);
	if(!bIgnoreFlank && Enemy != none)
	{
		if(!CanTurretFireAt(Enemy))
		{
			`AILog(GetFuncName()@self@outer@"Can't fire turret at target:"@Enemy@"looking for new target I can fire at....");
			// try to find a new enemy within our rotation clamps
			SelectTarget();
			
			// if we still can't fire at the new enemy, leave the turret
			if(!CanTurretFireAt(Enemy))
			{
				`AILog(GetFuncName()@self@outer@"Found no new target to shoot at.. leaving turret");
				bReachedTurret=false;
				AbortCommand(self);
			}
			else
			{
				`AILog(GetFuncName()@self@outer@"Found a new target!");
			}
		}
		else
		{
			`AILog(GetFuncName()@self@outer@"Can fire at target? what?");
		}

	}

}

function bool EnsureWereOnATurret()
{
	// try and go to our last turret
	if( !bReachedTurret && !HasValidTurret() )
	{
		//debug
		`AILog("Has no current turret! trying to go to the last turret we were using ("@LastTurret@")");

		if( LastTurret != none && !LastTurret.ClaimTurret(AIOwner) )
		{
			//debug
			`AILog("Failed to use lastturret:"$LastTurret$", bailing...");

			GotoState('DelayFailure');
			return FALSE;
		}
		else
		{
			`AILog("Succesfully claimed "@LastTurret);
		}

	}
	return TRUE;
}

function LeaveTurret()
{
	local int OldHealth;
	local pawn Driver;
	`AILog(GetFuncName()@CurrentTurret@CurrentTurret.Driver);
	if( CurrentTurret != None )
	{
		LastTurret = CurrentTurret;
		LastTurret.UnclaimTurret( CoverOwner );
		
		Driver = LastTurret.Driver;
		OldHealth = Driver.Health;
		
		if(Driver.Health <= 0)
		{
			Driver.Health=1;
		}		
		LastTurret.DriverLeave( TRUE );
		Driver.Health=OldHealth;
	}

	// restore the fire tickets flag
	bUseFireTickets = default.bUseFireTickets;
	ClearTimer( 'SelectTarget' );

}

state InCombat
{
	function BeginState( Name PrevStateName )
	{
		Super.BeginState( PrevStateName );

		bUseFireTickets = FALSE;
		SetTimer( 1.f, TRUE, nameof(SelectTarget) );
	}

	function EndState( Name NextStateName )
	{
		super.EndState( NextStateName );
		LeaveTurret();
	}

	function Tick( float DeltaTime )
	{
		if( HasValidTurret() )
		{
			// have turret define how we should track enemies
			CurrentTurret.UpdateAIController( CoverOwner, DeltaTime );
		}

		super.Tick( DeltaTime );
	}

	/**
	 * Figure out current aim error and return fire rotation.
	 */
	function Rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
	{
		local Rotator	POVRot;
		local Vector	POVLoc;

		GetPlayerViewPoint( POVLoc, POVRot );

		// base aim error
		POVRot.Pitch += RandRange(-256,128);
		POVRot.Yaw	 += RandRange(-512,512);

		return POVRot;
	}

	// check to see if the target is outside our rotation clamps
	function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
	{
		local float EnemyDist;

		if (ChildCommand != None)
		{
			ChildCommand.AdjustEnemyRating(out_Rating, EnemyPawn);
			return;
		}

		if( bReachedTurret )
		{
			// scale enemy rating if they're really close
			EnemyDist = VSize(GetEnemyLocation(EnemyPawn) - CurrentTurret.Location);
			if (EnemyDist < 512.f)
			{
				out_Rating *= 1.f + (1.f - (EnemyDist/512.f));
			}
			// check to see if we could shoot them
			if (CanTurretFireAt(EnemyPawn))
			{
				out_Rating *= 1.5f;
			}
			else
			{
				out_Rating *= 0.5f;
			}
		}

		Super.AdjustEnemyRating( out_Rating, EnemyPawn );
	}


Begin:
	//debug
	`AILog( "BEGIN TAG"@HasValidTurret()@IsAtTurret(), 'State' );

	if( HasValidTurret() )
	{
		while( HasValidTurret() )
		{
			// make sure we're at the turret
			if( !IsAtTurret() )
			{
				//debug
				`AILog( "Moving to turret", 'Turret' );
				
				SetMovePoint( CurrentTurret.GetEntryLocation() );
				if( Pawn.ReachedPoint( CurrentTurret.GetEntryLocation(), None ) )
				{
					bReachedTurret = TRUE;
				}

				if( !CurrentTurret.TryToDrive(Pawn) )
				{
					//debug
					`AILog( "FAILED TO DRIVE" );

					//@hack!!!
					Pawn.SetLocation( CurrentTurret.GetEntryLocation() );
					CurrentTurret.TryToDrive(Pawn);
					bReachedTurret = TRUE;

					Sleep(1.f);
				}
				else
				{
					bReachedTurret = TRUE;
				}
			}
			else
			{
				// found an enemy to fire at
				if( SelectTarget() &&
					CanTurretFireAt( FireTarget ) )
				{
					// attempt to set enemy to the firetarget
					// this is needed to correctly aim at the pawn bone locations, if firing at a player
					SetEnemy(Pawn(FireTarget));

					//debug
					`AILog( "Aiming at an enemy...?"@Enemy@FireTarget@LineOfSightTo(FireTarget), 'Turret' );

					// Aim at them and shoot
					StartFiring();
					do
					{
						Sleep(2.f);
						CheckInterruptCombatTransitions();
						`AILog("can still fire at?"@FireTarget@CanTurretFireAt(FireTarget));
					}
					until(!IsFiringWeapon() || !HasValidTarget() || !CanTurretFireAt( FireTarget ));
					`AILog("No longer firing at target, valid target?"@HasValidTarget());
					StopFiring();
					Focus = None;
				}
				else
				{
					//debug
					`AILog( "Idle look..."@FireTarget@Enemy@SelectTarget()@CanTurretFireAt( FireTarget ), 'Turret' );

					Sleep( 1.0f );
					CurrentTurret.AIIdleNotification( CoverOwner );
				}
			}

			// added to avoid infinite recursion
			Sleep( 0.1f );
		}
	}
	else
	{
		//debug
		`AILog( "Error: No turret, can't gun!" );

		Sleep( 1.f );
	}

	// Check combat transitions
	CheckCombatTransition();
}
