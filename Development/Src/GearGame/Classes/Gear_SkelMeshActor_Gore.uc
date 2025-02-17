/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Gear_SkelMeshActor_Gore extends SkeletalMeshActorMAT
	placeable;

/** Cached pointer to gore manager node. */
var		GearAnim_GoreSystem		GoreNode;

/** Set of damage types that should not cause gore to appear on this mesh */
var() array< class<DamageType> > NoGoreDamageTypes;

/** Create unique MICs for mesh */
simulated function PostBeginPlay()
{
	local int i, NumMaterials;

	Super.PostBeginPlay();

	// Create MICs for each section, so gore system can modify them
	NumMaterials = SkeletalMeshComponent.SkeletalMesh.Materials.length;
	for(i=0; i<NumMaterials; i++)
	{
		SkeletalMeshComponent.CreateAndSetMaterialInstanceConstant(i);
	}
}

/** Used to find the gore node in this anim tree */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local GearAnim_GoreSystem Node;

	Super.PostInitAnimTree(SkelComp);

	// Iterate over all GearAnim_GoreSystem - but should only have one
	foreach SkeletalMeshComponent.AllAnimNodes(class'GearAnim_GoreSystem', Node)
	{
		if(GoreNode == None)
		{
			GoreNode = Node;
		}
		else
		{
			`log("Gear_SkelMeshActor_Gore: More than one gore node found!"@SkeletalMeshComponent.AnimTreeTemplate);
		}
	}

	if(GoreNode == None)
	{
		`log("Gear_SkelMeshActor_Gore: No gore node found!"@SkeletalMeshComponent.AnimTreeTemplate);
	}
}

/** Route damage to gore node */
event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int i;

	// First check if this is a damage type we should ignore
	for(i=0; i<NoGoreDamageTypes.length; i++)
	{
		if(ClassIsChildOf(DamageType, NoGoreDamageTypes[i]))
		{
			return;
		}
	}

	if(ClassIsChildOf(DamageType, class'GDT_Explosive'))
	{
		GoreNode.UpdateGoreDamageRadial(HitLocation, Damage, FALSE);
	}
	else
	{
		GoreNode.UpdateGoreDamage(HitInfo.BoneName, HitLocation, Damage);
	}
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		bAlwaysUseInstanceWeights=TRUE
	End Object
}