/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GameHUD extends HUD
	native
	abstract;


/** Total amount of time to draw the chapter title */
var float TotalTitleDrawTime;
/** Total amount of time to fade the chapter title in and out */
var float TotalTitleFadeTime;
/** The time we started drawing the chapter title */
var float TitleDrawStartTime;
/** The chapter title to draw */
var String ChapterTitleString;
/** The act title to draw */
var String ActTitleString;

enum EGameHUDColor
{
	GHD_WHITE,
	GHD_BLACK,
};


function SetHUDDrawColor( EGameHUDColor eColor, byte Alpha = 255 )
{
	switch ( eColor )
	{
	case GHD_WHITE:
		Canvas.SetDrawColor( 240, 240, 240, Alpha );
		break;

	case GHD_BLACK:
		Canvas.SetDrawColor( 0, 0, 0, Alpha );
		break;
	}
}


/** Start the process of drawing the chapter and act titles */
simulated function StartDrawingChapterTitle( String ChapterName, String ActName, float TotalDrawTime, float TotalFadeTime )
{
	TotalTitleDrawTime = TotalDrawTime;
	TotalTitleFadeTime = TotalFadeTime;
	TitleDrawStartTime = WorldInfo.TimeSeconds;
	ChapterTitleString = ChapterName;
	ActTitleString = ActName;
}

/** Stop the process of drawing the chapter and act titles */
simulated function StopDrawingChapterTitle()
{
	TotalTitleDrawTime = 0.0f;
	TitleDrawStartTime = 0.0f;
}


defaultproperties
{

}
