/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTScoreInfoPanel extends UTDrawPanel;

var() linearColor BarColor;


var() texture2D PanelTex;
var() TextureCoordinates PanelCoords;
var() float PanelPct;

var() texture2D BarTex;
var() TextureCoordinates BarCoords;
var() float BarPct;

var() texture2D LogoTex;
var() TextureCoordinates LogoCoords;
var() vector2D LogoPos;
var() vector2D LogoSize;

var() font GameTypeFont;
var() font MapNameFont;
var() font RulesFont;

var() float MapNameYPad;

var() Color GameTypeColor;
var() Color MapNameColor;
var() Color RulesColor;

/** Cached reference to the HUDSceneOwner */
var UTUIScene_Hud UTHudSceneOwner;
var transient string LastMapPlayed;


event PostInitialize()
{
	Super.PostInitialize();
	UTHudSceneOwner = UTUIScene_Hud( GetScene() );
}

event DrawPanel()
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;
	local float X,Y, XL,YL, PanelSize, RulesY, Scaler;
	local string Work;
	local class<UTGame> GIC;
	local float TempClipX;

	if ( MapNameFont == none )
	{
		return;
	}

	WI = UTHudSceneOwner.GetWorldInfo();
	GRI = UTGameReplicationInfo(WI.GRI);

	Canvas.Font = MapNameFont;
	Canvas.StrLen("Q",XL,YL);

	PanelSize = Canvas.ClipY * PanelPct;

	// Draw the Panel and the Icon

	Canvas.SetPos(0,0);
	Canvas.DrawColorizedTile(PanelTex, PanelSize, PanelSize, PanelCoords.U, PanelCoords.V, PanelCoords.UL, PanelCoords.VL, BarColor);

	// Draw the Icon

	X = PanelSize * 0.5 - (LogoSize.X * PanelSize * 0.5);
	Y = PanelSize * 0.5 - (LogoSize.Y * PanelSize * 0.5);

	Canvas.SetPos(X,Y);
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.DrawTile(LogoTex, LogoSize.X * PanelSize, LogoSize.Y * PanelSize, LogoCoords.U, LogoCoords.V, LogoCoords.UL, LogoCoords.VL);

	// Draw the bar.

	Y = PanelSize - (YL * BarPct);
	Canvas.SetPos(PanelSize + 2, Y);
	Canvas.DrawColorizedTile(BarTex, Canvas.ClipX - PanelSize - 2, YL * BarPct, BarCoords.U,BarCoords.V,BarCoords.UL,BarCoords.VL, BarColor);

	// Draw the Map Name

	Canvas.DrawColor = MapNameColor;
	Canvas.SetPos(PanelSize + 6, Y + (YL * BarPct * 0.5) - (YL * 0.5) );
	Work = WI.GetMapName();
	if (Work~="EnvyEntry")
	{
		Work = LastMapPlayed;
	}
	else
	{
		LastMapPlayed = Work;
	}

	Canvas.DrawText( Work );

	// Cache the position of the Captures Message here

	RulesY = Y + (YL * BarPct);

	// Draw the Game Type

	if ( WI != none )
	{
		GIC = class<UTGame>(WI.GetGameClass());
	}

	if ( GameTypeFont != none )
	{
		if ( GIC != none )
		{
			Work = GIC.default.GameName;
		}
		else
		{
			Work = "GAMETYPE";
		}

		Canvas.Font  = GameTypeFont;
		Canvas.StrLen(Work,XL,YL);

		if (XL > Canvas.CLipX - PanelSize - 2)
		{
			Scaler = (Canvas.CLipX - (PanelSize * 1.2) - 2) / XL;
		}
		else
		{
			Scaler = 1.0;
		}

		Canvas.SetPos(PanelSize + 6, Y-YL*Scaler); //YL * 0.5 - (YL * 0.5) );
		Canvas.DrawColor = GameTypeColor;
		Canvas.DrawText(Work,,Scaler,Scaler);
	}


	if ( RulesFont != none )
	{
		if ( GIC != none )
		{
			Work = GIC.static.GetEndOfMatchRules(GRI.GoalScore, GRI.TimeLimit);
		}
		else
		{
			Work = "Testing";
		}

		Canvas.Font = RulesFont;
		Canvas.StrLen(Work,XL,YL);
		Canvas.SetPos(PanelSize + 6,RulesY);
		Canvas.DrawColor = RulesColor;
		TempClipX = Canvas.ClipX;
		Canvas.ClipX = PanelSize + 6 + XL + 1;
		Canvas.DrawText(Work);
		Canvas.ClipX = TempClipX;
	}
}



defaultproperties
{
}
