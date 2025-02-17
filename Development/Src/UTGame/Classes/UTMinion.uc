/** this volume automatically crouches console players as there's no manual crouch on the console controls  
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 FIXME NOT EXTENDING UTPAWN - instead, common base class UTAnimatedPawn
 */
class UTMinion extends UTPawn
	native(AI);

var byte TeamNum;

cpptext
{
	virtual void physWalking(FLOAT deltaTime, INT Iterations);
	virtual UBOOL moveToward(const FVector &Dest, AActor *GoalActor);
	virtual void stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
}

simulated native function byte GetTeamNum();

native function GetBoundingCylinder(out float CollisionRadius, out float CollisionHeight) const;

function PossessedBy(Controller C, bool bVehicleTransition)
{
	Super.PossessedBy(C, bVehicleTransition);
	TeamNum = C.GetTeamNum();
}

function RestartMinion(NavigationPoint StartSpot)
{
	// initialize and start it up
	SetLocation(startspot.Location);
	SetAnchor(startSpot);

	//Possess(Pawn, false);
	Controller.RouteGoal = None;
	PlayTeleportEffect(true, true);
	SetMovementPhysics();
	if (Physics == PHYS_Walking)
		SetPhysics(PHYS_Falling);

	GotoState('auto');
}



native function RestorePreRagdollCollisionComponent();

State Dying
{
	event Timer()
	{
		Health = 100;

		// recover from Ragdoll
		StartFeignDeathRecoveryAnim();

		// FIXME - fast version of FindPlayerStart
		RestartMinion(WorldInfo.Game.FindPlayerStart(Controller, TeamNum));
	}

	simulated event BeginState(Name PreviousStateName)
	{
		local Actor A;
		local array<SequenceEvent> TouchEvents;
		local int i;

		SetDyingPhysics();

		SetCollision(true, false);

		foreach TouchingActors(class'Actor', A)
		{
			if (A.FindEventsOfClass(class'SeqEvent_Touch', TouchEvents))
			{
				for (i = 0; i < TouchEvents.length; i++)
				{
					SeqEvent_Touch(TouchEvents[i]).NotifyTouchingPawnDied(self);
				}
				// clear array for next iteration
				TouchEvents.length = 0;
			}
		}
		foreach BasedActors(class'Actor', A)
		{
			A.PawnBaseDied();
		}
		SetTimer(5.0, false);
	}
}

// FIXME - foot placement?  only when totally stopped (moving and turning)

defaultproperties
{
	bCanPickupInventory=false
	bCanCrouch=false
	bMuffledHearing=false
	bCanStrafe=false
	bEnableFootPlacement=false

	Begin Object Name=WPawnSkeletalMeshComponent
		SkeletalMesh=SkeletalMesh'CH_Minion.Mesh.SK_CH_Minion'
		AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'
		PhysicsAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseMale_Physics'
		AnimTreeTemplate=AnimTree'CH_Minion.AT_CH_Minion'
	End Object

	Health=50
	CurrCharClassInfo=class'UTGame.UTFamilyInfo_Human_Male'
}
