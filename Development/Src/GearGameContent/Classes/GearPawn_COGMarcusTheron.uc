
/** 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGMarcusTheron extends GearPawn_COGMarcus;

defaultproperties
{
	// this is hardweights only need to make it so he always breaks all
	GoreSkeletalMesh=SkeletalMesh'COG_MarcusFenix.COG_Marcus_Theron_Gore'
	GorePhysicsAsset=PhysicsAsset'COG_MarcusFenix.COG_Marcus_Theron_Physics'

	HelmetType=class'Item_Helmet_Marcus_Theron'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_MarcusFenix.Cog_Marcus_Theron'
		PhysicsAsset=PhysicsAsset'COG_MarcusFenix.COG_Marcus_Theron_Physics_New'
		AnimSets.Add(AnimSet'COG_MarcusFenix.Animations.Theron_Disguise')
		bEnableFullAnimWeightBodies=TRUE
	End Object
}
