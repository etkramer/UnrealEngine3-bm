/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTDrawPlayerListPanel extends UTDrawPanel
	native(UI);

var() Font CaptionFont;
var() Font TextFont;
var() bool bTeamMode;
var() color TeamCaptionColors[2];
var() color TeamTextColors[2];

var float FullHeight;

var localized string TeamCaptions[2];
var localized string Opponents;


cpptext
{
	virtual void Tick_Widget(FLOAT DeltaTime);
}

event DrawPanel()
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;
	local PlayerReplicationInfo PRI;
	local float TextYL, CaptionYL, XL,YL, W, AW, Scale, ResScale;

	local float Y;
	local int i,TeamIdx,TeamCnt;
	local string s;
	local Vector2D ViewportSize;
	local int Counts[2];

	WI = GetScene().GetWorldInfo();

	if ( WI == none || TextFont == none || CaptionFont == none )
	{
		return;
	}

	GRI = UTGameReplicationInfo(WI.GRI);
	if (GRI==none)
	{
		return;
	}


	for (i=0;i<GRI.PRIArray.Length;i++)
	{
		if (GRI.PRIArray[i].Team != none && GRI.PRIArray[i].Team.TeamIndex<2)
		{
			Counts[GRI.PRIArray[i].Team.TeamIndex]++;
		}
	}


	bTeamMode = GRI.GameClass.default.bTeamGame;

	Canvas.Font = TextFont;
	Canvas.Strlen("Q",XL,YL);
	TextYL = YL;

	Canvas.Font = CaptionFont;
	Canvas.Strlen("Q",XL,YL);
	CaptionYL = YL;

//	TextYL = TextFont.GetMaxCharHeight();
//	CaptionYL = CaptionFont.GetMaxCharHeight();

	W = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport);

	y = 0;
	GetViewportSize(ViewportSize);

	ResScale = ViewportSize.Y / 768;
	AW = 23 * ResScale * 1.2173;

	TeamCnt = bTeamMode ? 2 : 1;
	for (TeamIdx=0;TeamIdx<TeamCnt;TeamIdx++)
	{
		Canvas.Font = CaptionFont;
		Canvas.DrawColor = TeamCaptionColors[TeamIdx];
		Canvas.SetPos(0,Y);

        if (bTeamMode)
        {
        	s = TeamCaptions[TeamIdx] @"("$String(Counts[TeamIdx])$")";
        }
        else
        {
        	s = Opponents;
        }

		Canvas.DrawText( s );
		y += CaptionYL;

		Canvas.Font = TextFont;
		Canvas.DrawColor = TeamTextColors[TeamIdx];
		for (i=0;i<GRI.PRIArray.Length;i++)
		{
			PRI = GRI.PRIArray[i];
			if (PRI != none && !PRI.bOnlySpectator && (!bTeamMode || (PRI.Team != none && PRI.Team.TeamIndex == TeamIdx)) )
			{
				if (PRI.bReadyToPlay)
				{
					Canvas.SetPos(0,Y);
					Canvas.DrawTile(Texture2D'UI_HUD.HUD.UI_HUD_BaseC',28*ResScale*0.7,23*ResScale*0.7,821,183,28,23);
				}
				Canvas.SetPos(AW,Y);
				Canvas.TextSize(PRI.GetPlayerAlias(),XL,YL);
				if (XL > W-AW)
				{
					Scale = (W-AW) / XL;
				}
				else
				{
					Scale = 1.0;
				}

				Canvas.DrawTextClipped(PRI.GetPlayerAlias(),,Scale,Scale);
				y += (TextYL * 0.8 * Scale);

			}
		}

		SetPosition(Y, UIFACE_Bottom, EVALPOS_PixelOwner);
	}
}

event Tick_Widget(float DeltaTime)
{
	local int i,cnt;
	local bool bHasSpecs;

	local WorldInfo WI;
	local UTGameReplicationInfo GRI;
	local PlayerReplicationInfo PRI;
	local float TextYL, CaptionYL;


	WI = GetScene().GetWorldInfo();
	if ( WI == none || TextFont == none || CaptionFont == none )
	{
		return;
	}

	GRI = UTGameReplicationInfo(WI.GRI);
	if (GRI==none)
	{
		return;
	}


	for (i=0;i<GRI.PRIArray.Length;i++)
	{
		PRI = GRI.PRIArray[i];
		if ( PRI != none )
		{
			if (PRI.bOnlySpectator)
			{
				bHasSpecs = true;
			}

			Cnt++;
		}
	}

	TextYL = TextFont.GetMaxCharHeight();
	CaptionYL = CaptionFont.GetMaxCharHeight();

	FullHeight = Cnt * TextYL + (bTeamMode ? 2 * TextYL : TextYL);
	FullHeight += (bTeamMode) ? 2 * CaptionYL : CaptionYL;
	if (bHasSpecs)
	{
		FullHeight += CaptionYL + TextYL;
	}

	// TODO - Once we have sizable panels, the height here

}

defaultproperties
{
	bRequiresTick=true
}
