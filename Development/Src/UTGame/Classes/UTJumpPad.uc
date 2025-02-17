/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
//=============================
// UTJumppad - bounces players/bots up
//
//=============================
class UTJumpPad extends NavigationPoint
	native
	placeable
	hidecategories(VehicleUsage);

var		vector				JumpVelocity;
var()	PathNode			JumpTarget;
var()	SoundCue			JumpSound;
var()	float				JumpTime;
var()	float				JumpAirControl;
var AudioComponent			JumpAmbientSound;
cpptext
{
	virtual void addReachSpecs(AScout *Scout, UBOOL bOnlyChanged=0);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostEditMove(UBOOL bFinished);
	UBOOL CalculateJumpVelocity(AScout *Scout);
}

event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
{
	if ( (UTPawn(Other) == None) || (Other.Physics == PHYS_None) || UTPawn(Other).IsHero() )
		return;

	PendingTouch = Other.PendingTouch;
	Other.PendingTouch = self;
}

event PostTouch(Actor Other)
{
	local UTPawn P;

	P = UTPawn(Other);
	if (P == None || P.Physics == PHYS_None || P.DrivenVehicle != None)
	{
		return;
	}
	if ( P.bNotifyStopFalling )
	{
		P.StoppedFalling();
	}

	if ( UTBot(P.Controller) != None )
	{
		if ( Other.GetGravityZ() > WorldInfo.DefaultGravityZ )
			UTBot(P.Controller).Focus = UTBot(P.Controller).FaceActor(2);
		else
			P.Controller.Focus = JumpTarget;
		P.Controller.Movetarget = JumpTarget;
		if ( P.Physics != PHYS_Flying )
			P.Controller.MoveTimer = 2.0;
		P.DestinationOffset = 50;
	}
	if ( P.Physics == PHYS_Walking )
	{
		P.SetPhysics(PHYS_Falling);
		P.bReadyToDoubleJump = true;
	}
	P.Velocity = JumpVelocity;
	if ( (WorldInfo.WorldGravityZ != WorldInfo.DefaultGravityZ) && (Other.GetGravityZ() == WorldInfo.WorldGravityZ) )
	{
		P.Velocity *= sqrt(Other.GetGravityZ()/WorldInfo.DefaultGravityZ);
	}
	P.AirControl = JumpAirControl;
	P.Acceleration = vect(0,0,0);
	if ( JumpSound != None )
		P.PlaySound(JumpSound);
}

event bool SuggestMovePreparation(Pawn Other)
{
	local Vehicle V;

	V = Vehicle(Other);
	if (V != None)
	{
		V.DriverLeave(false);
	}

	return false;
}

defaultproperties
{
	bDestinationOnly=true
	bCollideActors=true
	JumpTime=2.0
	JumpAirControl=0.05
	bHidden=false
	bBlockedForVehicles=true
	JumpSound=SoundCue'A_Gameplay.JumpPad.Cue.A_Gameplay_JumpPad_Activate_Cue'
	bMovable=false
	bNoDelete=true
	bStatic=false
	bSpecialMove=true
	Components.Remove(Sprite)
	Components.Remove(Sprite2)
	GoodSprite=None
	BadSprite=None

	Begin Object Name=CollisionCylinder
		CollideActors=true
	End Object


 	// define here as lot of sub classes which have moving parts will utilize this
	Begin Object Class=DynamicLightEnvironmentComponent Name=JumpPadLightEnvironment
	    bDynamic=FALSE
		bCastShadows=FALSE
	End Object
	Components.Add(JumpPadLightEnvironment)

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Pickups.jump_pad.S_Pickups_Jump_Pad'
		CollideActors=false
		Scale3D=(X=1.0,Y=1.0,Z=1.0)
		Translation=(X=0.0,Y=0.0,Z=-47.0)

		CastShadow=FALSE
		bCastDynamicShadow=FALSE
		bAcceptsLights=TRUE
		bForceDirectLightMap=TRUE
		LightingChannels=(BSP=TRUE,Dynamic=FALSE,Static=TRUE,CompositeDynamic=FALSE)
		LightEnvironment=JumpPadLightEnvironment
		bUseAsOccluder=FALSE
	End Object
 	Components.Add(StaticMeshComponent0)

	Begin Object Class=UTParticleSystemComponent Name=ParticleSystemComponent1
		Translation=(X=0.0,Y=0.0,Z=-35.0)
		Template=particleSystem'Pickups.jump_pad.P_Pickups_Jump_Pad_FX'
		bAutoActivate=true
		SecondsBeforeInactive=1.0f
	End Object
	Components.Add(ParticleSystemComponent1)

	Begin Object Class=AudioComponent Name=AmbientSound
		SoundCue=SoundCue'A_Gameplay.JumpPad.JumpPad_Ambient01Cue'
		bAutoPlay=true
		bUseOwnerLocation=true
		bShouldRemainActiveIfDropped=true
		bStopWhenOwnerDestroyed=true
	End Object
	JumpAmbientSound=AmbientSound
	Components.Add(AmbientSound)
}
