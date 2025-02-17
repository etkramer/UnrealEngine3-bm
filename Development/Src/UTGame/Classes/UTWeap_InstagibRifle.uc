/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_InstagibRifle extends UTWeapon
	abstract
	HideDropDown;

var array<MaterialInterface> TeamSkins;
var array<ParticleSystem> TeamMuzzleFlashes;

//-----------------------------------------------------------------
// AI Interface

function float GetAIRating()
{
	return AIRating;
}

function float RangedAttackTime()
{
	return 0;
}

simulated function SetSkin(Material NewMaterial)
{
	local int TeamIndex;

	if ( NewMaterial == None ) 	// Clear the materials
	{
		if ( Instigator != None )
		{
			TeamIndex = Instigator.GetTeamNum();
		}
		if (TeamIndex > TeamSkins.length)
		{
			TeamIndex = 0;
		}
		Mesh.SetMaterial(0,TeamSkins[TeamIndex]);
	}
	else
	{
		Super.SetSkin(NewMaterial);
	}
}

simulated function AttachWeaponTo(SkeletalMeshComponent MeshCpnt, optional name SocketName)
{
	local int TeamIndex;

	TeamIndex = Instigator.GetTeamNum();
	if (TeamIndex > TeamMuzzleFlashes.length)
	{
		TeamIndex = 0;
	}
	MuzzleFlashPSCTemplate = TeamMuzzleFlashes[TeamIndex];

	Super.AttachWeaponTo(MeshCpnt, SocketName);
}

function byte BestMode()
{
	return WorldInfo.GRI.OnSameTeam(Instigator.Controller.Enemy, Instigator) ? 1 : 0;
}

defaultproperties
{
	MuzzleFlashSocket=MF
	MuzzleFlashDuration=0.33;

	WeaponColor=(R=160,G=0,B=255,A=255)
	FireInterval(0)=+1.1
	FireInterval(1)=+1.1
	InstantHitMomentum(0)=+100000.0
	FireOffset=(X=20,Y=5)
	PlayerViewOffset=(X=17,Y=10.0,Z=-8.0)
	bCanThrow=false
	bExportMenuData=false

	// Weapon SkeletalMesh
	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object

	InstantHitDamage(0)=1000
	InstantHitDamage(1)=1000

	InstantHitDamageTypes(0)=class'UTDmgType_Instagib'
	InstantHitDamageTypes(1)=class'UTDmgType_Instagib'

	WeaponFireAnim(0)=WeaponFireInstigib
	WeaponFireAnim(1)=WeaponFireInstigib

	AIRating=+1.0
	CurrentRating=+1.0
	bInstantHit=true
	bSplashJump=false
	bRecommendSplashDamage=false
	bSniping=true
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0
	InventoryGroup=12
	GroupWeight=0.5

	ShotCost(0)=0
	ShotCost(1)=0

	IconX=400
	IconY=129
	IconWidth=22
	IconHeight=48
	IconCoordinates=(U=722,V=479,UL=166,VL=42)

	CrossHairCoordinates=(U=320,V=0,UL=64,VL=64)
}
