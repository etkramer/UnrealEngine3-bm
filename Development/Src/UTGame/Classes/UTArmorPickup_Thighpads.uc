/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickup_Thighpads extends UTArmorPickupFactory;

/** CanUseShield()
returns how many shield units P could use
*/
function int CanUseShield(UTPawn P)
{
	return Max(0,ShieldAmount - P.ThighpadArmor);
}

/** AddShieldStrength()
add shield to appropriate P armor type.
*/
function AddShieldStrength(UTPawn P)
{
	P.ThighpadArmor = Max(ShieldAmount, P.ThighpadArmor);
}

defaultproperties
{
	ShieldAmount=30
	bIsSuperItem=false
	RespawnTime=30.000000
	MaxDesireability=1.000000
	PickupSound=SoundCue'A_Pickups.Armor.Cue.A_Pickups_Armor_Thighpads_Cue'

	Begin Object Name=ArmorPickUpComp
		StaticMesh=StaticMesh'Pickups.ThighPads.Mesh.S_Pickups_ThighPads'
		Scale3D=(X=3.0,Y=3.0,Z=3.0)
		Translation=(Z=-30.0)
	End Object
}
