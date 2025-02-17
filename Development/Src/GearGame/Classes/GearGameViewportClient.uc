/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameViewportClient extends GameViewportClient
	native;

cpptext
{
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);

	/**
	* Callback to allow game viewport to override the splitscreen settings
	* @param NewSettings - settings to modify
	* @param SplitScreenType - current splitscreen type being used
	*/
	virtual void OverrideSplitscreenSettings(FSystemSettingsData& SplitscreenSettings,ESplitScreenType SplitScreenType) const;
}

/** The previous transition */
var ETransitionType PrevTransitionType;
/** Flag to tell next load transition to make the screen black */
var bool bMakeNextLoadTransitionBlack;

event bool Init(out string OutError)
{
	if(!Super.Init(OutError))
	{
		return false;
	}


	return true;
}

event PostRender(Canvas Canvas)
{
	local GearPC PC;
	local DebugCameraController DCC;
	Super.PostRender(Canvas);

	PC = GearPC(GamePlayers[0].Actor);
	if (PC == None)
	{
		DCC = DebugCameraController(GamePlayers[0].Actor);
		if ( DCC == None )
		{
			Canvas.SetOrigin(0,0);
			Canvas.SetPos(0,0);
			Canvas.SetDrawColor(0,0,0,255);
			Canvas.DrawTile(Canvas.DefaultTexture,Canvas.ClipX,Canvas.ClipY,0,0,2,2);
		}
	}
}

function DrawTransition(Canvas Canvas)
{
	// if the current transition is loading see if we should make the screen black
	if( Outer.TransitionType == TT_Loading )
	{
		if ( bMakeNextLoadTransitionBlack )
		{
			Canvas.SetOrigin(0,0);
			Canvas.SetPos(0,0);
			Canvas.DrawColor = MakeColor(0,0,0,255);
			Canvas.DrawTile(Texture2D'WhiteSquareTexture',Canvas.ClipX,Canvas.ClipY,0,0,2,2);
		}
	}
	//  if the current tranisition is NOT loading but it was last frame, reset the flag for
	//  making the screen black
	else if ( PrevTransitionType == TT_Loading )
	{
		bMakeNextLoadTransitionBlack = false;
	}

	// keep track of the previous transition
	PrevTransitionType = Outer.TransitionType;
}


/**
 * Sets the value of ActiveSplitscreenConfiguration based on the desired split-screen layout type, current number of players, and any other
 * factors that might affect the way the screen should be layed out.
 */
function UpdateActiveSplitscreenType()
{
	local int Idx;
	local LocalPlayer LP;
	local GearPC WPC;
	local GearPlayerCamera WCam;
	local ESplitscreenType PreviousSplitType;

	PreviousSplitType = GetSplitscreenConfiguration();

	if ( ShouldForceFullscreenViewport() )
	{
		ActiveSplitscreenType = eSST_NONE;
	}
	else
	{
		Super.UpdateActiveSplitscreenType();
	}

	if ( PreviousSplitType != GetSplitscreenConfiguration() )
	{
		// don't let camera smoothly interp to new configuration, just go there.
		for (Idx=0; Idx<GamePlayers.Length; ++Idx)
		{
			LP = GamePlayers[Idx];
			WPC = GearPC(LP.Actor);
			if (WPC != None)
			{
				WCam = GearPlayerCamera(WPC.PlayerCamera);
				if (WCam != None)
				{
					WCam.ResetInterpolation();
				}
			}
		}
	}
}

/** called before rending subtitles to allow the game viewport to determine the size of the subtitle area
 * @param Min top left bounds of subtitle region (0 to 1)
 * @param Max bottom right bounds of subtitle region (0 to 1)
 */
event GetSubtitleRegion(out vector2D MinPos, out vector2D MaxPos)
{
	if ( ShouldForceFullscreenViewport() )
	{
		// since in cinematic mode we're going to resize the first viewport to cover everything
		// use the single viewport subtitle region
		MaxPos.X = 1.0f;
		MaxPos.Y = 0.9f;
	}
	else
	{
		Super.GetSubtitleRegion(MinPos, MaxPos);
	}
}

defaultproperties
{
	UIControllerClass=class'GearGame.GearUIInteraction'
}
