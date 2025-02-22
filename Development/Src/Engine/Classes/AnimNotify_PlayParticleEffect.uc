/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AnimNotify_PlayParticleEffect extends AnimNotify
	native(Anim);

/** The Particle system to play **/
var() ParticleSystem PSTemplate;

/** If this effect should be considered extreme content **/
var() bool bIsExtremeContent;

/** If this particle system should be attached to the location.**/
var() bool bAttach;

/** The socketname in which to play the particle effect.  Looks for a socket name first then bone name **/
var() name SocketName;

/** The bone name in which to play the particle effect. Looks for a socket name first then bone name **/
var() name BoneName;


cpptext
{
	// AnimNotify interface.
	virtual void Notify( class USkeletalMeshComponent* SkelComponent );
}

defaultproperties
{

}

