/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor_LeviathanTentacle_Base extends SkeletalMeshActor
	abstract
	notplaceable;

/** Used for replicating state of tentacle */
var	repnotify bool bIsBroken;

/** Bones/bodies to set to physics when broken */
var array<name> BreakBodyNames;

/** Impulse to apply to pieces when arm explodes */
var()	float	GoreExplodeForce;

/** Name of socket to attach grenade to when tagging */
var()	name	GrenadeSocketName;

replication
{
	if(Role == ROLE_Authority)
		bIsBroken;
}

simulated function ReplicatedEvent(name VarName )
{
	if(VarName == 'bIsBroken')
	{
		if(bIsBroken)
		{
			BreakTentacle();
		}
		else
		{
			RestoreTentacle();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Break off end of tentacle - set to physics and set rigid weights */
simulated function BreakTentacle()
{
	local int i, ConstraintIndex;
	local RB_ConstraintInstance	Constraint;

	// First unfix (make physics) all desired bodies
	SkeletalMeshComponent.PhysicsAssetInstance.SetNamedBodiesFixed(FALSE, BreakBodyNames, SkeletalMeshComponent, TRUE, FALSE);

	// Iterate over bodies
	for(i=0; i<BreakBodyNames.length; i++)
	{
		SkeletalMeshComponent.AddInstanceVertexWeightBoneParented(BreakBodyNames[i]);

		// Hack to skip 2-part gore bit
		if(BreakBodyNames[i] == 'b_Tentacle_13')
		{
			continue;
		}

		// Find Constraint and 
		ConstraintIndex = SkeletalMeshComponent.FindConstraintIndex(BreakBodyNames[i]);
		if( ConstraintIndex != INDEX_NONE )
		{
			Constraint = SkeletalMeshComponent.PhysicsAssetInstance.Constraints[ConstraintIndex];
			if(!Constraint.bTerminated)
			{
				// ..break Constraint
				Constraint.TermConstraint();
			}
		}
	}

	// Set some velocity for bits
	SkeletalMeshComponent.AddRadialImpulse(SkeletalMeshComponent.GetBoneLocation('b_Tentacle_11'), 10000.0, GoreExplodeForce, RIF_Constant, TRUE);

	SkeletalMeshComponent.ToggleInstanceVertexWeights(TRUE);
}

/** Restore tentacle, turn off physics and rigid weighting */
simulated function RestoreTentacle()
{
	SkeletalMeshComponent.PhysicsAssetInstance.SetNamedBodiesFixed(TRUE, BreakBodyNames, SkeletalMeshComponent, FALSE, FALSE);

	SkeletalMeshComponent.ToggleInstanceVertexWeights(FALSE);

	SkeletalMeshComponent.UpdateRBBonesFromSpaceBases(FALSE, TRUE);
}

/** Handle toggle action - used for breaking/unbreaking tentacle */
simulated function OnToggle(SeqAct_Toggle action)
{
	if (action.InputLinks[0].bHasImpulse)
	{
		if(!bIsBroken)
		{
			bIsBroken = TRUE;
			BreakTentacle();
		}
	}
	else if (action.InputLinks[1].bHasImpulse)
	{
		if(bIsBroken)
		{
			bIsBroken = FALSE;
			RestoreTentacle();
		}
	}
}

/** Called to attach the grenade to the tentacle */
simulated function AttachGrenadeToTentacle(GearProj_Grenade Grenade)
{
	Grenade.SetBase(self,, SkeletalMeshComponent, GrenadeSocketName);
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		bHasPhysicsAssetInstance=TRUE
		PhysicsWeight=1.0
	End Object

	GoreExplodeForce=500.0

	//BreakBodyNames=("b_Tentacle_GibPiece_01","b_Tentacle_GibPiece_02","b_Tentacle_GibPiece_03","b_Tentacle_GibPiece_04","b_Tentacle_GibPiece_05","b_Tentacle_11","b_Tentacle_12","b_Tentacle_13","b_Tentacle_14","b_Tentacle_15")
	BreakBodyNames=("b_Tentacle_GibPiece_04","b_Tentacle_GibPiece_05","b_Tentacle_15")
	GrenadeSocketName="GrenadeSocket"

	Tag="DeckTentacle"
	Physics=PHYS_Interpolating
	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=TRUE
}