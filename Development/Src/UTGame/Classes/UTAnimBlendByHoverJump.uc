/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAnimBlendByHoverJump extends UTAnimBlendByFall
	Native(Animation);

var const transient Pawn	OwnerP;
var const transient UTHoverVehicle OwnerHV;
cpptext
{
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent );
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );
}


defaultproperties
{
	bIgnoreDoubleJumps=true
}
