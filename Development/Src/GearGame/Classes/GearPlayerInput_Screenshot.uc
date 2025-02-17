/**
 * 
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPlayerInput_Screenshot extends GearPlayerInput
	config(Input);

var bool bDetachedFromPlayer;

var PointLightComponent LightComp;
var bool bLightCompAttached;
var Actor LightAttachee;

function UpdateJoystickInput(float DeltaTime)
{
	local float NewRadius, NewBrightness;
	if (bLightCompAttached && IsButtonActive(GB_LeftTrigger))
	{
		// radius controlled by left stick left/right
		NewRadius = aStrafe * DeltaTime * 256.f;
		LightComp.Radius += NewRadius;
		// brightness by left stick up/down
		NewBrightness = LightComp.Brightness + (aBaseY * DeltaTime * 256.f);
		LightComp.SetLightProperties(NewBrightness);
	}
}

function HandleButtonInput_A(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		ConsoleCommand("shot");
	}
}

function HandleButtonInput_X(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		// reattach the camera
		if (bDetachedFromPlayer)
		{
			`log("Re-attaching camera to player");
			bDetachedFromPlayer = FALSE;
			Outer.GotoState('PlayerWalking');
			SetCameraMode('default');
		}
		// separate the camera
		else
		{
			`log("Detaching camera from player");
			bDetachedFromPlayer = TRUE;
			if (GearDemoRecSpectator(Outer) != None)
			{
				GearDemoRecSpectator(Outer).bLockRotationToViewTarget = FALSE;
			}
			Outer.GotoState('ScreenshotMode');
			SetCameraMode('screenshot');
		}
	}
}

function HandleButtonInput_Y(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if (!WorldInfo.IsPlayingDemo())
		{
			// single-step pause
			if (IsPaused())
			{
				WorldInfo.Pauser = None;
				SetTimer( 0.1f,FALSE,nameof(self.PauseGame), self );
			}
			else
			{
				// toggle the HUD
				myHUD.bShowHUD = !myHUD.bShowHUD;
			}
		}
		else
		{
			if (WorldInfo.bPlayersOnly)
			{
				WorldInfo.bPlayersOnly = FALSE;
				SetTimer( 0.1f,FALSE,nameof(self.PauseGame), self );
			}
			else
			{
				// toggle the HUD
				myHUD.bShowHUD = !myHUD.bShowHUD;
			}
		}
	}
}

function HandleButtonInput_B(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if (bLightCompAttached)
		{
			if (IsButtonActive(GB_LeftTrigger))
			{
				LightComp.SetTranslation(LightAttachee.Location - Location);
			}
			else
			{
				LightAttachee.DetachComponent(LightComp);
				bLightCompAttached = FALSE;
			}
		}
		else
		{
			LightAttachee = MyGearPawn;
			if (LightAttachee == None)
			{
				LightAttachee = LastPawn;
			}
			if (LightAttachee == None)
			{
				LightAttachee = ViewTarget;
			}
			LightAttachee.AttachComponent(LightComp);
			bLightCompAttached = TRUE;
			LightComp.SetTranslation(LightAttachee.Location - Location);
		}
	}
}

function HandleButtonInput_Start(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		WorldInfo.bPlayersOnly = !WorldInfo.bPlayersOnly;
	}
}

function HandleButtonInput_LeftBumper(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if (!WorldInfo.IsPlayingDemo())
		{
			if (!bRecording)
			{
				ConsoleCommand("DEMOREC"@GetURLMap()$"_"$DemoCount);
				bRecording = TRUE;
				DemoCount++;
				ClientMessage("Started recording demo");
			}
			else
			{
				ConsoleCommand("DEMOSTOP");
				bRecording = FALSE;
				ClientMessage("Stopped recording demo");
			}
		}
		else
		{
			WorldInfo.TimeDilation = 0.5f;
		}
	}
	else
	{
		if (WorldInfo.IsPlayingDemo())
		{
			WorldInfo.TimeDilation = 1.f;
		}
	}
}

function HandleButtonInput_RightBumper(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if (!bRecording)
		{
			ConsoleCommand("DEMOPLAY");
		}
	}
}

function PauseGame()
{
	if (!WorldInfo.IsPlayingDemo())
	{
		WorldInfo.Pauser = PlayerReplicationInfo;
	}
	else
	{
		WorldInfo.bPlayersOnly = TRUE;
	}
}

defaultproperties
{
	Begin Object Class=PointLightComponent Name=LightComponent0
		bEnabled=TRUE
		LightColor=(R=245,G=174,B=122,A=255)
		Brightness=10
		Radius=256
	End Object
	LightComp=LightComponent0
}
