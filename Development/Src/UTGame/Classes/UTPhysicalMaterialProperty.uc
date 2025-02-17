/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTPhysicalMaterialProperty extends PhysicalMaterialPropertyBase
	native;

/** Type of material this is (dirt, gravel, brick, etc) used for looking up material specific effects */
var() name MaterialType;

/** struct for list to map material types supported by an actor to impact sounds and effects */
struct native MaterialImpactEffect
{
	var name MaterialType;
	var SoundCue Sound;
	var array<MaterialInterface> DecalMaterials;
	/** How long the decal should last before fading out **/
	var float DurationOfDecal;
	/** MaterialInstance param name for dissolving the decal **/
	var name DecalDissolveParamName;
	var float DecalWidth;
	var float DecalHeight;
	var ParticleSystem ParticleTemplate;


	StructDefaultProperties
	{
		DurationOfDecal=4.0f
		DecalDissolveParamName="DissolveAmount"
	}
};

/** Struct for list to map materials to sounds, for sound only applications (e.g. tires) */
struct native MaterialSoundEffect
{
	var name MaterialType;
	var SoundCue Sound;
};

/** Struct for list to map materials to a particle effect */
struct native MaterialParticleEffect
{
	var name MaterialType;
	var ParticleSystem ParticleTemplate;
};


DefaultProperties
{

}