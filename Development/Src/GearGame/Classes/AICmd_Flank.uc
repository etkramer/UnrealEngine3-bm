/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class AICmd_Flank extends AICommand
	within GearAI;


/** points adjacent to the direct route to enemy. We add cost to these points when looking for a flank path so that
 * the AI will prefer going a different route
 */
var array<NavigationPoint> AvoidPoints;

auto state TryToFlank
{
	final function CreateAvoidList()
	{
		local int PathIdx;
		local NavigationPoint CurrentNode;

		AvoidPoints = RouteCache;
		foreach RouteCache(CurrentNode)
		{
			for (PathIdx = 0; PathIdx < CurrentNode.PathList.length; PathIdx++)
			{
				if ( CurrentNode.PathList[PathIdx].End.Actor != None &&
					AvoidPoints.Find(NavigationPoint(CurrentNode.PathList[PathIdx].End.Actor)) == INDEX_NONE )
				{
					AvoidPoints.AddItem(NavigationPoint(CurrentNode.PathList[PathIdx].End.Actor));
				}
			}
		}
	}

	/** adds TransientCost to all avoid points so the AI will only use them if it must */
	final function AddCostToAvoidPoints()
	{
		local NavigationPoint CurrentNode;

		foreach AvoidPoints(CurrentNode)
		{
			CurrentNode.TransientCost = 50000;
		}
	}

	/** @return whether an enemy is exposed to the AI (flanked, not simply popping up from cover) */
	final function bool IsEnemyExposed()
	{
		local Pawn			P;
		local bool			bTryToClose;
		local GearWeapon	MyGearWeapon;

		MyGearWeapon = GearWeapon(Pawn.Weapon);
		// try to get into best effective range of weapon unless being shot at
		if( MyGearWeapon == None || !MyGearWeapon.bSniping )
		{
			bTryToClose = !IsUnderHeavyFire();
		}

		foreach Squad.AllEnemies(class'Pawn', P)
		{
			// trace directly to center; if we can hit that, enemy is pretty well exposed to us
			if ( (!bTryToClose || IsMediumRange(P.Location)) &&
				FastTrace(P.Location, Pawn.GetWeaponStartTraceLocation()) )
			{
				`AILog(P @ "is exposed to me");
				return TRUE;
			}
		}

		return FALSE;
	}

	/** this is where we do the actual pathfinding after adding the avoid costs and such */
	final function Actor GenerateAlternatePath()
	{
		class'Path_AvoidFireFromCover'.static.AvoidFireFromCover( Pawn );
		class'Goal_AtActor'.static.AtActor(Pawn, Enemy, 0.0, false);
		return FindPathToward(Enemy);
	}

Begin:
	`AILog("Attempting to flank enemy" @ Enemy);
	// we need a valid enemy for this command
	if (!HasValidEnemy())
	{
		`AILog("No enemy");
		GotoState('DelayFailure');
	}
	// construct the avoid list from the direct route to the enemy
	if (GeneratePathTo(Enemy) == None)
	{
		// failed to find path
		`AILog("No path");
		GotoState('DelayFailure');
	}
	Sleep(0.0); // sleep a tick to spread out work
	CreateAvoidList();
	Sleep(0.0); // and again
FlankStart:
	if (!HasValidEnemy())
	{
		`AILog("Lost enemy");
		Status = 'Failure';
		PopCommand(self);
	}
	AddCostToAvoidPoints();
	if (GenerateAlternatePath() == None)
	{
		`AILog("Couldn't find alternate path to enemy");
		Status = 'Failure';
		PopCommand(self);
	}
	// roadie run unless can see enemy or almost at enemy
	bShouldRoadieRun = GetDifficultyLevel() > DL_Casual && RouteCache.length > 5 && !IsUnderHeavyFire() &&
				!LineOfSightTo((FireTarget != None) ? FireTarget : Enemy);
	// go in steps so we can re-evaluate our situation
	RouteCache.length = Min(RouteCache.length, 3);
	SetMoveGoal(Enemy,, true,, true, false);
	CheckInterruptCombatTransitions();
	if (IsEnemyExposed())
	{
		`AILog("Successfully flanked enemy");
		if (!IsUnderHeavyFire() && GetNearEnemyDistance(Pawn.Location) > EnemyDistance_Melee)
		{
			`AILog("- shoot at enemy from here");
			FireFromOpen(-1);
		}
		Status = 'Success';
		PopCommand(self);
	}
	Goto('FlankStart');
}
