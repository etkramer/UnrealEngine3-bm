
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustDrone extends GearPawn_LocustDroneBase
	config(Pawn);

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,4,-2);
}

defaultproperties
{
	DefaultInventory(0)=class'GearGame.GearWeap_LocustAssaultRifle'
	DefaultInventory(1)=class'GearGame.GearWeap_LocustPistol'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=343,V=0,UL=48,VL=63)

	HelmetType=class'Item_Helmet_LocustDroneRandom'

	ControllerClass=class'GearAI_Locust'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDrone'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustDrone"
	MasterGUDBankClassNames(1)="GearGameContent.GUDData_LocustDroneB"
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Grunt.FaceFX.Drone_Efforts'

	PhysHRMotorStrength=(X=200,Y=0)

	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_MantleOverLocust'

	NoticedGUDSEvent=GUDEvent_NoticedDrone
	NoticedGUDSPriority=20

}
