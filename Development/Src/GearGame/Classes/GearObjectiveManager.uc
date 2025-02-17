/*****************************************************************************
 * GearObjectiveManager
 * Manages the objective system for the GearPC
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *****************************************************************************/
class GearObjectiveManager extends Object within GearPC
	native;

/*****************************************************************************
	Structs, enums, and variables
*****************************************************************************/

/** List of current objectives */
var array<ObjectiveInfo> Objectives;

/** Amount of time an objective will draw on the screen since it was last updated. */
var const float TotalDisplayTime;
/** The maximum number of objectives to show at one time. */
var const int TotalObjectivesToDraw;

/** Displays when there is no objective */
var localized String ObjectivesCompleteText;

/*****************************************************************************
	Native functions
*****************************************************************************/

/**
* The text in the objective structure is actually a tagname to the localized string,
* so we have to go retrieve it from the localization system.
*/
native simulated function String RetrieveObjectiveString( String TagName );

/*****************************************************************************
	Script functions
 *****************************************************************************/

/** Handles the update from kismet to the objective system. */
final function OnManageObjectives( SeqAct_ManageObjectives Action )
{
	// Add or Update an objective
	if ( Action.InputLinks[0].bHasImpulse )
	{
		AddObjective( Action.ObjectiveName, Action.ObjectiveDesc, Action.bNotifyPlayer );
	}
	// Completed an objective
	else if ( Action.InputLinks[1].bHasImpulse )
	{
		CompleteObjective( Action.ObjectiveName, Action.bNotifyPlayer );
	}
	// Failed an objective
	else if ( Action.InputLinks[2].bHasImpulse )
	{
		FailObjective( Action.ObjectiveName, Action.bNotifyPlayer );
	}
}

/** Clears the objective list */
final public function ClearObjectives()
{
	Objectives.length = 0;
}

/** Updates the objective list. */
final protected function UpdateObjective( Name ObjectiveName, string ObjectiveDesc, bool bUpdated, bool bCompleted, bool bFailed, bool bNotifyPlayer )
{
	local int Idx;

	// See if the objective is in the list
	Idx = Objectives.Find( 'ObjectiveName',ObjectiveName );

	// If it's not in the list add it.
	if (Idx == -1)
	{
		Idx = Objectives.length;
		Objectives.length = Idx + 1;
		Objectives[Idx].ObjectiveName = ObjectiveName;
	}

	// Update the description, time, and flags
	Objectives[Idx].bUpdated = bUpdated;
	Objectives[Idx].bCompleted = bCompleted;
	Objectives[Idx].bFailed = bFailed;
	Objectives[Idx].ObjectiveDesc = ObjectiveDesc;
	Objectives[Idx].UpdatedTime = WorldInfo.TimeSeconds;
	Objectives[Idx].bNotifyPlayer = bNotifyPlayer;

	// Replicate to the client if not a local controller
	if ( !IsLocalPlayerController() )
	{
		ClientReplicateObjective( Objectives[Idx], Idx );
	}
}

/** Adds or updates an objective */
final function AddObjective( Name ObjName, string ObjDesc, bool bNotifyPlayer )
{
	local int Idx;
	local bool bUpdate;

	if ( ObjName != '' )
	{
		// See if the objective is in the list
		Idx = Objectives.Find( 'ObjectiveName', ObjName );

		// Set the updated flag based on whether the objective already exists or not.
		bUpdate = (Idx == -1) ? FALSE : TRUE;

		// Update the objective
		UpdateObjective( ObjName, ObjDesc, bUpdate, FALSE, FALSE, bNotifyPlayer );

		// Play sound if needed
		if ( bNotifyPlayer )
		{
			if ( bUpdate )
			{
				ClientPlaySound( SoundCue'Interface_Audio.Objectives.UpdateObjective' );
			}
			else
			{
				ClientPlaySound( SoundCue'Interface_Audio.Objectives.AddObjective' );
			}
		}
	}
}

/** Completes an objective from the list and optionally plays a sound. */
final function CompleteObjective( Name ObjName, bool bNotifyPlayer )
{
	local int Idx;

	if ( ObjName != '' )
	{
		// Make sure the objective already exists
		Idx = Objectives.Find('ObjectiveName',ObjName);
		if (Idx != -1 )
		{
			// Update the objective
			UpdateObjective( ObjName, Objectives[Idx].ObjectiveDesc, FALSE, TRUE, FALSE, bNotifyPlayer );

			// Play sound if needed
			if ( bNotifyPlayer )
			{
				ClientPlaySound( SoundCue'Interface_Audio.Objectives.CompleteObjective' );
			}
		}
	}
}

