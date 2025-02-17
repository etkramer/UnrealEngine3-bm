/**
 * Base class for behavior of Reavers
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AICmd_Base_Reaver extends AICommand_Base_Combat
	within GearAI_Reaver;

var Actor	LastTarget;
var float	NextMoveToEnemyTime;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();
	GotoState('InCombat');
}

state InCombat
{

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );

	if (Reaver.Physics == PHYS_Interpolating)
	{
		`AILog("Still flying...");
		bAllowedToFireWeapon = (WorldInfo.TimeSeconds > Reaver.AllowRocketFireTime);
		if( bAllowedToFireWeapon )
		{
			StartFiring();
		}		
		Sleep(1.0);
	}
	else if (Enemy != None)
	{
		bAllowedToFireWeapon = ((Vehicle(Enemy) != None) || !Reaver.bSuppressRocketsOnLand) && 
								(WorldInfo.TimeSeconds > Reaver.AllowRocketFireTime);
		if (bAllowedToFireWeapon)
		{
			StartFiring();
		}
		else
		{
			StopFiring();
		}
		if (Reaver.CheckLegAttack(Enemy))
		{
			StopFiring();
			if (Focus != None)
			{
				SetFocalPoint( Enemy.Location );
				Focus = None;
			}
			Reaver.Throttle = 0.0;
			Reaver.Steering = 0.0;
			Sleep(1.0);
		}
		// If allowed to move to the enemy
		else if (WorldInfo.TimeSeconds > NextMoveToEnemyTime)
		{
			// Try to move
			Focus = Enemy;
			SetEnemyMoveGoal(false, 350.0);

			// If move failed (ie can't move out of combat zone)
			if (ChildStatus != 'Success')
			{
				//debug
				`AILog( "Failed to move to enemy... set NextMoveToEnemyTime" );

				// Don't try again for a little
				NextMoveToEnemyTime = WorldInfo.TimeSeconds + 1.5f;
				// in Horde, force Reaver to be allowed to fire if it can't reach its enemy
				if (GearGameHorde_Base(WorldInfo.Game) != None)
				{
					StartFiring();
				}

				Sleep(0.5f);
			}
		}
		else
		{
			if (GearGameHorde_Base(WorldInfo.Game) != None)
			{
				StartFiring();
			}
			Sleep(0.5f);
		}
	}
	else
	{
		Sleep(0.5);
	}

	Reaver.ZeroMovementVariables();
	
	Goto('Begin');
}

defaultproperties
{
	LastTarget=none
}
