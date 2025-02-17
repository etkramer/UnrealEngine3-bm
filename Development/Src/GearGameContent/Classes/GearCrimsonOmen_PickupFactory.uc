/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearCrimsonOmen_PickupFactory extends GearCrimsonOmenPickupFactoryBase;


defaultproperties
{
	TickGroup=TG_PreAsyncWork

	bBlocked=TRUE		// do not use to create paths by default
	bNotBased=TRUE
	bDestinationOnly=TRUE
	bOnlyReplicateHidden=FALSE

	bNoDelete=TRUE

	TagPickupSound=SoundCue'Interface_Audio.Objectives.CogTagsCue'
	bDrawNoSymbolIfNeeded=FALSE

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_Tags.COG_Tags_MS_SMesh'
		bCastDynamicShadow=FALSE
		bOwnerNoSee=TRUE
		CollideActors=FALSE
		Scale=1.0
		Translation=(Z=-15)
	End Object
	PickupMesh=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	PickupAction={(
			ActionName=COGTagIcon,
			ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
								(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=348,UL=68,VL=75)))	),
			)}

	TagID=COGTAG_None
}
