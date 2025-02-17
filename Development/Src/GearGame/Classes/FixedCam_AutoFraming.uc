/**
 * "Fixed" camera mode.  Views through a CameraActor in the level.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class FixedCam_AutoFraming extends Object
	native(Camera)
	config(Camera);


/** Ref to the camera object that owns this mode object. */
var transient GearFixedCamera	FixedCam;


var() const protected float RelativeWeight_TargetPawn;
var() const protected float RelativeWeight_Speaker;
var() const protected float RelativeWeight_Normal;

var protected transient vector LastLookat;
var() const protected float LookatInterpSpeed;

var protected transient float LastFOV;
var() const protected float FOVInterpSpeed;

// test var
var() bool bDoFOVAdj;


simulated native function UpdateCamera(CameraActor CamActor, Pawn TargetPawn, float DeltaTime, out TViewTarget OutVT);

simulated function UpdateCamera2(CameraActor CamActor, Pawn TargetPawn, float DeltaTime, out TViewTarget OutVT)
{
	// @todo: clean this up and do it in native code


	local GearPawn GP;
	local array<GearPawn> VisibleTeammateList;

	local int Idx;
	local float RelWeight, TotalWeight;
	local array<float> RelativeWeights;
	local vector WeightedLookat;


	local vector BoundingSphereCenter, BoxMin, BoxMax;
	local float	BoundingSphereRadius;

	local vector CamX, CamY, CamZ, LeftExtent, RightExtent, CamToExtentNorm;
	local float Angle, MaxSphereExtentFOV;


	// always at the camactor loc
	OutVT.POV.Location = CamActor.Location;

	// this is just simple TargetPawn tracking
	//local GearPawn TrackPawn;
	//TrackPawn = GearPawn(TargetPawn);
	//OutVT.POV.FOV = CamActor.FOVAngle;
	//OutVT.POV.Rotation = rotator(TrackPawn.Location - CamActor.Location);
	//OutVT.POV.Location = CamActor.Location;



	// autoframing rules
	// try to keep all teammates onscreen
	// favor the speaker
	// strongly favor the TargetPawn
	// only consider visible pawns (trace check)
	// set FOV (nearly) as tight as possible


	// get list of pawns we'll try and keep onscreen.
	foreach CamActor.WorldInfo.AllPawns(class'GearPawn', GP)
	{
		if ( GP.IsSameTeam(TargetPawn) && GP.FastTrace(GP.Location, CamActor.Location) )
		{
			VisibleTeammateList.AddItem(GP);

			// figure and store the relative weighting of this teammate
			if (GP == TargetPawn)
			{
				RelWeight = RelativeWeight_TargetPawn;
			}
			//@fixme, include GUDS here?
			else if ( (GP.CurrentlySpeakingLine != None) && (GP.CurrentSpeakLineParams.Priority >= Speech_GUDS) )
			{
				RelWeight = RelativeWeight_Speaker;
			}
			else
			{
				RelWeight = RelativeWeight_Normal;
			}

			RelativeWeights.AddItem(RelWeight);
			TotalWeight += RelWeight;
		}
	}

	// no one visible, just use default camera orientation and bail
	if (VisibleTeammateList.length == 0)
	{
		OutVT.POV.Rotation = CamActor.Rotation;
		OutVT.POV.FOV = CamActor.FOVAngle;
		return;
	}


	// normalize relative weights and calc weighted lookat point
	for (Idx=0; Idx<RelativeWeights.length; ++Idx)
	{
		RelativeWeights[Idx] /= TotalWeight;
		WeightedLookat += VisibleTeammateList[Idx].Location * RelativeWeights[Idx];

		//BoundingSphereCenter += VisibleTeammateList[Idx].Location / float(RelativeWeights.length);
	}

	if (!FixedCam.bResetCameraInterpolation)
	{
		// interpolate for smoothness
		WeightedLookat = VInterpTo(LastLookat, WeightedLookat, DeltaTime, LookatInterpSpeed);
	}

	OutVT.POV.FOV = CamActor.FOVAngle;
	OutVT.POV.Rotation = rotator(WeightedLookat - CamActor.Location);

//	CamActor.DrawDebugSphere(WeightedLookat, 16, 10, 255, 255, 0);

	LastLookat = WeightedLookat;


	// try to set an optimal fov
	BoxMin = VisibleTeammateList[0].Location;
	BoxMax = VisibleTeammateList[0].Location;
	foreach VisibleTeammateList(GP)
	{
		// @fixme, do this in above loop
		GrowBox(BoxMin, BoxMax, GP.Location);
	}

	BoundingSphereCenter = (BoxMin + BoxMax) * 0.5f;
	BoundingSphereRadius = FMax( (BoxMax.X-BoxMin.X), (BoxMax.Y-BoxMin.Y) );
	BoundingSphereRadius = FMax( BoundingSphereRadius, (BoxMax.Z-BoxMin.Z) );
	BoundingSphereRadius *= 0.5f;
	BoundingSphereRadius = FMax(400, BoundingSphereRadius);		// to prevent it getting to small.  what's a sensible limit here?  could get tight on faces, which might be cool

//	CamActor.DrawDebugSphere(BoundingSphereCenter, BoundingSphereRadius, 12, 255, 255, 255);

	// make sure hfov is large enough to see entire sphere
	// @todo, maybe make sure vfov is large enough as well?  only if losing guys off bottom/top is a big problem.
	GetAxes(OutVT.POV.Rotation, CamX, CamY, CamZ);

	LeftExtent = BoundingSphereCenter - CamY * BoundingSphereRadius;
	RightExtent = BoundingSphereCenter + CamY * BoundingSphereRadius;

	// left
	CamToExtentNorm = Normal(LeftExtent - OutVT.POV.Location);
	Angle = Abs(ACos(CamToExtentNorm dot CamX) * RadToDeg);
	MaxSphereExtentFOV = FMax(MaxSphereExtentFOV, Angle);

	// right
	CamToExtentNorm = Normal(RightExtent - OutVT.POV.Location);
	Angle = Abs(ACos(CamToExtentNorm dot CamX) * RadToDeg);
	MaxSphereExtentFOV = FMax(MaxSphereExtentFOV, Angle);

	// convert to full angles
	MaxSphereExtentFOV *= 2.f;

	if (!FixedCam.bResetCameraInterpolation)
	{
		MaxSphereExtentFOV = FInterpTo(LastFOV, MaxSphereExtentFOV, DeltaTime, FOVInterpSpeed);
	}
	if (bDoFOVAdj)
	{
		OutVT.POV.FOV = MaxSphereExtentFOV;
	}
	LastFOV = MaxSphereExtentFOV;


}


simulated private function GrowBox(out vector BoxMin, out vector BoxMax, vector Pt)
{
	BoxMin.X = FMin(BoxMin.X, Pt.X);
	BoxMin.Y = FMin(BoxMin.Y, Pt.Y);
	BoxMin.Z = FMin(BoxMin.Z, Pt.Z);

	BoxMax.X = FMax(BoxMax.X, Pt.X);
	BoxMax.Y = FMax(BoxMax.Y, Pt.Y);
	BoxMax.Z = FMax(BoxMax.Z, Pt.Z);
}

defaultproperties
{
	RelativeWeight_TargetPawn=1.f
	RelativeWeight_Speaker=1.f
	RelativeWeight_Normal=1.f

	bDoFOVAdj=TRUE
	FOVInterpSpeed=2.f
	LookatInterpSpeed=1.f
}