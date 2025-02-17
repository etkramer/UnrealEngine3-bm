/**
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *	Advanced version of SkeletalMeshActor which uses an AnimTree instead of having a single AnimNodeSequence defined in its defaultproperties
 */

class SkeletalMeshActorMAT extends SkeletalMeshActor
	native(Anim)
	placeable;

cpptext
{
	virtual void GetAnimControlSlotDesc(TArray<struct FAnimSlotDesc>& OutSlotDescs);
	virtual void PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets);
	virtual void PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bLooping);
	virtual void PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos);
	virtual void PreviewFinishAnimControl();
	virtual void PreviewSetMorphWeight(FName MorphNodeName, FLOAT MorphWeight);
	virtual void PreviewSetSkelControlScale(FName SkelControlName, FLOAT Scale);

	/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
	virtual void SetAnimWeights( const TArray<struct FAnimSlotInfo>& SlotInfos );
}

/** Array of Slots */
var transient Array<AnimNodeSlot>	SlotNodes;

/** Start AnimControl. Add required AnimSets. */
native function MAT_BeginAnimControl(Array<AnimSet> InAnimSets);

/** Update AnimTree from track info */
native function MAT_SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping);

/** Update AnimTree from track weights */
native function MAT_SetAnimWeights(Array<AnimSlotInfo> SlotInfos);

/** End AnimControl. Release required AnimSets */
native function MAT_FinishAnimControl();

native function MAT_SetMorphWeight(name MorphNodeName, float MorphWeight);

native function MAT_SetSkelControlScale(name SkelControlName, float Scale);


/** Called when we start an AnimControl track operating on this Actor. Supplied is the set of AnimSets we are going to want to play from. */
simulated event BeginAnimControl(Array<AnimSet> InAnimSets)
{
	MAT_BeginAnimControl(InAnimSets);
}

/** Called each from while the Matinee action is running, with the desired sequence name and position we want to be at. */
simulated event SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping)
{
	MAT_SetAnimPosition(SlotName, ChannelIndex, InAnimSeqName, InPosition, bFireNotifies, bLooping);
}

/** Called when we are done with the AnimControl track. */
simulated event FinishAnimControl()
{
	MAT_FinishAnimControl();
}

/** Called each frame by Matinee to update the weight of a particular MorphNodeWeight. */
simulated event SetMorphWeight(name MorphNodeName, float MorphWeight)
{
	MAT_SetMorphWeight(MorphNodeName, MorphWeight);
}

/** Called each frame by Matinee to update the scaling on a SkelControl. */
simulated event SetSkelControlScale(name SkelControlName, float Scale)
{
	MAT_SetSkelControlScale(SkelControlName, Scale);
}

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		Animations=None
	End Object
}
