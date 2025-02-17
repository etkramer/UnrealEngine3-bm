/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickup_Helmet extends UTArmorPickupFactory;

/** CanUseShield()
returns how many shield units P could use
*/
function int CanUseShield(UTPawn P)
{
	return Max(0,ShieldAmount - P.HelmetArmor);
}

/** AddShieldStrength()
add shield to appropriate P armor type.
*/
function AddShieldStrength(UTPawn P)
{
	P.HelmetArmor = Max(ShieldAmount, P.HelmetArmor);
}

defaultproperties
{
	ShieldAmount=20
	bIsSuperItem=false
	RespawnTime=30.000000
	MaxDesireability=1.000000
	PickupSound=SoundCue'A_Pickups.Armor.Cue.A_Pickups_Armor_Helmet_Cue'

	Begin Object Name=ArmorPickUpComp
	    StaticMesh=StaticMesh'Pickups.Armor_Helmet.Mesh.S_UN_Pickups_Helmet'
		Scale3D=(X=2.0,Y=2.0,Z=2.0)
	End Object
}
