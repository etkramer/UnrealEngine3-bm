
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGNilesSamson extends GearPawn_COGGear
	config(Pawn);

/** redshirt helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=49,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_COGThree'

	ControllerClass=class'GearAI_Dom'

// AI CANNOT BE GORED IN SP - SAVES MEMORY
    GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'COG_Minh.Minh_Kim'
		PhysicsAsset=PhysicsAsset'COG_Minh.Minh_Kim_Physics'
	End Object
}
