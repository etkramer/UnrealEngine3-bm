/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDecal_BloodPool extends GearDecal;

/** MIC used for scaling the pool over time */
var MaterialInstanceConstant MIC;

/** Current scaling value */
var transient float ScaleValue;

/** Current max scale value */
var float ScaleMax;

/** Range of max scale values */
var vector2d ScaleMaxRange;

/** Increment per second amount */
var float ScaleIncrement;

/**
 * Overridden to implement scaling of the MIC.
 */
simulated event Tick(float DeltaTime)
{
	if (MIC != None)
	{
		if (ScaleValue < ScaleMax)
		{
			ScaleValue += ScaleIncrement * DeltaTime;
			MIC.SetScalarParameterValue('Blood_Scale',ScaleValue);
		}
	}
	Super.Tick(DeltaTime);
}

/**
 * Overridden to initialize the MIC.
 */
simulated protected function SharedAttachmentInit(Actor TargetAttachment, vector RayDir)
{
	if (TargetAttachment != None)
	{
		// create the MIC
		MIC = new(Outer) class'MaterialInstanceConstant';
		MIC.SetParent(GetDecalMaterial());
		// init the scaling value
		ScaleMax = GetRangeValueByPct(ScaleMaxRange,RandRange(0.f,1.f));
		ScaleValue = 0.f;
		MIC.SetScalarParameterValue('Blood_Scale',ScaleValue);
		// set the actual MIC as the material
		SetDecalMaterial(MIC);
	}
	Super.SharedAttachmentInit(TargetAttachment,RayDir);
}

defaultproperties
{
	ScaleMaxRange=(X=0.8,Y=1.2f)
	ScaleIncrement=0.25f
	LifeSpan=60.0
	DecalMaterial=DecalMaterial'BloodPoolTest.Bleed_decal'
	Width=225
	Height=225
	bNoClip=TRUE
	bProjectOnBSP=TRUE
	bProjectOnSkeletalMeshes=FALSE
	bProjectOnStaticMeshes=TRUE 
	bProjectOnTerrain=FALSE // TEMP
	DepthBias=-0.00002
}
