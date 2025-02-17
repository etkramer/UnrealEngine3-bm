/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTScoreboardClockPanel extends UTDrawPanel;

var() Texture2D Background;
var() TextureCoordinates BackCoords;
var() LinearColor BackColor;

var() font ClockFont;
var() vector2d ClockPos;

/** Cached reference to the HUDSceneOwner */
var UTUIScene_Hud UTHudSceneOwner;

event PostInitialize()
{
	Super.PostInitialize();
	UTHudSceneOwner = UTUIScene_Hud( GetScene() );
}


event DrawPanel()
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;
	local string Clock;
	local float TextWidth, TextHeight;
	local float PadWidth, PadHeight;

	WI = UTHudSceneOwner.GetWorldInfo();
	GRI = UTGameReplicationInfo(WI.GRI);
	if (GRI != None && !GRI.bMatchISOver)
	{
		if ( ClockFont != none )
		{
			Clock = class'UTHUD'.static.FormatTime( GRI.TimeLimit != 0 ? GRI.RemainingTime : GRI.ElapsedTime );
			Canvas.Font = ClockFont;
			Canvas.StrLen(Clock, TextWidth, TextHeight);
			Canvas.StrLen("00", PadWidth, PadHeight);

			// Draw the background
			if ( Background != none )
			{
				Canvas.ClipY = TextHeight + TextHeight*0.25f;
				Canvas.SetPos( Canvas.ClipX - (TextWidth + PadWidth), 0);
				Canvas.DrawColorizedTile(Background, TextWidth + PadWidth, Canvas.ClipY, BackCoords.U,BackCoords.V,BackCoords.UL,BackCoords.VL, BackColor);
			}

			Canvas.SetDrawColor(255,255,255,255);
			Canvas.SetPos( Canvas.ClipX - (TextWidth + PadWidth*0.5f), TextHeight*0.1 );
			Canvas.DrawText(Clock);
		}
	}
}



defaultproperties
{
}
