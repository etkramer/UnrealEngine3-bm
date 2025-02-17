
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GSM_Move2IdleTransition extends GearSpecialMove;

function bool CanChainMove(ESpecialMove NextMove)
{
	// all special moves can chain/interrupt move2idle
	return TRUE;
}

function bool CanOverrideMoveWith(ESpecialMove NewMove)
{
	// all special moves can override this move...
	return TRUE;
}

protected function bool InternalCanDoSpecialMove()
{
	return (!PawnOwner.IsDoingASpecialMove());
}

function SpecialMoveStarted(bool bForced, ESpecialMove PrevMove)
{
	Super.SpecialMoveStarted(bForced,PrevMove);

	if( PawnOwner.WorldInfo.NetMode != NM_DedicatedServer )
	{
		PlaySlideToStopParticleEffect(PawnOwner.Rotation);
	}

	// Push Pawns only on locally owned clients or server.
	// This equals to the Pawn having a controller set
	if( PawnOwner.Controller != None )
	{
		// Force Pawn to keep moving
		PawnOwner.bForceMaxAccel	= TRUE;
		// Set acceleration to match direction Pawn is currently moving to.
		PawnOwner.Acceleration		= Normal(PawnOwner.Velocity) * PawnOwner.AccelRate * SpeedModifier;
	}
}

function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	Super.SpecialMoveEnded(PrevMove, NextMove);

	// Disable forced movement
	PawnOwner.bForceMaxAccel = FALSE;

	// Turn off Root motion on mesh.
	// We do this here, so we can transition properly to other special moves using root motion.
	PawnOwner.Mesh.RootMotionMode = RMM_Ignore;
}


/**
 * This will play the slide particle effect by tracing down and then spawning a particle there
 */
simulated function PlaySlideToStopParticleEffect(Rotator SpawnRotation )
{
	local vector PawnLoc;
	local float CurrHeight;

	local Actor TraceActor;

	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;

	local Emitter SlideDust;

	// kk here we need to do a tracez0r down down into the ground baby!
	// NOTE: this will eventually be moved to c++ land
	PawnLoc		= PawnOwner.Location;
	CurrHeight	= PawnOwner.GetCollisionHeight();

	TraceStart	= PawnLoc;
	TraceDest	= PawnLoc - ( Vect(0, 0, 1 ) * CurrHeight ) - Vect(0, 0, 15 );

	// trace down and see what we are standing on
	TraceActor = PawnOwner.Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, PawnOwner.TRACEFLAG_PhysicsVolumes );

	//DrawDebugLine( TraceStart, TraceDest, 255, 0, 0, TRUE);

	if( TraceActor != none )
	{
		//PawnOwner.DrawDebugCoordinateSystem( out_HitLocation, SpawnRotation, 100, TRUE );
		SlideDust = GearGRI(PawnOwner.WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Slide', out_HitLocation, SpawnRotation );
		SlideDust.ParticleSystemComponent.ActivateSystem();
	}
	// otherwise we hit nothing and are in the air
	else
	{
		//`Log( " We are in the air" );
	}

}

defaultproperties
{
}
