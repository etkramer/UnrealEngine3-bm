/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/

class GDO_PropaneTank_Locust extends GDO_PropaneTank
	config(Weapon);

defaultproperties
{
	Begin Object Name=StaticMeshComponent_Tank
		StaticMesh=StaticMesh'GOW_Explodables.SM.GOW_Explodables_Locust_Barrel02'
	End Object

	DestroyedStaticMesh=StaticMesh'GOW_Explodables.SM.GOW_Explodables_Locust_Barrel02_Destroyed'
}