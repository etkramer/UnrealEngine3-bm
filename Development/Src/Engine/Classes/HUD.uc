//=============================================================================
// HUD: Superclass of the heads-up display.
//
//Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class HUD extends Actor
	native
	config(Game)
	transient;

//=============================================================================
// Variables.

var	const	color	WhiteColor, GreenColor, RedColor;

var PlayerController 	PlayerOwner; // always the actual owner
var HUD 				HudOwner;	 // Used for sub-huds like the scoreboard

var PlayerReplicationInfo ViewedInfo;	// The PRI of the current view target

var float ProgressFadeTime;
var Color MOTDColor;

var ScoreBoard 	Scoreboard;

/** Tells whether the game was paused due to lost focus */
var transient bool LostFocusPaused;

// Visibility flags

var config	bool 	bShowHUD;				// Is the hud visible
var			bool	bShowScores;			// Is the Scoreboard visible
var			bool	bShowDebugInfo;			// if true, show properties of current ViewTarget

var()		bool	bShowBadConnectionAlert;     // Display indication of bad connection

var globalconfig bool	bMessageBeep;				// If true, any console messages will make a beep

var globalconfig float HudCanvasScale;    	// Specifies amount of screen-space to use (for TV's).

// Console Messages

struct native ConsoleMessage
{
	var string Text;
	var color TextColor;
	var float MessageLife;
	var PlayerReplicationInfo PRI;
};

var array<ConsoleMessage> ConsoleMessages;
var const Color 		ConsoleColor;

var globalconfig int 	ConsoleMessageCount;
var globalconfig int 	ConsoleFontSize;
var globalconfig int 	MessageFontOffset;

var int MaxHUDAreaMessageCount;

// Localized Messages
struct native HudLocalizedMessage
{
    // The following block of variables are set when the message is entered;
    // (Message being set indicates that a message is in the list).

	var class<LocalMessage> Message;
	var String StringMessage;
	var int Switch;
	var float EndOfLife;
	var float Lifetime;
	var float PosY;
	var Color DrawColor;
	var int FontSize;

    // The following block of variables are cached on first render;
    // (StringFont being set indicates that they've been rendered).

	var Font StringFont;
	var float DX, DY;
	var bool Drawn;
	var int Count;
	var object OptionalObject;
};
var() transient HudLocalizedMessage LocalMessages[8];

var() float ConsoleMessagePosX, ConsoleMessagePosY; // DP_LowerLeft

/**
 * Canvas to Draw HUD on.
 * NOTE: a new Canvas is given every frame, only draw on it from the HUD::PostRender() event */
var	/*const*/ Canvas	Canvas;

//
// Useful variables
//

/** Used to create DeltaTime */
var transient	float	LastHUDRenderTime;
/** Time since last render */
var	transient	float	RenderDelta;
/** Size of ViewPort in pixels */
var transient	float	SizeX, SizeY;
/** Center of Viewport */
var transient	float	CenterX, CenterY;
/** Ratio of viewport compared to native resolution 1024x768 */
var	transient	float	RatioX, RatioY;

var globalconfig array<name> DebugDisplay;		// array of strings specifying what debug info to display for viewtarget actor
									// base engine types include "AI", "physics", "weapon", "net", "camera", and "collision"

//=============================================================================
// Utils
//=============================================================================

// Draw3DLine  - draw line in world space. Should be used when engine calls RenderWorldOverlays() event.

native final function Draw3DLine(vector Start, vector End, color LineColor);
native final function Draw2DLine(int X1, int Y1, int X2, int Y2, color LineColor);

event PostBeginPlay()
{
	super.PostBeginPlay();

	PlayerOwner = PlayerController(Owner);
}

// SpawnScoreBoard - Sets the scoreboard

function SpawnScoreBoard(class<Scoreboard> ScoringType)
{
	if ( ScoringType != None )
		Scoreboard = Spawn(ScoringType, PlayerOwner);

	if (ScoreBoard!=None)
    	ScoreBoard.HudOwner = self;
}

// Clean up

