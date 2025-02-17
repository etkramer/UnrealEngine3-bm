/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDebugFaceCamera extends GearCameraBase
	config(Camera);

var() float FaceDist;

/** debug-only camera, used to see character's face for testing FaceFX and stuff */
simulated function UpdateCamera(Pawn P, float DeltaTime, out TViewTarget OutVT)
{
	local GearPawn GP;
	local vector HeadPos, CamPos, Unused;
	local rotator Rot;

	GP = GearPawn(P);

	if (GP != None)
	{
		P.GetActorEyesViewPoint(Unused, Rot);

		HeadPos = GP.Mesh.GetBoneLocation(GearPawn(P).HeadBoneNames[0], 0);
		CamPos = HeadPos + vector(Rot) * FaceDist;

		Rot = rotator(HeadPos - CamPos);

		OutVT.POV.Location = CamPos;
		OutVT.POV.Rotation = Rot;
	}
}

defaultproperties
{
	FaceDist=48.f
}