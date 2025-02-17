/**
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_NPCMaleWithHatAndBag extends GearPawn_NPCBase
	config(Pawn);



defaultproperties
{
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Neutral_Stranded_01.Neutral_Stranded_01'
		PhysicsAsset=PhysicsAsset'Neutral_Stranded_01.Neutral_Stranded_01_Physics'
		AnimSets(0)=AnimSet'Neutral_Stranded_01.Male_Stranded_NPC_Animset'
		Translation=(Z=-74)
	End Object
}
