
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustReaverPassenger extends GearPawn_LocustDroneBase
	config(Pawn);

function bool Died(Controller Killer, class<DamageType> GearDamageType, vector HitLocation)
{
	local Vehicle_Reaver_Base Reaver;
	local bool bDied;
	
	bCollideWorld = TRUE;

	Reaver = Vehicle_Reaver_Base(Base);

	bDied = Super.Died( Killer, GearDamageType, HitLocation ); 

	if(Reaver != None && bDied)
	{
		Reaver.GunnerDied(self);
	}

	return bDied;
}

simulated function UpdateMeshBoneControllers(float DeltaTime)
{
	Super.UpdateMeshBoneControllers(DeltaTime);

	// If not doing a mirror transition, see if Pawn should be aiming left or right.
	// And trigger proper mirror transition.
	if( !bDoingMirrorTransition )
	{
		if( AimOffsetPct.X < 0 && !bWantsToBeMirrored )
		{
			SetMirroredSide(TRUE);
		}
		else if( AimOffsetPct.X >= 0 && bWantsToBeMirrored )
		{
			SetMirroredSide(FALSE);
		}
	}
}

function SetMovementPhysics();

/**
 * Never turn on left hand IK. Drone is carrying weapon one handed.
 */
simulated function bool ShouldLeftHandIKBeOn()
{
	return FALSE;
}

/** just explode when shot by centaur using particles */
simulated function bool ShouldUseSimpleEffectDeath(class<GearDamageType> GearDamageType)
{
	if( ClassIsChildOf(GearDamageType, class'GearGame.GDT_RocketCannon') ||
		ClassIsChildOf(GearDamageType, class'GearGame.GDT_ReaverCannonCheap'))
	{
		return TRUE;
	}
	else
	{
		return Super.ShouldUseSimpleEffectDeath(GearDamageType);
	}
}

defaultproperties
{
	Begin Object Name=GearPawnMesh
		AnimTreeTemplate=AnimTree'Locust_Reaver_Anim.AT_Locust_ReaverPassenger'
		AnimSets.Empty
		AnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel'
		AnimSets(1)=AnimSet'Locust_Grunt.Locust_Grunt_OnReaver'
		//Translation=(Z=0)
	End Object

	SpecialMoveClasses(SM_DeathAnim)=class'GSM_DeathAnimFallFromBeast'
	SpecialMoveClasses(SM_DeathAnimFire)=None

	bCollideWorld=FALSE
	bAllowInventoryDrops=FALSE
	bEnableEncroachCheckOnRagdoll=TRUE
	bSimulateGravity=FALSE
	bRespondToExplosions=FALSE

	bNoDeathGUDSEvent=TRUE
	NoticedGUDSEvent=GUDEvent_None
}
