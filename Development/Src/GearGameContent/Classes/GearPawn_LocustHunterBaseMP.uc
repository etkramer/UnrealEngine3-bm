
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustHunterBaseMP extends GearPawn_LocustBaseMP
	abstract
	config(Pawn);

defaultproperties
{
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDrone'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustGrenadierMP"
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Grunt.FaceFX.Drone_Efforts'

	PhysHRMotorStrength=(X=5000,Y=0)

	Begin Object Name=CollisionCylinder
	    CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object
	MeatShieldMorphTargetName="Meatshield_Morph"
}

