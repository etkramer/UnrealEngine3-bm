/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickup_Vest extends UTArmorPickupFactory;

/** CanUseShield()
returns how many shield units P could use
*/
function int CanUseShield(UTPawn P)
{
	return Max(0,ShieldAmount - P.VestArmor);
}

/** AddShieldStrength()
add shield to appropriate P armor type.
*/
function AddShieldStrength(UTPawn P)
{
	P.VestArmor = Max(ShieldAmount, P.VestArmor);
}

defaultproperties
{
	ShieldAmount=50
	bIsSuperItem=false
	RespawnTime=30.000000
	MaxDesireability=1.000000
	PickupSound=SoundCue'A_Pickups.Armor.Cue.A_Pickups_Armor_Chest_Cue'

	Begin Object Name=ArmorPickUpComp
		StaticMesh=StaticMesh'Pickups.Armor.Mesh.S_Pickups_Armor'
		Translation=(X=0.0,Y=0.0,Z=-50.0)
	End Object

}
