/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGBenjaminCarmine extends GearPawn_COGGear
	config(Pawn);

/** redshirt helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=294,V=0,UL=53,VL=63)
	HelmetType=class'Item_Helmet_COGTwo'

	ControllerClass=class'GearAI_Dom'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGCarmine'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGCarmine"
	NeedsRevivedGUDSEvent=GUDEvent_CarmineNeedsRevived
	WentDownGUDSEvent=GUDEvent_CarmineWentDown

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
