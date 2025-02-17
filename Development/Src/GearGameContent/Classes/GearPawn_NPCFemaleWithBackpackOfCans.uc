/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_NPCFemaleWithBackpackOfCans extends GearPawn_NPCBase
	config(Pawn);



defaultproperties
{
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Neutral_Stranded_06.Stranded_06'
		PhysicsAsset=PhysicsAsset'Neutral_Stranded_06.Neutral_Stranded_06'
		AnimSets(0)=AnimSet'Neutral_Stranded_04.Woman_Strande_Animset'
		Translation=(Z=-74)
	End Object
}
