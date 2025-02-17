/** 
 * Grapple rope object for grappling enemies.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GrappleRope extends GrappleRopeBase;


defaultproperties
{
	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		TickGroup=TG_DuringAsyncWork
		bCastShadows=FALSE
	End Object
	Components.Add(MyLightEnvironment)

	Begin Object Class=StaticMeshComponent Name=HookMesh0
		StaticMesh=StaticMesh'AB_Truck_Assault.Meshes.SM_Grappling_Hook'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockRigidBody=FALSE
		LightEnvironment=MyLightEnvironment
	End Object
	HookMesh=HookMesh0
	Components.Add(HookMesh0)

	FlightTime=1.f

	TmpStartClimbOffset=(X=-64,Z=-190);
}