event Destroyed()
{
    if( ScoreBoard != None )
    {
		ScoreBoard.Destroy();
		ScoreBoard = None;
    }
	Super.Destroyed();
}

//=============================================================================
// Execs
//=============================================================================

/* hides or shows HUD */
exec function ToggleHUD()
{
	bShowHUD = !bShowHUD;
}

exec function ShowHUD()
{
	ToggleHUD();
}

/* toggles displaying scoreboard
*/
exec function ShowScores()
{
	SetShowScores(!bShowScores);
}

/** sets bShowScores to a specific value (not toggle) */
exec function SetShowScores(bool bNewValue)
{
	bShowScores = bNewValue;
	if (ScoreBoard != None)
	{
		Scoreboard.ChangeState(bShowScores);
	}
}

/* toggles displaying properties of player's current viewtarget
 DebugType input values supported by base engine include "AI", "physics", "weapon", "net", "camera", and "collision"
*/
exec function ShowDebug(optional name DebugType)
{
	local int i;
	local bool bFound;

	if (DebugType == 'None')
	{
		bShowDebugInfo = !bShowDebugInfo;
	}
	else
	{
		bShowDebugInfo = true;

		// remove debugtype if already in array
		for (i = 0; i < DebugDisplay.Length && !bFound; i++)
		{
			if ( DebugDisplay[i] == DebugType )
			{
				DebugDisplay.Remove(i, 1);
				bFound = true;
			}
		}
		if (!bFound)
		{
			DebugDisplay[DebugDisplay.Length] = DebugType;
		}

		SaveConfig();
	}
}

function bool ShouldDisplayDebug(name DebugType)
{
	local int i;

	for ( i=0; i<DebugDisplay.Length; i++ )
	{
		if ( DebugDisplay[i] == DebugType )
		{
			return true;
		}
	}
	return false;
}

/**
 *	Finds the nearest pawn of the given class (excluding the owner's pawn) and
 *	plays the specified FaceFX animation.
 */
exec function FXPlay(class<Pawn> aClass, string FXAnimPath)
{
	local Pawn P, ClosestPawn;
	local float ThisDistance, ClosestPawnDistance;
	local string FxAnimGroup;
	local string FxAnimName;
	local int dotPos;

	if ( WorldInfo.NetMode == NM_Standalone )
	{
		ClosestPawn = None;
		ClosestPawnDistance = 10000000.0;
		ForEach DynamicActors(class'Pawn', P)
		{
			if( ClassIsChildOf(P.class, aClass) && (P != PlayerController(Owner).Pawn) )
			{
				ThisDistance = VSize(P.Location - PlayerController(Owner).Pawn.Location);
				if(ThisDistance < ClosestPawnDistance)
				{
					ClosestPawn = P;
					ClosestPawnDistance = ThisDistance;
				}
			}
		}

		if( ClosestPawn.Mesh != none )
	    {
			dotPos = InStr(FXAnimPath, ".");
			if( dotPos != -1 )
			{
				FXAnimGroup = Left(FXAnimPath, dotPos);
				FXAnimName  = Right(FXAnimPath, Len(FXAnimPath) - dotPos - 1);
				ClosestPawn.Mesh.PlayFaceFXAnim(None, FXAnimName, FXAnimGroup, none);
			}
	    }
	}
}

/**
 *	Finds the nearest pawn of the given class (excluding the owner's pawn) and
 *	stops any currently playing FaceFX animation.
 */
exec function FXStop(class<Pawn> aClass)
{
	local Pawn P, ClosestPawn;
	local float ThisDistance, ClosestPawnDistance;

	if ( WorldInfo.NetMode == NM_StandAlone )
	{
		ClosestPawn = None;
		ClosestPawnDistance = 10000000.0;
		ForEach DynamicActors(class'Pawn', P)
		{
			if( ClassIsChildOf(P.class, aClass) && (P != PlayerController(Owner).Pawn) )
			{
				ThisDistance = VSize(P.Location - PlayerController(Owner).Pawn.Location);
				if(ThisDistance < ClosestPawnDistance)
				{
					ClosestPawn = P;
					ClosestPawnDistance = ThisDistance;
				}
			}
		}

		if( ClosestPawn.Mesh != none )
	    {
			ClosestPawn.Mesh.StopFaceFXAnim();
		}
	}
}

