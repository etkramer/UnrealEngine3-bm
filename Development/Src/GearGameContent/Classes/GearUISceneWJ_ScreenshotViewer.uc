/**
 * Simple scene for viewing screenshots fullscreen.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_ScreenshotViewer extends GearUISceneWJ_PopupBase;

var	transient	UILabel	lblScore;
var	transient	UIImage imgScreenshot;

/* == UnrealScript == */

function string GetFriendlyGameType(EGearMPTypes GameTypeId, string GameTypeString)
{
	local GearGameInfoSummary GameType;
	GameType = class'GearUIDataStore_GameResource'.static.GetGameTypeProvider( GameTypeId );
	if ( GameType != None )
	{
		return GameType.GameName;
	}
	return GameTypeString;
}

/**
 * Sets up the screenshot for viewing.
 *
 * @param	ScreenshotImage		the screenshot texture to display
 * @param	ScreenshotMetadata	info about the screenshot, gametype, mapname, etc.
 */
function SetScreenshotInfo( Texture2DDynamic ScreenshotImage, out const ScreenshotInfo ScreenshotMetadata )
{
	local bool bIsWidescreen;
	local float SSAspectRatio;
	local UIImage OtherImage;

	SSAspectRatio = float(ScreenshotMetadata.Width) / float(ScreenshotMetadata.Height);
	bIsWidescreen = Abs(SSAspectRatio - ASPECTRATIO_Widescreen) < 0.0001;
	if ( bIsWidescreen )
	{
		imgScreenshot = UIImage(FindChild('imgScreenshotHD',true));
		OtherImage = UIImage(FindChild('imgScreenshotSD',true));
	}
	else
	{
		imgScreenshot = UIImage(FindChild('imgScreenshotSD',true));
		OtherImage = UIImage(FindChild('imgScreenshotHD',true));
	}

	lblScore.SetDataStoreBinding(
		GetFriendlyGameType(EGearMPTypes(ScreenshotMetadata.GameTypeId), ScreenshotMetadata.GameTypeFriendly) @ GameToMapConjunction @ GetMapNameString(ScreenshotMetadata.MapName)
		$ "<Strings:GearGame.GearUISceneWJ_ScreenshotViewer.Score>" @ ScreenshotMetadata.Rating
	);

	OtherImage.SetVisibility(false);
	imgScreenshot.SetVisibility(true);
	imgScreenshot.SetValue(ScreenshotImage);
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	Super.InitializeWidgetReferences();

	btnbarMain = UICalloutButtonPanel(FindChild('btnbarCommon',true));
	lblScore = UILabel(FindChild('lblScore',true));
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	Super.SetupCallbacks();
	btnbarMain.SetButtonCallback('CloseWindow', CalloutButtonClicked);
}

DefaultProperties
{

}
