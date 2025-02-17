/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTCTFScoreboardPanel extends UTTDMScoreboardPanel;

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
	Canvas.StrLen(Spot, Width, Height);
	DrawString( Spot, RightColumnPosX+RightColumnWidth-Width, YPos,FontIndex,FontScale);

	return RightColumnPosX;
}

defaultproperties
{
	bDrawPlayerNum=true
	HeaderTitle_Score="Score"
}
