/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAttachment_Enforcer extends UTWeaponAttachment;

/** second mesh for dual-wielding, constructed by duplicating Mesh if no default */
var SkeletalMeshComponent DualMesh;
var SkeletalMeshComponent DualOverlayMesh;

var ParticleSystemComponent DualMuzzleFlashPSC;
var UTExplosionLight DualMuzzleFlashLight;

/** toggled on each fire effect in the dual wielding case to alternate which gun effects get played on */
var bool bFlashDual;

var ParticleSystem TracerTemplate;

function SetSkin(Material NewMaterial)
{
	local int i;

	Super.SetSkin(NewMaterial);

	if (DualMesh != None)
	{
		for (i = 0; i < Mesh.Materials.length; i++)
		{
			DualMesh.SetMaterial(i, Mesh.GetMaterial(i));
		}
	}
}

simulated function SetDualWielding(bool bNowDual)
{
	local UTPawn P;
	local vector LeftScale;

	P = UTPawn(Instigator);
	if (P != None)
	{
		if (bNowDual)
		{
			if (DualMesh == None && Mesh != None)
			{
				DualMesh = new(self) Mesh.Class(Mesh);

				// reverse the mesh, like we do in 1st person
				LeftScale = DualMesh.Scale3D;
				LeftScale.X *= -1;
				DualMesh.SetScale3D(LeftScale);
			}
			if (DualMesh != None)
			{
				P.Mesh.AttachComponentToSocket(DualMesh, P.WeaponSocket2);

				// Weapon Mesh Shadow
				DualMesh.SetShadowParent(P.Mesh);
				DualMesh.SetLightEnvironment(P.LightEnvironment);

				if (P.ReplicatedBodyMaterial != None)
				{
					SetSkin(P.ReplicatedBodyMaterial);
				}
				if ( MuzzleFlashSocket != 'None' &&
					(MuzzleFlashPSCTemplate != None || MuzzleFlashAltPSCTemplate != None) )
				{
					DualMuzzleFlashPSC = new(self) class'UTParticleSystemComponent';
					DualMuzzleFlashPSC.bAutoActivate = false;
					DualMuzzleFlashPSC.SetOwnerNoSee(true);
					DualMesh.AttachComponentToSocket(DualMuzzleFlashPSC, MuzzleFlashSocket);
				}
				P.SetWeapAnimType(EWAT_DualPistols);

				if (OverlayMesh != None && DualOverlayMesh == None)
				{
					DualOverlayMesh = new(self) OverlayMesh.Class(OverlayMesh);
					DualOverlayMesh.SetParentAnimComponent(DualMesh);
					DualOverlayMesh.SetScale3D(DualMesh.Scale3D);
				}
				if (DualOverlayMesh != None && OverlayMesh.bAttached)
				{
					P.Mesh.AttachComponentToSocket(DualOverlayMesh, P.WeaponSocket2);
				}
			}
		}
		else
		{
			if (DualMesh != None && DualMesh.bAttached)
			{
				P.Mesh.DetachComponent(DualMesh);
				P.SetWeapAnimType(WeapAnimType);
			}
			if (DualOverlayMesh != None && DualOverlayMesh.bAttached)
			{
				P.Mesh.DetachComponent(DualOverlayMesh);
			}
			bFlashDual = false;
		}
	}
}

simulated function CreateOverlayMesh()
{
	Super.CreateOverlayMesh();

	if (DualMesh != None)
	{
		DualOverlayMesh = new(self) OverlayMesh.Class(OverlayMesh);
		DualOverlayMesh.SetParentAnimComponent(DualMesh);
		DualOverlayMesh.SetScale3D(DualMesh.Scale3D);
	}
}

simulated function SetWeaponOverlayFlags(UTPawn OwnerPawn)
{
	local int i;

	Super.SetWeaponOverlayFlags(OwnerPawn);

	if (OverlayMesh != None && DualOverlayMesh != None)
	{
		if (OverlayMesh.bAttached)
		{
			if (!DualOverlayMesh.bAttached)
			{
				OwnerPawn.Mesh.AttachComponentToSocket(DualOverlayMesh, OwnerPawn.WeaponSocket2);
			}

			for (i = 0; i < DualOverlayMesh.GetNumElements(); i++)
			{
				DualOverlayMesh.SetMaterial(i, OverlayMesh.GetMaterial(i));
			}
		}
		else
		{
			OwnerPawn.Mesh.DetachComponent(DualOverlayMesh);
		}
	}
}

