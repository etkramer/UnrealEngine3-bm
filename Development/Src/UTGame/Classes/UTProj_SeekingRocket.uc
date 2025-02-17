/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTProj_SeekingRocket extends UTProj_Rocket
	native;

var Actor Seeking;
var vector InitialDir;
var bool bSuperSeekAirTargets;

/** The last time a lock message was sent */
var float	LastLockWarningTime;

/** How long before re-sending the next Lock On message update */
var float	LockWarningInterval;

/** Tracking strength multiplier */
var float BaseTrackingStrength;

replication
{
    if( bNetInitial && (Role==ROLE_Authority) )
        Seeking, InitialDir;
}

cpptext
{
	virtual void TickSpecial(float DeltaTime);
}

defaultproperties
{
    MyDamageType=class'UTDmgType_SeekingRocket'
    LifeSpan=8.000000
    bRotationFollowsVelocity=true
	CheckRadius=0.0
	BaseTrackingStrength=1.0
}
