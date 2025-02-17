
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAnim_BlendBySpeed extends AnimNodeBlendList
		native(Anim);

/** How fast they are moving this frame.							*/
var float			Speed;
/** Last Channel being used											*/
var int				LastChannel;		
/** How fast to blend when going up									*/
var() float			BlendUpTime;		
/** How fast to blend when going down								*/
var() float			BlendDownTime;
/** When should we start blending back down							*/
var() float			BlendDownPerc;
/** Weights/ constraints used for transition between child nodes	*/
var() array<float>	Constraints;

cpptext
{
	/**
	 * Blend animations based on an Owner's velocity.
	 *
	 * @param DeltaSeconds	Time since last tick in seconds.
	 */
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight );

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
	
	// AnimNodeBlendBySpeed interface

	/** 
	 *	Function called to calculate the speed that should be used for this node. 
	 *	Allows subclasses to easily modify the speed used.
	 */
	 virtual FLOAT CalcSpeed();
}

defaultproperties
{
	bSkipTickWhenZeroWeight=TRUE
	BlendUpTime=0.1;
    BlendDownTime=0.1;
    BlendDownPerc=0.2;
	Constraints=(0,180,350,900);
}
