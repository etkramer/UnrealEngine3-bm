/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTAnimBlendByFlying extends UTAnimBlendBase
	native(Animation);

//Order is important here (corresponds to blendlist node children)
var const enum EFlyingState
{
	Flying_NotFlying,
	Flying_OpeningWings,
	Flying_Flying,
	Flying_ClosingWings
} FlyingState;

/** Access to the flying pawn */
var UTPawn Pawn;

/** Access to the state of the flying animation */
var UTAnimBlendBase FlyingMode;

/** Access to the aim offset controlling flight direction */
var AnimNodeAimOffset FlyingDir;

/** Does this pawn have a special start anim to play */
var() name StartingAnimName;
var bool bHasStartingAnim;

/** Does this pawn have a special end anim to play */
var() name EndingAnimName;
var bool bHasEndingAnim;

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
	
	void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );

	/** Notification to this blend that a child UAnimNodeSequence has reached the end and stopped playing. Not called if child has bLooping set to true or if user calls StopAnim. */
	virtual void OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);
	
	void TestBlend();
}

/** Force an update of the flying state now. */
native function UpdateFlyingState();

defaultproperties
{
	Children(0)=(Name="Not Flying",Weight=0.8)
	Children(1)=(Name="Flying",Weight=0.2)
	bFixNumChildren=true

	StartingAnimName=Wings_Open
	EndingAnimName=Wings_Close
}
