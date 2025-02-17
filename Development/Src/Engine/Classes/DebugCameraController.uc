// PCF Begin (Debug Camera)
//-----------------------------------------------------------
// Debug Camera Controller
//
// To turn it on, please press Alt+C or both (left and right) analogs on xbox pad
// After turning:
//   WASD  | Left Analog - moving
//   Mouse | Right Analog - rotating
//   Shift | XBOX_KeyB - move faster
//   Q/E   | LT/RT - move Up/Down
//   Enter | XBOX_KeyA - to call "FreezeRendering" console command
//   Alt+C | LeftThumbstick - to toggle debug camera
//
// * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//-----------------------------------------------------------
class DebugCameraController extends PlayerController;

var PlayerController        OryginalControllerRef;
var Player                  OryginalPlayer;
var bool                    bIsFrozenRendering;
var	DrawFrustumComponent	DrawFrustum;

simulated event PostBeginPlay()
{
    super.PostBeginPlay();

    // if hud is existing, delete it and create new hud for debug camera
	if ( myHUD != None )
		myHUD.Destroy();
	myHUD = Spawn( class'DebugCameraHUD', self);
}

 /*
 *  Function called on activation debug camera controller
 */
 function OnActivate( PlayerController PC )
{
    if(DrawFrustum==None) {
        DrawFrustum = new(PC.PlayerCamera) class'DrawFrustumComponent';
    }
    DrawFrustum.SetHidden( false );
    PC.SetHidden(false);
    PC.PlayerCamera.SetHidden(false);

    DrawFrustum.FrustumAngle = PC.PlayerCamera.CameraCache.POV.FOV;
    DrawFrustum.SetAbsolute(true, true, false);
    DrawFrustum.SetTranslation(PC.PlayerCamera.CameraCache.POV.Location);
    DrawFrustum.SetRotation(PC.PlayerCamera.CameraCache.POV.Rotation);

    PC.PlayerCamera.AttachComponent(DrawFrustum);
    ConsoleCommand("show camfrustums"); //called to render camera frustums from oryginal player camera
}

 /*
 *  Function called on deactivation debug camera controller
 */
function OnDeactivate( PlayerController PC )
{
    DrawFrustum.SetHidden( true );
    ConsoleCommand("show camfrustums");
    PC.PlayerCamera.DetachComponent(DrawFrustum);
    PC.SetHidden(true);
    PC.PlayerCamera.SetHidden(true);
}

//function called from key bindings command to save information about
// turrning on/off FreezeRendering command.
exec function SetFreezeRendering()
{
     ConsoleCommand("FreezeRendering");
     bIsFrozenRendering = !bIsFrozenRendering;
}

//function called from key bindings command
exec function MoreSpeed()
{
    bRun = 2;
}

//function called from key bindings command
exec function NormalSpeed()
{
    bRun = 0;
}

/*
 * Switch from debug camera controller to local player controller
 */
function DisableDebugCamera()
{
    if( OryginalControllerRef != none )
    {
        // restore FreezeRendering command state before quite
        if( bIsFrozenRendering==true ) {
            ConsoleCommand("FreezeRendering");
            bIsFrozenRendering = false;
        }
        if( OryginalPlayer != none )
        {
            OnDeactivate( OryginalControllerRef );
            OryginalPlayer.SwitchController( OryginalControllerRef );
            OryginalControllerRef = none;
        }
    }
}

defaultproperties
{
 	InputClass=class'Engine.DebugCameraInput'
    OryginalControllerRef=None
    OryginalPlayer=None
    bIsFrozenRendering=false

    DrawFrustum=none
	bHidden=FALSE
	bHiddenEd=FALSE

}
// PCF End