//=============================================================================
// Debugging
//=============================================================================

// DrawRoute - Bot Debugging

function DrawRoute(Pawn Target)
{
	local int			i;
	local Controller	C;
	local vector		Start, RealStart, Dest;
	local bool			bPath;
	local Actor			FirstRouteCache;

	C = Target.Controller;
	if ( C == None )
		return;
	if ( C.CurrentPath != None )
		Start = C.CurrentPath.Start.Location;
	else
		Start = Target.Location;
	RealStart = Start;

	if ( C.bAdjusting )
	{
		Draw3DLine(C.Pawn.Location, C.GetAdjustLocation(), MakeColor(255,0,255,255));
		Start = C.GetAdjustLocation();
	}

	if( C.RouteCache.Length > 0 )
	{
		FirstRouteCache = C.RouteCache[0];
	}

	Dest = C.GetDestinationPosition();

	// show where pawn is going
	if ( (C == PlayerOwner)
		|| (C.MoveTarget == FirstRouteCache) && (C.MoveTarget != None) )
	{
		if ( (C == PlayerOwner) && (Dest != vect(0,0,0)) )
		{
			if ( C.PointReachable(Dest) )
			{
				Draw3DLine(C.Pawn.Location, Dest, MakeColor(255,255,255,255));
				return;
			}
			C.FindPathTo(Dest);
		}
		if( C.RouteCache.Length > 0 )
		{
			for ( i=0; i<C.RouteCache.Length; i++ )
			{
				if ( C.RouteCache[i] == None )
					break;
				bPath = true;
				Draw3DLine(Start,C.RouteCache[i].Location,MakeColor(0,255,0,255));
				Start = C.RouteCache[i].Location;
			}
			if ( bPath )
				Draw3DLine(RealStart,Dest,MakeColor(255,255,255,255));
		}
	}
	else if (Target.Velocity != vect(0,0,0))
		Draw3DLine(RealStart,Dest,MakeColor(255,255,255,255));

	if ( C == PlayerOwner )
		return;

	// show where pawn is looking
	Draw3DLine(Target.Location + Target.BaseEyeHeight * vect(0,0,1), C.GetFocalPoint(), MakeColor(255,0,0,255));
}

/**
 * Pre-Calculate most common values, to avoid doing 1200 times the same operations
 */
function PreCalcValues()
{
	// Size of Viewport
	SizeX	= Canvas.SizeX;
	SizeY	= Canvas.SizeY;

	// Center of Viewport
	CenterX = SizeX * 0.5;
	CenterY = SizeY * 0.5;

	// ratio of viewport compared to native resolution
	RatioX	= SizeX / 1024.f;
	RatioY	= SizeY / 768.f;
}

/**
 * PostRender is the main draw loop.
 */
