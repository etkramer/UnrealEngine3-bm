/**
 * Spawnable spherical fog volume.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDynamicFogVolume_Spherical extends FogVolumeSphericalDensityInfo
	abstract
	notplaceable
	native(Weapon);

//
// Note: these are purely client-side at the moment.
//

/** Ref to an artist-created archetype.  Data-driven ftw. */
var() FogVolumeSphericalDensityInfo	FogVolumeArchetype;

/**
 * This is the GearDamageType which is used to look up the per material modifications.
 *  The issue is that we have really strong PPE .  And some of the fog materials don't look correct at all.
 **/
var protected class<GearDamageType> DamageTypeToUseForPerLevelMaterialEffects;

/** This is the duration that the MITV should run for **/
var protected float DurationForMITV;

function PostBeginPlay()
{
	if( FogVolumeArchetype != None )
	{
		StartFogVolume();
	}

	super.PostBeginPlay();
}

simulated function StartFogVolume()
{

	// note: AutomaticMeshComponent must exist before attaching DensityComponent
	AutomaticMeshComponent = new(self) class'StaticMeshComponent' (FogVolumeArchetype.AutomaticMeshComponent);
	DensityComponent = new(self) class'FogVolumeSphericalDensityComponent' (FogVolumeArchetype.DensityComponent);

	bEnabled = DensityComponent.bEnabled;

	if (AutomaticMeshComponent != None)
	{
		AttachComponent(AutomaticMeshComponent);
	}
	if (DensityComponent != None)
	{
		AttachComponent(DensityComponent);
	}

	// copy this by hand, so the artists can manipulate it
	SetDrawScale(FogVolumeArchetype.DrawScale);

	SetMaterialParamsFromWorldInfo();

}


/** This will set the Specific Material Params on the Material that this fog volume has if the damage type is found in the WorldInfo **/
simulated function SetMaterialParamsFromWorldInfo()
{
	local GearMapSpecificInfo GSI;
	local EffectParam EP;
	local VectorParam VP;
	local FloatParam FP;
	local MaterialInstance MI;

	// now check to see if this is using the per map values
	GSI = GearMapSpecificInfo(WorldInfo.GetMapInfo());
	if( GSI != none )
	{
		if( GSI.ColorConfig != none )
		{
			// look over all of the effects
			foreach GSI.ColorConfig.EffectParams( EP )
			{
				// if there are any for my damage type
				if( DamageTypeToUseForPerLevelMaterialEffects == EP.DamageType )
				{
					// so here we need to check to see if this an MITV or an MIC
					if( MaterialInstanceTimeVarying(DensityComponent.FogMaterial) != none )
					{
						`log( "MITV! " $ DensityComponent.FogMaterial );
						MI = new(outer) class'MaterialInstanceTimeVarying';
					}
					else
					{
						MI = new(outer) class'MaterialInstanceConstant';
					}


					MI.SetParent( DensityComponent.FogMaterial );
					DensityComponent.FogMaterial = MI;

					// do all of the setting
					foreach EP.VectorParams( VP )
					{
						MI.SetVectorParameterValue( VP.Name, MakeLinearColor( VP.Value.X, VP.Value.Y, VP.Value.Z, 1.0 ) );
						//`log( "Setting Vect:" @ VP.Name @ VP.Value );
					}

					foreach EP.FloatParams( FP )
					{
						MI.SetScalarParameterValue( FP.Name, FP.Value );
						//`log( "Setting Float:" @ FP.Name @ FP.Value );
					}
				}
			}
		}

		// now reattach so the Material is propagated to the render thread
		ReattachComponent(DensityComponent);
	}
}



defaultproperties
{
	RemoteRole=ROLE_None

	// we'll be creating new objects from the archetype template for these
	Components.Remove(FogVolumeComponent0)
	Components.Remove(AutomaticMeshComponent0)
	AutomaticMeshComponent=None
	DensityComponent=None

	// so we can spawn these dynamically
	bStatic=FALSE
	bNoDelete=FALSE
}
