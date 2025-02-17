/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDiscoverables_PickupFactory extends GearDiscoverablesPickupFactoryBase;


defaultproperties
{
	bBlocked=TRUE		// do not use to create paths by default
	bNotBased=TRUE
	bDestinationOnly=FALSE
	bOnlyReplicateHidden=FALSE
	bNoDelete=TRUE
	RemoteRole=ROLE_None
	bBlockActors=false

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_Tags.COG_Tags_MS_SMesh'
		bCastDynamicShadow=FALSE
		bOwnerNoSee=TRUE
		CollideActors=FALSE
		Scale=1.0
		Translation=(Z=-15)
		LightEnvironment=PickupLightEnvironment
	End Object
	PickupMesh=StaticMeshComponent0
	DISC_MeshComponent=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	bDrawNoSymbolIfNeeded=FALSE

	DISC_DiscoverableType=eDISC_None
	DISC_PickupType=eGDPT_Ground

	PickupAction={(
			ActionName=DiscoverIcon,
			ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=348,UL=68,VL=75)))	),
			)}
}
