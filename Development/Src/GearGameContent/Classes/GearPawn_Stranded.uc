
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_Stranded extends GearPawn_Infantry
	config(Pawn);


/**
 * This allows the player to run up to the pawn and cause the buy menu to appear when the player
 * touches the collision cylinder of the stranded!
 **/
// auto state WaitingForCustomer
// {
// 
// 	/** event when pickup is touched by an actor */
// 	event Touch( Actor Other, vector HitLocation, vector HitNormal )
// 	{
// 		local GearPawn P;
// 
// 		// don't take touch events from non players
// 		P = GearPawn(Other);
// 		if( P == none )
// 		{
// 			return;
// 		}
// 
// 		GearInventoryManager(P.InvManager).bShowUpgradesScreen = TRUE;
// 
// 	}
// 
// 
// 	/** event when pickup is touched by an actor */
// 	event UnTouch( Actor Other )
// 	{
// 		local GearPawn P;
// 
// 		// don't take touch events from non players
// 		P = GearPawn(Other);
// 		if( P == none )
// 		{
// 			return;
// 		}
// 
// 		GearInventoryManager(P.InvManager).bShowUpgradesScreen = FALSE;
// 	}
// }


defaultproperties
{
	bCollideWhenPlacing=false
	bCanTeleport=true
	bCollideActors=true
	bNoEncroachCheck=false

	bUsesBodyStances=FALSE
	//FullBodyNodeName="Custom_FullBody"

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'NPC1.NPC'
		PhysicsAsset=PhysicsAsset'NPC1.NPC_Physics'
		AnimSets(0)=AnimSet'NPC1.anims.animset_COG_NPC'
		AnimTreeTemplate=AnimTree'NPC1.npc1_animtree'
    	Translation=(Z=-74)

		BlockZeroExtent=true
		CollideActors=true
		BlockRigidBody=true
		bOwnerNoSee=TRUE
	End Object

	CrouchHeight=+60.0
	CrouchRadius=+34.0
	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object

	ControllerClass=class'GearAI_Stranded'


	HelmetType=class'Item_Helmet_None'

	Health=1000

	bBlockCamera=FALSE
}
