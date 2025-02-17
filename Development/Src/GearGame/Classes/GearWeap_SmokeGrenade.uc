/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_SmokeGrenade extends GearWeap_GrenadeBase;

var MeshComponent PickupMeshComp;

function GearProj_Grenade ThrowGrenade()
{
	if (GearGame(WorldInfo.Game) != None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_ThrowingSmokeGrenade, Instigator);
	}
	return super.ThrowGrenade();
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.GrenadeSmoke;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.GrenadeSmoke;
}
*/

static function PrimitiveComponent GetPickupMesh(PickupFactory Factory)
{
	return default.PickupMeshComp;
}


/** This will return the COG Emissive Color for this weapon for this map **/
// smokes are always this color
static simulated function LinearColor GetWeaponEmisColor_COG( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 1;
	LC.G = 3;
	LC.B = 8;
	LC.A = 1;

	return LC;
}

/** This will return the Locust Emissive Color for this weapon for this map **/
// smokes are always this color
static simulated function LinearColor GetWeaponEmisColor_Locust( WorldInfo TheWorldInfo, EWeaponClass WeapID )
{
	local LinearColor LC;

	LC.R = 1;
	LC.G = 3;
	LC.B = 8;
	LC.A = 1;

	return LC;
}


defaultproperties
{
	AIRating=-1.1
	DamageTypeClassForUI=class'GDT_SmokeGrenade'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=192,V=240,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=339,UL=128,VL=23)

	WeaponProjectiles(0)=class'GearProj_SmokeGrenade'
	InstantHitDamageTypes(0)=None
	AmmoTypeClass=class'GearAmmoType_GrenadeSmoke'

	ThrowGrenadeSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeFireCue'
	GrenadeSwingSound=SoundCue'Weapon_Grenade.SmokeGrenade.SmokeGrenadeSwingCue'

// 	// Weapon Mesh Transform
// 	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
// 		StaticMesh=StaticMesh'COG_Frag_Grenade.Mesh.COG_Grunt_FragGrenade_SM'
// 		CollideActors=false
// 		BlockActors=false
// 		BlockZeroExtent=false
// 		BlockNonZeroExtent=false
// 		BlockRigidBody=false
// 		Translation=(X=-9.0,Y=3.0,Z=3.50)
// 		Scale=0.67
// 		bCastDynamicShadow=FALSE
//     End Object
// 	Mesh=StaticMeshComponent0
// 	DroppedPickupMesh=StaticMeshComponent0
//

    // Weapon Mesh
	Begin Object Name=WeaponMesh
	    SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Smoke_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Smoke_Grenade_Physics'
		AnimTreeTemplate=AnimTree'COG_Bolo_Grenade.Animations.AT_BoloGrenade'
		AnimSets(0)=AnimSet'COG_Bolo_Grenade.GrenadeAnims'
	End Object
	Mesh=WeaponMesh

	Begin Object Class=SkeletalMeshComponent Name=WeaponPickMesh
	    SkeletalMesh=SkeletalMesh'COG_Bolo_Grenade.Smoke_Grenade'
		PhysicsAsset=PhysicsAsset'COG_Bolo_Grenade.Smoke_Grenade_Physics'
		Rotation=(Pitch=16384)
		Translation=(Z=-20)
	End Object

// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh1
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.COG_BoloGrenade_Pickup1'
// 		CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object
//
// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh2
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.COG_BoloGrenade_Pickup2'
// 		CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object
//
// 	Begin Object Class=StaticMeshComponent Name=GrenadeMesh3
// 	    StaticMesh=StaticMesh'COG_Bolo_Grenade.COG_BoloGrenade_Pickup3'
// 		CollideActors=FALSE
// 		bCastDynamicShadow=TRUE
// 		Scale=1.0f
// 		Translation=(Z=-20)
// 	End Object

	DroppedPickupMesh=WeaponPickMesh

	Begin Object Class=StaticMeshComponent Name=PickupMesh0
		StaticMesh=StaticMesh'COG_Bolo_Grenade.COG_BoloGrenade_Pickup2'
		Translation=(Z=-20)
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
	End Object
	PickupMeshComp=PickupMesh0

	LC_EmisDefaultCOG=(R=1.0,G=3.0,B=8.0,A=1.0)
	LC_EmisDefaultLocust=(R=15.0,G=2.0,B=0.0,A=1.0)

	WeaponEquipSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeUnStowCue'
	WeaponDeEquipSound=SoundCue'Weapon_Grenade.InkGrenade.InkGrenadeStowCue'

	WeaponID=WC_SmokeGrenade
}
