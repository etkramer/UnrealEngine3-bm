/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Attack_FireFromCover extends AICommand
	within GearAI_Cover;

/** GoW global macros */

/** Simple constructor that pushes a new instance of the command for the AI */
static function bool FireFromCover( GearAI_Cover AI )
{
	local AICmd_Attack_FireFromCover Cmd;

	if( AI != None )
	{
		Cmd = new(AI) class'AICmd_Attack_FireFromCover';
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

	bFailedToFireFromCover = FALSE;
	LastFireFromCoverTime = WorldInfo.TimeSeconds;

	TriggerAttackGUDS();
	AIOwner.SetTimer( 15.f, TRUE, nameof(AIOwner.TriggerAttackGUDS) );

	GotoState( 'Firing' );
}

function Popped()
{
	Super.Popped();

	bFailedToFireFromCover = (Status != 'Success');

	AIOwner.ClearTimer('TriggerAttackGUDS');

	// Always clear cover action
	ResetCoverAction();
}

function AdjustEnemyRating(out float out_Rating, Pawn EnemyPawn)
{
	if (ChildCommand != None)
	{
		ChildCommand.AdjustEnemyRating(out_Rating, EnemyPawn);
		return;
	}

	if( out_Rating > 0 &&
		MyGearPawn != None &&
		!GetCoverAction( Cover, EnemyPawn,, TRUE, 'Enemy' ) )
	{
		//debug
		`AILog( "- no cover action to:"@EnemyPawn, 'Enemy' );

		out_Rating = -1.f;
	}

	Super.AdjustEnemyRating(out_Rating, EnemyPawn);
}

function bool ShouldSelectTarget()
{
	// Don't select new target when sniping
	if( !IsUsingSuppressiveWeapon() )
	{
		return FALSE;
	}
	return Super.ShouldSelectTarget();
}

/**
 * Selects the first enemy that is medium range and attackable from our
 * current cover.
 */
final function private bool SelectEnemyAndCoverAction()
{
	local int Idx;

	FireTarget = None;
	if( TargetList.Length > 0 )
	{
		// check from current position
		for( Idx = 0; Idx < TargetList.Length; Idx++)
		{
			if( GetCoverAction( Cover, TargetList[Idx], Controller(TargetList[Idx]) == None ) )
			{
				FireTarget = TargetList[Idx];
				ShotTarget = Pawn(FireTarget);
				return TRUE;
			}
		}
	}

	// try to select the closest enemy that we can shoot at from cover
	if( SelectTarget() )
	{

		//debug
		`AILog( "Selected FireTarget... "@FireTarget );

		if( GetCoverAction( Cover, None ) )
		{
			MyGearPawn.ShouldCrouch( PendingFireLinkItem.SrcType == CT_MidLevel );
			return TRUE;
		}
		return FALSE;
	}

	//debug
	`AILog( "Couldn't select enemy" );

	// no dice
	return FALSE;
}

final function private bool SelectEnemyAndAmbushAction()
{
	local bool bWantsToBeMirrored;
	local float DotP;
	local Vector X, Y, Z;

	if( GetCoverAction( Cover, None, FALSE ) ||
		GetCoverAction( Cover, None, TRUE  ) )
	{
		if( PendingFireLinkItem.SrcAction == CA_Default )
		{
			GetAxes( Cover.Link.GetSlotRotation(Cover.SlotIdx), X, Y, Z );
			DotP = (GetFireTargetLocation() - Cover.Link.GetSlotLocation(Cover.SlotIdx)) DOT Y;
			if( DotP < 0 )
			{
				bWantsToBeMirrored = TRUE;
			}
			MyGearPawn.SetMirroredSide( bWantsToBeMirrored );
		}

		return TRUE;
	}

	return FALSE;
}

function bool IsEnemyExposed()
{
	local Vector TestLocation;
	local Pawn	 P;
	local Rotator Rot;

	Rot = Pawn.Rotation;
	P = Pawn(FireTarget);
	if( P != None )
	{
		TestLocation = P.GetPawnViewLocation();
		Rot = Rotator(GetFireTargetLocation() - Pawn.Location);
	}
	else
	{
		TestLocation = FireTarget.Location;
		Rot = Rotator(TestLocation-Pawn.Location);
	}

	return CanSeeByPoints( Pawn.GetWeaponStartTraceLocation(), TestLocation, Rot );
}



state Firing `DEBUGSTATE
{
Begin:
	//debug
	`AILog( "BEGIN TAG"@bTurtle@IsReloading()@IsSwitchingWeapons()@IsUsingSuppressiveWeapon(), 'State' );

	// check for any interrupt transitions
	CheckInterruptCombatTransitions();

	// NOTE: Be on look out for bugs b/c of competing transition animations
	// Select our weapon
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

	// Moved this up here so that AI doesn't need to get back into cover before moving
	// but we still need to allow time for them to get back in if they didn't move
	Sleep( 0.5f );

	// If turtling command given while reloading
	if( bTurtle )
	{
		`AILog("Turtling...");
		// Jump to end
		Goto( 'Finish' );
	}

PrepareFire:
	//debug
	`AILog( "PREPAREFIRE TAG"@Enemy );

	// if not using a suppressive weapon and can pop out from cover
	if( !IsUsingSuppressiveWeapon() &&
		SelectEnemyAndAmbushAction() )
	{
		//debug
		`AILog( "Pending ambush action"@PendingFireLinkItem.SrcAction@PendingFireLinkItem.SrcType@IsTransitioning() );

		// wait for any transitions to finish
		// before transitioning again
		LastAnimTransitionTime = WorldInfo.TimeSeconds;
		do
		{
			Sleep( 0.25f );
		} until( !IsTransitioning() );

		// If should wait for transition (this function will mirror pawn if needed)
		if( ShouldWaitForTransition( PendingFireLinkItem.SrcAction ) )
		{
			//debug
			`AILog( "Wait for mirror transition", 'Combat' );

			// While still transitioning, wait
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );
		}

		// If 360 aiming
		if( PendingFireLinkItem.SrcAction == CA_Default )
		{
			// Stand up in mid level cover
			if( MyGearPawn.CoverType == CT_MidLevel )
			{
				//debug
				`AILog( "stand up" );

				// Stand up
				MyGearPawn.ShouldCrouch( TRUE );
			}
		}
		else
		{
			// Wait for action anim
			SetCoverAction( PendingFireLinkItem.SrcAction );
		}

		// Sleep until done doing action
		LastAnimTransitionTime = WorldInfo.TimeSeconds;
		do
		{
			Sleep( 0.25f );
		} until( !IsTransitioning() );

		// Only lean out so long
		SetTimer( 1.f+FRand()*5.f, FALSE, nameof(NotifyFireLineBlocked) );

		// wait for no enemies, or an exposed enemy
		while( !bFailedToFireFromCover	&&
				HasAnyEnemies()			&&
				HasValidTarget()		&&
			   (IsLeaning() || ShouldDo360Aiming( GetFireTargetLocation() )) &&
			   !IsEnemyExposed() )
		{
			//debug
			`AILog( "Waiting for enemy to come out"@bFailedToFireFromCover@HasValidTarget()@IsLeaning()@ShouldDo360Aiming( GetFireTargetLocation() )@IsEnemyExposed(), 'Loop' );

			Sleep( 0.25f );
		}

		// If aborted cover action
		if( bFailedToFireFromCover )
		{
			//debug
			`AILog( "Reset cover action b/c enemy didn't expose himself" );

			// Hang out for a moment
			Sleep( 1.f+FRand()*5.f );
			Goto('ReturnToCover');
		}
		else
		// if we have a valid enemy
		// If could get fire ticket
		if( HasValidTarget() &&
			CanFireAt(FireTarget, Pawn.GetWeaponStartTraceLocation()) &&
			ClaimFireTicket() )
		{
 			// fire at them
			Goto('FireWeapon');
		}
		else
		{
			//debug
			`AILog("Invalid target, or no ticket, or can't fire at"@HasValidTarget()@FireTarget);

			// sometimes give up and move
			bFailedToFireFromCover = (FRand() < 0.2);

			// otherwise return to cover and exit
			Goto('ReturnToCover');
		}
	}
	else
	// try to pop out from cover
	if( SelectEnemyAndCoverAction() )
	{
		//debug
		`AILog( "Got action"@PendingFireLinkItem.SrcAction@PendingFireLinkItem.SrcType@IsTransitioning(), 'Combat' );

		// wait for any transitions to finish
		// before transitioning again
		LastAnimTransitionTime = WorldInfo.TimeSeconds;
		do
		{
			Sleep( 0.25f );
		} until( !IsTransitioning() );

		// If should wait for transition (this function will mirror pawn if needed)
		if( ShouldWaitForTransition( PendingFireLinkItem.SrcAction ) )
		{
			//debug
			`AILog( "Wait for mirror transition", 'Combat' );

			// While still transitioning, wait
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );
		}

		// If could get fire ticket
		if( ClaimFireTicket() )
		{
			// If 360 aiming...
			if( PendingFireLinkItem.SrcAction == CA_Default )
			{
				// Stand up in mid level cover
				if( MyGearPawn.CoverType == CT_MidLevel )
				{
					//debug
					`AILog( "Stand up" );

					// Stand up
					MyGearPawn.ShouldCrouch( TRUE );
				}
			}
			else
			{
				// Set action and wait for action anim to finish
				SetCoverAction( PendingFireLinkItem.SrcAction );
			}

			// Sleep until done doing action
			LastAnimTransitionTime = WorldInfo.TimeSeconds;
			do
			{
				Sleep( 0.25f );
			} until( !IsTransitioning() );
			`AILog("Done transitioning, going to fire"@MyGearPawn.DoingAnimationTransition()@IsTransitioning());
			Goto( 'FireWeapon' );
		}
		else
		{
			// Otherwise, no fire ticket available

			//debug
			`AILog( "Failed to get fire ticket... peek", 'Combat' );

			PeekFromCover();

			//debug
			`AILog( "Finished peek command..."@bFailedToFireFromCover@CanClaimFireTicket(GearPawn(Enemy)));

			if( bFailedToFireFromCover )
			{
				Sleep( 0.5f );
			}
			else
			if( CanClaimFireTicket(GearPawn(Enemy)) )
			{
				Goto( 'PrepareFire' );
			}
		}
	}
	else
	{
		//debug
		`AILog( "Not able to fire at enemy"@MyGearPawn.CoverAction, 'Combat' );
		bFailedToFireFromCover = TRUE;

		Sleep( 0.5f );
	}
	Goto('Finish');

