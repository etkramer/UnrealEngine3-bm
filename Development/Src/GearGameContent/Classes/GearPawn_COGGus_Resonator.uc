/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn_COGGus_Resonator extends GearPawn_COGGus
	config(Pawn);

/** Sonic Resonator Mesh Attachment **/
var StaticMeshComponent SonicResonatorMesh;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Attach Sonic Resonator
	if( Mesh != None && SonicResonatorMesh != None )
	{
		// attach the head mesh to the body
		SonicResonatorMesh.SetShadowParent(Mesh);
		SonicResonatorMesh.SetLightEnvironment(LightEnvironment);
		SonicResonatorMesh.SetHidden(FALSE);
		Mesh.AttachComponentToSocket(SonicResonatorMesh, 'Resonator');
	}
}

defaultproperties
{
	Begin Object Class=StaticMeshComponent Name=SonicResonator0
		CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=TRUE
		AlwaysLoadOnClient=TRUE
		StaticMesh=StaticMesh'COG_SonicResonator.COG_SonicResonator_All'
		bCastDynamicShadow=FALSE
	End Object
	SonicResonatorMesh=SonicResonator0
}