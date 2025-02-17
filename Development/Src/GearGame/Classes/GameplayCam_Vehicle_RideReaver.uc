/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 * Camera mode for when player is in a vehicle.
 */
class GameplayCam_Vehicle_RideReaver extends GameplayCam_Vehicle
	native(Camera)
	config(Camera);

var()	float	FOVBlendTime;
var()	float	OnLandAdjustZ;

cpptext
{
	virtual FLOAT GetFOVBlendTime(class APawn* Pawn);
	virtual FLOAT GetBlendTime(class APawn* Pawn);
}

simulated native function GetBaseViewOffsets(Pawn ViewedPawn, EGearCam_ViewportTypes ViewportConfig, float DeltaTime, out Vector out_Low, out Vector out_Mid, out Vector out_High);

simulated native function GetCameraOrigin(Pawn TargetPawn, out vector OriginLoc, out rotator OriginRot);

simulated function vector GetCameraWorstCaseLoc( Pawn TargetPawn )
{
	local Vector CamLoc, FullMoveLoc;
	local Vehicle_RideReaver_Base RR;
	local GearWeaponPawn WP;

	CamLoc = Super.GetCameraWorstCaseLoc( TargetPawn );

	// Put camera in the center of the interp path with same offset as camera from seat
	RR = Vehicle_RideReaver_Base(TargetPawn);
	if (RR == None)
	{
		WP = GearWeaponPawn(TargetPawn);
		if (WP != None)
		{
			RR = Vehicle_RideReaver_Base(WP.MyVehicle);
		}
	}

	if( RR != None && !RR.bPlayLanding && !IsZero(RR.LastOrigInterpLocation) )
	{
		FullMoveLoc = RR.LastOrigInterpLocation + (CamLoc - RR.Location);
		CamLoc = VLerp(CamLoc, FullMoveLoc, RR.DodgeCamAmount);
	}

	return CamLoc;
}

defaultproperties
{
	OnLandAdjustZ=100.f
	FOVBlendTime=0.13
	BlendTime=0.1
	bSkipCameraCollision=TRUE
}