simulated function DetachFrom(SkeletalMeshComponent MeshCpnt)
{
	if (MeshCpnt != None)
	{
		if (DualMesh != None && DualMesh.bAttached)
		{
			MeshCpnt.DetachComponent(DualMesh);
		}
		if (DualOverlayMesh != None && DualOverlayMesh.bAttached)
		{
			MeshCpnt.DetachComponent(DualOverlayMesh);
		}
	}
	Super.DetachFrom(MeshCpnt);
}

simulated function ThirdPersonFireEffects(vector HitLocation)
{
	local UTPawn P;

	Super.ThirdPersonFireEffects(HitLocation);

	SpawnTracer(GetEffectLocation(), HitLocation);

	P = UTPawn(Instigator);
	if (P != None && P.bDualWielding)
	{
		// override recoil and play only on hand that's firing
		P.GunRecoilNode.bPlayRecoil = false;
		if (bFlashDual)
		{
			P.LeftRecoilNode.bPlayRecoil = true;
		}
		else
		{
			P.RightRecoilNode.bPlayRecoil = true;
		}

		bFlashDual = !bFlashDual;
	}
}

simulated function MuzzleFlashTimer()
{
	Super.MuzzleFlashTimer();

	if (DualMuzzleFlashLight != None)
	{
		DualMuzzleFlashLight.SetEnabled(false);
	}
	if (DualMuzzleFlashPSC != None && !bMuzzleFlashPSCLoops)
	{
		DualMuzzleFlashPSC.DeactivateSystem();
	}
}

simulated function CauseMuzzleFlash()
{
	if (!bFlashDual)
	{
		Super.CauseMuzzleFlash();
	}
	else
	{
		// only enable muzzleflash light if performance is high enough
		if (!WorldInfo.bDropDetail && !class'Engine'.static.IsSplitScreen() )
		{
			if (DualMuzzleFlashLight == None)
			{
				if (MuzzleFlashLightClass != None)
				{
					DualMuzzleFlashLight = new(self) MuzzleFlashLightClass;
					if (DualMesh != None && DualMesh.GetSocketByName(MuzzleFlashSocket) != None)
					{
						DualMesh.AttachComponentToSocket(DualMuzzleFlashLight, MuzzleFlashSocket);
					}
					else if (OwnerMesh != None)
					{
						OwnerMesh.AttachComponentToSocket(DualMuzzleFlashLight, UTPawn(OwnerMesh.Owner).WeaponSocket2);
					}
				}
			}
			else
			{
				DualMuzzleFlashLight.ResetLight();
			}
		}

		if (DualMuzzleFlashPSC != None)
		{
			if (!bMuzzleFlashPSCLoops || !DualMuzzleFlashPSC.bIsActive)
			{
				if (Instigator != None && Instigator.FiringMode == 1 && MuzzleFlashAltPSCTemplate != None)
				{
					DualMuzzleFlashPSC.SetTemplate(MuzzleFlashAltPSCTemplate);
				}
				else
				{
					DualMuzzleFlashPSC.SetTemplate(MuzzleFlashPSCTemplate);
				}

				SetMuzzleFlashParams(DualMuzzleFlashPSC);
				DualMuzzleFlashPSC.ActivateSystem();
			}
		}

		// Set when to turn it off.
		SetTimer(MuzzleFlashDuration,false,'MuzzleFlashTimer');
	}
}

simulated function StopMuzzleFlash()
{
	Super.StopMuzzleFlash();

	if (DualMuzzleFlashPSC != None)
	{
		DualMuzzleFlashPSC.DeactivateSystem();
	}
}

simulated function SpawnTracer(vector EffectLocation, vector HitLocation)
{
	local ParticleSystemComponent E;
	local vector Dir;

	Dir = HitLocation - EffectLocation;
	if ( VSizeSq(Dir) > 14400.0 )
	{
		E = WorldInfo.MyEmitterPool.SpawnEmitter(TracerTemplate, EffectLocation, rotator(Dir));
		E.SetVectorParameter('LinkBeamEnd', HitLocation);
	}
}

simulated function FirstPersonFireEffects(Weapon PawnWeapon, vector HitLocation)
{
	Super.FirstPersonFireEffects(PawnWeapon, HitLocation);

	SpawnTracer(UTWeapon(PawnWeapon).GetEffectLocation(), HitLocation);
}

