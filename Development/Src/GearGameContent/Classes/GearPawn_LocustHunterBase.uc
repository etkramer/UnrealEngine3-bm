/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_LocustHunterBase extends GearPawn_LocustBase
	abstract
	config(Pawn);

/** This is the percentage in which to drop grenades on death **/
var config float PercentToDropGrenades;

simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'b_MF_Spine_03' );
	BreakConstraint( vect(0,100,0), vect(0,0,10), 'b_MF_UpperArm_R' );
}
	 
// hunters always drop a grenade
function DropExtraWeapons( class<DamageType> DamageType )
{
	if( ( bAllowInventoryDrops == TRUE ) && ( FRand() <= PercentToDropGrenades ) )
	{
		DropExtraWeapons_Worker( DamageType, class'GearGame.GearWeap_FragGrenade' );
	}
}



defaultproperties
{
	DefaultInventory.Empty()

	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=196,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Locust'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDrone'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustGrenadier"
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Grunt.FaceFX.Drone_Efforts'

	PhysHRMotorStrength=(X=5000,Y=0)

	Begin Object Name=CollisionCylinder
	    CollisionRadius=+0030.000000
		CollisionHeight=+0072.000000
	End Object

	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_MantleOverLocust'

	NoticedGUDSPriority=40
	NoticedGUDSEvent=GUDEvent_NoticedHunter

	MeatShieldMorphTargetName="Meatshield_Morph"
}

