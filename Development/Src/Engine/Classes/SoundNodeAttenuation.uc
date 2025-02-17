/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeAttenuation extends SoundNode
	native( Sound )
	collapsecategories
	hidecategories( Object )
	editinlinenew;

enum SoundDistanceModel
{
	ATTENUATION_Linear,
	ATTENUATION_Logarithmic,
	ATTENUATION_Inverse,
	ATTENUATION_LogReverse,
	ATTENUATION_NaturalSound
};

enum ESoundDistanceCalc
{
	SOUNDDISTANCE_Normal,
	SOUNDDISTANCE_InfiniteXYPlane,
	SOUNDDISTANCE_InfiniteXZPlane,
	SOUNDDISTANCE_InfiniteYZPlane,
};

var( Attenuation )		bool					bAttenuate<ToolTip=Enable attenuation via volume>;
var( Spatialization )	bool					bSpatialize<ToolTip=Enable the source to be positioned in 3D>;

/** What kind of attenuation model to use */
var( Attenuation )		SoundDistanceModel		DistanceModel<ToolTip=The type of volume versus distance algorithm to use>;

/** How to calculate the distance from the sound to the listener */
var( Attenuation )		ESoundDistanceCalc		DistanceType<ToolTip=Special attenuation modes>;

/* The settings for attenuating. */
var( Attenuation )		rawdistributionfloat	MinRadius;
var( Attenuation )		rawdistributionfloat	MaxRadius;
var( Attenuation )		float					dBAttenuationAtMax<ToolTip=The volume at maximum distance in deciBels>;

/* The settings for attenuating with a low pass filter. */
var( LowPassFilter )	bool					bAttenuateWithLowPassFilter<ToolTip=Enable attenuation via low pass filter>;
var( LowPassFilter )	rawdistributionfloat	LPFMinRadius;
var( LowPassFilter )	rawdistributionfloat	LPFMaxRadius;

cpptext
{
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return ::Max<FLOAT>(CurrentMaxDistance,MaxRadius.GetValue(0.9f)); }
}

defaultproperties
{
	Begin Object Class=DistributionFloatUniform Name=DistributionMinRadius
		Min=400
		Max=400
	End Object
	MinRadius=(Distribution=DistributionMinRadius)

	Begin Object Class=DistributionFloatUniform Name=DistributionMaxRadius
		Min=5000
		Max=5000
	End Object
	MaxRadius=(Distribution=DistributionMaxRadius)
	dBAttenuationAtMax=-60;

	Begin Object Class=DistributionFloatUniform Name=DistributionLPFMinRadius
		Min=1500
		Max=1500
	End Object
	LPFMinRadius=(Distribution=DistributionLPFMinRadius)

	Begin Object Class=DistributionFloatUniform Name=DistributionLPFMaxRadius
		Min=5000
		Max=5000
	End Object
	LPFMaxRadius=(Distribution=DistributionLPFMaxRadius)

	bSpatialize=true
	bAttenuate=true
	bAttenuateWithLowPassFilter=true
}
