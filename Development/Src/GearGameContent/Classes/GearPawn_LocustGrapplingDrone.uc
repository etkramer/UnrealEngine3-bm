/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustGrapplingDrone extends GearPawn_LocustDrone;

var GearInv_GrapplingHook	GrapplingHook;

function bool CanUseGrapplingHook()
{
	return TRUE;
}

simulated function CreateGrapplingHook(Vector MarkerLocation, Rotator MarkerRotation)
{
	// Destroy previous one if it exists.
	DestroyGrapplingHook();

	GrapplingHook = Spawn(class'GearInv_GrapplingHook', Self,, MarkerLocation, MarkerRotation);
	GrapplingHook.SetHardAttach(TRUE);
	GrapplingHook.SetBase(Base);
}

simulated function DestroyGrapplingHook()
{
	if( GrapplingHook != None )
	{
		// detect the "fall" damage type and simulate it on clients
		if (Role < ROLE_Authority && HitDamageType == class'GDT_FallOffGrappleRope')
		{
			GrapplingHook.KnockOff(None);
		}
		// if it's ragdolled, let it die on it's own
		if (GrapplingHook.Physics != PHYS_RigidBody)
		{
			GrapplingHook.Destroy();
		}
		GrapplingHook = None;
	}
}

simulated function GearAnim_Slot GetGrapplingHookAnimSlot()
{
	return GearAnim_Slot(GrapplingHook.GrapplingHookMesh.FindAnimNode('MainSlot'));
}

defaultproperties
{
	DurationBeforeDestroyingDueToNotBeingSeen=5.0f

	HelmetType=class'Item_Helmet_LocustMiniGunner'
	ShoulderPadLeftType=class'Item_ShoulderPad_LocustThreeCylinders'
	ShoulderPadRightType=class'Item_ShoulderPad_LocustThreeCylinders'

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_LocustAssaultRifle'

	Begin Object Name=GearPawnMesh
		AnimSets.Add(AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Grapple')
	End Object
}
