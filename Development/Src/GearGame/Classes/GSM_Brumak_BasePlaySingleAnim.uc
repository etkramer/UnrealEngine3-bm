
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GSM_Brumak_BasePlaySingleAnim extends GSM_BasePlaySingleAnim
	native(SpecialMoves)
	abstract;

/** Reference to Brumak Pawn. */
var GearPawn_LocustBrumakBase	Brumak;

/** Setup for playing camera shakes on nearby human players, synched w/ Brumak animations */
var Name		NearbyPlayerSynchedCameraAnimName;
var Vector2D	NearbyPlayerSynchedCameraAnimRadius;
var float		NearbyPlayerSynchedCameraAnimScale;
/** Dampen camera shake when targeting */
var	bool		bCamShakeDampenWhenTargeting;

function SpecialMoveStarted( bool bForced, ESpecialMove PrevMove )
{
	Super.SpecialMoveStarted( bForced, PrevMove );

	// Keep reference to Brumak
	Brumak = GearPawn_LocustBrumakBase(PawnOwner);

	if( NearbyPlayerSynchedCameraAnimName != '' )
	{
		PlaySpatializedCustomCameraAnim(NearbyPlayerSynchedCameraAnimName, NearbyPlayerSynchedCameraAnimRadius, SpeedModifier, BlendInTime, BlendOutTime, FALSE, FALSE, FALSE, TRUE, NearbyPlayerSynchedCameraAnimScale, bCamShakeDampenWhenTargeting);
	}
}

defaultproperties
{
	bCamShakeDampenWhenTargeting=FALSE
	NearbyPlayerSynchedCameraAnimRadius=(X=1280,Y=5120)
	NearbyPlayerSynchedCameraAnimScale=1.f
}