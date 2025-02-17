/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTArmorPickup_ShieldBelt extends UTArmorPickupFactory;

var class<UTDroppedItemPickup> DroppedPickupClass;

/**
* CanUseShield() returns how many shield units P could use
*
* @Returns returns how many shield units P could use
*/
function int CanUseShield(UTPawn P)
{
	return Max(0,ShieldAmount - P.ShieldBeltArmor);
}

/**
* AddShieldStrength() add shield to appropriate P armor type.
*
* @Param	P 	The UTPawn to give shields to
*/
function AddShieldStrength(UTPawn P)
{
	local MaterialInterface ShieldMat;

	// Get the proper shield material
	ShieldMat = P.GetShieldMaterialInstance(WorldInfo.Game.bTeamGame);

	// Assign it
	P.ShieldBeltArmor = Max(ShieldAmount, P.ShieldBeltArmor);
	P.ShieldBeltPickupClass = DroppedPickupClass;
	if (P.GetOverlayMaterial() == None)
	{
		P.SetOverlayMaterial(ShieldMat);
	}
}



defaultproperties
{
	ShieldAmount=100
	bIsSuperItem=true
	RespawnTime=60.000000
	MaxDesireability=1.500000
	PickupStatName=PICKUPS_SHIELDBELT
	PickupSound=SoundCue'A_Pickups.Shieldbelt.Cue.A_Pickups_Shieldbelt_Activate_Cue'

	Begin Object Name=ArmorPickUpComp
	    StaticMesh=StaticMesh'Pickups.Armor_ShieldBelt.Mesh.S_UN_Pickups_Shield_Belt'
		Scale3D=(X=1.5,Y=1.5,Z=1.5)
	End Object

	bHasLocationSpeech=true
	LocationSpeech(0)=SoundNodeWave'A_Character_IGMale.BotStatus.A_BotStatus_IGMale_HeadingForTheShieldBelt'
	LocationSpeech(1)=SoundNodeWave'A_Character_Jester.BotStatus.A_BotStatus_Jester_HeadingForTheShieldBelt'
	LocationSpeech(2)=SoundNodeWave'A_Character_Othello.BotStatus.A_BotStatus_Othello_HeadingForTheShieldBelt'

	DroppedPickupClass=class'UTDroppedShieldBelt'
}
