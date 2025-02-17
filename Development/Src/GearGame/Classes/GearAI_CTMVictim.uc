/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAI_CTMVictim extends GearAI
	config(AI);

function bool CanExecutePawn( GearPawn GP )
{
	// restrict meatflag to executing players to whom he has line of sight
	return (Super.CanExecutePawn(GP) && LineOfSightTo(GP));
}

state Action_Idle
{
Begin:

	// Select our weapon
	if( SelectWeapon() )
	{
		`AILog("Selected weapon");
		Sleep( 0.25f );
	}
	// If we have enemies
	if( HasAnyEnemies() )
	{
		// Try to notice one which will put us into combat
		CheckNoticedEnemy();
		Sleep(1.f);
	}
	else
	{
		if (FRand() > 0.6f)
		{
			SetFocalPoint(Pawn.Location + vector(Pawn.Rotation + (rot(0,1,0) * RandRange(-16384,16384))) * 256.f,false);
			`AILog("bored"@`showvar(GetBasedPosition(FocalPosition)));
			FinishRotation();
			Sleep(RandRange(2.f,3.f));
		}
		else
		{
			Sleep(1.f);
		}
	}
	Goto('Begin');
}
defaultproperties
{
	DefaultCommand=class'AICmd_Base_CTMVictim'

	// remove it from the class list so we can mess with it
	DefaultReactConditionClasses.Remove(class'AIReactCond_Stumble')

	Begin Object Class=AIReactCond_Stumble Name=DamageStumble0
		bIgnoreLegShotStumbles=true
	End Object
	DefaultReactConditions.Add(DamageStumble0)
}
