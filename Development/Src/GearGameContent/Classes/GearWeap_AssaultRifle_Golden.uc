/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_AssaultRifle_Golden extends GearWeap_AssaultRifle
	notplaceable;

/** The MIC to use for the golden magazine **/
var MaterialInstanceConstant MIC_Golden;

// overridden to make magazine golden
simulated function KActorSpawnable SpawnPhysicsMagazine(Vector SpawnLoc, Rotator SpawnRot)
{
	local KActorSpawnable TheMag;
	local MaterialInstanceConstant NewMIC;

	TheMag = Super.SpawnPhysicsMagazine( SpawnLoc, SpawnRot );

	NewMIC = new(TheMag) class'MaterialInstanceConstant';
	NewMIC.SetParent( MIC_Golden );
	TheMag.StaticMeshComponent.SetMaterial( 0, NewMIC );

	return TheMag;
}



defaultproperties
{
	Begin Object Name=WeaponMesh
		Materials(0)=MaterialInstanceConstant'ALL_WeaponShaders.SinglePlayer.COG_WeaponShader_Lancer_Golden_SP'
	End Object

	Begin Object Name=MagazineMesh0
		Materials(0)=MaterialInstanceConstant'ALL_WeaponShaders.SinglePlayer.COG_WeaponShader_Lancer_Golden_SP'
	End Object

	MIC_Golden=MaterialInstanceConstant'ALL_WeaponShaders.SinglePlayer.COG_WeaponShader_Lancer_Golden_SP'

}