event PostRender()
{
	local float		XL, YL, YPos;

	// Set up delta time
	RenderDelta = WorldInfo.TimeSeconds - LastHUDRenderTime;

	// Pre calculate most common variables
	if ( SizeX != Canvas.SizeX || SizeY != Canvas.SizeY )
	{
		PreCalcValues();
	}

	// Set PRI of view target
	if ( PlayerOwner != None )
	{
		if ( PlayerOwner.ViewTarget != None )
		{
			if ( Pawn(PlayerOwner.ViewTarget) != None )
			{
				ViewedInfo = Pawn(PlayerOwner.ViewTarget).PlayerReplicationInfo;
			}
			else
			{
				ViewedInfo = PlayerOwner.PlayerReplicationInfo;
			}
		}
		else if ( PlayerOwner.Pawn != None )
		{
			ViewedInfo = PlayerOwner.Pawn.PlayerReplicationInfo;
		}
		else
		{
			ViewedInfo = PlayerOwner.PlayerReplicationInfo;
		}

		// draw any debug text in real-time
		PlayerOwner.DrawDebugTextList(Canvas,RenderDelta);
	}

	if ( bShowDebugInfo )
	{
		Canvas.Font = class'Engine'.Static.GetTinyFont();
		Canvas.DrawColor = ConsoleColor;
		Canvas.StrLen("X", XL, YL);
		YPos = 0;
		PlayerOwner.ViewTarget.DisplayDebug(self, YL, YPos);

		if (ShouldDisplayDebug('AI') && (Pawn(PlayerOwner.ViewTarget) != None))
		{
			DrawRoute(Pawn(PlayerOwner.ViewTarget));
		}
	}
	else if ( bShowHud )
	{
		if ( bShowScores )
		{
			if ( ScoreBoard != None )
			{
				ScoreBoard.Canvas = Canvas;
				ScoreBoard.DrawHud();
				if ( Scoreboard.bDisplayMessages )
				{
					DisplayConsoleMessages();
				}
			}
		}
		else
		{
			DrawHud();

			if ( PlayerOwner.ProgressTimeOut > WorldInfo.TimeSeconds )
			{
				DisplayProgressMessage();
			}

			DisplayConsoleMessages();
			DisplayLocalMessages();
		}
	}
	else
	{
		DrawDemoHUD();
	}

	if ( bShowBadConnectionAlert )
	{
		DisplayBadConnectionAlert();
	}

	LastHUDRenderTime = WorldInfo.TimeSeconds;
}

/**
 * The Main Draw loop for the hud.  Gets called before any messaging.  Should be subclassed
 */
function DrawHUD()
{
	PlayerOwner.DrawHud( Self );

	DrawEngineHUD();
}

/**
 * Called instead of DrawHUD() if bShowHUD==false
 */
function DrawDemoHUD();

/**
 * Special HUD for Engine demo
 */
function DrawEngineHUD()
{
	local	float	CrosshairSize;
	local	float	xl,yl,Y;
	local	String	myText;

	// Draw Copyright Notice
	Canvas.SetDrawColor(255, 255, 255, 255);
	myText = "UnrealEngine3";
	Canvas.Font = class'Engine'.Static.GetSmallFont();
	Canvas.StrLen(myText, XL, YL);
	Y = YL*1.67;
	Canvas.SetPos( CenterX - (XL/2), YL*0.5);
	Canvas.DrawText(myText, true);

	Canvas.SetDrawColor(200,200,200,255);
	myText = "Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.";
	Canvas.Font = class'Engine'.Static.GetTinyFont();
	Canvas.StrLen(myText, XL, YL);
	Canvas.SetPos( CenterX - (XL/2), Y);
	Canvas.DrawText(myText, true);

	if(PlayerOwner != None && Pawn(PlayerOwner.ViewTarget) != None)
	{
		// Draw Temporary Crosshair
		CrosshairSize = 4;
		Canvas.SetPos(CenterX - CrosshairSize, CenterY);
		Canvas.DrawRect(2*CrosshairSize + 1, 1);

		Canvas.SetPos(CenterX, CenterY - CrosshairSize);
		Canvas.DrawRect(1, 2*CrosshairSize + 1);
	}
}


/**
 * display progress messages in center of screen
 */
