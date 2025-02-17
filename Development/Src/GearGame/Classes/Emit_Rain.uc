/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_Rain extends Emitter
	notplaceable;

//
// Handles rain cylinder around a pawn, splashes, etc
//

var protected transient GearPC				BasePC;

/** Ambient rain sound. */
var() protected const SoundCue				RainLoopCue;
var protected transient AudioComponent      RainLoopAC;

// @fixme, these need to handle being inside
var() protected const bool		bDoRaindropsOnPawn;
var() protected const float		RainDropsOnPawnInterval;
var() protected const int		NumRainDropsToFireAtOnceOnPawn;

/** How far above the BasePawn to spawn. */
var() protected float			ZDistAbovePawn;


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		TurnOnRainSound();
	}

	if (bDoRaindropsOnPawn)
	{
		SetTimer( RainDropsOnPawnInterval, TRUE, nameof(DropRainDropsOntoPawn) );
	}
}

simulated function SetBasedOnPlayer(GearPC PC)
{
	BasePC = PC;

	if (PC.MyGearPawn != None)
	{
		PC.MyGearPawn.bShouldPlayWetFootSteps = TRUE;
	}
}

simulated function FadeOutRain()
{
	// turn off emitter, let particles dissipate
	ParticleSystemComponent.DeactivateSystem();

	// turn off sound
	TurnOffRainSound();

	ClearTimer('DropRainDropsOntoPawn');
}

simulated function Destroyed()
{
	// @fixme, if lifetome of 2 rain effects intersects on same pawn, this
	// could be turn it off when it wants to be on.  want a refcount-like approach here?
	if ( (BasePC != None) && (BasePC.MyGearPawn != None) )
	{
		BasePC.MyGearPawn.bShouldPlayWetFootSteps = FALSE;
	}
}

simulated protected function TurnOnRainSound()
{
	RainLoopAC = CreateAudioComponent( RainLoopCue, TRUE, TRUE );
	if (RainLoopAC != None)
	{
		RainLoopAC.bAutoDestroy = TRUE;
		RainLoopAC.bStopWhenOwnerDestroyed = FALSE;
		RainLoopAC.Location = Location;
		RainLoopAC.FadeIn( 2.f, 1.0f );
	}
}

simulated protected function TurnOffRainSound()
{
	if (RainLoopAC != None)
	{
		RainLoopAC.FadeOut( 3.0f, 0.0f );
		RainLoopAC = None;
	}
}

simulated function Tick(float DeltaTime)
{
	local vector POVLoc, NewLoc;
	local rotator POVRot, NewRot;

	super.Tick(DeltaTime);
	//DrawDebugSphere(Location, 12, 10, 255, 255, 0);

	// calc new emitter location.  This should be directly above the camera.
	BasePC.GetPlayerViewPoint(POVLoc, POVRot);
	
	NewLoc = POVLoc;
	NewLoc.Z += ZDistAbovePawn;

	NewRot = POVRot;		//@Fixme, just keep it steady instead or rotating with camera?  doesnt matter maybe
	NewRot.Pitch = 0;
	NewRot.Roll = 0;

	SetLocation(NewLoc);
	SetRotation(NewRot);
}

simulated protected function Emitter GetPawnImpactEmitter( vector SpawnLocation, rotator SpawnRotation )
{
	return GearGRI(WorldInfo.GRI).GOP.GetRainEmitter_Self( SpawnLocation, SpawnRotation );
}

/**
 * This will trace a line down from above the pawn and then hit the pawn's Ragdoll physic bodies.
 * At the impact location it will then grab an emitter from the object pool and ActiveSystem on that
 * emitter making a nice randomized location rain drop on the pawn.
 *
 * @TODO:  possibly cache the relative locations
 **/
simulated protected function DropRainDropsOntoPawn()
{
	local vector DistAboveHead;
	local vector TraceFrom, TraceEnd, Extent;
	local TraceHitInfo HitInfo;
	local Actor HitActor;

	local vector PS_Location;
	local vector PS_HitNormal;

	local float XDist;
	local float YDist;

	local int RainDropCount;

	local Emitter EM_RainImpact;
	local GearPawn BasePawn;

	if ( (BasePC == None) || (BasePC.MyGearPawn == None) )
	{
		return;
	}

	BasePawn = BasePC.MyGearPawn;

	// check if sky is clear over our heads
	TraceEnd = BasePawn.Location + vect(0,0,1) * 2048.f;
	TraceFrom = BasePawn.Location;
	Extent.X = BasePawn.GetCollisionRadius();
	Extent.Y = Extent.X;
	Extent.Z = Extent.X;
	HitActor = Trace(PS_Location, PS_HitNormal, TraceEnd, TraceFrom,, Extent);
	if (HitActor != None)
	{
		// something above our heads protecting us, no splashes on pawn
		return;
	}

	for( RainDropCount = 0; RainDropCount < NumRainDropsToFireAtOnceOnPawn; ++RainDropCount)
	{
		//class'GearPawn_COGMarcus'.default.CylinderComponent.CollisionRadius
		// the default CollisionRadius for our pawns is 34 for most of them
		// we probably need a getClass() function that can be called
		XDist = (44) * RandRange( -1.0, 1.0 );
		YDist = (44) * RandRange( -1.0, 1.0 );

		DistAboveHead = vect(1,0,0) * XDist + vect(0,1,0) * YDist + Vect(0,0,300);

		TraceFrom = BasePawn.Location + DistAboveHead;
		TraceEnd = BasePawn.Location + DistAboveHead * Vect(0,0,-1);

		TraceComponent( PS_Location, PS_HitNormal, BasePawn.Mesh, TraceEnd, TraceFrom, , HitInfo );
		//DrawDebugLine( TraceFrom, PS_Location, 255, 0, 0, TRUE);

		// get the emitter to use for this raindrop
		EM_RainImpact = GetPawnImpactEmitter( PS_Location, Rotator(PS_HitNormal) );

		EM_RainImpact.SetBase(BasePawn); // attach so the rain splash doesn't trail off into space when we are moving

		EM_RainImpact.ParticleSystemComponent.ActivateSystem();
	}
}

simulated function SetHeight(float Height)
{
	local vector NewRelLoc;

	ZDistAbovePawn = Height;

	NewRelLoc.Z = Height;
	SetRelativeLocation(NewRelLoc);
}


defaultproperties
{
	Components.Remove(ArrowComponent0)
	Components.Remove(Sprite)

	bDestroyOnSystemFinish=TRUE
	Begin Object Name=ParticleSystemComponent0
		Template=ParticleSystem'Effects_Gameplay.Rain.P_FX_Rain_Player_Area01'
		SecondsBeforeInactive=0.f
		//bOnlyOwnerSee=TRUE
	End Object

	bNetInitialRotation=true
	RainLoopCue=SoundCue'Ambient_Loops.Water.water_rainroof03Cue'

	bDoRaindropsOnPawn=TRUE
	RainDropsOnPawnInterval=0.15f
	NumRainDropsToFireAtOnceOnPawn=2

	ZDistAbovePawn=64

	bNoDelete=false
}
