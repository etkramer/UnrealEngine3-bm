/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
 
class SpeedTree extends Object
	native(SpeedTree);
	
/** Helper object to allow SpeedTree to be script exposed.										*/
var duplicatetransient native const pointer	SRH{class FSpeedTreeResourceHelper};

// Editor-accesible variables

/** Random seed for tree creation.																*/
var() int							RandomSeed;						
/** Sink the tree partially underground.														*/
var() float							Sink;

/** The probability of a shadow ray being blocked by the leaf material. */
var(Lighting) float LeafStaticShadowOpacity;

/** Branch material.																			*/
var(Material) MaterialInterface		BranchMaterial;
/** Frond material.																				*/
var(Material) MaterialInterface		FrondMaterial;
/** Leaf material.																				*/
var(Material) MaterialInterface		LeafMaterial;
/** Billboard material.																			*/
var(Material) MaterialInterface		BillboardMaterial;

// SpeedWind variables (explained in the SpeedTreeCAD documentation)
var(Wind) float						MaxBendAngle;
var(Wind) float						BranchExponent;
var(Wind) float						LeafExponent;
var(Wind) float						Response;
var(Wind) float						ResponseLimiter;
var(Wind) float						Gusting_MinStrength;
var(Wind) float						Gusting_MaxStrength;
var(Wind) float						Gusting_Frequency;
var(Wind) float						Gusting_MinDuration;
var(Wind) float						Gusting_MaxDuration;
var(Wind) float						BranchHorizontal_LowWindAngle;
var(Wind) float						BranchHorizontal_LowWindSpeed;
var(Wind) float						BranchHorizontal_HighWindAngle;
var(Wind) float						BranchHorizontal_HighWindSpeed;
var(Wind) float						BranchVertical_LowWindAngle;
var(Wind) float						BranchVertical_LowWindSpeed;
var(Wind) float						BranchVertical_HighWindAngle;
var(Wind) float						BranchVertical_HighWindSpeed;
var(Wind) float						LeafRocking_LowWindAngle;
var(Wind) float						LeafRocking_LowWindSpeed;
var(Wind) float						LeafRocking_HighWindAngle;
var(Wind) float						LeafRocking_HighWindSpeed;
var(Wind) float						LeafRustling_LowWindAngle;
var(Wind) float						LeafRustling_LowWindSpeed;
var(Wind) float						LeafRustling_HighWindAngle;
var(Wind) float						LeafRustling_HighWindSpeed;

cpptext
{
	void StaticConstructor(void);

	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();
	
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();

	virtual	INT	GetResourceSize(void);
	virtual	FString	GetDetailedDescription( INT InIndex );
	virtual FString	GetDesc(void);
	
	UBOOL IsInitialized();
}

defaultproperties
{
	RandomSeed=1
	
	LeafStaticShadowOpacity=0.5

	// SpeedWind values
	MaxBendAngle=35.0
	BranchExponent=1.0
	LeafExponent=1.0
	Response=0.1
	ResponseLimiter=0.01
	Gusting_MinStrength=0.25
	Gusting_MaxStrength=1.25
	Gusting_Frequency=0.4
	Gusting_MinDuration=2.0
	Gusting_MaxDuration=15.0
	BranchHorizontal_LowWindAngle=3.0
	BranchHorizontal_LowWindSpeed=1.5
	BranchHorizontal_HighWindAngle=3.0
	BranchHorizontal_HighWindSpeed=1.5
	BranchVertical_LowWindAngle=4.0
	BranchVertical_LowWindSpeed=2.0
	BranchVertical_HighWindAngle=4.0
	BranchVertical_HighWindSpeed=2.0
	LeafRocking_LowWindAngle=5.0
	LeafRocking_LowWindSpeed=1.0
	LeafRocking_HighWindAngle=5.0
	LeafRocking_HighWindSpeed=3.0
	LeafRustling_LowWindAngle=7.0
	LeafRustling_LowWindSpeed=0.1
	LeafRustling_HighWindAngle=5.0
	LeafRustling_HighWindSpeed=15.0
}