/** Fails an objective from the list and optionally plays a sound. */
final function FailObjective( Name ObjName, bool bNotifyPlayer )
{
	local int Idx;

	if ( ObjName != '' )
	{
		// Make sure the objective already exists
		Idx = Objectives.Find( 'ObjectiveName', ObjName );
		if (Idx != -1 )
		{
			// Update the objective
			UpdateObjective( ObjName, Objectives[Idx].ObjectiveDesc, FALSE, FALSE, TRUE, bNotifyPlayer );

			// Play sound if needed
			if ( bNotifyPlayer )
			{
				ClientPlaySound( SoundCue'Interface_Audio.Objectives.FailObjective' );
			}
		}
	}
}

/** Loops backwards through all of the objectives and draws them to the HUD in the top left corner of the screen. */
final function DrawObjectives( GearHUD_Base GHUD, GearPC PC, optional float AlphaPercent=1.f, optional bool bUseUpdateCheck=false, optional bool bIsGameoverScreen=false )
{
	local float DrawX, DrawY, ObjectiveWidth;
	local int Idx;
	local float TextSizeX, TextSizeY;
	local byte TextAlpha, BGAlpha;
	local EGearHUDColor TextColor;
	local float BackgroundOffset, ObjTime, CalcAlpha;
	local bool bDrawStrike;
	local int StrikeCount, NumActiveObjs, NumNonActiveToDraw, NumObjsToDraw, ObjectiveCount;
	local String ObjDesc;

	// Draw nothing if there is no opacity
	if ( AlphaPercent <= 0.f )
	{
		return;
	}

	// Set the font
	GHUD.Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);

	// Initialize some variables

	BackgroundOffset = 2.f;
	if ( bIsGameoverScreen )
	{
		DrawX = 8;
		DrawY = 2;
	}
	else
	{
		DrawX = GHUD.SafeZoneLeft;
		DrawY = GHUD.SafeZoneTop + BackgroundOffset;
	}

	StrikeCount = 0;
	NumObjsToDraw = TotalObjectivesToDraw;

	// If we have objectives...
	if ( Objectives.Length > 0 )
	{
		// See how many active objectives are left
		for ( Idx = Objectives.Length-1; Idx >= 0; Idx-- )
		{
			if ( !Objectives[Idx].bCompleted && !Objectives[Idx].bFailed )
			{
				NumActiveObjs++;
			}
		}
		NumNonActiveToDraw = max( 0, NumObjsToDraw-NumActiveObjs );

		// Draw each one
		for ( Idx = 0; Idx < Objectives.Length; Idx++ )
		{
			// Don't draw more than 'TotalObjectivesToDraw' number of objectives
			if ( NumObjsToDraw <= 0 )
			{
				break;
			}

			// abort if the description is invalid
			if (Objectives[Idx].ObjectiveDesc == "")
			{
				continue;
			}

			// We can only draw 'TotalObjectivesToDraw' number of objectives so we may have to skip some completed ones to
			// get to the uncompleted ones.
			if ( Objectives[Idx].bCompleted || Objectives[Idx].bFailed )
			{
				if ( (Objectives[Idx].bCompleted && ((WorldInfo.TimeSeconds - Objectives[Idx].UpdatedTime) >= TotalDisplayTime)) || (NumNonActiveToDraw <= 0) )
				{
					continue;
				}
				else
				{
					NumNonActiveToDraw--;
				}
			}

			if ( bUseUpdateCheck && (!Objectives[Idx].bNotifyPlayer || ((WorldInfo.TimeSeconds - Objectives[Idx].UpdatedTime) >= TotalDisplayTime)) )
			{
				continue;
			}

			// Figure out the alpha based on updated time
			ObjTime = WorldInfo.TimeSeconds - Objectives[Idx].UpdatedTime;
			if ( bUseUpdateCheck )
			{
				CalcAlpha = (ObjTime > TotalDisplayTime-2.0f) ? 1.f - ((ObjTime - (TotalDisplayTime-2.0f))/2.f) : 1.f;
				if ( CalcAlpha <= 0.f )
				{
					CalcAlpha = 0.f;
				}
				AlphaPercent *= CalcAlpha;
			}

			// No opacity so continue
			if ( AlphaPercent <= 0.f )
				continue;

			// Determine the alphas, colors, and whether to strike through or not
			if ( Objectives[Idx].bCompleted )
			{
				TextAlpha = 120;
				BGAlpha = 100;
				TextColor = eWARHUDCOLOR_WHITE;
				bDrawStrike = true;
			}
			else if ( Objectives[Idx].bFailed )
			{
				TextAlpha = 255;
				BGAlpha = 200;
				TextColor = eWARHUDCOLOR_RED;
				bDrawStrike = true;
			}
			else
			{
				TextAlpha = 255;
				BGAlpha = 200;
				TextColor = eWARHUDCOLOR_WHITE;
			}

			// Get the objective text
			ObjDesc = RetrieveObjectiveString( Objectives[Idx].ObjectiveDesc );

			// Background
			GHUD.Canvas.TextSize( ObjDesc, TextSizeX, TextSizeY );
			ObjectiveWidth = bIsGameoverScreen ? GHUD.Canvas.ClipX : fmax(TextSizeX+16, (GHUD.Canvas.ClipX/3));

			// Move the objectives down if the countdown timer is showing and will cause overlap
			if ( !bIsGameoverScreen && (ObjectiveCount == 0) && GHUD.CountdownTimerIsShowing() && ((DrawX - 8.0f + ObjectiveWidth + 5.0f) > GHUD.GetCountdownXPosition()) )
			{
				DrawY += TextSizeY + 6.f;
			}

			GHUD.SetHUDDrawColor( eWARHUDCOLOR_BLACK, BGAlpha*AlphaPercent );
			GHUD.DrawObjectiveBack( Texture2D'Warfare_HUD.WarfareHUD_main', 156, 29, 21, 19, DrawX-8, DrawY-BackgroundOffset, ObjectiveWidth, TextSizeY+2*BackgroundOffset );

			// Label
			GHUD.Canvas.SetPos(DrawX,DrawY);
			GHUD.SetHUDDrawColor( TextColor, TextAlpha*AlphaPercent );
			GHUD.Canvas.DrawText( ObjDesc );

			// Strike through
			if ( bDrawStrike )
			{
				DrawStrikeThru( GHUD, StrikeCount, DrawX-8, DrawY-BackgroundOffset, bIsGameoverScreen ? GHUD.Canvas.ClipX : fmax(TextSizeX+16, (GHUD.Canvas.ClipX/3)), TextSizeY, TextColor, 255*AlphaPercent );
				StrikeCount++;
			}

			DrawY += TextSizeY + 6.f;
			bDrawStrike = false;
			NumObjsToDraw--;
			ObjectiveCount++;
		}
	}
	// Otherwise pretend all objectives are completed
	else if ( !bUseUpdateCheck )
	{
		// Background
		GHUD.Canvas.TextSize( ObjectivesCompleteText, TextSizeX, TextSizeY );
		ObjectiveWidth = bIsGameoverScreen ? GHUD.Canvas.ClipX : fmax(TextSizeX+16, (GHUD.Canvas.ClipX/3));

		// Move the objectives down if the countdown timer is showing and will cause overlap
		if ( !bIsGameoverScreen && GHUD.CountdownTimerIsShowing() && ((DrawX - 8.0f + ObjectiveWidth + 5.0f) > GHUD.GetCountdownXPosition()) )
		{
			DrawY += TextSizeY + 6.f;
		}

		GHUD.SetHUDDrawColor( eWARHUDCOLOR_BLACK, 130*AlphaPercent );
		GHUD.DrawObjectiveBack( Texture2D'Warfare_HUD.WarfareHUD_main', 156, 29, 21, 19, DrawX-8, DrawY-BackgroundOffset, bIsGameoverScreen ? GHUD.Canvas.ClipX : fmax(TextSizeX+16, (GHUD.Canvas.ClipX/3)), TextSizeY+2*BackgroundOffset );

		// Label
		GHUD.SetHUDDrawColor( eWARHUDCOLOR_WHITE, 255*AlphaPercent );
		GHUD.Canvas.SetPos(DrawX,DrawY);
		GHUD.Canvas.DrawText( ObjectivesCompleteText );
	}
}

/** Draws a strike through */
final protected function DrawStrikeThru( GearHUD_Base GHUD, int StrikeCount, float DrawX, float DrawY, float Width, float Height, EGearHUDColor eColor, byte Alpha )
{
	GHUD.Canvas.SetPos( DrawX, DrawY );
	GHUD.SetHUDDrawColor( eColor, Alpha );

	switch ( StrikeCount % 4 )
	{
	case 0:
		GHUD.Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 0, 0, 512, 32 );
		break;
	case 1:
		GHUD.Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 512, 0, -512, 32 );
		break;
	case 2:
		GHUD.Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 0, 32, 512, -32 );
		break;
	case 3:
		GHUD.Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 512, 32, -512, -32 );
		break;
	}
}

defaultproperties
{
	TotalDisplayTime=5.0f
	TotalObjectivesToDraw=3
}
