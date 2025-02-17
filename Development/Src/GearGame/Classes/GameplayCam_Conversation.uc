/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameplayCam_Conversation extends GearGameplayCameraMode
	dependson(GearPlayerCamera)
	native(Camera)
	config(Camera);

/** Internal.  Keeps track of who we were looking at last. */
var private transient Actor LastFocusActor;

/** Internal.  Who the camera is attached to. */
var private transient Actor CameraTarget;

/** Interpolation speeds used by the focus point code. */
var const vector2d CameraInterpSpeedRange;

/** FOVs of the "in focus" field, used by the focus point */
var const vector2d CameraInFocusFOV;

/** Internal.  Used to force the focus point to be set, instead of just updating it's location. */
var private transient bool bSetFocusPointNextTick;

simulated function OnBecomeActive(Pawn InCamTarget, GearGameplayCameraMode PrevMode)
{
	local GearSpeechManager GSM;

	CameraTarget = InCamTarget;

	// looking at cameratarget by default
	LastFocusActor = InCamTarget;
	bSetFocusPointNextTick = TRUE;

	GSM = GearGRI(GameplayCam.PlayerCamera.WorldInfo.GRI).SpeechManager;
	if (GSM != None)
	{
		GSM.BeginDialogueTracking();
	}

	super.OnBecomeActive(InCamTarget, PrevMode);
}

simulated function OnBecomeInActive(Pawn TargetPawn, GearGameplayCameraMode NewMode)
{
	local GearSpeechManager GSM;

	GSM = GearGRI(GameplayCam.PlayerCamera.WorldInfo.GRI).SpeechManager;
	if (GSM != None)
	{
		GSM.EndDialogueTracking();
	}

	super.OnBecomeInActive(TargetPawn, NewMode);
}

simulated native function GetCameraOrigin(Pawn TargetPawn, out vector OriginLoc, out rotator OriginRot);

simulated function bool SetFocusPoint(Pawn ViewedPawn)
{
	local Actor FocusActor;
	local GearSpeechManager GSM;
	local GearPawn FocusGP;
	local Actor Speaker, Addressee;

//	local vector FocusLoc;
	local name FocusBoneName;

	GSM = GearGRI(GameplayCam.PlayerCamera.WorldInfo.GRI).SpeechManager;

	if (GSM != None)
	{
		if (GSM.DialogueStack.Length > 0)
		{
			Speaker = GSM.DialogueStack[0].Speaker;
			Addressee = GSM.DialogueStack[0].Addressee;
		}

		// look at whoever's on top of the stack
		// @todo: don't look if line is almost over or if line is too short?
		if (Speaker == None)
		{
			// no one talking right now, just look at what we were looking at before
			FocusActor = LastFocusActor;
		}
		else if (Speaker == CameraTarget)
		{
			// camera target is talking...
			if (Addressee == None)
			{
				// leave focus where it was before
				FocusActor = LastFocusActor;
			}
			else
			{
				// focus on addressee
				FocusActor = Addressee;
			}
		}
		else
		{
			// someone other than the camera target is talking
			FocusActor = Speaker;
		}

		// just to be sure
		if (FocusActor == None)
		{
			FocusActor = CameraTarget;
		}
		
		// debug
//		if (FocusActor != LastFocusActor)
//		{
//			`log("*** changing focus from"@LastFocusActor@"to"@FocusActor);
//			`log("   CamTarget"@CameraTarget@"Addressee"@Addressee@GSM.DialogueStack.Length);
//		}

		// figure out exactly where to look
		FocusGP = GearPawn(FocusActor);
		if (FocusGP != None)
		{
			if (FocusActor != CameraTarget)
			{
				// looking at a gearpawn, focus on his head
				FocusBoneName = FocusGP.NeckBoneName;
				//FocusLoc = FocusGP.Mesh.GetBoneLocation(FocusBoneName);
			}
			else
			{
				//// look cameratarget in the face, so focus on a spot behind him
				//FocusBoneName = '';

				//FocusLoc = FocusGP.Location;
				//FocusLoc.Z += FocusGP.BaseEyeHeight;
				//FocusLoc += (vect(-2048,0,0) >> FocusGP.Rotation);
			}
		}
		else
		{	
			// just focus on the actor's loc
			FocusBoneName = '';
//			FocusLoc = FocusActor.Location;
		}

		// actually set the focus point
		if ( (FocusActor == LastFocusActor) && (!bSetFocusPointNextTick) )
		{
			// using same actor, just update loc
			//GameplayCam.UpdateFocusLocation(FocusLoc);
		}
		else
		{
			// new actor, set new focus point
//			`log("*** !!! Conv camera focusing on"@FocusActor);
			GameplayCam.SetFocusOnActor(FocusActor, FocusBoneName, CameraInterpSpeedRange, CameraInFocusFOV,, TRUE, TRUE, FALSE );
			bSetFocusPointNextTick = FALSE;
		}

		// save for next time
		LastFocusActor = FocusActor;

		return TRUE;	//(LastFocusActor != None);
	}

	// no one speaking, so we aren't setting a focus
	return FALSE;
}

defaultproperties
{
	// GearCam_Conversation vars

	CameraInterpSpeedRange=(X=3.f,Y=3.f)
	CameraInFocusFOV=(X=3.f,Y=3.f)

	// GearCameraMode vars

	ViewOffset={(
		OffsetHigh=(X=-175,Y=0,Z=12),
		OffsetLow=(X=-175,Y=0,Z=12),
		OffsetMid=(X=-175,Y=0,Z=12),
		)}

	BlendTime=0.15


	TargetRelativeCameraOriginOffset=(X=100,Y=0,Z=0)
	WorstLocOffset=(X=-8,Y=10,Z=90)

	StrafeLeftAdjustment=(X=0,Y=-60,Z=0)
	StrafeRightAdjustment=(X=0,Y=60,Z=0)
	StrafeOffsetScalingThreshold=400

	InterpLocSpeed=12.f
}

