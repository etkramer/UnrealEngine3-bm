/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAnimBlendByTurnInPlace extends UTAnimBlendBase
	native(Animation);

var() float	RootYawSpeedThresh;
var() float TurnInPlaceBlendSpeed;
var const transient UTPawn OwnerUTP;

cpptext
{
	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim( float DeltaSeconds, FLOAT TotalWeight  );
	virtual void OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime);
}

defaultproperties
{
	Children(0)=(Name="Idle",Weight=1.0)
	Children(1)=(Name="TurnInPlace")
	bFixNumChildren=true
}
