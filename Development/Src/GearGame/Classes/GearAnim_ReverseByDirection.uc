/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_ReverseByDirection extends AnimNodeSequence
	native(Anim);

/** If TRUE invert Left and Right */
var()		bool	bInvertDirection;

/** Flag telling if direction is reversed or not */
var	const	bool	bReversed;
var const	FLOAT	SliderPosition;

/** Internal cached pointer to GearPawn Owner */
var	const transient GearPawn		GearPawnOwner;

cpptext
{
	// AnimNode interface
	virtual void InitAnim(USkeletalMeshComponent* MeshComp, UAnimNodeBlendBase* Parent);
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}
