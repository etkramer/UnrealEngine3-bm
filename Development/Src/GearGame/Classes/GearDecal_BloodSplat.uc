/**
 * Base decals for any blood splats - post player impact, or gore chunks.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDecal_BloodSplat extends GearDecal;

defaultproperties
{
	LifeSpan=85.f
	bRandomizeRotation=TRUE
	bRandomizeScaling=TRUE
	RandomScalingRange=(X=1.f,Y=3.5f)

	DecalMaterial=DecalMaterial'War_Gameplay_Decals.Textures.BodyHitBloodDecal'
	Width=32.f
	Height=32.f
}
