/**
 * Hammer of Dawn Beam.
 * An instance of a HOD beam, handles sequencing of a single shot
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class HOD_Beam extends HOD_BeamBase
	config(Weapon)
	notplaceable;

defaultproperties
{
	CoolDownSound=SoundCue'Weapon_HammerDawn.Weapons.HODBeamEndCue'
	ImpactSound=SoundCue'Weapon_HammerDawn.Weapons.HoDStartImpactCue'

	Begin Object Class=AudioComponent Name=FireLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.Weapons.HODBeamLoopCue'
	End Object
	FireLoopSound=FireLoopSound0
	Components.Add(FireLoopSound0);

	Begin Object Class=AudioComponent Name=RippingEarthLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.Weapons.HODRippingEarthLoopCue'
	End Object
	RippingEarthLoopSound=RippingEarthLoopSound0
	Components.Add(RippingEarthLoopSound0);

	Begin Object Class=AudioComponent Name=WarmupSound0
		SoundCue=SoundCue'Weapon_HammerDawn.Weapons.HoDStartWarmupCue'
	End Object
	WarmupSound=WarmupSound0
	Components.Add(WarmupSound0);

	// main beam effects
	Begin Object Name=PSC_MainBeam0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Main_Beam'
	End Object

	// tendril beam effects
	PS_TendrilBeam=ParticleSystem'COG_HOD.Effects.COG_HOD_Little_Beam'
	PS_TendrilImpact=ParticleSystem'COG_HOD.Effects.COG_HOD_Little_Beam_EndPoints'

	PS_GroundDamage=ParticleSystem'COG_HOD.Effects.COG_HOD_Main_Beam_Impact_Test'

	Begin Object Name=PSC_BerserkerImpact0
		Template=ParticleSystem'COG_HOD.Effects.COG_HOD_Damage'
	End Object
	PSC_BerserkerImpact=PSC_BerserkerImpact0
	Components.Add(PSC_BerserkerImpact0)

	//PS_EntryEffect=ParticleSystem'COG_HOD.Effects.COG_HOD_Damage'
	//PS_ExitEffect=ParticleSystem'COG_HOD.Effects.COG_HOD_Damage'

	ScorchDecal=DecalMaterial'COG_HOD.Materials.HOD_Decal'


	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_HOD'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		// effects are handled in the main beam effect
		ParticleEmitterTemplate=None
		ExplosionSound=None
		ExploLight=None

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=800
		ExploShakeOuterRadius=2000

		KnockdownRadius=0
		CringeRadius=0

		FractureMeshRadius=500.0
		FracturePartVel=1500.0

		bAllowPerMaterialFX=TRUE
	End Object
	InitialExplosionTemplate=ExploTemplate0

}


