/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTGameViewportClient extends GameViewportClient
	native
	config(Game);

var localized string LevelActionMessages[6];

/** This is the remap name for UTFrontEnd so we can display a more friendly name **/
var localized string UTFrontEndString;

/** This is the remap name for UTM-MissionSelection so we can display a more friendly name **/
var localized string UTMMissionSelectionString;

/** Font used to display map name on loading screen */
var font LoadingScreenMapNameFont;

/** Font used to display game type name on loading screen */
var font LoadingScreenGameTypeNameFont;

/** Font used to display map hint message on loading screen */
var font LoadingScreenHintMessageFont;


cpptext
{
	virtual void SetDropDetail(FLOAT DeltaSeconds);
}

event PostRender(Canvas Canvas)
{
	local int i;
	local ETransitionType OldTransitionType;
	local AudioDevice AD;

	OldTransitionType = Outer.TransitionType;
	if (Outer.TransitionType == TT_None)
	{
		for (i = 0; i < Outer.GamePlayers.length; i++)
		{
			if (Outer.GamePlayers[i].Actor != None)
			{
				// count as loading if still using temp locally spawned PC on client while waiting for connection
				if (Outer.GamePlayers[i].Actor.WorldInfo.NetMode == NM_Client && Outer.GamePlayers[i].Actor.Role == ROLE_Authority)
				{
					Outer.TransitionType = TT_Loading;
					break;
				}
			}
		}

		AD = class'Engine'.static.GetAudioDevice();
		if (AD != None)
		{
			if (Outer.TransitionType != TT_None)
			{
				AD.TransientMasterVolume = 0.0;
			}
			else if (AD.TransientMasterVolume == 0.0)
			{
				AD.TransientMasterVolume = 1.0;
			}
		}
	}

	Super.PostRender(Canvas);

	Outer.TransitionType = OldTransitionType;
}


/**
 * Locates a random localized hint message string for the specified two game types.  Usually the first game type
 * should always be "UTDeathmatch", since you always want those strings included regardless of game type
 *
 * @param GameType1Name Name of the first game type we're interested in
 * @param GameType2Name Name of the second game type we're interested in
 *
 * @return Returns random hint string for the specified game types
 */
native final function string LoadRandomLocalizedHintMessage( string GameType1Name, string GameType2Name );