function DisplayProgressMessage()
{
    local int i, LineCount;
    local float FontDX, FontDY;
    local float X, Y;
    local int Alpha;
    local float TimeLeft;

    TimeLeft = PlayerOwner.ProgressTimeOut - WorldInfo.TimeSeconds;

    if ( TimeLeft >= ProgressFadeTime )
	Alpha = 255;
    else
	Alpha = (255 * TimeLeft) / ProgressFadeTime;

    LineCount = 0;

    for (i = 0; i < ArrayCount(PlayerOwner.ProgressMessage); i++)
    {
		if (PlayerOwner.ProgressMessage[i] == "")
			continue;

		LineCount++;
    }

    if ( (WorldInfo.GRI != None) && (WorldInfo.GRI.MessageOfTheDay != "") )
    {
		LineCount++;
    }

    Canvas.Font = class'Engine'.Static.GetMediumFont();
    Canvas.TextSize ("A", FontDX, FontDY);

    X = (0.5 * HudCanvasScale * Canvas.SizeX) + (((1.0 - HudCanvasScale) / 2.0) * Canvas.SizeX);
    Y = (0.5 * HudCanvasScale * Canvas.SizeY) + (((1.0 - HudCanvasScale) / 2.0) * Canvas.SizeY);

    Y -= FontDY * (float (LineCount) / 2.0);

    for (i = 0; i < ArrayCount(PlayerOwner.ProgressMessage); i++)
    {
		if (PlayerOwner.ProgressMessage[i] == "")
			continue;

		Canvas.DrawColor = MakeColor(255, 255, 255);
		Canvas.DrawColor.A = Alpha;

		Canvas.TextSize (PlayerOwner.ProgressMessage[i], FontDX, FontDY);
		Canvas.SetPos (X - (FontDX / 2.0), Y);
		Canvas.DrawText (PlayerOwner.ProgressMessage[i]);

		Y += FontDY;
    }

    if( (WorldInfo.GRI != None) && (WorldInfo.NetMode != NM_StandAlone) )
    {
		Canvas.DrawColor = MOTDColor;
		Canvas.DrawColor.A = Alpha;

		if( WorldInfo.GRI.MessageOfTheDay != "" )
		{
			Canvas.TextSize (WorldInfo.GRI.MessageOfTheDay, FontDX, FontDY);
			Canvas.SetPos (X - (FontDX / 2.0), Y);
			Canvas.DrawText (WorldInfo.GRI.MessageOfTheDay);
			Y += FontDY;
		}
    }
}


// DisplayBadConnectionAlert() - Warn user that net connection is bad
function DisplayBadConnectionAlert();	// Subclass Me

//=============================================================================
// Messaging.
//=============================================================================

function ClearMessage( out HudLocalizedMessage M )
{
	M.Message = None;
    M.StringFont = None;
}

// Console Messages

function Message( PlayerReplicationInfo PRI, coerce string Msg, name MsgType, optional float LifeTime )
{
	if ( bMessageBeep )
		PlayerOwner.PlayBeepSound();

	if ( (MsgType == 'Say') || (MsgType == 'TeamSay') )
		Msg = PRI.GetPlayerAlias()$": "$Msg;

	AddConsoleMessage(Msg,class'LocalMessage',PRI,LifeTime);
}



/**
 * Display current messages
 */
function DisplayConsoleMessages()
{
    local int Idx, XPos, YPos;
    local float XL, YL;

	if ( ConsoleMessages.Length == 0 )
		return;

    for (Idx = 0; Idx < ConsoleMessages.Length; Idx++)
    {
		if ( ConsoleMessages[Idx].Text == "" || ConsoleMessages[Idx].MessageLife < WorldInfo.TimeSeconds )
		{
			ConsoleMessages.Remove(Idx--,1);
		}
    }

    XPos = (ConsoleMessagePosX * HudCanvasScale * Canvas.SizeX) + (((1.0 - HudCanvasScale) / 2.0) * Canvas.SizeX);
    YPos = (ConsoleMessagePosY * HudCanvasScale * Canvas.SizeY) + (((1.0 - HudCanvasScale) / 2.0) * Canvas.SizeY);

    Canvas.Font = class'Engine'.Static.GetSmallFont();
    Canvas.DrawColor = ConsoleColor;

    Canvas.TextSize ("A", XL, YL);

    YPos -= YL * ConsoleMessages.Length; // DP_LowerLeft
    YPos -= YL; // Room for typing prompt

    for (Idx = 0; Idx < ConsoleMessages.Length; Idx++)
    {
		if (ConsoleMessages[Idx].Text == "")
		{
			continue;
		}
		Canvas.StrLen( ConsoleMessages[Idx].Text, XL, YL );
		Canvas.SetPos( XPos, YPos );
		Canvas.DrawColor = ConsoleMessages[Idx].TextColor;
		Canvas.DrawText( ConsoleMessages[Idx].Text, false );
		YPos += YL;
    }
}

/**
 * Add a new console message to display.
 */
