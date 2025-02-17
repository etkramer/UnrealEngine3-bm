class AICmd_Base_CTMVictim extends AICommand_Base_Combat
	within GearAI_CTMVictim;

/** GoW global macros */

function Pushed()
{
	Super.Pushed();

	GotoState('InCombat');
}

state InCombat
{
	final function CheckExecute()
	{
		local GearPawn P;

		// check for enemies to execute
		if (bCanExecute && Squad != None)
		{
			foreach Squad.AllEnemies(class'GearPawn', P)
			{
				if ( P.IsDBNO() && VSize(GetEnemyLocation(P) - Pawn.Location) < 1024.0 &&
					!Squad.IsTryingToExecute(P) )
				{
					`AILog("Execute enemy:" @ P);
					GoExecuteEnemy(P);
					return;
				}
			}
		}
	}

Begin:
	//debug
	`AILog( "BEGIN TAG", 'State' );
	CheckExecute();
	SelectEnemy();
	if (Enemy != None && (IsUnderHeavyFire() || IsEnemyVisible(Enemy)))
	{
		FireFromOpen();
	}
	else
	{
		StopFiring();
		// maybe reload since we have nothing else to do right now
		if (FRand() > 0.25 && MyGearPawn != None && MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.AmmoUsedCount > 0)
		{
			MyGearPawn.MyGearWeapon.ForceReload();
		}
	}
	Sleep(0.5);
	Goto('Begin');
}
