/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTTDMScoreboardPanel extends UTScoreboardPanel;

/** Sets the header strings using localized values */
function SetHeaderStrings()
{
	HeaderTitle_Name = Localize( "Scoreboards", "Name", "UTGameUI" );
	HeaderTitle_Score = Localize( "Scoreboards", "Score", "UTGameUI" );
}

/** Draw the panel headers. */
function DrawScoreHeader()
{
	local float xl,yl,columnWidth, numXL, numYL;

	if ( HeaderFont != none )
	{
		Canvas.SetDrawColor(255,255,255,255);

		Canvas.Font = Fonts[EFT_Large].Font;
		Canvas.StrLen("0000",numXL,numYL);

		Canvas.Font = HeaderFont;
		Canvas.SetPos(Canvas.ClipX * HeaderXPct,Canvas.ClipY * HeaderYPos);
		Canvas.DrawTextClipped(HeaderTitle_Name);

		Canvas.StrLen(HeaderTitle_Score,xl,yl);
		RightColumnWidth = xl;
		columnWidth = Max(xl+0.25f*numXL, numXL);
		RightColumnPosX = Canvas.ClipX - columnWidth;
		Canvas.SetPos(RightColumnPosX,Canvas.ClipY * HeaderYPos);
		Canvas.DrawTextClipped(HeaderTitle_Score);
	}
}

/**
* Draw the Player's Score
*/
function float DrawScore(UTPlayerReplicationInfo PRI, float YPos, int FontIndex, float FontScale)
{
	local string Spot;
	local float Width, Height;

	// Draw the player's Kills
	Spot = GetPlayerScore(PRI);
	Canvas.Font = Fonts[FontIndex].Font;
	Canvas.StrLen( Spot, Width, Height );
	DrawString( Spot, RightColumnPosX+RightColumnWidth-Width, YPos,FontIndex,FontScale);

	return RightColumnPosX;
}

function DrawTeamScore()
{
	local string ScoreStr;
	local float xl,yl, xPos;
	local WorldInfo WI;
	local int ScoreToDraw;

	Canvas.DrawColor = class'UTHUD'.default.Whitecolor;
	if ( ScoreFont != none )
	{
		WI = GetScene().GetWorldInfo();
		ScoreStr = "0";

		Canvas.Font = Font'UI_Fonts_Final.HUD.MF_Large';
		if ( WI != none && WI.GRI != none && (WI.GRI.Teams.Length > AssociatedTeamIndex) && (WI.GRI.Teams[AssociatedTeamIndex] != None) )
		{
			ScoreToDraw = Min(WI.GRI.Teams[AssociatedTeamIndex].Score, 9999);
			ScoreStr = string(ScoreToDraw);
		}
		Canvas.StrLen(ScoreStr,XL,YL);
		xPos = Canvas.ClipX * ScorePosition.X - XL * 0.5;
		Canvas.SetPos(xPos, Canvas.ClipY * ScorePosition.Y - YL * 0.5);
		Canvas.DrawText(ScoreStr);
	}
}

/** Get the header color */
function LinearColor GetHeaderColor()
{
	local LinearColor LC;
	local Color C;
	class'UTHUD'.static.GetTeamColor(AssociatedTeamIndex, LC,C);
	LC.A = 1.0f;
	return LC;
}

function string GetLeftMisc(UTPlayerReplicationInfo PRI)
{
	if ( PRI != None )
	{
		if (LeftMiscStr != "")
		{
			return UserString(LeftMiscStr, PRI);
		}
		else
		{
			if ( PRI.WorldInfo.GRI.OnSameTeam(PRI, PlayerOwner) )
			{
				return ""$PRI.GetLocationName();
			}
			else
			{
				return "";
			}
		}
	}
	return "LMisc";
}



defaultproperties
{
	bDrawPlayerNum=false
}