FireWeapon:
	//debug
	`AILog( "FIREWEAPON TAG" );

	// fire at the enemy
	Focus = FireTarget;
	// Clear timer in case we were using sniper rifle
	ClearTimer( 'NotifyFireLineBlocked' );

	// Sleep if weapon has aim time
	if( WeaponAimDelay > 0.f )
	{
		MyGearPawn.SetWeaponAlert(WeaponAimDelay);
		//debug
		`AILog( "Delay weapon fire by"@WeaponAimDelay, 'Weapon' );

		Sleep( WeaponAimDelay );
	}

	StartFiring();
	// wait for the weapon to finish firing
	do
	{
		Sleep( 0.25f );

		//debug
		`AILog( "Firing weapon..."@MyGearPawn.ShouldDelayFiring(), 'Loop' );
		if ( (MyGearPawn.MyGearWeapon == None || !MyGearPawn.MyGearWeapon.bNoAnimDelayFiring) &&
			 MyGearPawn.DoingAnimationTransition() )
		{
			//debug
			`AILog("Unable to transition, aborting fire");

			StopFiring();
		}
	} until( (!IsFiringWeapon() && !IsMountingWeapon()) || IsReloading() || bFailedToFireFromCover );


	// NOTE: Be on look out for bugs b/c of competing transition animations
	// Select our weapon
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

	// heavy weapons cancel reload when unmounting and such so leave the AI here until it's done
	while (MyGearPawn.IsCarryingAHeavyWeapon() && IsReloading())
	{
		`AILog("Wait for heavy weapon to finish reloading", 'Loop');
		Sleep(0.25);
	}

	//debug
	`AILog("Finished firing"@IsFiringWeapon()@bFailedToFireFromCover );

	StopFiring();
	Sleep(0.25f);

	if( bFailedToFireFromCover )
	{
		SetAcquireCover( ACT_DesireBetter );
	}

ReturnToCover:
	//debug
	`AILog( "RETURNTOCOVER TAG"@bFailedToFireFromCover, 'State' );

	// return to cover
	ReleaseFireTicket();
	ResetCoverAction();

	// Sleep until done resetting action
	LastAnimTransitionTime = WorldInfo.TimeSeconds;
	do
	{
		Sleep( 0.25f );
	} until( !IsTransitioning() );

Finish:
	//debug
	`AILog( "FINISH TAG", 'State' );

	Status = bFailedToFireFromCover ? 'Failure' : 'Success';
	PopCommand( self );
}

defaultproperties
{
	bIsStationaryFiringCommand=true
}
