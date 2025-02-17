/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGDizzy extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=348,V=65,UL=57,VL=68)
	HelmetType=class'Item_Helmet_COGDizzyCowboyHat'

	ControllerClass=class'GearAI_Baird'
	// Dizzy's very tightly scripted in SP, let's try it with no efforts or GUDS
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_Empty'
	//MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGDizzy"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown

// AI CANNOT BE GORED IN SP - SAVES MEMORY
    GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Dizzy.COG_Dizzy'
		PhysicsAsset=PhysicsAsset'COG_Dizzy.COG_Dizzy_Physics'
	End Object
}
