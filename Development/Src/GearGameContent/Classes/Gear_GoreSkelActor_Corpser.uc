/**
*	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class Gear_GoreSkelActor_Corpser extends Gear_SkelMeshActor_Gore
	placeable;

var SkeletalMesh	GoreMesh;
var	PhysicsAsset	GorePhysAsset;

/** Used to replicate when corpser is gored */
var	repnotify bool	bGored;

var()	array<name>		GoreBreakList;

/** How much to throw apart giblets */
var()	float			RadialExplosion;

replication
{
	if(Role == ROLE_Authority)
		bGored;
};

/** Handles replicated events (client only) */
simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'bGored')
	{
		GoreStateChanged();
		return;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Handler for kismet action (server only) */
function OnGibCorpser(SeqAct_GibCorpser Action)
{
	if(Role == ROLE_Authority)
	{
		// Set flag to be replicated toclient
		bGored = TRUE;
		GoreStateChanged();
	}
}

/** When gore state changes, this function does the actual goring work (client + server) */
simulated function GoreStateChanged()
{
	local int i, ConstraintIndex, NumMaterials;
	local RB_ConstraintInstance	Constraint;

	if(bGored)
	{
		// Clear MICs
		NumMaterials = SkeletalMeshComponent.SkeletalMesh.Materials.length;
		for(i=0; i<NumMaterials; i++)
		{
			SkeletalMeshComponent.SetMaterial(i, None);
		}

		// Change out physics asset and mesh
		SkeletalMeshComponent.SetPhysicsAsset(None);

		SkeletalMeshComponent.bDisableWarningWhenAnimNotFound = TRUE;
		SkeletalMeshComponent.SetSkeletalMesh(GoreMesh, TRUE);
		SkeletalMeshComponent.bDisableWarningWhenAnimNotFound = FALSE;

		SkeletalMeshComponent.SetBlockRigidBody(TRUE);
		SkeletalMeshComponent.SetRBCollidesWithChannel(RBCC_Default, TRUE);
		SkeletalMeshComponent.SetPhysicsAsset(GorePhysAsset);
		SkeletalMeshComponent.SetHasPhysicsAssetInstance(TRUE);
		SkeletalMeshComponent.PhysicsWeight = 1.0;

		// Change to rigid body - this moves the actor and unfixes all bodies
		SetPhysics(PHYS_RigidBody);

		// Break desired joints
		for(i=0; i<GoreBreakList.length; i++)
		{
			ConstraintIndex = SkeletalMeshComponent.FindConstraintIndex(GoreBreakList[i]);
			if(ConstraintIndex != INDEX_NONE)
			{
				Constraint = SkeletalMeshComponent.PhysicsAssetInstance.Constraints[ConstraintIndex];
				Constraint.TermConstraint();
			}
		}

		// ADd explosion impulse to throw parts away
		SkeletalMeshComponent.AddRadialImpulse(Location, 1000.0, RadialExplosion, RIF_Constant, TRUE);
	}
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_Corpser.Corpser'
		AnimTreeTemplate=AnimTree'Locust_Corpser.Anim.Corpser_Blend_AnimTree_01'
		PhysicsAsset=PhysicsAsset'Locust_Corpser.Corpser_Physics'
	End Object

	GoreMesh=SkeletalMesh'Locust_Corpser.Locust_Corpser_Gore'
	GorePhysAsset=PhysicsAsset'Locust_Corpser.Locust_Corpser_Gore_Physics'

	RemoteRole=ROLE_SimulatedProxy
	bAlwaysRelevant=true

	RadialExplosion=1000.0

	GoreBreakList=("Gore_mech_helmet_1","Gore_mech_helmet_2","Gore_mech_helmet_3","gore_face_1","gore_face_2","gore_face_3","Lft_LowerArm_02","Lft_LowerArm_03","Lft_LowerArm_04","Rt_LowerArm_02","Rt_Claw3","Rt_Claw4","gore_belly_1");

}