function AddConsoleMessage(string M, class<LocalMessage> InMessageClass, PlayerReplicationInfo PRI, optional float LifeTime)
{
	local int Idx, MsgIdx;
	MsgIdx = -1;
	// check for beep on message receipt
	if( bMessageBeep && InMessageClass.default.bBeep )
	{
		PlayerOwner.PlayBeepSound();
	}
	// find the first available entry
	if (ConsoleMessages.Length < ConsoleMessageCount)
	{
		MsgIdx = ConsoleMessages.Length;
	}
	else
	{
		// look for an empty entry
		for (Idx = 0; Idx < ConsoleMessages.Length && MsgIdx == -1; Idx++)
		{
			if (ConsoleMessages[Idx].Text == "")
			{
				MsgIdx = Idx;
			}
		}
	}
    if( MsgIdx == ConsoleMessageCount || MsgIdx == -1)
    {
		// push up the array
		for(Idx = 0; Idx < ConsoleMessageCount-1; Idx++ )
		{
			ConsoleMessages[Idx] = ConsoleMessages[Idx+1];
		}
		MsgIdx = ConsoleMessageCount - 1;
    }
	// fill in the message entry
	if (MsgIdx >= ConsoleMessages.Length)
	{
		ConsoleMessages.Length = MsgIdx + 1;
	}

    ConsoleMessages[MsgIdx].Text = M;
	if (LifeTime != 0.f)
	{
		ConsoleMessages[MsgIdx].MessageLife = WorldInfo.TimeSeconds + LifeTime;
	}
	else
	{
		ConsoleMessages[MsgIdx].MessageLife = WorldInfo.TimeSeconds + InMessageClass.default.LifeTime;
	}

    ConsoleMessages[MsgIdx].TextColor = InMessageClass.static.GetConsoleColor(PRI);
    ConsoleMessages[MsgIdx].PRI = PRI;
}

//===============================================
// Localized Message rendering

function LocalizedMessage
(
	class<LocalMessage>		InMessageClass,
	PlayerReplicationInfo	RelatedPRI_1,
	string					CriticalString,
	int						Switch,
	float					Position,
	float					LifeTime,
	int						FontSize,
	color					DrawColor,
	optional object			OptionalObject
)
{
	local int i, LocalMessagesArrayCount, MessageCount;

    if( InMessageClass == None || CriticalString == "" )
	{
		return;
	}

	if( bMessageBeep && InMessageClass.default.bBeep )
		PlayerOwner.PlayBeepSound();

    if( !InMessageClass.default.bIsSpecial )
    {
	    AddConsoleMessage( CriticalString, InMessageClass, RelatedPRI_1 );
		return;
    }

	LocalMessagesArrayCount = ArrayCount(LocalMessages);
    i = LocalMessagesArrayCount;
	if( InMessageClass.default.bIsUnique )
	{
		for( i = 0; i < LocalMessagesArrayCount; i++ )
		{
		    if( LocalMessages[i].Message == InMessageClass )
			{
				if ( InMessageClass.default.bCountInstances && (LocalMessages[i].StringMessage ~= CriticalString) )
				{
					MessageCount = (LocalMessages[i].Count == 0) ? 2 : LocalMessages[i].Count + 1;
				}
				break;
			}
		}
	}
	else if ( InMessageClass.default.bIsPartiallyUnique )
	{
		for( i = 0; i < LocalMessagesArrayCount; i++ )
		{
		    if( ( LocalMessages[i].Message == InMessageClass )
				&& InMessageClass.static.PartiallyDuplicates(Switch, LocalMessages[i].Switch, OptionalObject, LocalMessages[i].OptionalObject) )
				break;
		}
	}

    if( i == LocalMessagesArrayCount )
    {
	    for( i = 0; i < LocalMessagesArrayCount; i++ )
	    {
		    if( LocalMessages[i].Message == None )
				break;
	    }
    }

    if( i == LocalMessagesArrayCount )
    {
	    for( i = 0; i < LocalMessagesArrayCount - 1; i++ )
		    LocalMessages[i] = LocalMessages[i+1];
    }

    ClearMessage( LocalMessages[i] );

	// Add the local message to the spot.
	AddLocalizedMessage(i, InMessageClass, CriticalString, Switch, Position, LifeTime, FontSize, DrawColor, MessageCount, OptionalObject);

}