function DrawTransition(Canvas Canvas)
{
	local int Pos;
	local string MapName, Desc;
	local string ParseStr;
	local class<UTGame> GameClass;
	local string HintMessage;
	local bool bAllowHints;
	local string GameClassName;

	// if we are doing a loading transition, set up the text overlays forthe loading movie
	if (Outer.TransitionType == TT_Loading)
	{
		bAllowHints = true;

		// we want to show the name of the map except for a number of maps were we want to remap their name
		if( "UTFrontEnd" == Outer.TransitionDescription )
		{
			MapName = UTFrontEndString; //"Main Menu"

			// Don't bother displaying hints while transitioning to the main menu (since it should load pretty quickly!)
			bAllowHints = false;
		}
		else if( "UTM-MissionSelection" == Outer.TransitionDescription )
		{
			MapName = UTMMissionSelectionString; //"Mission Selection"

			// No hints while loading mission selection (since it should load fast!)
			bAllowHints = false;
		}
		else
		{
			MapName = Outer.TransitionDescription;
		}

		class'Engine'.static.RemoveAllOverlays();

		// pull the map prefix off the name
		Pos = InStr(MapName,"-");
		if (Pos != -1)
		{
			MapName = right(MapName, (Len(MapName) - Pos) - 1);
		}

		// pull off anything after | (gametype)
		Pos = InStr(MapName,"|");
		if (Pos != -1)
		{
			MapName = left(MapName, Pos);
		}

		// get the class represented by the GameType string
		GameClass = class<UTGame>(FindObject(Outer.TransitionGameType, class'Class'));
		Desc = "";

		if (GameClass == none)
		{
			// Some of the game types are in UTGameContent instead of UTGame. Unfortunately UTGameContent has not been loaded yet so we have to get its base class in UTGame
			// to get the proper description string.
			Pos = InStr(Outer.TransitionGameType, ".");

			if(Pos != -1)
			{
				ParseStr = Right(Outer.TransitionGameType, Len(Outer.TransitionGameType) - Pos - 1);

				Pos = InStr(ParseStr, "_Content");

				if(Pos != -1)
				{
					ParseStr = Left(ParseStr, Pos);

					ParseStr = "UTGame." $ ParseStr;

					GameClass = class<UTGame>(FindObject(ParseStr, class'Class'));

					if(GameClass != none)
					{
						Desc = GameClass.default.GameName;
					}
				}
			}
		}
		else
		{
			Desc = GameClass.default.GameName;
		}

		`log("Desc:" @ Desc);


		// NOTE: The position and scale values are in resolution-independent coordinates (between 0 and 1).

		// NOTE: The position and scale values will be automatically corrected for aspect ratio (to match the movie image)

		// Game type name
		class'Engine'.static.AddOverlay(LoadingScreenGameTypeNameFont, Desc, 0.1822, 0.435, 1.0, 1.0, false);

		// Map name
		class'Engine'.static.AddOverlay(LoadingScreenMapNameFont, MapName, 0.1822, 0.46, 2.0, 2.0, false);

		// We don't want to draw hints for the Main Menu or FrontEnd maps, so we'll make sure we have a valid game class
		if( bAllowHints )
		{
			// Grab game class name if we have one
			GameClassName = "";
			if( GameClass != none )
			{
				GameClassName = string( GameClass.Name );
			}

			// Draw a random hint!
			// NOTE: We always include deathmatch hints, since they're generally appropriate for all game types
			HintMessage = LoadRandomLocalizedHintMessage( string( class'UTDeathmatch'.Name ), GameClassName);
			if( Len( HintMessage ) > 0 )
			{
				class'Engine'.static.AddOverlayWrapped( LoadingScreenHintMessageFont, HintMessage, 0.1822, 0.585, 1.0, 1.0, 0.7 );
			}
		}
	}
	else if (Outer.TransitionType == TT_Precaching)
	{
		Canvas.Font = class'UTHUD'.static.GetFontSizeIndex(3);
		Canvas.SetPos(0, 0);
		Canvas.SetDrawColor(0, 0, 0, 255);
		Canvas.DrawRect(Canvas.SizeX, Canvas.SizeY);
		Canvas.SetDrawColor(255, 0, 0, 255);
		Canvas.SetPos(100,200);
		Canvas.DrawText("Precaching...");
	}
}

function RenderHeader(Canvas Canvas)
{
	Canvas.Font = class'UTHUD'.static.GetFontSizeIndex(3);
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.SetPos(100,100);
	Canvas.DrawText("Tell Josh Adams if you see this");
}

/**
 * Sets the value of ActiveSplitscreenConfiguration based on the desired split-screen layout type, current number of players, and any other
 * factors that might affect the way the screen should be layed out.
 */
function UpdateActiveSplitscreenType()
{
	if ( GamePlayers.Length == 0 || (GamePlayers[0].Actor != None && GamePlayers[0].Actor.IsA('UTEntryPlayerController')) )
	{
		ActiveSplitscreenType = eSST_NONE;
	}
	else
	{
		Super.UpdateActiveSplitscreenType();
	}
}

defaultproperties
{
	UIControllerClass=class'UTGame.UTGameInteraction'
	LoadingScreenMapNameFont=MultiFont'UI_Fonts_Final.Menus.Fonts_AmbexHeavyOblique'
	LoadingScreenGameTypeNameFont=MultiFont'UI_Fonts_Final.Menus.Fonts_AmbexHeavyOblique'
	LoadingScreenHintMessageFont=MultiFont'UI_Fonts_Final.HUD.MF_Medium'
}
