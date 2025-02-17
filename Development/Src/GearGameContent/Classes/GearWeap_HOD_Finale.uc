
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_HOD_Finale extends GearWeap_HOD;

defaultproperties
{
	BeamManagerClass=class'HOD_BeamManager_Finale'

	TargetingBeamFireSound=SoundCue'Weapon_HammerDawn.HODCopter.HODFire01Cue'
	TargetingBeamPosFeedbackSound=SoundCue'Weapon_HammerDawn.HODCopter.HODTargetPositive01Cue'
	TargetingBeamNegFeedbackSound=SoundCue'Weapon_HammerDawn.HODCopter.HODTargetNegative01Cue'
	TargetingBeamLockedFeedbackSound=SoundCue'Weapon_HammerDawn.HODCopter.HODTargetLocked01Cue'

	Begin Object Name=TargetingBeamLoopSound0
		SoundCue=SoundCue'Weapon_HammerDawn.HODCopter.HODFireBeam01Cue'
	End Object
}
