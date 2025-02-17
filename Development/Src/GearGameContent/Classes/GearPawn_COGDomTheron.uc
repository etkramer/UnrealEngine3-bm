
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGDomTheron extends GearPawn_COGDom;

defaultproperties
{
	// this is hardweights only need to make it so he always breaks all
	GoreSkeletalMesh=SkeletalMesh'COG_Dom.COG_Dom_Theron_Gore'
	GorePhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_Theron_Physics'

	HelmetType=class'Item_Helmet_Dom_Theron'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Dom.Cog_Dom_Theron'
		PhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_Theron_Physics_New'
		AnimSets.Add(AnimSet'COG_MarcusFenix.Animations.Theron_Disguise')
		bEnableFullAnimWeightBodies=TRUE
	End Object
}