/**
 * Add the actual message to the array.  Made easier to tweak in a subclass
 *
 * @Param	Index				The index in to the LocalMessages array to place it.
 * @Param	InMessageClass		Class of the message
 * @Param	CriticialString		String of the message
 * @Param	Switch				The message switch
 * @Param	Position			Where on the screen is the message
 * @Param	LifeTime			How long does this message live
 * @Param	FontSize			How big is the message
 * @Param	DrawColor			The Color of the message
 */
function AddLocalizedMessage
(
	int						Index,
	class<LocalMessage>		InMessageClass,
	string					CriticalString,
	int						Switch,
	float					Position,
	float					LifeTime,
	int						FontSize,
	color					DrawColor,
	optional int			MessageCount,
	optional object			OptionalObject
)
{
	LocalMessages[Index].Message		= InMessageClass;
	LocalMessages[Index].Switch			= Switch;
	LocalMessages[Index].EndOfLife		= LifeTime + WorldInfo.TimeSeconds;
	LocalMessages[Index].StringMessage	= CriticalString;
	LocalMessages[Index].LifeTime		= LifeTime;
	LocalMessages[Index].PosY			= Position;
	LocalMessages[Index].DrawColor		= DrawColor;
	LocalMessages[Index].FontSize		= FontSize;
	LocalMessages[Index].Count			= MessageCount;
	LocalMessages[Index].OptionalObject	= OptionalObject;
}


function GetScreenCoords(float PosY, out float ScreenX, out float ScreenY, out HudLocalizedMessage InMessage )
{
    ScreenX = 0.5 * Canvas.ClipX;
    ScreenY = (PosY * HudCanvasScale * Canvas.ClipY) + (((1.0f - HudCanvasScale) * 0.5f) * Canvas.ClipY);

    ScreenX -= InMessage.DX * 0.5;
    ScreenY -= InMessage.DY * 0.5;
}

function DrawMessage(int i, float PosY, out float DX, out float DY )
{
    local float FadeValue;
    local float ScreenX, ScreenY;

	FadeValue = FMin(1.0, LocalMessages[i].EndOfLife - WorldInfo.TimeSeconds);

	Canvas.DrawColor = LocalMessages[i].DrawColor;
	Canvas.DrawColor.A = FadeValue * Canvas.DrawColor.A;
	Canvas.Font = LocalMessages[i].StringFont;
	GetScreenCoords( PosY, ScreenX, ScreenY, LocalMessages[i] );
	DX = LocalMessages[i].DX / Canvas.ClipX;
    DY = LocalMessages[i].DY / Canvas.ClipY;

	DrawMessageText(LocalMessages[i], ScreenX, ScreenY);
    LocalMessages[i].Drawn = true;
}

function DrawMessageText(HudLocalizedMessage LocalMessage, float ScreenX, float ScreenY)
{
	Canvas.SetPos( ScreenX, ScreenY );
	Canvas.DrawTextClipped( LocalMessage.StringMessage, false );
}