simulated function vector GetEffectLocation()
{
	local SkeletalMeshComponent RealMesh;
	local vector Result;

	// swap to the dual mesh if necessary
	RealMesh = Mesh;
	Mesh = (bFlashDual && DualMesh != None) ? DualMesh : Mesh;
	Result = Super.GetEffectLocation();
	Mesh = RealMesh;

	return Result;
}

simulated function ChangeVisibility(bool bIsVisible)
{
	Super.ChangeVisibility(bIsVisible);

	if (DualMesh != None)
	{
		DualMesh.SetHidden(!bIsVisible);
	}
}

defaultproperties
{
	// Weapon SkeletalMesh
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'WP_Enforcers.Mesh.SK_WP_Enforcer_3P_Mid'
	End Object

	WeapAnimType=EWAT_Pistol

	DefaultImpactEffect=(Sound=SoundCue'A_Weapon_BulletImpacts.Cue.A_Weapon_Impact_Stone_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	DefaultAltImpactEffect=(Sound=SoundCue'A_Weapon_BulletImpacts.Cue.A_Weapon_Impact_Stone_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)

	BulletWhip=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_AmmoWhip_Cue'
	bMakeSplash=true
	
	ImpactEffects(0)=(MaterialType=Dirt,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(1)=(MaterialType=Gravel,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(2)=(MaterialType=Sand,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(3)=(MaterialType=Dirt_Wet,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMud_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(4)=(MaterialType=Energy,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(5)=(MaterialType=WorldBoundary,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(6)=(MaterialType=Flesh,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(7)=(MaterialType=Flesh_Human,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(8)=(MaterialType=Kraal,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(9)=(MaterialType=Necris,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(10)=(MaterialType=Robot,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(11)=(MaterialType=Foliage,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFoliage_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(12)=(MaterialType=Glass,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactGlass_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(13)=(MaterialType=Liquid,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(14)=(MaterialType=Water,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'Envy_Effects.Particles.P_WP_Water_Splash_Small',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(15)=(MaterialType=ShallowWater,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'Envy_Effects.Particles.P_WP_Water_Splash_Small',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(16)=(MaterialType=Lava,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(17)=(MaterialType=Slime,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(18)=(MaterialType=Metal,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(19)=(MaterialType=Snow,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactSnow_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(20)=(MaterialType=Wood,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWood_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	ImpactEffects(21)=(MaterialType=NecrisVehicle,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)

	AltImpactEffects(0)=(MaterialType=Dirt,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(1)=(MaterialType=Gravel,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(2)=(MaterialType=Sand,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactDirt_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(3)=(MaterialType=Dirt_Wet,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMud_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(4)=(MaterialType=Energy,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(5)=(MaterialType=WorldBoundary,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactEnergy_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(6)=(MaterialType=Flesh,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(7)=(MaterialType=Flesh_Human,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(8)=(MaterialType=Kraal,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(9)=(MaterialType=Necris,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFlesh_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(10)=(MaterialType=Robot,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(11)=(MaterialType=Foliage,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactFoliage_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(12)=(MaterialType=Glass,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactGlass_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(13)=(MaterialType=Liquid,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(14)=(MaterialType=Water,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'Envy_Effects.Particles.P_WP_Water_Splash_Small',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(15)=(MaterialType=ShallowWater,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'Envy_Effects.Particles.P_WP_Water_Splash_Small',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(16)=(MaterialType=Lava,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(17)=(MaterialType=Slime,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWater_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(18)=(MaterialType=Metal,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(19)=(MaterialType=Snow,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactSnow_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(20)=(MaterialType=Wood,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactWood_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)
	AltImpactEffects(21)=(MaterialType=NecrisVehicle,Sound=SoundCue'A_Weapon_Enforcer.Cue.A_Weapon_Enforcer_ImpactMetal_Cue',ParticleTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcer_Impact',DecalMaterials=(MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal01',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal02',MaterialInterface'WP_Enforcers.Decals.MI_WP_Enforcer_Impact_Decal03'),DecalWidth=16.0,DecalHeight=16.0)


	WeaponClass=class'UTWeap_Enforcer'

	MuzzleFlashLightClass=class'UTGame.UTEnforcerMuzzleFlashLight'
	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcers_3P_MuzzleFlash'
	MuzzleFlashDuration=0.33


	TracerTemplate=ParticleSystem'WP_Enforcers.Effects.P_WP_Enforcers_Tracer'
}
