
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGGus extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=147,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Gus'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGGus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGGus"
	NeedsRevivedGUDSEvent=GUDEvent_GusNeedsRevived
	WentDownGUDSEvent=GUDEvent_GusWentDown
	FAS_ChatterNames.Add("COG_Gus.FaceFX.Gus_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Gus.FaceFX.Gus_FaceFX_Efforts'

// AI CANNOT BE GORED IN SP - SAVES MEMORY
    GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()


	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Gus.COG_Gus'
		PhysicsAsset=PhysicsAsset'COG_Gus.COG_Gus_Physics'
	End Object
}