function DisplayLocalMessages()
{
	local float PosY, DY, DX;
    local int i, j, LocalMessagesArrayCount, AreaMessageCount;
    local float FadeValue;
    local int FontSize;

	// early out
	if ( LocalMessages[0].Message == None )
		return;

 	Canvas.Reset(true);
	LocalMessagesArrayCount = ArrayCount(LocalMessages);

    // Pass 1: Layout anything that needs it and cull dead stuff.
    for( i = 0; i < LocalMessagesArrayCount; i++ )
    {
		if( LocalMessages[i].Message == None )
		{
			break;
		}

		LocalMessages[i].Drawn = false;

		if( LocalMessages[i].StringFont == None )
		{
			FontSize = LocalMessages[i].FontSize + MessageFontOffset;
			LocalMessages[i].StringFont = GetFontSizeIndex(FontSize);
			Canvas.Font = LocalMessages[i].StringFont;
			Canvas.TextSize( LocalMessages[i].StringMessage, DX, DY );
			LocalMessages[i].DX = DX;
			LocalMessages[i].DY = DY;

			if( LocalMessages[i].StringFont == None )
			{
				`warn( "LayoutMessage("$LocalMessages[i].Message$") failed!" );

				for( j = i; j < LocalMessagesArrayCount - 1; j++ )
					LocalMessages[j] = LocalMessages[j+1];
				ClearMessage( LocalMessages[j] );
				i--;
				continue;
			}
		}

		FadeValue = (LocalMessages[i].EndOfLife - WorldInfo.TimeSeconds);

		if( FadeValue <= 0.0 )
		{
			for( j = i; j < LocalMessagesArrayCount - 1; j++ )
				LocalMessages[j] = LocalMessages[j+1];
			ClearMessage( LocalMessages[j] );
			i--;
			continue;
		}
     }

    // Pass 2: Go through the list and draw each stack:
    for( i = 0; i < LocalMessagesArrayCount; i++ )
	{
		if( LocalMessages[i].Message == None )
			break;

		if( LocalMessages[i].Drawn )
			continue;

		PosY = LocalMessages[i].PosY;
		AreaMessageCount = 0;

		for( j = i; j < LocalMessagesArrayCount; j++ )
		{
			if( LocalMessages[j].Drawn || (LocalMessages[i].PosY != LocalMessages[j].PosY) )
			{
				continue;
			}

			DrawMessage( j, PosY, DX, DY );

			PosY += DY;
			AreaMessageCount++;
		}
		if ( AreaMessageCount > MaxHUDAreaMessageCount )
		{
			LocalMessages[i].EndOfLife = WorldInfo.TimeSeconds;
		}
    }
}

static function Font GetFontSizeIndex(int FontSize)
{
	if ( FontSize == 0 )
	{
		return class'Engine'.Static.GetTinyFont();
	}
	else if ( FontSize == 1 )
	{
		return class'Engine'.Static.GetSmallFont();
	}
	else if ( FontSize == 2 )
	{
		return class'Engine'.Static.GetMediumFont();
	}
	else if ( FontSize == 3 )
	{
		return class'Engine'.Static.GetLargeFont();
	}
	else
	{
		return class'Engine'.Static.GetLargeFont();
	}
}


/* returns Red (0.f) -> Yellow -> Green (1.f) Color Ramp */
static function Color GetRYGColorRamp( float Pct )
{
	local Color GYRColor;

	GYRColor.A = 255;

	if ( Pct < 0.34 )
	{
    	GYRColor.R = 128 + 127 * FClamp(3.f*Pct, 0.f, 1.f);
		GYRColor.G = 0;
		GYRColor.B = 0;
	}
    else if ( Pct < 0.67 )
	{
    	GYRColor.R = 255;
		GYRColor.G = 255 * FClamp(3.f*(Pct-0.33), 0.f, 1.f);
		GYRColor.B = 0;
	}
    else
	{
		GYRColor.R = 255 * FClamp(3.f*(1.f-Pct), 0.f, 1.f);
		GYRColor.G = 255;
		GYRColor.B = 0;
	}

	return GYRColor;
}

/**
 * Called when the player owner has died.
 */
function PlayerOwnerDied()
{
}

/**
 *	Pauses or unpauses the game due to main window's focus being lost.
 *	@param Enable tells whether to enable or disable the pause state
 */
event OnLostFocusPause(bool Enable)
{
	if ( LostFocusPaused == Enable )
		return;

	if ( WorldInfo.NetMode != NM_Client )
	{
		LostFocusPaused = Enable;
		PlayerOwner.SetPause(Enable);
	}
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bHidden=true
	RemoteRole=ROLE_None

	WhiteColor=(R=255,G=255,B=255,A=255)
	ConsoleColor=(R=153,G=216,B=253,A=255)
	GreenColor=(R=0,G=255,B=0,A=255)
	RedColor=(R=255,G=0,B=0,A=255)

	MOTDColor=(R=255,G=255,B=255,A=255)
	ProgressFadeTime=1.0
	ConsoleMessagePosY=0.8
	MaxHUDAreaMessageCount=3

	LostFocusPaused=false
}
