/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_DarkWretchGib extends KActorSpawnable;

var Emitter FX;

auto state TimeTilDestroySelf
{

begin:
	SetHidden(false);
	sleep( 1 );
	sleep( Rand(2) );

	FX = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Locust_Wretch.Effects.P_Wretch__InitExplo', Location, Rotation );
	FX.SetDrawScale( 2.0 );
	FX.ParticleSystemComponent.ActivateSystem();
	// @msew fix me PlaySound( SoundCue'Weapons.LocustPistolFire01Cue' );

//	sleep( 0.1f );
	Destroy();
}




defaultproperties
{
	Begin Object Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'Neutral_Stranded_01.Gore_Skull1_CP_SMesh'
    	BlockNonZeroExtent=False
		Materials(0)=Material'Locust_Wretch.Locust_WretchD_LP_Mat_Glow_Gore'
	End Object

	RemoteRole=ROLE_None
//	StaticMeshComponent=StaticMeshComponent0
//	CollisionComponent=StaticMeshComponent0
//	Components.Remove(Sprite)
//	Components.Remove(CollisionCylinder)
//	Components.Add(StaticMeshComponent0)
}
