/**
 * This will hold all of per map color configs. 
 *
 * examples:  AR light colors, ink cloud colors, weapon emissive light colors, brightness of the weapon glow shader
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPerMapColorConfig extends Object
	native
	editinlinenew
	hidecategories(object);


/** These are all of the colors that weapons have which we want to modify on a per level basis **/
struct native WeaponColorDatum
{
	/** This is for the sake of editing, knowing which entry is which weapon.  It is filled in by PostLoad(). */
	var() const editconst EWeaponClass Type;

	/** The colors to use for this weapon's emissive **/ 
	var() LinearColor EmissiveCOG;

	/** The colors to use for this weapon's emissive **/ 
	var() LinearColor EmissiveLocust;

	/** The muzzle flash light for normal firing **/
	var() Color MuzzleFlashLightColor;
	/** The muzzle flash light for ActiveReload firing **/
	var() Color MuzzleFlashLightColor_AR;

	/** Dropped weapons spec **/
	var(GlowyShader) float Weap_SpecMult;
	/** Dropped weapons glow **/
	var(GlowyShader) float Weap_DroppedGlow;
	/** Dropped weapons glow alpha **/
	var(GlowyShader) float Weap_DropGlowAlpha;

	// these do not seem to be propagated correctly :-( ronP knows the issue
	structdefaultproperties
	{
		EmissiveCOG=(R=2.0,G=4.0,B=8.0,A=1.0)
		EmissiveLocust=(R=60.0,G=1.0,B=0.1,A=1.0)
		MuzzleFlashLightColor=(R=245,G=174,B=122,A=255)
		MuzzleFlashLightColor_AR=(R=245,G=174,B=122,A=255)

		Weap_SpecMult=5.0f
		Weap_DroppedGlow=1.0f
		Weap_DropGlowAlpha=1.0f
	}
};
/** This is the global value that can be overridden per weapon if needed **/
var(Weapons) WeaponColorDatum DefaultWeaponColor;
var(Weapons) array<WeaponColorDatum> WeaponColors;


struct native RimShaderDatum
{
	var() LinearColor EmisMult;
	var() LinearColor TeamColor;
	var() LinearColor RimColor;

	structdefaultproperties
	{
	}
};
/** This is the global value that can be overridden per character if needed **/
var(RimShader) RimShaderDatum DefaultRimShaderCOG;
var(RimShader) RimShaderDatum DefaultRimShaderLocust;
var(RimShader) array<RimShaderDatum> RimShaders; // per character?



/** struct to store name / values for vector params **/
struct native VectorParam
{
	var() name Name;
	var() vector Value;
};

/** struct to store name / values for float params **/
struct native FloatParam
{
	var() name Name;
	var() float Value;
};


struct native EffectParam
{
	/** The DamageType to have per map modifications **/
	var() class<DamageType> DamageType;

	/** VectorParams to set**/
	var() array<VectorParam> VectorParams;
	/** FloatParams to set**/
	var() array<FloatParam> FloatParams;
};

/** This is the list of Material Params that we are going to apply this damage type (only implemented for explosions atm).**/
var() array<EffectParam> EffectParams;



cpptext
{
protected:
	void RefreshTypeData();

public:
	virtual void PostLoad();
	virtual void PostEditChange( class FEditPropertyChain& PropertyThatChanged );
}




defaultproperties
{
	DefaultRimShaderCOG=(EmisMult=(R=1.0f,G=1.0f,B=1.0f,A=1.0f),TeamColor=(R=0.0f,G=9.0f,B=16.0f,A=1.0f),RimColor=(R=0.625f,G=0.585f,B=0.517f,A=1.0f))
	DefaultRimShaderLocust=(EmisMult=(R=1.0f,G=1.0f,B=1.0f,A=1.0f),TeamColor=(R=9.0f,G=-0.5f,B=-0.5f,A=1.0f),RimColor=(R=0.645f,G=0.618f,B=0.566f,A=1.0f))
	
	WeaponColors(WC_Boltock)=(Type=WC_Boltock)
	WeaponColors(WC_Boomshot)=(Type=WC_Boomshot)
	WeaponColors(WC_BoomerFlail)=(Type=WC_BoomerFlail)
	WeaponColors(WC_BrumakMainGun)=(Type=WC_BrumakMainGun)
	WeaponColors(WC_BrumakSideGun)=(Type=WC_BrumakSideGun)
	WeaponColors(WC_BrumakSideGunOther)=(Type=WC_BrumakSideGunOther)
	WeaponColors(WC_FragGrenade)=(Type=WC_FragGrenade)
	WeaponColors(WC_GasGrenade)=(Type=WC_GasGrenade)
	WeaponColors(WC_Gnasher)=(Type=WC_Gnasher)
	WeaponColors(WC_HOD)=(Type=WC_HOD)
	WeaponColors(WC_Hammerburst)=(Type=WC_Hammerburst)
	WeaponColors(WC_InkGrenade)=(Type=WC_InkGrenade)
	WeaponColors(WC_KingRavenTurret)=(Type=WC_KingRavenTurret)
	WeaponColors(WC_Lancer)=(Type=WC_Lancer)
	WeaponColors(WC_Longshot)=(Type=WC_Longshot)
	WeaponColors(WC_Minigun)=(Type=WC_Minigun)
	WeaponColors(WC_Mortar)=(Type=WC_Mortar)
	WeaponColors(WC_Scorcher)=(Type=WC_Scorcher)
	WeaponColors(WC_SmokeGrenade)=(Type=WC_SmokeGrenade)
	WeaponColors(WC_Snub)=(Type=WC_Snub)
	WeaponColors(WC_TorqueBow)=(Type=WC_TorqueBow)
	WeaponColors(WC_Troika)=(Type=WC_Troika)
	WeaponColors(WC_TyroTurrent)=(Type=WC_TyroTurrent)
	WeaponColors(WC_WretchMeleeSlash)=(Type=WC_WretchMeleeSlash)
	WeaponColors(WC_FragGrenadeExplosion)=(Type=WC_FragGrenadeExplosion)
	WeaponColors(WC_TorqueBowArrowExplosion)=(Type=WC_TorqueBowArrowExplosion)
	WeaponColors(WC_MortarExplosion)=(Type=WC_MortarExplosion)
	WeaponColors(WC_BoomshotExplosion)=(Type=WC_BoomshotExplosion)
	WeaponColors(WC_BrumakRocketExplosion)=(Type=WC_BrumakRocketExplosion)
	WeaponColors(WC_LocustBurstPistol)=(Type=WC_LocustBurstPistol)
	WeaponColors(WC_CentaurCannon)=(Type=WC_CentaurCannon)
	WeaponColors(WC_ReaverRocket)=(Type=WC_ReaverRocket)
	WeaponColors(WC_RocketLauncherTurret)=(Type=WC_RocketLauncherTurret)
	WeaponColors(WC_Shield)=(Type=WC_Shield)
}
