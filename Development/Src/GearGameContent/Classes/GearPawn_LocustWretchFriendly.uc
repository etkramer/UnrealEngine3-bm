
/**
 * Friendly Wretch
 * Test for AnimSynch system
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustWretchFriendly extends GearPawn_LocustWretch;

function JumpOffPawn()
{
	ScriptTrace();
	Super.JumpOffPawn();
}


/** 
 * State Thrown.
 * Wretch is put in this state when grabbed by Marcus and then thrown!
 */
state GRABTEST_Thrown
{
	simulated event HitWall(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
	{
		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ Wall);
		GRABTEST_ThownCollision();
	}

	simulated event Landed(vector HitNormal, actor FloorActor )
	{
		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ FloorActor);
		GRABTEST_ThownCollision();
	}
	
	simulated event Bump(Actor Other, PrimitiveComponent OtherComp, Vector HitNormal)
	{
		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ Other);
		
		// Don't collide with Leader throwing us.	
		if( Other != InteractionPawn )
		{
			GRABTEST_ThownCollision();
		}
	}
}

simulated function GRABTEST_ThownCollision()
{
	// Play falling down in rag doll.
	PlayFallDown();
	GotoState('GRABTEST_FallingDownInRagDoll');
}

state GRABTEST_FallingDownInRagDoll
{
Begin:
	sleep(0.2f);
	
	// Wait until body comes to rest
	if( VSize(Velocity) > 1.f )
	{
		goto('Begin');
	}

	Sleep(0.2f);

	// Wait until body comes to rest
	if( VSize(Velocity) > 1.f )
	{
		goto('Begin');
	}

	// recover from ragdoll pose
	ServerDoSpecialMove(SM_RecoverFromRagdoll, TRUE);
	GotoState('');
}

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=+34.0
		CollisionHeight=+36.4	// Scaled down by 30%
	End Object

	Begin Object Name=GearPawnMesh
		Scale=0.7f	// friendly wretch is smaller so it can be grabbed.
	End Object

	// Pawn to Pawn Interactions
	SpecialMoveClasses(SM_GrabWretchFollower)	=class'GSM_GrabWretchFollower'
}