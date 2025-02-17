//=============================================================================
// VehicleMovementEffect
//  Is the visual effect that is spawned by someone on a vehicle
//  
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================

class VehicleMovementEffect extends Actor
	native(Vehicle);

/** The static mesh that can be used as a speed effect*/
var staticmeshcomponent AirEffect;
/** slower than this will disable the effect*/
var float MinVelocityForAirEffect;
/** At this speed the air effect is at full level */
var float MaxVelocityForAirEffect;
/** the param in the material(0) of the AirEffect to scale from 0-1*/
var name AirEffectScalar;


var float AirMaxDelta;
var float AirCurrentLevel;

cpptext
{
	virtual void TickSpecial(FLOAT DeltaTime);
}
defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=AerialMesh
		StaticMesh=StaticMesh'Envy_Effects.Mesh.S_Air_Wind_Ball'//StaticMesh'Envy_Effects.Mesh.S_FX_Vehicle_Holocube_01'
		//Materials(0)=Material'Envy_Effects.Materials.M_Air_Wind_Ball'
		CollideActors=false
		CastShadow=false
		bAcceptsLights=false
		bOnlyOwnerSee=true
		MaxDrawDistance=7500
		BlockRigidBody=false
		BlockActors=false
		Scale3D=(X=80.0,Y=60.0,Z=60.0)
		Translation=(Z=-100.0)
		bUseAsOccluder=FALSE
		AbsoluteRotation=true
	End Object
	Components.Add(AerialMesh)
	AirEffect=AerialMesh

	MinVelocityForAirEffect=15000.0f
	MaxVelocityForAirEffect=850000.0f
	AirMaxDelta=0.05f;
	AirEffectScalar=Wind_Opacity
}