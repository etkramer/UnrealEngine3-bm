/**
 * GearHUD_Base
 * Gear Heads Up Display base
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUD_Base extends HUD
	native
	dependson(GearTypes)
	config(Game);

/** Use a minimal HUD? (just crosshair) */
var() config bool bMinimalHUD;

/** Reference to the material effect in the post process chain used for TacCom */
var MaterialEffect TacComEffect;
/** Are we currently doing the initial fade in of TacCom at the beginning of a new round? */
var bool bInitialTaccomFadeIn;

var localized string OverTimeString;

/** Set in DrawHUD so we can do stuff based on whether we are in an MP game or not **/
var bool bInMultiplayerMode;
/** Set in DrawHUD so we can do stuff based on whether we are in an MP Coop game or not */
var bool bInMPCoopMode;

/** List of debug draw functions */
var array<delegate<DebugDraw> > DebugDrawList;

/** Current fade color */
var transient color FadeColor;

/** Last fade alpha to interpolate from */
var transient float PreviousFadeAlpha;

/** Desired fade alpha to interpolate to */
var transient float DesiredFadeAlpha;

/** Current fade alpha */
var transient float FadeAlpha;

/** Current time for fade alpha interpolation */
var transient float FadeAlphaTime;

/** Delay before fading */
var transient float FadeAlphaDelay;

/** Total time for fade alpha interpolation */
var transient float DesiredFadeAlphaTime;

var bool bMBDisabled;

/** whether to make the target show unfriendly or not */
var bool bUnfriendlySpotted;

/** Red screen value for making screen go red when you die */
var const float MaxRednessOfDeathScreen;

/** Last time weapon info needed to be drawn */
var transient float LastWeaponInfoTime;

/** The animating Mortar crosshair tick */
var CanvasIcon MortarCrosshairTickIcon;
/** The mark saved to show where you last shot the Mortar */
var CanvasIcon MortarCrosshairSaveIcon;

enum EAssessSquadBackgroundType
{
	EASBT_Normal,
	EASBT_Dead,
};

struct native AssessSquadData
{
	var float NameYFromCenter;
	var float OrderYFromCenter;
	var float HeadYFromTopOfBackground;
};
var AssessSquadData SquadDisplayData;

struct native AssessSquadCoord
{
	var float XFromTopRight;
	var float YFromTopRight;
};

// structure containing coords for all teammates
struct native AllAssessSquadCoord
{
	var array<AssessSquadCoord> Coords;
};

/** AssessSquadCoords for each splitscreen type */
var array<AllAssessSquadCoord>	AssessSquadCoords;

/** Icons for drawing squadmates in assess mode */
var array<CanvasIcon > SquadBackgrounds;
/** Icon used for the nameplate of the squadmates */
var const CanvasIcon SquadBackgroundNamePlate;

/** enum of the actions of squadmates */
enum EAssessSquadStatus
{
	EASS_None,
	EASS_Normal,
	EASS_Attack,
	EASS_Down,
	EASS_Dead,
	EASS_Defensive,
};

/**
 * enum of the displayable action orders of squadmates
 *  ATTACK is using weapon icons
 */
enum EDisplayableSquadStatus
{
	eSO_DEFENSIVE,
	eSO_ATTACK,
	eSO_DOWN,
	eSO_REVIVE,
	eSO_DEAD,
	eSO_NORMAL,		// <-- Keep this at the bottom since we must use DeathMessageIcons
};

/** Strings for displaying the current action of the squadmate */
var array<CanvasIcon> SquadStatusIcons;

/** SquadIndicator structure and data */
struct native SquadLocatorDatum
{
	var float DirOfHit;
	var MaterialInstanceConstant MILocation;
};
var array<SquadLocatorDatum> SquadLocator;
var const int NumSquadLocators;
var int CurrSquadLocatorToUse;
var Material SquadLocatorMaterial;

/** Material for pulsing the weapon indicator */
var MaterialInstanceConstant ActiveReloadPulseMaterialConstant;
var MaterialInstanceConstant SuperSweetPulseMaterialConstant;
var Material ActiveReloadPulseMaterial;
var Material SuperSweetReloadPulseMaterial;

/** Sniper rifle overlay */
var MaterialInstanceConstant SniperOverlayMaterialConstant;
var Material SniperOverlayMaterial;
var bool bDrawSniperZoom;

/** all of our quick version 1 hit location chevron replacement */
var Material HitLocatorMaterial;

const MAX_HIT_LOCATORS = 3;
var int CurrHitLocatorDataIndex;

struct native HitLocatorDatum
{
	var float DirOfHit;
	var float HitLocationFadePercent;
	var MaterialInstanceConstant MILocation;
	var Actor ActorWhoShotMe;
	var vector HitLocation;
	var bool bDirLocked;
};

var HitLocatorDatum HitLocatorList[MAX_HIT_LOCATORS];

/** Current action possible, prioritized by EActionType */
var ActionInfo ActiveAction;
/** Previously active, kept around for blending */
var ActionInfo PreviousAction;

/** Fade in/out time for actions */
var config float ActionFadeTime;

/** Icon used for the weapon info */
var const CanvasIcon WeaponInfoFrame;

/** Icons used for the health indicators */
var const array<CanvasIcon> HealthIcons;
//@hack - damn textures aren't symmetrical, so this is needed to properly sync them
var const array<float> HealthIconsXOffset;

/** Material instance used for assess mode PPE */
var MaterialInstanceConstant AssessModeMatInst;

/** Current fade color for the overlay */
var LinearColor AssessModeFadeColor;

/** Fade in/out time for assess mode */
var config float AssessModeFadeTime;

var AudioComponent	TacComLoop;
struct native DeathData
{
	// we need to cache victim info here and not use the PRI because in some cases (nonplayers, player leaving the game, etc)
	// the PRI may become invalid while the message is still being displayed
	var string KillerName;
	var string VictimName;
	var byte KillerTeamIndex;
	var byte VictimTeamIndex;
	var class<GearDamageType> Damage;
	var EGearDeathType DeathType;
	var float					TimeRemaining;
	var int                     BloodIconIndex;
	var bool                    bShowBlood;
	var bool                    bUseMessageText;
	var string                  MessageText;
	var bool					bUseTeamColors;
	var bool bKillerDrawTeamColor;
	var bool bKillerIsCOG;
	var bool bVictimDrawTeamColor;
	var bool bVictimIsCOG;
	var bool					bAlwaysShow;
	var bool					bShowPlayerNames;
	var float Scale;
	var int GameSpecificID;

	structdefaultproperties
	{
		Scale=1.f
		GameSpecificID=-1
	}
};

var array<CanvasIcon> GameSpecificIcon;
var CanvasIcon RevivedIcon;

/** Total time for drawing the death data */
var const float DeathDataTime;
/** Fade time for death data */
var const float DeathDataFadeTime;
/** Array of strings to display when people die in multiplayer */
var array<DeathData> HUDMessages;

/** Total time for drawing the ammo data */
var const float AmmoDataTime;
/** Fade time for ammo data */
var const float AmmoDataFadeTime;

/** Enum for HUD colors in Gears */
enum EGearHUDColor
{
	eWARHUDCOLOR_WHITE,
	eWARHUDCOLOR_RED,
	eWARHUDCOLOR_BLACK,
	eWARHUDCOLOR_WHITE_DK,
	eWARHUDCOLOR_TEAMRED,
	eWARHUDCOLOR_TEAMBLUE,
	eWARHUDCOLOR_CHATICONRED,
	eWARHUDCOLOR_CHATICONBLUE,
};

/** Cinematic vars**/
var bool bPlayCinematic;
var float CinematicTextDisplayStartTime;

/** Array of blood icons which are used for the background of the kill messages**/
const BLOOD_ICON_AMT = 4;
var CanvasIcon BloodIcons[BLOOD_ICON_AMT];

/** Current Blood index.  so we don't repeat the same blood splat **/
var int CurrBloodIndex;

/** Values for safezone locations and viewport center */
var float SafeZoneLeft, SafeZoneRight, SafeZoneTop, SafeZoneBottom, SafeZoneBottomFromViewport, SafeZoneFriendlyCenterX, SafeZoneFriendlyCenterY;
/** set this to be true to have it recalculated */
var bool bRefreshSafeZone;

/** Scale of the weapon info */
var const float WeaponInfoScale;

/** The current splitscreen mode we are in */
var ESplitScreenType CurrSplitscreenType;

/** Percentage of the screen height drawable for death messages */
var float HUDMessagesScreenPercent;

/**
* Post process settings for bleed out effect.
*/
var PostProcessSettings BleedOutPPSettings;

/**
* Post process settings for taccom effect.
*/
var PostProcessSettings TaccomPPSettings;

/** structure to keep track of the weapon selector */
struct native WeaponSelectData
{
	var const int	TotalNumBlinks;
	var const float	TotalBlinkTime;
	var float		StartBlinkTime;
};
var WeaponSelectData	WeaponSelect;

/**
 * The possible results of an active reload
 */
enum EActiveReloadResultType
{
	eARResult_None,
	eARResult_Success,
	eARResult_SuperSuccess,
	eARResult_Failed,
};

/**
 * Structure to store the action reload data needed for the AR bar
 */
struct native ActionReloadBarData
{
	var const CanvasIcon		BackGround;		// the background
	var const CanvasIcon		Flash;			// for flashes
	var const CanvasIcon		TickMark;		// the moving indicator
	var const CanvasIcon		SuccessRegion;	// the texture to show success regions
	var const CanvasIcon		PerfectGrade;	// gradient for getting a perfect
	var const CanvasIcon		GoodGrade;		// gradient for getting a normal AR
	var const float				FlashDuration;	// amount of time in seconds the AR will flash when a result occurs
	var const float				FadeDuration;	// amount of time in seconds the AR will fade when a result occurs

	var float				TickMarkTime;	// the time the tick should be using to draw
	var EActiveReloadResultType	ARResult;	// if the button was pressed, this is the result
};
var ActionReloadBarData	ARBarData;

struct native ActionIconAnimData
{
	/** Time of the last time we made an icon switch for animating action icons */
	var float LastActionAnimationSwitch;
	/** Current index of the animating action icon */
	var int CurrentIconIndex;
};
var ActionIconAnimData	ActionIconAnimInfo;

/** Display for showing dead teammates */
var Material DeadTaccomSplatMaterial;
var MaterialInstanceConstant DeadTaccomSplatMaterialConstant;

/** Display for showing revived teammates */
var CanvasIcon ReviveTaccomIcon;

/** Icon to show which players are currently talking */
var CanvasIcon VoiceTaccomIcon;

/** Offset position for the voice taccom icon, this is the offset from the center of the squad circle. */
var float VoiceTaccomIconPos[2];

/** If TRUE, the scoreboard will keep itself hidden */
var bool bRestrictScoreboard;

/** Start time of crosshair fading */
var float CrosshairFadeStartTime;
/** Whether the crosshair fade is in or out */
var bool bIsFadingCrosshairIn;
/** Current opacity percentage of the crosshair fade */
var float CrosshairFadeOpacity;
/** Total fadetime of the crosshair */
var const float TotalCrosshairFadeTime;

/** Whether or not we are tracking player chat (usually TRUE when the taccom is up). */
var bool bTrackingChat;

/** Start time of taccom fading */
var float TaccomFadeStartTime;
/** Whether the taccom fade is in or out */
var bool bIsFadingTaccomIn;
/** Current opacity percentage of the taccom fade */
var float TaccomFadeOpacity;
/** Total fadetime of the taccom */
var const float TotalTaccomFadeTime;

/** Start time of coop pause fading */
var float CoopPauseFadeStartTime;
/** Whether the CoopPause fade is in or out */
var bool bIsFadingCoopPauseIn;
/** Current opacity percentage of the CoopPause fade */
var float CoopPauseFadeOpacity;
/** Total fadetime of the CoopPause */
var const float TotalCoopPauseFadeTime;

/** Whether to draw the weapon indicator or not */
var bool bDrawWeaponIndicator;
/** Start time of weapon fading */
var float WeaponFadeStartTime;
/** Whether the weapon fade is in or out */
var bool bIsFadingWeaponIn;
/** Current opacity percentage of the Weapon fade */
var float WeaponFadeOpacity;
/** Total fadetime of the Weapon */
var const float TotalWeaponFadeTime;

//--------------- Dramatic Text Draw Variables ----------------
/** Total amount of time to draw the dramatic string */
var float TotalDramaticStringDrawTime;
/** Total amount of time to fade the dramatic string in */
var float TotalDramaticStringFadeInTime;
/** Total amount of time to fade the dramatic string out */
var float TotalDramaticStringFadeOutTime;
/** The time we started drawing the dramatic strings */
var float DramaticStringDrawStartTime;
/** The first dramatic string to draw */
var String DramaticString1;
/** The second dramatic string to draw */
var String DramaticString2;
/** Vertical value of where to draw the dramatic text */
var float DramaticTextDrawYStart;
/** Amount of time for the lines to wait on fading in */
var float DramaticLineWaitTime;
/** Amount of time for the lines to fade prior to the text */
var float DramaticLineFadeTime;

//--------------- SubtitleArea Text Draw Variables ----------------
// This is for drawing text in the subtitle area... this is not ACTUALLY drawing subtitles
// but merely drawing strings in the area where subtitles normally would draw
/** Total amount of time to draw the string */
var float TotalSubtitleStringDrawTime;
/** Total amount of time to fade the subtitle string in */
var float TotalSubtitleStringFadeInTime;
/** Total amount of time to fade the subtitle string out */
var float TotalSubtitleStringFadeOutTime;
/** The time we started drawing the subtitle strings */
var float SubtitleStringDrawStartTime;
/** The subtitle string to draw */
var String SubtitleString;

/** localized string for displaying a weapon being taken in MP */
var localized String WeaponTakenString;

/** localized strings for coop-client pause screen */
var localized String ServerMessageString;
var localized String ServerPauseString;
var PlayerReplicationInfo ServerPausedByPRI;

/** localized string to display when the game is loading */
var localized String LoadingString;
var localized String CheckpointReachedString;

/** The last time a checkpoint was hit */
var float TimeOfLastCheckpoint;
var const float MinTimeBeforeFadeForCheckpoint;
var const float FadeTimeForCheckpoint;

/** This flag is used by clients of a coop game to determined the previous frame's pause state */
var bool bCoopClientWasPaused;

/** Distance at which you will see your DBNO teammate icon in full opacity */
var() config float DBNOFullOpacityDistance;
/** Distance at which your DBNO teammate icon stops fading out */
var() config float DBNOMaxFadeOpacityDistance;
/** The most faded your DBNO teammate icon will be */
var() config float DBNOMaxFadeOpacity;

/** Icon for background of countdown */
var CanvasIcon CountdownBgdIcon;
/** Countdown End time */
var float CountdownEndTime;
var float CountdownStartTime;
var float CountdownStopTime;
var const float CountdownTotalFadeInTime;
var const float CountdownTotalFadeOutTime;
var float CountdownTimeToDraw;
var bool bDrewCountdownLastFrame;

/** String to draw based on the kismet SeqAct_DrawMessage action */
var string KismetMessageText;
var float  KismetMessageEndTime;

/** The content reference to the UI scene for spectating */
var const GearUIScene_Base SpectatorUISceneReference;
/** The instance reference to the UI scene for spectating */
var GearUIScene_Base SpectatorUISceneInstance;

/** The content reference to the UI scene for pausing the campaign */
var const GearUIScenePause_Campaign PauseCampaignUISceneReference;
/** The content reference to the UI scene for pausing the MP */
var const GearUIScenePause_MP PauseMPUISceneReference;
/** The instance reference to the UI scene for Pausing */
var GearUIScenePause_Base PauseUISceneInstance;

/** The content reference to the Gameover UI scene */
var const GearUIScenePause_Gameover GameoverUISceneReference;
/** The instance reference to the Gameover UI Scene */
var GearUIScene_Base GameoverUISceneInstance;

/** The content reference to the UI scene for picking up a discoverable */
var const GearUIScene_Discover DiscoverUISceneReference;
/** The instance reference to the UI scene for pickup up a discoverable */
var GearUIScene_Discover DiscoverUISceneInstance;

/** Reference to the path choice scene */
var() GearUIScene_PathChoice PathChoiceSceneRef;

var transient float LastMortarTickPercent;

/** Enum for the different special icons that could be drawn in squadmate UI */
enum EGearSpecialIconType
{
	eGSIT_None,
	eGSIT_Leader,
	eGSIT_Flag,
	eGSIT_BuddyCOG,
	eGSIT_BuddyLocust
};

/** Icon of the leader */
var CanvasIcon LeaderIcon;
/** Icon of the meatflag */
var CanvasIcon MeatflagIcon;
/** Icon for a COG buddy */
var CanvasIcon BuddyIconCOG;
/** Icon for a Locust buddy */
var CanvasIcon BuddyIconLocust;

/** Whether to show names when spectating or not */
var bool bShowNamesWhenSpectating;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if( ActiveReloadPulseMaterialConstant == None )
	{
		ActiveReloadPulseMaterialConstant = new(outer) class'MaterialInstanceConstant';
		ActiveReloadPulseMaterialConstant.SetParent( ActiveReloadPulseMaterial );
	}
	if ( SuperSweetPulseMaterialConstant == None )
	{
		SuperSweetPulseMaterialConstant = new(outer) class'MaterialInstanceConstant';
		SuperSweetPulseMaterialConstant.SetParent( SuperSweetReloadPulseMaterial );
	}

	 GearPC(PlayerOwner).RefreshAllSafeZoneViewports();
}

/** Initiate the display of "no longer leader" - needed by GearHUDKTL */
function StartNoLongerLeaderDisplay( bool bLeaderIsKiller );

function SetHUDDrawColor( EGearHUDColor eColor, byte Alpha = 255 )
{
	switch ( eColor )
	{
		case eWARHUDCOLOR_WHITE:
			Canvas.SetDrawColor( 240, 240, 240, Alpha );
			break;

		case eWARHUDCOLOR_RED:
			Canvas.SetDrawColor( 170, 0, 0, Alpha );
			break;

		case eWARHUDCOLOR_BLACK:
			Canvas.SetDrawColor( 0, 0, 0, Alpha );
			break;

		case eWARHUDCOLOR_WHITE_DK:
			Canvas.SetDrawColor( 200, 200, 200, Alpha );
			break;

		case eWARHUDCOLOR_TEAMRED:
			Canvas.SetDrawColor( 205, 22, 22, Alpha );
			break;

		case eWARHUDCOLOR_TEAMBLUE:
			Canvas.SetDrawColor( 139, 255, 255, Alpha );
			break;

		case eWARHUDCOLOR_CHATICONRED:
			Canvas.SetDrawColor( 164, 63, 63, Alpha );
			break;

		case eWARHUDCOLOR_CHATICONBLUE:
			Canvas.SetDrawColor( 170, 192, 205, Alpha );
			break;
	}
}

/** Stubs for the DM version of the HUD */
function ShowPlayerName();

simulated event PostRender()
{
	`if(`notdefined(FINAL_RELEASE))
	local float		XL, YL, YPos;
	`endif
	local Font		OldFont;
	local GameViewportClient VPClient;
	local GearPRI OwnerPRI;
	local GearPC PC;
`if(`isdefined(_WATERMARK_))
	local string BuildDate;
`endif //END _WATERMARK_

	// is this a multiplayer game?
	bInMultiplayerMode = ((WorldInfo.GRI != None) && WorldInfo.GRI.IsMultiplayerGame()) ? TRUE : FALSE;
	bInMPCoopMode = ((WorldInfo.GRI != None) && WorldInfo.GRI.IsCoopMultiplayerGame()) ? TRUE : FALSE;

	// calculate the screen percentage for death and ammo messages
	HUDMessagesScreenPercent = bInMultiplayerMode ? 0.75f : 0.75f;

	// Set up delta time
	RenderDelta = WorldInfo.TimeSeconds - LastHUDRenderTime;

	PC = GearPC(PlayerOwner);
	if ( PC != None )
	{
		PC.DrawDebugTextList(Canvas,RenderDelta);
		if ( PC.Player != None )
		{
			VPClient = LocalPlayer(PlayerOwner.Player).ViewportClient;
			CurrSplitscreenType = VPClient.GetSplitscreenConfiguration();

			// NOTE: CalculateDeadZone is being cached now -
			if (bRefreshSafeZone)
			{
				CalculateDeadZone();
			}

			// Just in case SpectatorUISceneInstance gets created during split screen
			if ( SpectatorUISceneInstance != None )
			{
				SafeZoneBottom = Canvas.ClipY - (Canvas.ClipY * 0.9f * 0.836455f) - (Canvas.ClipY * 0.05f);
			}
			else
			{
				SafeZoneBottom = SafeZoneBottomFromViewport;
			}


			//VPClient.CalculatePixelCenter( SafeZoneFriendlyCenterX, SafeZoneFriendlyCenterY, LocalPlayer(PlayerOwner.Player), Canvas );
			SafeZoneFriendlyCenterX = Canvas.ClipX/2;
			SafeZoneFriendlyCenterY = Canvas.ClipY/2;

			PC.SceneYOffsetForSplitscreen = (CurrSplitscreenType == eSST_2P_VERTICAL) ? SafeZoneTop + (WeaponInfoFrame.VL*0.5f*WeaponInfoScale) : 0.f;
		}

		OwnerPRI = GearPRI(PlayerOwner.PlayerReplicationInfo);


//debug
`if(`notdefined(FINAL_RELEASE))
		if( PC.Pawn != None )
		{
			PC.Pawn.DrawPathStep( Canvas );
		}
`endif
	}

`if(`notdefined(FINAL_RELEASE))
	// WorldInfo is replicated out from the server in MP games
	if( !bMinimalHUD )
	{
		if( !WorldInfo.bPathsRebuilt )
		{
			OldFont = Canvas.Font;
			Canvas.SetPos(10,130);
			Canvas.Font = class'Engine'.Static.GetSmallFont();
			Canvas.DrawText("PATHS NEED TO BE REBUILT");
			Canvas.Font = OldFont;
		}
	}
`endif

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
	}

	if( bPlayCinematic && ( (WorldInfo.TimeSeconds - CinematicTextDisplayStartTime ) < 3 ) )
	{
		OldFont = Canvas.Font;
		Canvas.SetPos(130,130);
		Canvas.Font = class'Engine'.Static.GetLargeFont();
		Canvas.DrawText("CINEMATIC");
		Canvas.Font = OldFont;
	}
`if(`notdefined(FINAL_RELEASE))
	if ( bShowDebugInfo && PlayerOwner.ViewTarget != None )
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
	else
`endif
    if( bShowHud &&
		(OwnerPRI  == None || OwnerPRI.bShowHUD) )
	{
		PlayerOwner.DrawHud( Self );

		DrawHud();

		if ( PlayerOwner.ProgressTimeOut > WorldInfo.TimeSeconds )
		{
			DisplayProgressMessage();
		}

`if(`notdefined(FINAL_RELEASE))
		if (!bMinimalHUD)
		{
			DisplayConsoleMessages();
			DisplayLocalMessages();
		}
`endif
	}
	// make sure we didn't leave a stray spectator scene up
	else if (SpectatorUISceneInstance != None)
	{
		PC.ClientCloseScene(SpectatorUISceneInstance);
		SpectatorUISceneInstance = None;
		PC.RefreshAllSafeZoneViewports();
	}

`if(`notdefined(FINAL_RELEASE))
	if (GearGame(WorldInfo.Game) != None)
	{
		if (GearGame(WorldInfo.Game).UnscriptedDialogueManager.bEnableGUDSBrowsing)
		{
			GearGame(WorldInfo.Game).UnscriptedDialogueManager.GUDBrowserRender(Canvas);
		}
		else if (GearGame(WorldInfo.Game).UnscriptedDialogueManager.bEnableEffortsBrowsing)
		{
			GearGame(WorldInfo.Game).UnscriptedDialogueManager.EffortsBrowserRender(Canvas);
		}
	}
`endif

	// draw message info
	DrawHUDMessages(RenderDelta);

	// Don't show the dramatic text if the pause menu is open or bShowHud is false in an MP match
	// We want it to show when bShowHud is false in SP so we can see it over the cinematics
	if ( !HideHUDFromScenes() &&
		 (!bInMultiplayerMode || bShowHud) )
	{
		UpdateDramaticText();
	}

	if (FadeAlpha != 0.f)
	{
		Canvas.SetOrigin(0,0);
		Canvas.SetPos(0,0);
		Canvas.DrawColor = MakeColor(FadeColor.R,FadeColor.G,FadeColor.B,FadeAlpha);
		Canvas.DrawTile(Texture2D'WhiteSquareTexture',Canvas.ClipX,Canvas.ClipY,0,0,2,2);
	}

	if( LoadingTextIsDrawing() )
	{
		OldFont = Canvas.Font;
		DrawLoadingText();
		Canvas.Font = OldFont;
	}

	LastHUDRenderTime = WorldInfo.TimeSeconds;
}

event Destroyed()
{
	if ( PauseUISceneInstance != None )
	{
		PauseUISceneInstance.CloseScene(PauseUISceneInstance, true, true);
	}
	if ( GameoverUISceneInstance != None )
	{
		GameoverUISceneInstance.CloseScene(GameoverUISceneInstance, true, true);
	}

	if ( WorldInfo != None && WorldInfo.Game != None )
	{
		WorldInfo.Game.ForceClearUnpauseDelegates(Self);
	}

	Super.Destroyed();
	ResetTaccomOpacity();
}

/**
 * Reset the HUD
 */
function Reset()
{
	local GearPlayerCamera Cam;

	ActiveAction.bActive = FALSE;
	ActiveAction.BlendPct = 0.f;
	PreviousAction.bActive = FALSE;
	PreviousAction.BlendPct = 0.f;
	ClearHUDAfterShowToggle();

	CrosshairFadeStartTime = 0.0f;
	bIsFadingCrosshairIn = FALSE;
	CrosshairFadeOpacity = 0.0f;

	TaccomFadeStartTime = 0.0f;
	bIsFadingTaccomIn = FALSE;
	bInitialTaccomFadeIn = FALSE;
	ResetTaccomOpacity();

	CoopPauseFadeStartTime = 0.0f;
	bIsFadingCoopPauseIn = FALSE;
	CoopPauseFadeOpacity = 0.0f;

	WeaponFadeStartTime = 0.0f;
	bDrawWeaponIndicator = FALSE;
	bIsFadingWeaponIn = FALSE;
	WeaponFadeOpacity = 0.0f;

	TimeOfLastCheckpoint = -1;

	Cam = GearPlayerCamera(PlayerOwner.PlayerCamera);
	if ( Cam != None )
	{
		Cam.SetColorScale( vect(1,1,1) );
	}

	ClearHitLocatorData();

	GearPC(PlayerOwner).RefreshAllSafeZoneViewports();
}

/** Clear out the display messages */
simulated function ClearHUDAfterShowToggle()
{
	LastWeaponInfoTime = -9999;
	HUDMessages.length = 0;
}

final function GearPawn ResolvePawn(optional out Vehicle WPVehicle, optional out GearVehicle GVehicle)
{
	local GearPawn WP;
	local GearWeaponPawn WeapPawn;
	local Turret TurretPawn;
	WP = GearPawn(PlayerOwner.Pawn);
	if ( WP == None || WP.bDeleteMe )
	{
		// use the vehicle
		WPVehicle = Vehicle(PlayerOwner.Pawn);
		GVehicle = GearVehicle(WPVehicle);
		WeapPawn = GearWeaponPawn(WPVehicle);

		// Handle being in passenger seat
		if(WeapPawn != None)
		{
			GVehicle = WeapPawn.MyVehicle;
		}
		if (WPVehicle != None)
		{
			WP = GearPawn(WPVehicle.Driver);
		}
		// try the viewtarget
		if (WP == None)
		{
			WP = GearPawn(PlayerOwner.ViewTarget);
		}
		// maybe a turret?
		if (WP == None)
		{
			TurretPawn = Turret(PlayerOwner.Pawn);
			if(TurretPawn != None)
			{
				WP = GearPawn(TurretPawn.Driver);
			}
		}
	}
	else
	// If based on another gear pawn (ie brumak) use the base
	if( GearPawn(WP.Base) != None )
	{
		WP = GearPawn(WP.Base);
	}

	return WP;
}

/**
 * The Main Draw loop for the hud.  Get's called before any messaging.  Should be subclassed
 */
function DrawHUD()
{
	local GearPC PC;
	local GearPawn WP;
	local bool bIsTurreting;
	local float XL, YL;
	local Vehicle WPVehicle;
	local GearVehicle GVehicle;

	PC = GearPC(PlayerOwner);

	// See if we should show/hide the spectator scene
	CheckSpectatorScene();

	// handle any debug draws
	foreach DebugDrawList(DebugDraw)
	{
		DebugDraw(self);
	}

	if (KismetMessageText != "")
	{

		if(KismetMessageEndTime > 0 && KismetMessageEndTime <= WorldInfo.TimeSeconds)
		{
			KismetMessageText = "";
			KismetMessageEndTime = 0;
		}
		else
		{
			Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);//Font'Warfare_HUD.WarfareHUD_Font18pt';
			Canvas.TextSize(KismetMessageText,XL,YL);
			Canvas.SetPos(Canvas.ClipX/2 - XL/2 - 16, Canvas.ClipY/3 - YL/2 - 16);
			Canvas.SetDrawColor(47,47,47,128);
			Canvas.DrawTile(Texture2D'EngineResources.WhiteSquareTexture',XL + 32, YL + 32, 0, 0, 2, 2);
			Canvas.SetPos(Canvas.ClipX/2 - XL/2, Canvas.ClipY/3 - YL/2);
			SetHUDDrawColor(eWARHUDCOLOR_WHITE);
			Canvas.DrawText(KismetMessageText,FALSE);
		}
	}

	// if the hud isn't turned off
	if ( bShowHud &&
		 !HideHUDFromScenes() )
	{
		WP = ResolvePawn(WPVehicle,GVehicle);
		if (PC != None)
		{
			// If we are not dead dead (dead)
			if (WP != None && (WP.Health > 0 || WP.IsDBNO()))
			{
				// if in assess mode
				if( PC.bAssessMode || (TaccomFadeOpacity > 0.f) )
				{
					// still draw the DBNO indicator in assess mode
					if (WP.IsDBNO())
					{
						DrawHealth(WP.Health, WP.DefaultHealth, WP.IsDBNO());
					}
					// only draw assess mode
					DrawAssessMode(PC,WP);
				}
				else
				{
					// Give game specific HUD a chance to draw special icons above the heads of players
					DrawSpecialPlayerIcons();

					bIsTurreting = (PC.GetStateName() == 'PlayerTurreting');
					if ( (WPVehicle == None && WP != None) || (GVehicle != None && GVehicle.WantsCrosshair(PlayerOwner)) || bIsTurreting )
					{
						if(GVehicle != None)
						{
							// Draw recent completed/received objectives
							if ( PC.ObjectiveMgr != None )
							{
								PC.ObjectiveMgr.DrawObjectives( self, PC, 1.f, TRUE );
							}
							if(GVehicle.bDrawHealthOnHUD)
							{
								DrawHealth(GVehicle.Health, GVehicle.GetVehicleDefaultHealth(), FALSE);
							}
						}
						else if ( WP != None )
						{
							// Draw recent completed/received objectives
							if ( PC.ObjectiveMgr != None )
							{
								PC.ObjectiveMgr.DrawObjectives( self, PC, 1.f, TRUE );
							}

							// draw health indicator
							DrawHealth(WP.Health, WP.DefaultHealth, WP.IsDBNO());
						}

						DrawHitLocationIndicator();

						// crosshair
						DrawWeaponCrosshair(PC,WP);

						// and weapon info (if desired)
						if(GVehicle == None || GVehicle.ShouldShowWeaponOnHUD(PC))
						{
							UpdateWeaponIndicator(PC, WP, (PlayerOwner.Pawn != None) ? PlayerOwner.Pawn : Pawn(PlayerOwner.ViewTarget));
						}
					}

					if (!PC.IsSpectating())
					{
						// draw any action icons
						DrawActions(RenderDelta);

						DrawDownedButNotOutTeammates(PC,WP);
					}

					if ( bInMultiplayerMode && bShowNamesWhenSpectating )
					{
						DrawSpectatingNames(PC, WP);
					}
				}
			}
			else if (!PC.IsInState('ScreenshotMode'))
			{
				if (GearSpectatorPoint(PC.ViewTarget) == None &&
					(PC.PlayerReplicationInfo == None || !PC.PlayerReplicationInfo.bOnlySpectator) )
				{
					// if just died then delay a second before popping up the omen so that they can enjoy the death anim
					if (WP != None && (TimeSince(WP.TimeOfDeath) > 1.f || WP.bWasDBNO))
					{
						if (WP.bWasDBNO)
						{
							DrawDeadInfo(FClamp(TimeSince(WP.TimeOfDeath)/2.f - 0.5f,0.5f,1.f));
						}
						else
						{
							DrawDeadInfo(FClamp(TimeSince(WP.TimeOfDeath)/2.f - 0.5f,0.f,1.f));
						}
					}
				}
				if( PC.bAssessMode || TaccomFadeOpacity > 0.f )
				{
					DrawAssessMode(PC,WP);
				}
				else
				{
					if ( bInMultiplayerMode && bShowNamesWhenSpectating )
					{
						DrawSpectatingNames(PC, WP);
					}
				}
			}
		}

		// Draw the dramatic text
		if ( !IsDramaticTextDrawing() )
		{
			// draw the countdown
			if ( !UpdateCountdown() && bInMultiplayerMode )
			{
				DrawRespawnTimer();
			}
		}

		// Draw the subtitle text
		UpdateSubtitleText();

		if ( bInMultiplayerMode )
		{
			if ( PC.IsInState('Reviving') )
			{
				// make sure we can draw the button mash action icon
				DrawActions(RenderDelta);
			}
		}
	}
}

/** Returns the respawn time interval for the game */
simulated function int GetRespawnTimeInterval()
{
	return 15;
}

/** Draw the timer that tells the player when they will respawn */
simulated final protected function DrawRespawnTimer()
{
	local String TimeString;
	local float TextWidth, TextHeight, DrawX, DrawY;
	local int TimeInterval, TimeToDraw;
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None &&
		GearPC(PlayerOwner).bWaitingToRespawn &&
		GRI.GameStatus == GS_RoundInProgress)
	{
		DrawX = Canvas.ClipX/2;
		DrawY = SafeZoneTop + GetCountdownHeight() + 3.f;

		TimeToDraw = GRI.RespawnTime;
		TimeInterval = GetRespawnTimeInterval();
		if ( TimeInterval > 0 )
		{
			TimeToDraw = TimeToDraw % TimeInterval;
		}
		TimeString = CreateTimeString( TimeToDraw );
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro24);
		Canvas.TextSize( TimeString, TextWidth, TextHeight );
		DrawX -= TextWidth/2;
		DrawStringWithBackground( DrawX, DrawY, TimeString );
	}
}

/** Update the drawing of the weapon icon */
final protected function UpdateWeaponIndicator(GearPC PC, GearPawn WP, Pawn PawnOwner)
{
	local GearWeapon Weap;
	local bool bIsCurrentlyOn;
	local GearPRI WPRI;

	WPRI = GearPRI(PC.PlayerReplicationInfo);
	Weap = GearWeapon(PawnOwner.Weapon);

	if (Weap != None && WPRI != None && PC.CurrentPPType != eGPP_GameOver)
	{
		bIsCurrentlyOn = (bInMultiplayerMode || PC.bIsTargeting || PC.IsButtonActive(GB_RightTrigger) || Weap.IsReloading() || Weap.IsFiring() || PC.IsInCombat() || WPRI.bAlwaysDrawWeaponIndicatorInHUD || ((WorldInfo.TimeSeconds - LastWeaponInfoTime) < 4.0f)) ? TRUE : FALSE;

		// weapon is about to hide (or if we are not always showing it then do not fade it)
		if ( bDrawWeaponIndicator && !bIsCurrentlyOn && !WPRI.bAlwaysDrawWeaponIndicatorInHUD )
		{
			bDrawWeaponIndicator = FALSE;
			bIsFadingWeaponIn = FALSE;
			WeaponFadeStartTime = WorldInfo.TimeSeconds - (TotalWeaponFadeTime * (1.f - WeaponFadeOpacity));
		}
		// weapon is about to show
		else if ( !bDrawWeaponIndicator && bIsCurrentlyOn )
		{
			bDrawWeaponIndicator = TRUE;
			bIsFadingWeaponIn = TRUE;
			WeaponFadeStartTime = WorldInfo.TimeSeconds - (TotalWeaponFadeTime * WeaponFadeOpacity);
		}

		// Determine what the opacity of the Weapon should be.
		if ( bIsFadingWeaponIn || bDrawWeaponIndicator )
		{
			if ( (WeaponFadeStartTime > 0.f) && ((WorldInfo.TimeSeconds - WeaponFadeStartTime) < TotalWeaponFadeTime) )
			{
				WeaponFadeOpacity = (WorldInfo.TimeSeconds - WeaponFadeStartTime) / TotalWeaponFadeTime;
			}
			else
			{
				WeaponFadeOpacity = 1.f;
				bIsFadingWeaponIn = FALSE;
			}
		}
		else if ( !bIsFadingWeaponIn && ((WorldInfo.TimeSeconds - WeaponFadeStartTime) < TotalWeaponFadeTime) && (WeaponFadeStartTime > 0.f) )
		{
			WeaponFadeOpacity = 1.f - ((WorldInfo.TimeSeconds - WeaponFadeStartTime) / TotalWeaponFadeTime);
		}
		else
		{
			WeaponFadeOpacity = 0.f;
		}

		if( WeaponFadeOpacity <= 0.f )
		{
			return;
		}

		// we always want to see the weapon indicator at full opacity
		if( WPRI.bAlwaysDrawWeaponIndicatorInHUD )
		{
			WeaponFadeOpacity = 1.0f;
		}

		DrawWeaponInfo(PC, WP, Weap, Canvas.ClipX - SafeZoneRight - WeaponInfoFrame.UL,SafeZoneTop,255 * WeaponFadeOpacity, FALSE, WeaponInfoScale, TRUE, FALSE, TRUE);
	}
	else
	{
		WeaponFadeStartTime = 0.0f;
		bDrawWeaponIndicator = FALSE;
		bIsFadingWeaponIn = FALSE;
		WeaponFadeOpacity = 0.0f;
	}
}

/**
 * Sets the active action for displaying on the HUD.
 */
final event bool SetActionInfo(EActionType NewActionType, ActionInfo NewAction, optional bool bMirrorImage)
{
	local int Idx;
	local Name CurrActionName;

	CurrActionName = ActiveAction.ActionName;

	// if there currently is an active action
	if (ActiveAction.bActive)
	{
		// if the new priority is higher than the previously higher one (0 is highest) or it is a different action
		if ( NewActionType < ActiveAction.ActionType ||
			 NewAction.ActionName != ActiveAction.ActionName ||
			 NewAction.ToolTipText != ActiveAction.ToolTipText ||
			 bMirrorImage != ActiveAction.bMirror ||
			 ( ((NewAction.ActionIconDatas.length > 0) && (ActiveAction.ActionIconDatas.length > 0)) &&
			   (NewAction.ActionIconDatas[0].ActionIcons[0] != ActiveAction.ActionIconDatas[0].ActionIcons[0]) )
		   )
		{
			// then de-activate this one
			PreviousAction = ActiveAction;
			PreviousAction.bActive = FALSE;
		}
		else
		{
			// otherwise ignore
			return FALSE;
		}
	}
	// set the new active action
	ActiveAction = NewAction;
	ActiveAction.ActionType = NewActionType;
	ActiveAction.bActive = TRUE;
	ActiveAction.BlendPct = 0.f;
	ActiveAction.ActivateTime = WorldInfo.TimeSeconds;
	ActiveAction.bMirror = bMirrorImage;
	// cache some draw info for the icons
	for (Idx = 0; Idx < ActiveAction.ActionIconDatas.Length; Idx++)
	{
		// total the icon sizes
		ActiveAction.SizeX += ActiveAction.ActionIconDatas[Idx].ActionIcons[0].UL;
		// if not the last element
		if (Idx < ActiveAction.ActionIconDatas.Length - 1)
		{
			// add any spacing
			ActiveAction.SizeX += ActiveAction.IconSpacing;
		}
		// pick the max height
		ActiveAction.SizeY = FMax(ActiveAction.SizeY,ActiveAction.ActionIconDatas[Idx].ActionIcons[0].VL);
	}
	ActiveAction.bCached = TRUE;

	ActionIconAnimInfo.CurrentIconIndex = 0;
	ActionIconAnimInfo.LastActionAnimationSwitch = WorldInfo.TimeSeconds;

	// See if we should trigger the event delegates for being able to execute someone
	if ( (CurrActionName != ActiveAction.ActionName) && (ActiveAction.ActionName == 'CurbStomp') )
	{
		GearPC(PlayerOwner).TriggerGearEventDelegates( eGED_Executions );
	}

	return TRUE;
}

/**
 * Clears the active action if it matches the specified type.
 */
final function bool ClearActionInfoByType(EActionType ActionType)
{
	if (ActiveAction.bActive && ActionType == ActiveAction.ActionType)
	{
		// disable the active action
		ActiveAction.bActive = FALSE;
		ActiveAction.ActionType = AT_None;
		// move to previous so it can be blended
		PreviousAction = ActiveAction;
		return TRUE;
	}
	return FALSE;
}

/** Clear the active action regardless of its type */
final function ClearActiveActionInfo()
{
	ActiveAction.bActive = FALSE;
	ActiveAction.ActionType = AT_None;
}

simulated function GetSubtitleSize( out float Width, out float Height )
{
	if ( GearPC(PlayerOwner).IsShowingSubtitles() )
	{
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro24);
		Canvas.TextSize( "W", Width, Height );
	}
}

/** See if we need to remap the buttons based on alternate control schemes */
final function RemapButtonIcon( out ActionInfo Action )
{
	local GearPC GPC;
	local ActionIconData ButtonAIcon, MeatbagIcon;
	local GearPRI PRI;

	GPC = GearPC(PlayerOwner);

	if (Action.ActionName != '')
	{
		// Check for trigger swap
		if ( (GPC.PlayerReplicationInfo != None) && (GearPRI(GPC.PlayerReplicationInfo).TrigConfig != WTCO_Default) )
		{
			// Suicide bomb must show the 'Left Trigger' instead of the 'Right Trigger'
			if ( Action.ActionName == 'SuicideBomb' )
			{
				Action.ActionIconDatas[0].ActionIcons[0].U = 62;
				Action.ActionIconDatas[0].ActionIcons[0].V = 375;
				Action.ActionIconDatas[0].ActionIcons[0].UL = 34;
				Action.ActionIconDatas[0].ActionIcons[0].VL = 42;
			}
		}

		// Check for CurbStomp, we have to take care of hiding the Meatbag when you have a shield
		// and for alternate controls
		if ( Action.ActionName == 'CurbStomp' )
		{
			// If the player has a shield
			if ( GPC.MyGearPawn != None && GPC.MyGearPawn.EquippedShield != None )
			{
				// There's a shield so we only need to do the alternate controls check
				if ( GPC.bUseAlternateControls )
				{
					// Swap 'X' to 'Y' only
					Action.ActionIconDatas[0].ActionIcons[0].U = 389;
					Action.ActionIconDatas[0].ActionIcons[0].V = 421;
					Action.ActionIconDatas[0].ActionIcons[0].UL = 35;
					Action.ActionIconDatas[0].ActionIcons[0].VL = 43;
				}
			}
			// No shield so we must add the MeatBag icon
			else
			{
				ButtonAIcon.ActionIcons.length = 1;
				ButtonAIcon.ActionIcons[0].Texture = Texture2D'Warfare_HUD.WarfareHUD_ActionIcons';
				ButtonAIcon.ActionIcons[0].U = 212;
				ButtonAIcon.ActionIcons[0].V = 314;
				ButtonAIcon.ActionIcons[0].UL = 35;
				ButtonAIcon.ActionIcons[0].VL = 43;
				MeatbagIcon.ActionIcons.length = 1;
				MeatbagIcon.ActionIcons[0].Texture = Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons';
				MeatbagIcon.ActionIcons[0].U = 0;
				MeatbagIcon.ActionIcons[0].V = 303;
				MeatbagIcon.ActionIcons[0].UL = 58;
				MeatbagIcon.ActionIcons[0].VL = 95;
				Action.ActionIconDatas.InsertItem( 0, ButtonAIcon );
				Action.ActionIconDatas.InsertItem( 1, MeatbagIcon );

				// Now check for alternate controls
				if ( GPC.bUseAlternateControls )
				{
					// Curbstomp has X button as 3rd icon... must make swap with Y
					Action.ActionIconDatas[2].ActionIcons[0].U = 389;
					Action.ActionIconDatas[2].ActionIcons[0].V = 421;
					Action.ActionIconDatas[2].ActionIcons[0].UL = 35;
					Action.ActionIconDatas[2].ActionIcons[0].VL = 43;
				}
			}
		}
		// Check for alternate controls
		else if ( GPC.bUseAlternateControls )
		{
			// Evades must now show the 'X' instead of the 'A'
			if ( Action.ActionName == 'EvadeFromCover' )
			{
				Action.ActionIconDatas[0].ActionIcons[0].U = 330;
				Action.ActionIconDatas[0].ActionIcons[0].V = 314;
				Action.ActionIconDatas[0].ActionIcons[0].UL = 45;
				Action.ActionIconDatas[0].ActionIcons[0].VL = 32;
			}
			// Anything previously shown as 'X' is now 'Y'
			else if ( Action.ActionIconDatas[0].ActionIcons[0].U == 330 &&
					  Action.ActionIconDatas[0].ActionIcons[0].V == 314 )
			{
				Action.ActionIconDatas[0].ActionIcons[0].U = 389;
				Action.ActionIconDatas[0].ActionIcons[0].V = 421;
				Action.ActionIconDatas[0].ActionIcons[0].UL = 35;
				Action.ActionIconDatas[0].ActionIcons[0].VL = 43;
			}
			// Anything previously shown as 'Y' is now 'StickClick'
			else if ( Action.ActionIconDatas[0].ActionIcons[0].U == 176 &&
				Action.ActionIconDatas[0].ActionIcons[0].V == 314 )
			{
				PRI = GearPRI(GPC.PlayerReplicationInfo);
				if (PRI != none &&
					(PRI.StickConfig == WSCO_SouthPaw || PRI.StickConfig == WSCO_Legacy))
				{
					if (Action.Actionname == 'LookAtSomething')
					{
						Action.IconAnimationSpeed=0.1f;
					}
					else
					{
						Action.IconAnimationSpeed=0.1f;
					}
					Action.ActionIconDatas[0].ActionIcons.length = 2;
					Action.ActionIconDatas[0].ActionIcons[0].U = 223;
					Action.ActionIconDatas[0].ActionIcons[0].V = 269;
					Action.ActionIconDatas[0].ActionIcons[0].UL = 35;
					Action.ActionIconDatas[0].ActionIcons[0].VL = 64;
					Action.ActionIconDatas[0].ActionIcons[1].U = 258;
					Action.ActionIconDatas[0].ActionIcons[1].V = 269;
					Action.ActionIconDatas[0].ActionIcons[1].UL = 35;
					Action.ActionIconDatas[0].ActionIcons[1].VL = 64;
					Action.ActionIconDatas[0].ActionIcons[1].Texture = Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons';
					Action.ActionIconDatas[0].ActionIcons[0].Texture = Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons';
				}
				else
				{
					if (Action.Actionname == 'LookAtSomething')
					{
						Action.IconAnimationSpeed=0.1f;
					}
					else
					{
						Action.IconAnimationSpeed=0.1f;
					}
					Action.ActionIconDatas[0].ActionIcons.length = 2;
					Action.ActionIconDatas[0].ActionIcons[0].U = 425;
					Action.ActionIconDatas[0].ActionIcons[0].V = 422;
					Action.ActionIconDatas[0].ActionIcons[0].UL = 35;
					Action.ActionIconDatas[0].ActionIcons[0].VL = 64;
					Action.ActionIconDatas[0].ActionIcons[1].U = 460;
					Action.ActionIconDatas[0].ActionIcons[1].V = 422;
					Action.ActionIconDatas[0].ActionIcons[1].UL = 36;
					Action.ActionIconDatas[0].ActionIcons[1].VL = 64;
					Action.ActionIconDatas[0].ActionIcons[1].Texture = Texture2D'Warfare_HUD.WarfareHUD_ActionIcons';
					Action.ActionIconDatas[0].ActionIcons[0].Texture = Texture2D'Warfare_HUD.WarfareHUD_ActionIcons';
					Action.IconAnimationSpeed=0.1f;
				}
			}
		}
	}
}

/**
 * Does the actual work of drawing an ActionInfo.
 */
final function DrawActionInfo(float DeltaTime, const out ActionInfo ActionToDraw)
{
	local int Alpha, Idx;
	local float DrawX, DrawY, TextW, TextH, StartY, SubTitleH, SubTitleW;
	local int IconIndex;
	local ActionInfo Action;

	Action = ActionToDraw;

	GetSubtitleSize( SubTitleW, SubTitleH );
	StartY = SafeZoneBottom + SubTitleH;

	Alpha = Action.BlendPct * 255.f;

	if (Alpha > 0)
	{
		// If there's a tooltip to draw, do it (this assumes that icons and tooltips won't mix)
		if (Len(Action.TooltipText) > 0)
		{
			Canvas.Font = class'Engine'.static.GetAdditionalFont(Action.FontIndex);
			Canvas.StrLen( Action.TooltipText, TextW, TextH );
			DrawX = SafeZoneFriendlyCenterX - TextW/2.f;
			DrawY = Canvas.ClipY - StartY - TextH;
			// draw the black shadow text
			Canvas.SetPos( DrawX+3, DrawY );
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			Canvas.DrawText( Action.TooltipText );
			// draw the text white
			Canvas.SetPos( DrawX, DrawY-3 );
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
			Canvas.DrawText( Action.TooltipText );
		}
		// else try to draw any icons
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );

			// See if we need to remap the button icon for alternate control scheme
			RemapButtonIcon( Action );

			if ( (Action.IconAnimationSpeed > 0.f) && (ActionIconAnimInfo.LastActionAnimationSwitch + Action.IconAnimationSpeed < WorldInfo.TimeSeconds) )
			{
				ActionIconAnimInfo.LastActionAnimationSwitch = WorldInfo.TimeSeconds;
				ActionIconAnimInfo.CurrentIconIndex = (ActionIconAnimInfo.CurrentIconIndex + 1) % Action.ActionIconDatas[0].ActionIcons.length;
			}

			if ( (GearPRI(PlayerOwner.PlayerReplicationInfo) == None) || GearPRI(PlayerOwner.PlayerReplicationInfo).bShowPictograms )
			{
				DrawX = SafeZoneFriendlyCenterX - Action.SizeX/2.f;
				// draw each icon
				for (Idx = 0; Idx < Action.ActionIconDatas.Length; Idx++)
				{
					IconIndex = ActionIconAnimInfo.CurrentIconIndex;
					if ( IconIndex >= Action.ActionIconDatas[Idx].ActionIcons.length )
					{
						IconIndex = 0;
					}

					DrawY = Canvas.ClipY - StartY - Action.SizeY;
					DrawY += Action.SizeY/2.f - Action.ActionIconDatas[Idx].ActionIcons[IconIndex].VL/2.f;

					// check to see if we need to mirror the last icon (ex. cover leans)
					if (Action.bMirror && Idx == Action.ActionIconDatas.Length - 1)
					{
						Canvas.SetPos( DrawX, DrawY );
						Canvas.DrawTile(Action.ActionIconDatas[Idx].ActionIcons[IconIndex].Texture,
							Action.ActionIconDatas[Idx].ActionIcons[IconIndex].UL,
							Action.ActionIconDatas[Idx].ActionIcons[IconIndex].VL,
							Action.ActionIconDatas[Idx].ActionIcons[IconIndex].U+Action.ActionIconDatas[Idx].ActionIcons[0].UL,
							Action.ActionIconDatas[Idx].ActionIcons[IconIndex].V,
							-Action.ActionIconDatas[Idx].ActionIcons[IconIndex].UL,
							Action.ActionIconDatas[Idx].ActionIcons[IconIndex].VL);
					}
					else
					{
						Canvas.DrawIcon(Action.ActionIconDatas[Idx].ActionIcons[IconIndex],DrawX,DrawY);
					}
					DrawX += Action.ActionIconDatas[Idx].ActionIcons[IconIndex].UL + Action.IconSpacing;
				}
			}
		}
	}
}

/**
 * Draw the current action.
 */
final function DrawActions(float DeltaTime)
{
	// FIXME: Not sure if this is the best avenue to go.
	if ( !PlayerOwner.IsSpectating() )
	{
		// if we have an active action
		if (ActiveAction.bActive)
		{
			// check for any auto deactivates
			if (FALSE && ActiveAction.AutoDeActivateTimer > 0.f && WorldInfo.TimeSeconds - ActiveAction.ActivateTime > ActiveAction.AutoDeActivateTimer)
			{
				// disable this action
				ActiveAction.bActive = FALSE;
				PreviousAction = ActiveAction;
			}
			// otherwise let's draw the sucker
			else
			{
				// figure out the blend pct
				ActiveAction.BlendPct = FClamp(ActiveAction.BlendPct + (1.f/ActionFadeTime * DeltaTime),0.f,1.f);
				// and draw
				DrawActionInfo(DeltaTime,ActiveAction);
			}
		}
		// check the previous action to see if it's still blending out
		if (PreviousAction.BlendPct > 0.f)
		{
			PreviousAction.BlendPct = FClamp(PreviousAction.BlendPct - (1.f/ActionFadeTime * DeltaTime),0.f,1.f);
			DrawActionInfo(DeltaTime,PreviousAction);
		}
	}
}


function DrawMPStatus(GearPC PC)
{
	local string Text;

	Canvas.Font = class'Engine'.Static.GetSmallFont();
	SetHUDDrawColor( eWARHUDCOLOR_WHITE );
	if (PC.IsSpectating())
	{
		Text = "Press fire to spawn...";
	}
	// if any text was set, then draw it
	if (Text != "")
	{
		Canvas.SetPos(SafeZoneLeft, Canvas.ClipY - SafeZoneBottom);
		Canvas.DrawText(Text,FALSE);
	}
}

/**
 * Delegate hook for anything that wants a debug draw.
 *
 * @see: DebugDraws, DrawHUD()
 */
delegate DebugDraw(GearHUD_Base H)
{
}

/**
 * Handy function to add/remove debug draw functions.
 */
function SetDebugDraw(delegate<DebugDraw> DebugDrawFunc, bool bAdd)
{
	local int Idx;
	Idx = DebugDrawList.Find(DebugDrawFunc);
	if (bAdd && Idx == -1)
	{
		DebugDrawList[DebugDrawList.Length] = DebugDrawFunc;
	}
	else
	if (!bAdd && Idx != -1)
	{
		DebugDrawList.Remove(Idx,1);
	}
}

function ToggleDebugDraw(delegate<DebugDraw> DebugDrawFunc)
{
	local int Idx;
	Idx = DebugDrawList.Find(DebugDrawFunc);
	if (Idx != INDEX_None)
	{
		`log("Removing debug draw function");
		DebugDrawList.Remove(Idx,1);
	}
	else
	{
		`log("Adding debug draw function");
		DebugDrawList.AddItem(DebugDrawFunc);
	}
}

function bool IsDebugDrawActive(delegate<DebugDraw> DebugDrawFunc)
{
	return DebugDrawList.Find(DebugDrawFunc) != INDEX_None;
}

/**
 * Special HUD for Engine demo, overridden to prevent copyright info, etc.
 */
function DrawEngineHUD();

/** Start a color Fade */
simulated function ColorFade( Color NewFadeColor, byte FromAlpha, byte ToAlpha, float FadeTime )
{
	// set the new fade color/alpha
	PreviousFadeAlpha	= FromAlpha;
	FadeAlpha = FromAlpha;
	DesiredFadeAlpha	= ToAlpha;
	FadeColor			= NewFadeColor;

	// set the new fade time
	DesiredFadeAlphaTime	= FadeTime;
	FadeAlphaTime			= 0.f;
}

/**
 * Overridden to handle interpolating current fade if any.
 */
function Tick(float DeltaTime)
{
	if (FadeAlphaTime != DesiredFadeAlphaTime)
	{
		if (FadeAlphaDelay > 0.f)
		{
			FadeAlphaDelay -= DeltaTime;
		}
		else
		{
			if (FadeAlphaTime > DesiredFadeAlphaTime)
			{
				FadeAlphaTime = FMax(FadeAlphaTime - DeltaTime, DesiredFadeAlphaTime);
			}
			else
			{
				FadeAlphaTime = FMin(FadeAlphaTime + DeltaTime, DesiredFadeAlphaTime);
			}
		}
		FadeAlpha = lerp( PreviousFadeAlpha, DesiredFadeAlpha, FadeAlphaTime/DesiredFadeAlphaTime );
	}
}


/**
 * This will fade in the camera from black.  The idea is to hide the occlusion query flickers
 * when you zip around the level fast like.
 **/
function SpectatorCameraFadeIn(optional float FadeInTime = 0.25)
{
	// don't mess with any current fading in co-op as it may be script controlled
	if (FadeAlphaTime == DesiredFadeAlphaTime || GearGRI(WorldInfo.GRI) == None || !GearGRI(WorldInfo.GRI).bIsCoop)
	{
		FadeColor = MakeColor(0,0,0,255);
		// if no active fade then start from black
		if (FadeAlpha == 0)
		{
			FadeAlpha = 255;
		}
		PreviousFadeAlpha = FadeAlpha;
		DesiredFadeAlpha = 0;
		FadeAlphaDelay = 0.025f;
		DesiredFadeAlphaTime = FadeInTime;
		FadeAlphaTime = 0.0f;
	}
}

function SpectatorCameraFadeOut(optional bool bAutoFadeIn, optional float FadeInTime = 0.3f)
{
	// don't mess with any current fading in co-op as it may be script controlled
	if (FadeAlphaTime == DesiredFadeAlphaTime || GearGRI(WorldInfo.GRI) == None || !GearGRI(WorldInfo.GRI).bIsCoop)
	{
		ColorFade(MakeColor(0,0,0,255),0,255,FadeInTime - 0.1f);
		if (bAutoFadeIn)
		{
			SetTimer( FadeInTime,FALSE,nameof(SpectatorCameraFadeIn) );
		}
	}
}

final function DrawDeadInfo(float AlphaPct)
{
	local int Idx;
	// not for cinematics
	if (PlayerOwner.bCinematicMode)
	{
		return;
	}
	// draw all the icons
	for (Idx = 0; Idx < HealthIcons.Length; Idx++)
	{
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, 255 * AlphaPct );
		Canvas.DrawIcon(HealthIcons[Idx],SafeZoneFriendlyCenterX - 0.5*HealthIcons[Idx].UL + HealthIconsXOffset[Idx],SafeZoneFriendlyCenterY - 0.5*HealthIcons[Idx].VL);
	}
}

/** Draw the omen to indicate health */
final function DrawHealth(int CurrentHealth, int DefaultHealth, bool bIsDBNO)
{
	local float BlendPct, HealthPct, AlphaPct;
	local int HealthIdx, Idx;
	// not for cinematics
	if (PlayerOwner.bCinematicMode)
	{
		return;
	}
	// draw the full omen if DBNO
	if (bIsDBNO)
	{
		DrawDeadInfo(0.5);
	}
	else
	// if we aren't dead
	if ( CurrentHealth != DefaultHealth )
	{
		// draw up to the icon depending on last shot and health
		//@fixme - try drawing all the time for now, need to sort out this design issue
		//ShotAtTime = WorldInfo.TimeSeconds - WP.LastShotAtTime;
		//if (WP.Health <= 0 || ShotAtTime < 4.f)
		//{
			// figure out current health pct
			HealthPct = FMin(CurrentHealth/float(DefaultHealth), 1.f);
			// max blend percent based on health
			BlendPct = FMax(1.f - HealthPct,0.35f);
			HealthIdx = Min(int((1.f - HealthPct) * /*HealthIcons.Length*/ 3.f),HealthIcons.Length - 1);
			// draw the current at full alpha
			AlphaPct = ((/* 1/HealthIcons.Length */0.333 * (3 - HealthIdx)) - HealthPct) * /*HealthIcons.Length*/3.0;
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, 255 * BlendPct * AlphaPct );
			Canvas.DrawIcon(HealthIcons[HealthIdx],SafeZoneFriendlyCenterX - 0.5*HealthIcons[HealthIdx].UL + HealthIconsXOffset[HealthIdx],SafeZoneFriendlyCenterY - 0.5*HealthIcons[HealthIdx].VL);
			// if not drawing at full
			if (AlphaPct < 1.f)
			{
				// try to draw the previous ones at full
				Idx = HealthIdx;
				while (Idx > 0)
				{
					Idx--;
					SetHUDDrawColor( eWARHUDCOLOR_WHITE, 255 * BlendPct );
					Canvas.DrawIcon(HealthIcons[Idx],SafeZoneFriendlyCenterX - 0.5*HealthIcons[Idx].UL + HealthIconsXOffset[Idx],SafeZoneFriendlyCenterY - 0.5*HealthIcons[Idx].VL);
				}
			}
		//}
	}
}

simulated function TargetingModeChanged( bool bIsTargetting )
{
	bIsFadingCrosshairIn = bIsTargetting && ShouldDrawCrosshair(GearPawn(PlayerOwner.Pawn));
	if ( bIsFadingCrosshairIn )
	{
		CrosshairFadeStartTime = WorldInfo.TimeSeconds - (TotalCrosshairFadeTime * CrosshairFadeOpacity);
	}
	else
	{
		CrosshairFadeStartTime = WorldInfo.TimeSeconds - (TotalCrosshairFadeTime * (1.f - CrosshairFadeOpacity));
	}
}

final function bool ShouldDrawCrosshair(GearPawn GP, optional out GearWeapon Weap)
{
	local GearVehicle Vehicle;
	local GearWeaponPawn WeapPawn;
	local GearTurret TurretPawn;

	if (!PlayerOwner.bCinematicMode)
	{
		// in a vehicle?
		Vehicle = GearVehicle(PlayerOwner.Pawn);
		if (Vehicle != None)
		{
			Weap = Vehicle.Seats[0].Gun;
			return Vehicle.WantsCrosshair(PlayerOwner);
		}

		// in a passenger seat?
		WeapPawn = GearWeaponPawn(PlayerOwner.Pawn);
		if(WeapPawn != None)
		{
			Weap = WeapPawn.MyVehicleWeapon;
			return WeapPawn.MyVehicle.WantsCrosshair(PlayerOwner);
		}

		// on a turret?
		TurretPawn = GearTurret(PlayerOwner.Pawn);
		if (TurretPawn != None)
		{
			Weap = GearWeapon(TurretPawn.Weapon);
			return TurretPawn.WantsCrosshair(PlayerOwner);
		}

		// normal gearpawn walking around?
		if (GP != None && GearWeapon(GP.Weapon) != None)
		{
			Weap = GearWeapon(GP.Weapon);
			return Weap.ShouldDrawCrosshair() && GP.CanFireWeapon() && !GP.ShouldDelayFiring();
		}
	}
	return FALSE;
}

final function DrawWeaponCrosshair(GearPC PC, GearPawn WP)
{
	local CanvasIcon CrosshairIcon;
	local float DrawX, DrawY, Scale, MortarTickPercent, YOffset;
	local GearWeapon Weap;
	local GearWeap_SniperRifle SniperGun;
	local bool bShouldDrawCrosshair;
	local GearWeap_HeavyMortarBase MortarWeap;

	bShouldDrawCrosshair = ShouldDrawCrosshair(PC.MyGearPawn,Weap);

	// Determine what the opacity of the crosshair should be.
	if ( bIsFadingCrosshairIn || bShouldDrawCrosshair )
	{
		if ( (CrosshairFadeStartTime > 0.f) && ((WorldInfo.TimeSeconds - CrosshairFadeStartTime) < TotalCrosshairFadeTime) )
		{
			CrosshairFadeOpacity = (WorldInfo.TimeSeconds - CrosshairFadeStartTime) / TotalCrosshairFadeTime;
		}
		else
		{
			CrosshairFadeOpacity = 1.f;
			bIsFadingCrosshairIn = FALSE;
		}
	}
	else if ( !bIsFadingCrosshairIn && ((WorldInfo.TimeSeconds - CrosshairFadeStartTime) < TotalCrosshairFadeTime) && (CrosshairFadeStartTime > 0.f) )
	{
		CrosshairFadeOpacity = 1.f - ((WorldInfo.TimeSeconds - CrosshairFadeStartTime) / TotalCrosshairFadeTime);
	}
	else
	{
		CrosshairFadeOpacity = 0.f;
	}

	if ( CrosshairFadeOpacity > 0.f )
	{
		// no red crosshair for mortar because you can't really hit enemies you're pointing at
		if (bUnfriendlySpotted && GearWeap_HeavyMortarBase(Weap) == None)
		{
			SetHUDDrawColor( eWARHUDCOLOR_RED, int(255.f*CrosshairFadeOpacity) );
		}
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, int(255.f*CrosshairFadeOpacity) );
		}

		// sniper overlay
		SniperGun = GearWeap_SniperRifle(Weap);
		if (SniperGun != None && WP.bIsZoomed && !PC.IsSpectating())
		{
			if( SniperOverlayMaterialConstant == None )
			{
				SniperOverlayMaterialConstant = new(outer) class'MaterialInstanceConstant';
				SniperOverlayMaterialConstant.SetParent( SniperOverlayMaterial );
			}

			// check for team here
			if( GearPawn_LocustBase(WP) == None )
			{
				SniperOverlayMaterialConstant.SetVectorParameterValue( 'SniperScopeColor', MakeLinearColor( 0.1f, 0.286f, 1.1f, 1.0f) ); // blue
			}
			else
			{
				SniperOverlayMaterialConstant.SetVectorParameterValue( 'SniperScopeColor', MakeLinearColor( 1.1f, 0.286f, 0.1f, 1.0f) ); // red
			}

			SetSniperAspectRatio( SniperOverlayMaterialConstant );

			Canvas.SetPos( 0, 0 );
			Canvas.DrawMaterialTile( SniperOverlayMaterialConstant, Canvas.ClipX, Canvas.ClipY );
		}

		// crosshair
		YOffset = 0.0;
		CrosshairIcon = Canvas.MakeIcon(Weap.GetCrosshairIcon(PC, YOffset));
		Scale = Weap.GetCrossHairScale();
		DrawX = 0.5*Canvas.ClipX - (CrosshairIcon.UL * 0.5f * Scale);
		DrawY = (0.5 - YOffset)*Canvas.ClipY - (CrosshairIcon.VL * 0.5f * Scale);
		Canvas.DrawIcon(CrosshairIcon,DrawX,DrawY,Scale);

		// draw the lame 'x' if aiming at a friendly
		if (!bUnfriendlySpotted && ActiveAction.bActive && ActiveAction.ActionType == AT_Player)
		{
			Canvas.SetDrawColor(180,180,180,180);
			Canvas.SetPos(Canvas.ClipX/2 - Texture2D'Warfare_HUD.HUD_FriendlyXHair'.GetSurfaceWidth()/2 - 1,Canvas.ClipY/2 - Texture2D'Warfare_HUD.HUD_FriendlyXHair'.GetSurfaceHeight()/2);
			Canvas.DrawTexture(Texture2D'Warfare_HUD.HUD_FriendlyXHair',1.0);
		}

		MortarWeap = GearWeap_HeavyMortarBase(Weap);
		if ( MortarWeap != None )
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, int(255.f*CrosshairFadeOpacity) );

			MortarTickPercent = MortarWeap.GetSlottedElevationPct(MortarWeap.LastLastElevationPct);
			if ( MortarTickPercent > 0.0f )
			{
				Canvas.DrawIcon( MortarCrosshairSaveIcon, DrawX, DrawY+86*Scale-MortarTickPercent*56*Scale, Scale );
			}

			MortarTickPercent = MortarWeap.GetSlottedElevationPct(MortarWeap.CurrentElevationPct);
			Canvas.DrawIcon( MortarCrosshairTickIcon, DrawX, DrawY+86*Scale-MortarTickPercent*56*Scale, Scale );

			if (LastMortarTickPercent < MortarTickPercent)
			{
				LastMortarTickPercent = FInterpTo(LastMortarTickPercent,MortarTickPercent,RenderDelta,16.f);
			}
			else
			{
				LastMortarTickPercent = MortarTickPercent;
			}
			Canvas.SetPos(DrawX + 90.f,DrawY + 60.f);
			Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Chrom20);
			Canvas.DrawText((50 + int(LastMortarTickPercent * 100))$"m",FALSE);
		}
	}

	SetHUDDrawColor( eWARHUDCOLOR_WHITE );
}

function DrawOvertime()
{
	StartDrawingDramaticText( OverTimeString, "", 8.0f, 0.25f, 2.0f, GetCountdownYPosition() + GetCountdownHeight() + GearPC(PlayerOwner).NumPixelsToMoveDownForDramaText(), 0.25f, 2.0f );
}

/** This will set the correct Material Paramters on the Overlay to make the snper work in 9:6 and 4:3 SP/SplitScreen **/
final native function SetSniperAspectRatio( MaterialInstanceConstant SniperOverlay );


/**
 * Adds a new hit locator entry for drawing in DrawHitLocationIndicator().
 */
final function AddNewHitLocatorData( Actor InstigatorA, int DamageReceived, optional vector HitLocation, optional bool bDirLocked )
{
	local GearPawn GP;
	local GearVehicle GVehicle;
	GP = ResolvePawn(,GVehicle);
	if (GP != None || GVehicle != None)
	{
		// wrap around the list if necessary
		if( ++CurrHitLocatorDataIndex >= MAX_HIT_LOCATORS )
		{
			CurrHitLocatorDataIndex = 0;
		}
		HitLocatorList[CurrHitLocatorDataIndex].ActorWhoShotMe = InstigatorA;
		HitLocatorList[CurrHitLocatorDataIndex].HitLocation = HitLocation;
		if (bDirLocked)
		{
			HitLocatorList[CurrHitLocatorDataIndex].DirOfHit = CalculateLocationMaterialEffect( HitLocatorList[CurrHitLocatorDataIndex].HitLocation );
		}
		HitLocatorList[CurrHitLocatorDataIndex].bDirLocked = bDirLocked;
		HitLocatorList[CurrHitLocatorDataIndex].HitLocationFadePercent = 1.0f;
		// initialize the material if needed
		if( HitLocatorList[CurrHitLocatorDataIndex].MILocation == None )
		{
			HitLocatorList[CurrHitLocatorDataIndex].MILocation = new(outer) class'MaterialInstanceConstant';
			HitLocatorList[CurrHitLocatorDataIndex].MILocation.SetParent( HitLocatorMaterial );
		}
		// extra warning based on the amount of damage done
		if (GP != None)
		{
			HitLocatorList[CurrHitLocatorDataIndex].MILocation.SetScalarParameterValue( 'HUD_DamageDir_Warning', DamageReceived/(GP.DefaultHealth * 0.5f) );
		}
	}
}

/** Clear the hit indicators */
final function ClearHitLocatorData()
{
	local int Idx;

	for ( Idx = 0; Idx < MAX_HIT_LOCATORS; Idx++ )
	{
		HitLocatorList[Idx].HitLocationFadePercent = 0.0f;
	}
}


/**
 * This will determine which value to update the hit locater material with
 **/
final function float CalculateLocationMaterialEffect( vector LocationToTest )
{
	local Vector Loc;
	local Rotator Rot;
	local float DirOfHit_L;

	local vector AxisX, AxisY, AxisZ;

	local vector ShotDirection;

	local bool bIsInFront;
	local vector2D	AngularDist;
	local Pawn P;
	local GearVehicle GVehicle;

	local float Multiplier;
	local float PositionInQuadrant;
	local float QuadrantStart;


	P = ResolvePawn(,GVehicle);
	if (GVehicle != None)
	{
		PlayerOwner.GetPlayerViewPoint( Loc, Rot );
	}
	// using the camera rotation location
	else if( PlayerOwner != None )
	{
		PlayerOwner.GetPlayerViewPoint( Loc, Rot );
	}
	else if( P != None )
	{
		Loc = P.Location;
		Rot = P.Rotation;
	}
	// we have nothing so return straight up
	else
	{
		return 0.0f;
	}

	GetAxes(Rot, AxisX, AxisY, AxisZ);
	ShotDirection = -1.f * Normal(Loc - LocationToTest);

	bIsInFront = GetAngularDistance( AngularDist, ShotDirection, AxisX, AxisY, AxisZ );
	GetAngularDegreesFromRadians( AngularDist );

	Multiplier = 0.25f / 90.f;

	PositionInQuadrant = Abs(AngularDist.X) * Multiplier;

	//`log( "bIsInFront: " $ bIsInFront @ "AngularDist: " $ AngularDist.X @ "PositionInQuadrant" @ PositionInQuadrant @ "Abs(AngularDist.X)" @ Abs(AngularDist.X) @ "Multiplier" @ Multiplier );

	// 0 - .25  UpperRight
	// .25 - .50 LowerRight
	// .50 - .75 LowerLeft
	// .75 - 1 UpperLeft
	if( bIsInFront == TRUE )
	{
	   if( AngularDist.X > 0 )
	   {
		   QuadrantStart = 0;
		   DirOfHit_L = QuadrantStart + PositionInQuadrant;
	   }
	   else
	   {
		   QuadrantStart = 0;
		   DirOfHit_L = QuadrantStart - PositionInQuadrant;
	   }
	}
	else
	{
		if( AngularDist.X > 0 )
		{
		   QuadrantStart = 0.50;
		   DirOfHit_L = QuadrantStart - PositionInQuadrant;
		}
	   else
	   {
		   QuadrantStart = 0.50;
		   DirOfHit_L = QuadrantStart + PositionInQuadrant;
	   }
	}
	//DirOfHit = QuadrantStart;

	DirOfHit_L = -1 * DirOfHit_L;

	//`log( "DirOfHit_L" @ DirOfHit_L @ "QuadrantStart" @ QuadrantStart @ "PositionInQuadrant" @ PositionInQuadrant @ "Abs(AngularDist.X)" @ Abs(AngularDist.X) @ "Multiplier" @ Multiplier );

	return DirOfHit_L;
}

function float CalculateRevivePercentAlive( GearPawn AGearPawn )
{
	return FClamp(1.0 - ((WorldInfo.TimeSeconds - AGearPawn.TimeOfDBNO) / GearGRI(WorldInfo.GRI).InitialRevivalTime), 0.0, 1.0);
}

simulated function SetBleedOutEffectSettings( float TotalTime )
{
	BleedOutPPSettings.Bloom_InterpolationDuration = TotalTime;
	BleedOutPPSettings.DOF_InterpolationDuration = TotalTime;
	BleedOutPPSettings.Scene_InterpolationDuration = TotalTime;

	BleedOutPPSettings.bEnableBloom = TRUE;
	BleedOutPPSettings.bEnableDOF = TRUE;
	BleedOutPPSettings.bEnableSceneEffect = TRUE;

	BleedOutPPSettings.Bloom_Scale = 1.0f;

	BleedOutPPSettings.DOF_BlurKernelSize = 5.0f;
	BleedOutPPSettings.DOF_FalloffExponent = 1.0f;
	BleedOutPPSettings.DOF_FocusDistance = 0.0f;
	BleedOutPPSettings.DOF_FocusInnerRadius = 0.0f;
	BleedOutPPSettings.DOF_FocusPosition = vect(0,0,0);
	BleedOutPPSettings.DOF_FocusType = FOCUS_Distance;
	BleedOutPPSettings.DOF_MaxFarBlurAmount = 0.7f;
	BleedOutPPSettings.DOF_MaxNearBlurAmount = 0.7f;

	BleedOutPPSettings.Scene_Desaturation = 0.0f;
	BleedOutPPSettings.Scene_HighLights = vect( 0.5f, 1.0f, 1.0f );
	BleedOutPPSettings.Scene_MidTones = vect( 1.f, 3.5f, 3.5f );
	BleedOutPPSettings.Scene_Shadows = vect( 0.005f, 0.005f, 0.005f );
}

simulated function UpdateBleedOutEffect()
{
	local GearPC PC;
	local GearPawn WP;
	local float NewTime;

	PC = GearPC(PlayerOwner);
	WP = GearPawn(PC.Pawn);

	if ( WP != None )
	{
		NewTime = WorldInfo.TimeSeconds - WP.TimeOfDBNO;
		BleedOutPPSettings.Bloom_InterpolationDuration = NewTime;
		BleedOutPPSettings.DOF_InterpolationDuration = NewTime;
		BleedOutPPSettings.Scene_InterpolationDuration = NewTime;
	}

	LocalPlayer(PlayerOwner.Player).UpdateOverridePostProcessSettings( BleedOutPPSettings );
}

simulated function SetTaccomPostProcessSettings()
{
	TaccomPPSettings = LocalPlayer(PlayerOwner.Player).CurrentPPInfo.LastSettings;

	TaccomPPSettings.bEnableBloom = TRUE;
	TaccomPPSettings.bEnableSceneEffect = TRUE;

	TaccomPPSettings.Bloom_InterpolationDuration = 0.25f;
	TaccomPPSettings.Bloom_Scale = 0.5f;

	TaccomPPSettings.Scene_Desaturation = 0.85f;
	TaccomPPSettings.Scene_InterpolationDuration = 0.25f;
}

final function DrawHitLocationIndicator()
{
	local float DrawX, DrawY;
	local int Index;
	// hitlocator material size is hard-coded to 256x256
	DrawX = CenterX - 128;
	DrawY = CenterY - 128;
	for( Index = 0; Index < MAX_HIT_LOCATORS; ++Index )
	{
		if( HitLocatorList[Index].HitLocationFadePercent > 0.f )
		{
			// calculate the rotation based on the hit location
			if ( !HitLocatorList[Index].bDirLocked )
			{
				if ( !IsZero(HitLocatorList[Index].HitLocation) )
				{
					HitLocatorList[Index].DirOfHit = CalculateLocationMaterialEffect( HitLocatorList[Index].HitLocation );
				}
				// calculate the rotation based on the current player orientation versus the actor who hit us
				else if ( HitLocatorList[Index].ActorWhoShotMe != None )
				{
					HitLocatorList[Index].DirOfHit = CalculateLocationMaterialEffect( HitLocatorList[Index].ActorWhoShotMe.Location );
				}
			}
			HitLocatorList[Index].MILocation.SetVectorParameterValue( 'HUD_Damage_Rotator', MakeLinearColor(HitLocatorList[Index].DirOfHit, 0.0f, 0.0f, 1.0) );
			HitLocatorList[Index].HitLocationFadePercent -= RenderDelta; //0.05;  // 5%
			HitLocatorList[Index].MILocation.SetVectorParameterValue( 'Hit_Fade', MakeLinearColor(HitLocatorList[Index].HitLocationFadePercent, 0.0f, 0.0f, 1.0) );
			Canvas.SetPos(DrawX,DrawY);
			Canvas.DrawMaterialTile(HitLocatorList[Index].MILocation, 256, 256, 0, 0, 1.f, 1.f);
		}
	}
}

final function DrawAmmo( GearWeapon Weap, int Alpha, float AmmoPerIcon, out float RemainingAmmo, out float DrawX, out float DrawY, float Scale, bool bHighlight )
{
	local CanvasIcon AmmoIcon;

	if ( RemainingAmmo > 0 )
	{
		AmmoIcon = Weap.HUDDrawData.AmmoIcon;
		Canvas.SetPos( DrawX, DrawY );
		if ( bHighlight )
		{
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
		}
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		}
		Canvas.DrawTile( AmmoIcon.Texture, ((RemainingAmmo-Weap.ActiveReload_NumBonusShots)/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo()*Scale, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, ((RemainingAmmo-Weap.ActiveReload_NumBonusShots)/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo(), AmmoIcon.VL );

		if ( Weap.IsReloading() )
		{
			DrawX += (RemainingAmmo/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo()*Scale;
			RemainingAmmo -= RemainingAmmo-Weap.ActiveReload_NumBonusShots;
		}
		else
		{
			DrawX += ((RemainingAmmo-Weap.ActiveReload_NumBonusShots)/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo()*Scale;
			RemainingAmmo -= RemainingAmmo-Weap.ActiveReload_NumBonusShots;
		}
	}
}

final function DrawSuperAmmo( GearWeapon Weap, int Alpha, float AmmoPerIcon, out float RemainingAmmo, out float DrawX, out float DrawY, float Scale, bool bHighlite )
{
	local CanvasIcon AmmoIcon;

	if ( Weap.ActiveReload_NumBonusShots > 0 )
	{
		AmmoIcon = Weap.HUDDrawDataSuper.AmmoIcon;

		// scalar for the coordinates of the panning texture, lower value enlarges, defaulted to 1.5
		//ActiveReloadPulseMaterialConstant.SetScalarParameterValue( 'HUD_Element_TScale',  );

		// brightness of the pulse
		//ActiveReloadPulseMaterialConstant.SetScalarParameterValue( 'HUD_Element_TValue',  );

		// color of the material and the alpha
		ActiveReloadPulseMaterialConstant.SetVectorParameterValue( 'HUD_Elementcolor', MakeLinearColor(0.25f, 0.25f, 0.25f, 1.0f*(Alpha/255.f)) );

		if ( bHighlite )
		{
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
		}
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		}
		Canvas.SetPos( DrawX, DrawY ) ;
		Canvas.DrawMaterialTile( ActiveReloadPulseMaterialConstant, (Weap.ActiveReload_NumBonusShots/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo()*Scale, AmmoIcon.VL*Scale, AmmoIcon.U/512, AmmoIcon.V/512, ((Weap.ActiveReload_NumBonusShots/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo())/512, AmmoIcon.VL/512 );
		DrawX += (Weap.ActiveReload_NumBonusShots/AmmoPerIcon)*Weap.GetPixelWidthOfAmmo()*Scale;
		RemainingAmmo -= Weap.ActiveReload_NumBonusShots;
	}
}

final function DrawEmptyAmmo( GearWeapon Weap, int Alpha, float DrawX, float DrawY, float Scale )
{
	local CanvasIcon AmmoIcon;

	AmmoIcon = Weap.HUDDrawData.AmmoIcon;
	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.SetDrawColor(47,47,47,Alpha);
	Canvas.DrawTile( AmmoIcon.Texture, AmmoIcon.UL*Scale, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, AmmoIcon.UL, AmmoIcon.VL );
}

final function DrawWeaponIcon( GearWeapon Weap, float DrawX, float DrawY, int Alpha, float Scale )
{
	local float FlashValue;

	if ( Weap.IsReloading() && (ARBarData.ARResult == eARResult_SuperSuccess) )
	{
		FlashValue = (WorldInfo.TimeSeconds % 0.25f) / 0.25f;
	}
	else
	{
		FlashValue = 1.f;
	}

	SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha*FlashValue );
	Canvas.DrawIcon(Weap.WeaponIcon,DrawX,DrawY,0.85f*Scale);
}

final function SetWeaponSelectColor( int Alpha )
{
	local float MaxWhite, MinWhite, CurrWhite, CurrTime, IntervalTime, CurrBlinkTime;
	local int CurrInterval;

	MaxWhite = 255;
	MinWhite = 200;
	CurrTime = WorldInfo.TimeSeconds;

	if ( CurrTime >= WeaponSelect.StartBlinkTime + WeaponSelect.TotalBlinkTime )
	{
		Canvas.SetDrawColor( MinWhite, MinWhite, MinWhite, Alpha );
	}
	else
	{
		IntervalTime = WeaponSelect.TotalBlinkTime/(WeaponSelect.TotalNumBlinks*2);
		CurrBlinkTime = CurrTime - WeaponSelect.StartBlinkTime;
		CurrInterval = int(CurrBlinkTime/IntervalTime);
		// Going from Max to Min
		if ( CurrInterval%2 == 0 )
		{
			CurrWhite = lerp( MaxWhite, MinWhite, (CurrBlinkTime-CurrInterval*IntervalTime)/IntervalTime );
			Canvas.SetDrawColor( CurrWhite, CurrWhite, CurrWhite, Alpha );
		}
		else
		{
			CurrWhite = lerp( MinWhite, MaxWhite, (CurrBlinkTime-CurrInterval*IntervalTime)/IntervalTime );
			Canvas.SetDrawColor( CurrWhite, CurrWhite, CurrWhite, Alpha );
		}
	}
}

final function ClearWeaponSelectData()
{
	WeaponSelect.StartBlinkTime = WorldInfo.TimeSeconds;
}

/**
 * Called when an AR is successful
 */
simulated function ActiveReloadSuccess( bool bDidSuperSweetReload )
{
	local GearPC PC;

	if ( bDidSuperSweetReload )
	{
		ARBarData.ARResult = eARResult_SuperSuccess;
		// Update the achievement for active reload
		PC = GearPC(PlayerOwner);
		if (PC != none && PC.ProfileSettings != none)
		{
			PC.ProfileSettings.UpdateAchievementProgression(eGA_ActiveReload, PC);
		}
	}
	else
	{
		ARBarData.ARResult = eARResult_Success;
	}
}

/**
 * Called when an AR is unsuccessful
 */
simulated function ActiveReloadFail( bool bFailedActiveReload )
{
	ARBarData.ARResult = eARResult_Failed;
}

/**
 * Resets the active reload data
 */
simulated function ResetActiveReload()
{
	ARBarData.ARResult = eARResult_None;
	ARBarData.TickMarkTime = WorldInfo.TimeSeconds;
}

final function DrawActionReloadBar( GearWeapon Weap, float DrawX, float DrawY, int Alpha, float Scale )
{
	local float ActiveReloadStartTime, PreReactionWindowDuration, SuperSweetSpotDuration, SweetSpotDuration;
	local int StartSuperSweetBox, EndSuperSweetBox, StartSweetBox, EndSweetBox, StopLocationX;
	local float BarLength, FadeAlphaPercent, FlashAlphaPercent, ARTimeLeft;
	local EGearHUDColor eARColor;

	// get the ar values
	Weap.GetActiveReloadValues(ActiveReloadStartTime,PreReactionWindowDuration,SuperSweetSpotDuration,SweetSpotDuration);

	// if the player failed everything is colored red, else it's white as normal
	if ( ARBarData.ARResult != eARResult_Failed )
	{
		eARColor = eWARHUDCOLOR_WHITE;
	}
	else
	{
		eARColor = eWARHUDCOLOR_RED;
	}

	// calc the time left in the AR
	ARTimeLeft = Weap.ReloadStartTime + Weap.ReloadDuration - WorldInfo.TimeSeconds;
	if ( ARTimeLeft < ARBarData.FadeDuration )
	{
		FadeAlphaPercent = ARTimeLeft / ARBarData.FadeDuration;
		FlashAlphaPercent = FadeAlphaPercent;
	}
	else
	{
		FadeAlphaPercent = 1.f;
		FlashAlphaPercent = FadeAlphaPercent;
	}

	// AR not attempted yet
	if ( !Weap.bActiveReloadAttempted )
	{
		// update the tick time
		ARBarData.TickMarkTime = WorldInfo.TimeSeconds - Weap.ReloadStartTime;

		// let's be trixy and speed up the tickmark until we hit the supersweetspot
		if ( ARBarData.TickMarkTime < Weap.AR_PossibleSuccessStartPoint )
		{
			ARBarData.TickMarkTime = ((ARBarData.TickMarkTime ** 4.f) / (Weap.AR_PossibleSuccessStartPoint ** 4.f)) * Weap.AR_PossibleSuccessStartPoint;
		}
	}
	// AR has been attempted
	else
	{
		// recalc time left
		if ( ARTimeLeft > ARBarData.FlashDuration - (WorldInfo.TimeSeconds - Weap.ReloadStartTime - ARBarData.TickMarkTime) )
		{
			ARTimeLeft = ARBarData.FlashDuration - (WorldInfo.TimeSeconds - Weap.ReloadStartTime - ARBarData.TickMarkTime);
		}

		// calc the alpha on the flash
		FlashAlphaPercent = ARTimeLeft / ARBarData.FlashDuration;
		FlashAlphaPercent = fmax( FlashAlphaPercent, 0.f );
		FlashAlphaPercent = fmin( FlashAlphaPercent, 1.f );

		FadeAlphaPercent = (ARBarData.FadeDuration - (WorldInfo.TimeSeconds - Weap.ReloadStartTime - ARBarData.TickMarkTime)) / ARBarData.FadeDuration;
		FadeAlphaPercent = fmax( FadeAlphaPercent, 0.f );
		FadeAlphaPercent = fmin( FadeAlphaPercent, 1.f );

		if ( FlashAlphaPercent < FadeAlphaPercent )
		{
			FadeAlphaPercent = FlashAlphaPercent;
		}
	}

	// early out if we alpha out
	if ( (FadeAlphaPercent <= 0.f) && (FlashAlphaPercent < 0.f) )
	{
		return;
	}

	BarLength = ARBarData.BackGround.UL * Scale;


	// draw background
	SetHUDDrawColor( eARColor, byte(Alpha*FadeAlphaPercent) );
	Canvas.SetPos( DrawX, DrawY );
	Canvas.DrawTile( ARBarData.BackGround.Texture, BarLength, ARBarData.BackGround.VL, ARBarData.BackGround.U, ARBarData.BackGround.V, ARBarData.BackGround.UL, ARBarData.BackGround.VL );

	// everything below is centered on the background
	DrawY += ARBarData.BackGround.VL * 0.5;

	// draw super sweet box
	StartSuperSweetBox = int(ActiveReloadStartTime / Weap.ReloadDuration * BarLength);
	EndSuperSweetBox = int((ActiveReloadStartTime + SuperSweetSpotDuration) / Weap.ReloadDuration * BarLength);
	SetHUDDrawColor( eARColor, byte(FlashAlphaPercent * 255.f) );
	Canvas.SetPos( DrawX + StartSuperSweetBox, DrawY - (ARBarData.PerfectGrade.VL * 0.5) );
	Canvas.DrawTile( ARBarData.PerfectGrade.Texture, EndSuperSweetBox-StartSuperSweetBox, ARBarData.PerfectGrade.VL, ARBarData.PerfectGrade.U, ARBarData.PerfectGrade.V, ARBarData.PerfectGrade.UL, ARBarData.PerfectGrade.VL );

	// draw sweet box
	StartSweetBox = EndSuperSweetBox - 4;
	EndSweetBox = int((ActiveReloadStartTime + SuperSweetSpotDuration + SweetSpotDuration) / Weap.ReloadDuration * BarLength);
	SetHUDDrawColor( eARColor, byte(FlashAlphaPercent * 255.f) );
	Canvas.SetPos( DrawX + StartSweetBox, DrawY - (ARBarData.SuccessRegion.VL * 0.5) );
	Canvas.DrawTile( ARBarData.SuccessRegion.Texture, EndSweetBox-StartSweetBox, ARBarData.SuccessRegion.VL, ARBarData.SuccessRegion.U, ARBarData.SuccessRegion.V, ARBarData.SuccessRegion.UL, ARBarData.SuccessRegion.VL );

	// draw tick mark
	SetHUDDrawColor( eARColor, Alpha*FlashAlphaPercent );
	StopLocationX = DrawX + (ARBarData.TickMarkTime / Weap.ReloadDuration * BarLength) - ARBarData.TickMark.UL/2;
	Canvas.SetPos( StopLocationX, DrawY - ARBarData.TickMark.VL/2 );
	Canvas.DrawTile( ARBarData.TickMark.Texture, ARBarData.TickMark.UL, ARBarData.TickMark.VL, ARBarData.TickMark.U, ARBarData.TickMark.V, ARBarData.TickMark.UL, ARBarData.TickMark.VL );

	// draw super sweet gradient
	if ( ARBarData.ARResult == eARResult_SuperSuccess )
	{
		SetHUDDrawColor( eARColor, Alpha*FlashAlphaPercent );
		Canvas.SetPos( DrawX, DrawY - (ARBarData.Flash.VL * 0.5) );
		Canvas.DrawTile( ARBarData.Flash.Texture, BarLength, ARBarData.Flash.VL, ARBarData.Flash.U, ARBarData.Flash.V, ARBarData.Flash.UL, ARBarData.Flash.VL );
	}
	// draw the other gradient
	else if ( (ARBarData.ARResult == eARResult_Success) || (ARBarData.ARResult == eARResult_Failed) )
	{
		SetHUDDrawColor( eARColor, Alpha*FlashAlphaPercent );
		// draw at the tick, clamped to the bar
		StopLocationX = Clamp(StopLocationX,DrawX + ARBarData.GoodGrade.UL/2,DrawX + BarLength - ARBarData.GoodGrade.UL/2);
		Canvas.SetPos( StopLocationX-ARBarData.GoodGrade.UL/2, DrawY - (ARBarData.GoodGrade.VL * 0.5) );
		Canvas.DrawTile( ARBarData.GoodGrade.Texture, ARBarData.GoodGrade.UL, ARBarData.GoodGrade.VL, ARBarData.GoodGrade.U, ARBarData.GoodGrade.V, ARBarData.GoodGrade.UL, ARBarData.GoodGrade.VL );
	}
}

final function DrawWeaponInfo(GearPC PC, GearPawn WP, GearWeapon Weap, float DrawX, float DrawY, int Alpha, optional bool bHighlight, optional float Scale = 1.f, optional bool bDrawAR, optional bool bDrawEmptyVersion, optional bool bIsMainIndicator)
{
	local float RemainingAmmo, AmmoPerIcon;
	local int IconCount;
	local string MagText;
	local float MagazineSize, OrigDrawX, OrigDrawY, Heat, HeatColorPercent;
	local EGearSpecialIconType SpecialIconType;
	local CanvasIcon SpecialIcon;
	local bool bHeatLimitedWeapon;

	if (Alpha == 0)
	{
		return;
	}

	OrigDrawX = DrawX;
	OrigDrawY = DrawY;
	Canvas.Font = Font'Warfare_HUD.WarfareHUD_font';

	// draw the weapon frame icon
	// draw the hilite if needed
	if (bHighlight)
	{
		SetWeaponSelectColor( Alpha );
		Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);
	}
	else
	{
		if (Weap != None)
		{
			bHeatLimitedWeapon = Weap.IsHeatLimited();
			Heat = Weap.GetCurrentBarrelHeat();
		}

		if( bHeatLimitedWeapon )
		{
			if ( Heat >= 0.65f )
			{
				Canvas.SetDrawColor( 247, 28, 36, Alpha*0.5f );
				Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);

				HeatColorPercent = (Heat - 0.65f) / 0.35f;
				SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha*(1.0f-HeatColorPercent*0.35f) );
				Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);
			}
			else
			{
				HeatColorPercent = Heat / 0.65f;
				Canvas.SetDrawColor( 247, 28, 36, Alpha*0.5f*HeatColorPercent );
				Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);

				SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
				Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);
			}
		}
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			Canvas.DrawIcon(WeaponInfoFrame,DrawX,DrawY,1.2f * Scale);
		}
	}

	// Return if we're drawing the icon empty
	if ( bDrawEmptyVersion )
	{
		return;
	}

	SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );

	if (Weap != None)
	{
		// draw the weapon icon
		DrawX = OrigDrawX + (20.f + Weap.IconXOffset)*Scale;
		DrawY = OrigDrawY + Weap.IconYOffset*Scale;

		// draw the reload indicator
		if ( Weap.IsReloading() && bDrawAR && Weap.Instigator == PC.Pawn )
		{
			DrawActionReloadBar( Weap, OrigDrawX, OrigDrawY+(WeaponInfoFrame.VL*1.2f*Scale)+ARBarData.TickMark.VL/4, Alpha, Scale );
		}

		DrawWeaponIcon( Weap, DrawX, DrawY, Alpha, Scale );

		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );

		// if there is any ammo
		MagazineSize = Weap.GetMagazineSize();
		if (MagazineSize > 0)
		{
			if (Weap.Instigator == PC.Pawn)
			{
				// draw the ammo bar
				DrawX = OrigDrawX + 60.f;
				DrawY = OrigDrawY + 45.f*Scale;
				IconCount = Weap.GetAmmoIconCount();
				RemainingAmmo = MagazineSize - Weap.AmmoUsedCount;
				if ( IconCount != 0 )
				{
					AmmoPerIcon = MagazineSize/float(IconCount);

					// empty spots
					DrawEmptyAmmo( Weap, Alpha, DrawX, DrawY, Scale );

					if ( Weap.IsReloading() )
					{
						DrawSuperAmmo( Weap, Alpha, AmmoPerIcon, RemainingAmmo, DrawX, DrawY, Scale, bHighlight );
						DrawAmmo( Weap, Alpha, AmmoPerIcon, RemainingAmmo, DrawX, DrawY, Scale, bHighlight );
					}
					else
					{
						DrawAmmo( Weap, Alpha, AmmoPerIcon, RemainingAmmo, DrawX, DrawY, Scale, bHighlight );
						DrawSuperAmmo( Weap, Alpha, AmmoPerIcon, RemainingAmmo, DrawX, DrawY, Scale, bHighlight );
					}
				}

				// magazine count
				DrawX = OrigDrawX + 4.f*Scale;
				DrawY -= 8.f*Scale;
				if (Weap.HasInfiniteSpareAmmo())
				{
					MagText = "";
				}
				else
				{
					MagText = string(int(Weap.SpareAmmoCount+MagazineSize-Weap.AmmoUsedCount));
					if (len(MagText) == 2)
					{
						MagText = "0"$MagText;
					}
					else if (len(MagText) == 1)
					{
						MagText = "00"$MagText;
					}
				}
				Canvas.SetPos(DrawX,DrawY-2);
				if (Weap.IsCriticalAmmoCount())
				{
					SetHUDDrawColor( eWARHUDCOLOR_RED, Alpha );
				}
				else if (bHighlight)
				{
					SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
				}
				else
				{
					SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
				}
				Canvas.DrawText(MagText, FALSE);
			}
		}

		// hack for gatling prototype
		if (Weap.Instigator == PC.Pawn)
		{
			if (bHeatLimitedWeapon)
			{
				DrawWeaponHeatIndicator(Weap, Heat, OrigDrawX, OrigDrawY, Alpha, Scale, bHighlight);
			}
			else if ( (GearWeap_HODBase(Weap) != None) && bInMultiplayerMode )
			{
				DrawHODAmmo(GearWeap_HODBase(Weap), WP, OrigDrawX, OrigDrawY, Alpha, Scale, bHighlight);
			}
		}

		// See if we should draw a special icon in the weapon indicator
		SpecialIconType = SpecialIconToDrawInWeaponIndicator(PC, bIsMainIndicator);
		if ( SpecialIconType != eGSIT_None )
		{
			if ( IsCOG(PC.PlayerReplicationInfo) )
			{
				SetHUDDrawColor( eWARHUDCOLOR_TEAMBLUE, Alpha );
			}
			else
			{
				SetHUDDrawColor( eWARHUDCOLOR_TEAMRED, Alpha );
			}

			SpecialIcon = (SpecialIconType == eGSIT_Leader) ? LeaderIcon : MeatflagIcon;
			Canvas.DrawIcon( SpecialIcon, Canvas.ClipX - SafeZoneRight - SpecialIcon.UL*Scale/2, SafeZoneTop + 4.0f*Scale, Scale );
		}
	}
}

/** Whether to draw the leader icon or not */
function EGearSpecialIconType SpecialIconToDrawInWeaponIndicator( GearPC PC, bool bIsMainWeaponIndicator )
{
	return eGSIT_None;
}

final private function DrawWeaponHeatIndicator(GearWeapon Weap, float Heat, float DrawX, float DrawY, float Alpha, float Scale, bool bHighlight)
{
	local float HeatDrawPercent;
	local CanvasIcon AmmoIcon;

	// this puts us in the ammo bar
	DrawX += 60.f;
	DrawY += 45.f*Scale;

	AmmoIcon = Weap.HUDDrawData.AmmoIcon;
	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.SetDrawColor(47,47,47,Alpha);
	Canvas.DrawTile( AmmoIcon.Texture, AmmoIcon.UL*Scale, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, AmmoIcon.UL, AmmoIcon.VL );

	// Determine the color to draw
	if ( Heat >= 0.5f )
	{
		HeatDrawPercent = (Heat-0.5f) / 0.5f;
		Canvas.SetDrawColor( 255-(18*HeatDrawPercent), 255-(227*HeatDrawPercent), 255-(219*HeatDrawPercent), Alpha );
	}
	else
	{
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha);
	}

	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.DrawTile( AmmoIcon.Texture, AmmoIcon.UL*Scale*Heat, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, AmmoIcon.UL*Heat, AmmoIcon.VL );
}

final private function DrawHODAmmo(GearWeap_HODBase Weap, GearPawn GP, float DrawX, float DrawY, float Alpha, float Scale, bool bHighlight)
{
	local CanvasIcon AmmoIcon;
	local float PercentHODLeft;

	// this puts us in the ammo bar
	DrawX += 60.f;
	DrawY += 45.f*Scale;

	AmmoIcon = Weap.HUDDrawData.AmmoIcon;
	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.SetDrawColor(47,47,47,Alpha);
	Canvas.DrawTile( AmmoIcon.Texture, AmmoIcon.UL*Scale, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, AmmoIcon.UL, AmmoIcon.VL );

	PercentHODLeft = Weap.BatteryLifeSec / Weap.default.BatteryLifeSec;

	// Determine the color to draw
	if ( PercentHODLeft <= 0.25f )
	{
		SetHUDDrawColor( bHighlight ? eWARHUDCOLOR_BLACK : eWARHUDCOLOR_RED, Alpha);
	}
	else
	{
		SetHUDDrawColor( bHighlight ? eWARHUDCOLOR_BLACK : eWARHUDCOLOR_WHITE, Alpha);
	}

	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.DrawTile( AmmoIcon.Texture, AmmoIcon.UL*Scale*PercentHODLeft, AmmoIcon.VL*Scale, AmmoIcon.U, AmmoIcon.V, AmmoIcon.UL*PercentHODLeft, AmmoIcon.VL );
}

/**
 * Callback for when a player has talked.
 *
 * @param TalkingPlayer		UniqueNetId for the player that just talked.
 */
function OnPlayerTalking(UniqueNetId TalkingPlayer)
{
	Local GearPRI TalkerPRI;

	TalkerPRI = ResolveUniqueNetID(TalkingPlayer);
	if ( TalkerPRI != None )
	{
		TalkerPRI.ChatFadeValue = 1.0;
		TalkerPRI.TaccomChatFadeStart = WorldInfo.TimeSeconds;
		TalkerPRI.ChatFadeTime = 1.5;
	}
}

/**
 * Finds the PRI for the player with the NetId passed in.
 *
 * @param TestID		ID to search for.
 *
 * @return a PRI for the player with the provided NetId.
 */
function GearPRI ResolveUniqueNetID(UniqueNetId TestID)
{
	local GearGRI GRI;
	local int i;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None)
	{
		for (i=0;i<GRI.PRIArray.Length;i++)
		{
			if (GRI.PRIArray[i] != None && GRI.PRIArray[i].UniqueId == TestID)
			{
				return GearPRI(GRI.PRIArray[i]);
			}
		}
	}
	return None;
}


/**
 * Sets a talking delegate so we can display chat icons.
 */
function TrackChat()
{
	local OnlineSubSystem OnlineSub;

	if (!bTrackingChat)
	{

		OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
		if (OnlineSub != None && OnLineSub.VoiceInterface != None)
		{
			OnlineSub.VoiceInterface.AddPlayerTalkingDelegate(OnPlayerTalking);
		}

		bTrackingChat = TRUE;

	}
}

/**
 * Removes the talking delegate we set to display chat icons.
 */
function IgnoreChat()
{
	local OnlineSubSystem OnlineSub;

	if ( bTrackingChat )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubSystem();
		if (OnlineSub != None && OnLineSub.VoiceInterface != None)
		{
			OnlineSub.VoiceInterface.ClearPlayerTalkingDelegate(OnPlayerTalking);
		}

		bTrackingChat = FALSE;
	}
}

/** Shows tac/com locators while bypassing the PP effect. */
function ShowSquadLocators(float Duration)
{
	TaccomFadeStartTime = WorldInfo.TimeSeconds + Duration;
	bInitialTaccomFadeIn = TRUE;
}

/**
 * Enables TAC/COM and related FX, fades in interface.  Called from GearPC.EnableAssessMode().
 */
function EnableAssessMode()
{
	bInitialTaccomFadeIn = FALSE;
	// start the looping tac/com sound
	TacComLoop.Play();
	// enable chat icon delegate
	TrackChat();
	// start fading in
	bIsFadingTaccomIn = TRUE;
	TaccomFadeStartTime = WorldInfo.TimeSeconds - (TotalTaccomFadeTime * TaccomFadeOpacity);
}

/**
 * Disables TAC/COM and related FX, starts fading out interface.  Called from GearPC.DisableAssessMode().
 */
final function DisableAssessMode()
{
	// clear the chat icon delegate
	IgnoreChat();
	// stop the looping tac/com sound
	TacComLoop.Stop();
	bIsFadingTaccomIn = FALSE;
	// start fading out
	TaccomFadeStartTime = WorldInfo.TimeSeconds - (TotalTaccomFadeTime * (1.f - TaccomFadeOpacity));
}

function EAssessSquadStatus GetSquadmateStatus( GearPawn AGearPawn )
{
	local GearAI AI;

	// Hostages are considered dead.
	if( AGearPawn.IsDBNO() && !AGearPawn.IsAHostage()  )
	{
		return EASS_Down;
	}

	if( AGearPawn.Health <= 0 )
	{
		return EASS_Dead;
	}

	AI = GearAI( AGearPawn.Controller );
	if ( AI != None )
	{
		switch ( AI.CombatMood )
		{
			case AICM_Normal:		return EASS_Normal;
			case AICM_Passive:		return EASS_Defensive;
			case AICM_Aggressive:	return EASS_Attack;
		}
	}

	return EASS_None;
}

/** Converts SquadmateStatus to DisplayableSquadmateStatus */
function EDisplayableSquadStatus ConvertSquadmateStatus( EAssessSquadStatus ASS_Status )
{
	switch ( ASS_Status )
	{
		case EASS_Normal:		return eSO_NORMAL;
		case EASS_Down:			return eSO_DOWN;
		case EASS_Dead:			return eSO_DEAD;
		case EASS_Defensive:	return eSO_DEFENSIVE;
		case EASS_Attack:		return eSO_ATTACK;
		default:				return eSO_NORMAL;
	}
}

final function DrawSquadLocator( Actor AWarPawn, float CenterXCoord, float CenterYCoord, float Scale, int Alpha )
{
	local float DrawX, DrawY;
	local int Idx;

	// If the array is empty fill it up
	if ( SquadLocator.length <= 0 )
	{
		CurrSquadLocatorToUse = 0;
		SquadLocator.length = NumSquadLocators;
		for ( Idx = 0; Idx < SquadLocator.length; Idx++ )
		{
			SquadLocator[Idx].MILocation = new(outer) class'MaterialInstanceConstant';
			SquadLocator[Idx].MILocation.SetParent( SquadLocatorMaterial );
		}
	}

	if ( AWarPawn != None )
	{
		SquadLocator[CurrSquadLocatorToUse].DirOfHit = CalculateLocationMaterialEffect( AWarPawn.Location );
	}
	SquadLocator[CurrSquadLocatorToUse].MILocation.SetVectorParameterValue( 'HUD_Location_Rotator', MakeLinearColor(SquadLocator[CurrSquadLocatorToUse].DirOfHit, 0.0f, 0.0f, 1.0) );
	SquadLocator[CurrSquadLocatorToUse].MILocation.SetScalarParameterValue( 'Hit_Fade', Alpha/255.f );

	DrawX = CenterXCoord - 128*Scale;
	DrawY = CenterYCoord - 128*Scale;
	Canvas.SetPos( DrawX, DrawY ) ;
	Canvas.DrawMaterialTile( SquadLocator[CurrSquadLocatorToUse].MILocation, 256*Scale, 256*Scale, 0, 0, 1.f, 1.f );

	CurrSquadLocatorToUse = (CurrSquadLocatorToUse + 1) % NumSquadLocators;
}

// Draw a glow ring around the background (used in Meatflag)
function DrawBackgroundRing( GearPawn AGearPawn, float CenterXCoord, float CenterYCoord, float Scale )
{
}

/** Returns the type of special icon to draw in the squadmate UI */
function EGearSpecialIconType GetSpecialIconType( GearPawn PawnToTest )
{
	return eGSIT_None;
}

final function DrawSquadMate( GearPawn AGearPawn, float CenterXCoord, float CenterYCoord, optional float Scale = 1.f, optional byte Alpha = 255, optional bool bIsDBNO = FALSE )
{
	local CanvasIcon BackIcon, HeadIcon, StatusIcon, SpecialIcon;
	local float DrawX, DrawY, TextX, TextY, TextScale;
	local EAssessSquadStatus Status;
	local EAssessSquadBackgroundType BackgroundType;
	local float PercentAlive;
	local class<GearDamageType> WeapDamageType;
	local bool bHasBeenRevived;
	local GearPRI PRI;
	local string NameToDraw;
	local EGearSpecialIconType SpecialIconType;

	if( AGearPawn != None )
	{
		TextScale = (Scale < 0.75f) ? 0.75f : Scale;
		Status = GetSquadmateStatus( AGearPawn );
		BackgroundType = (Status==EASS_Down || Status==EASS_Dead) ? EASBT_Dead : EASBT_Normal;

		// if this is the down but not out version of squadmate icon, and they aren't dying and are no
		// longer reviving, then they must've been revived
		if ( bIsDBNO && !AGearPawn.IsInState('Dying') &&  !AGearPawn.IsDBNO() )
		{
			bHasBeenRevived = TRUE;
		}

		// background
		PRI = GearPRI(AGearPawn.PlayerReplicationInfo);
		if ( AGearPawn.bIsBleedingOut && AGearPawn.IsDBNO() )
		{
			PercentAlive = CalculateRevivePercentAlive(AGearPawn);

			BackIcon = SquadBackgrounds[EASBT_Normal];
			Canvas.SetPos( CenterXCoord - BackIcon.UL*Scale/2, CenterYCoord - BackIcon.VL*Scale/2 );
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			Canvas.DrawTile( BackIcon.Texture, BackIcon.UL*Scale, BackIcon.VL*fmin((PercentAlive+0.01f),1.f)*Scale, BackIcon.U, BackIcon.V, BackIcon.UL, BackIcon.VL*fmin((PercentAlive+0.01f),1.f) );

			BackIcon = SquadBackgrounds[EASBT_Dead];
			Canvas.SetPos( CenterXCoord - BackIcon.UL*Scale/2, CenterYCoord - BackIcon.VL*Scale/2 + BackIcon.VL*Scale*PercentAlive );
			SetHUDDrawColor( eWARHUDCOLOR_RED, Alpha );
			Canvas.DrawTile( BackIcon.Texture, BackIcon.UL*Scale, BackIcon.VL*(1.f-PercentAlive)*Scale, BackIcon.U, BackIcon.V + BackIcon.VL*PercentAlive, BackIcon.UL, BackIcon.VL*(1.f-PercentAlive) );
		}
		else
		{
			if ( Status == EASS_Dead || AGearPawn.IsDBNO() )
			{
				BackIcon = SquadBackgrounds[EASBT_Dead];
				SetHUDDrawColor( eWARHUDCOLOR_RED, Alpha );
			}
			else
			{
				BackIcon = SquadBackgrounds[EASBT_Normal];
				SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
				Canvas.DrawIcon( SquadBackgroundNamePlate, CenterXCoord - SquadBackgroundNamePlate.UL*Scale/2, CenterYCoord - SquadBackgroundNamePlate.VL*Scale/2, Scale );
				// if the teammate has been revived the background needs to be white
				if ( bHasBeenRevived )
				{
					SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
				}
			}
			Canvas.DrawIcon( BackIcon, CenterXCoord - BackIcon.UL*Scale/2, CenterYCoord - BackIcon.VL*Scale/2, Scale );
		}

		// Draw a glow ring around the background (used in Meatflag)
		DrawBackgroundRing( AGearPawn, CenterXCoord, CenterYCoord, Scale );

		// draw indicator
		DrawSquadLocator( AGearPawn, CenterXCoord, CenterYCoord, Scale, Alpha );

		// draw the head
		if ( (bInMultiplayerMode && Status == EASS_Down) || Status == EASS_Dead )
		{
			SetHUDDrawColor( eWARHUDCOLOR_RED, Alpha );
		}
		else
		{
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		}
		HeadIcon = AGearPawn.HeadIcon;
		DrawY = CenterYCoord - SquadBackgrounds[BackgroundType].VL*Scale/2 - SquadDisplayData.HeadYFromTopOfBackground*Scale;
		Canvas.DrawIcon( HeadIcon, CenterXCoord - HeadIcon.UL*Scale/2, DrawY, Scale );

		// Draw a special icon (leader or flag)
		if ( AGearPawn.PlayerReplicationInfo != None )
		{
			SpecialIconType = GetSpecialIconType(AGearPawn);
			if ( SpecialIconType != eGSIT_None )
			{
				SetHUDDrawColor( IsCOG(AGearPawn.PlayerReplicationInfo) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED, Alpha );
				SpecialIcon = (SpecialIconType == eGSIT_Leader) ? LeaderIcon : MeatflagIcon;
				Canvas.DrawIcon( SpecialIcon, CenterXCoord + HeadIcon.UL/2*Scale, DrawY + HeadIcon.VL*Scale - SpecialIcon.VL*Scale, Scale );
			}
		}

		// move down for the name
		DrawY += HeadIcon.VL*Scale;
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );

		NameToDraw = (PRI != None) ? PRI.PlayerName : "";

		// draw the name
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);
		// stick their name on the HUD
		Canvas.TextSize( NameToDraw, TextX, TextY );
		DrawX = CenterXCoord - TextX*TextScale/2;
		// Draw Y is now based from where we drew the face icon....
		//DrawY = CenterYCoord + SquadDisplayData.NameYFromCenter*Scale;
		Canvas.SetPos( DrawX, DrawY );
		Canvas.DrawText( NameToDraw, FALSE, TextScale, TextScale );

		// move down for the status icon
		DrawY += TextY*TextScale + 2.0f;

		// draw the status
		if ( AGearPawn.Weapon != None )
		{
			WeapDamageType = AGearPawn.MyGearWeapon.default.DamageTypeClassForUI;
			if ( WeapDamageType != None )
			{
				StatusIcon = WeapDamageType.default.KilledByIcon;
				DrawX = CenterXCoord - StatusIcon.UL*Scale/2;
				// DrawY is not based off of the name string.
				//DrawY = CenterYCoord + (SquadDisplayData.OrderYFromCenter+4)*Scale;
				Canvas.SetPos( DrawX, DrawY );
				Canvas.DrawTile(StatusIcon.Texture, StatusIcon.UL*Scale, StatusIcon.VL*Scale, StatusIcon.U+StatusIcon.UL, StatusIcon.V, -StatusIcon.UL, StatusIcon.VL);
			}
		}

		// this condition is a bit of a hack but warpawn state isn't replicated so this is
		// the best way to determine this w/o adding another replicated variable.
		if ( AGearPawn.IsInState('Dying') || AGearPawn.bTearOff )
		{
			if( DeadTaccomSplatMaterialConstant == None )
			{
				DeadTaccomSplatMaterialConstant = new(outer) class'MaterialInstanceConstant';
				DeadTaccomSplatMaterialConstant.SetParent( DeadTaccomSplatMaterial );
			}

			DrawX = CenterXCoord - 128.f*Scale;
			DrawY = CenterYCoord - 256.f*Scale;
			Canvas.SetPos( DrawX, DrawY );
			DeadTaccomSplatMaterialConstant.SetScalarParameterValue( 'TaccomDead_Fade', 1.f-Alpha/255.f );
			Canvas.DrawMaterialTile( DeadTaccomSplatMaterialConstant, 256.f*Scale, 512.f*Scale );
		}
		// show revived icon
		else if ( bHasBeenRevived )
		{
			DrawX = CenterXCoord - ReviveTaccomIcon.UL*Scale/2;
			DrawY = CenterYCoord - ReviveTaccomIcon.VL*Scale/2;
			Canvas.SetPos( DrawX, DrawY );
			Canvas.DrawIcon( ReviveTaccomIcon, DrawX, DrawY, Scale );
		}

		// Draw the chat icon
		if(PRI != None)
		{
			if(PRI.ChatFadeValue > 0)
			{
				PRI.ChatFadeValue = 1.0 - (WorldInfo.TimeSeconds - PRI.TaccomChatFadeStart) / PRI.ChatFadeTime;
				if ( PRI.ChatFadeValue > 0 )
				{
					DrawX = CenterXCoord - (VoiceTaccomIcon.UL+VoiceTaccomIconPos[0])*Scale/2;
					DrawY = CenterYCoord - (VoiceTaccomIcon.VL+VoiceTaccomIconPos[1])*Scale/2;

					// Color the chat bubble depending on the team the player is on
					if( PRI.Team.TeamIndex == 0 )
					{
						SetHUDDrawColor( eWARHUDCOLOR_CHATICONBLUE, Alpha*PRI.ChatFadeValue );
					}
					else
					{
						SetHUDDrawColor( eWARHUDCOLOR_CHATICONRED, Alpha*PRI.ChatFadeValue );
					}

					Canvas.SetPos( DrawX, DrawY );
					Canvas.DrawIcon( VoiceTaccomIcon, DrawX, DrawY, Scale );
				}
				else
				{
					PRI.ChatFadeValue = 0.0;
				}
			}
		}
	}
}

function DrawStrikeThru( int StrikeCount, float DrawX, float DrawY, float Width, float Height, EGearHUDColor eColor, byte Alpha )
{
	Canvas.SetPos( DrawX, DrawY );
	SetHUDDrawColor( eColor, Alpha );

	switch ( StrikeCount % 4 )
	{
	case 0:
		Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 0, 0, 512, 32 );
		break;
	case 1:
		Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 512, 0, -512, 32 );
		break;
	case 2:
		Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 0, 32, 512, -32 );
		break;
	case 3:
		Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_Objective_Strike', Width, Height, 512, 32, -512, -32 );
		break;
	}
}

final function DrawCenteredText(int DrawX, int DrawY, string Text)
{
	local float XL, YL;
	Canvas.TextSize( Text, XL, YL );
	Canvas.SetPos( DrawX-XL/2, DrawY );
	Canvas.DrawText( Text );
}

final function vector2d GetSquadMateIconPos(GearPawn Pawn, vector Origin, rotator Direction, float Scale)
{
	local vector PawnLocation, OriginToPawn;
	local vector2d AngularDist;
	PawnLocation = Pawn.Location;
	PawnLocation.Z = Origin.Z;
	OriginToPawn = Normal( Origin - PawnLocation );
	// trying some a little different than a plain circle, feel free to delete if nobody cares for it
	AngularDist.X = asin(OriginToPawn dot Normal(vector(Direction) cross vect(0,0,1)));
	AngularDist.Y = asin(OriginToPawn dot vector(Direction));
	AngularDist.X = FClamp(Canvas.ClipX/2 + FClamp(AngularDist.X,-1.f,1.f) * (Canvas.ClipX/2 - (128.f * Scale)),SafeZoneLeft + 64.f,Canvas.ClipX - SafeZoneRight - 64.f);
	AngularDist.Y = FClamp(Canvas.ClipY/2 + FClamp(AngularDist.Y,-1.f,1.f) * (Canvas.ClipY/2 - (128.f * Scale)),SafeZoneTop + 64.f,Canvas.ClipY - SafeZoneBottom - 64.f);
	return AngularDist;

}

/** Whether to draw the face icon when in taccom or not */
function bool IsPawnValidForAssessMode( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	local GearPRI TestPRI;
	// if not the same pawn and the pawn is alive/dbno
	if (PawnToTest != LocalPawn && (PawnToTest.Health > 0 || ShouldDrawDBNOIndicator(PawnToTest)) && !PawnToTest.bHidden && Vehicle_Centaur_Base(PawnToTest.DrivenVehicle) == None)
	{
		TestPRI = GearPRI(PawnToTest.PlayerReplicationInfo);
		// if spectating then we want to view everyone
		if (PlayerOwner.IsSpectating())
		{
			return TRUE;
		}
		else if ( LocalPawn != None && LocalPawn.IsSameTeam(PawnToTest) )
		{
			if (WorldInfo.GRI != None && !WorldInfo.GRI.IsMultiplayerGame())
			{
				// in campaign only show squadmates
				return PawnToTest.IsA('GearPawn_COGDom') || PawnToTest.IsA('GearPawn_COGMarcus') || (TestPRI != None && (TestPRI.bForceShowInTaccom || TestPRI.SquadName == LocalPRI.SquadName));
			}
			return TRUE;
		}
	}
	return FALSE;
}

/** Whether to draw the name above a player's head or not in taccom */
function bool ShouldDrawNameAboveHeadInTaccom( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	// only show other players who are DBNO or alive
	if (LocalPawn != PawnToTest && (PawnToTest.Health > 0 || PawnToTest.IsDBNO()))
	{
		// show if spectating, or DBNO w/ teammates
		return LocalPawn == None || !LocalPawn.IsDBNO() || LocalPawn.IsSameTeam(PawnToTest);
	}
	return FALSE;
}

/** Whether to draw the name above a player's head or not in taccom when they're face icon is showing */
function bool ShouldDrawNameAboveHeadInTaccomWhenFaceIconIsShown( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	return TRUE;
}

/**
 * Clears the current TacCom effect.
 * @note - do not call this every frame since it will break splitscreen if called by the second viewport!
 */
final function ResetTaccomOpacity()
{
	TaccomFadeOpacity = 0.f;
	// and set the scalar parm on the effect
	if (TacComEffect != None)
	{
		MaterialInstanceConstant(TacComEffect.Material).SetScalarParameterValue('Opacity',0.f);
		TacComEffect.bShowInGame = FALSE;
	}
}

final function DrawAssessMode(GearPC PC, GearPawn GP)
{
	local GearPawn OtherGP;
	local GearPRI PRI;
	local vector2d SquadMateIconPos;
    local float Distance, ScaleModifier;
    local vector CameraLoc;
    local rotator CameraRot;

	// cache the reference if needed
	if (TacComEffect == None)
	{
		TacComEffect = MaterialEffect(LocalPlayer(PlayerOwner.Player).PlayerPostProcess.FindPostProcessEffect('TacCom'));
	}

	// Determine what the opacity of the taccom screen should be.
	if ( bIsFadingTaccomIn || PC.bAssessMode )
	{
		if ( (TaccomFadeStartTime > 0.f) && ((WorldInfo.TimeSeconds - TaccomFadeStartTime) < TotalTaccomFadeTime) )
		{
			TaccomFadeOpacity = (WorldInfo.TimeSeconds - TaccomFadeStartTime) / TotalTaccomFadeTime;
		}
		else
		{
			TaccomFadeOpacity = 1.f;
		}
			TacComEffect.bShowInGame = TRUE;
	}
	else if ( !bIsFadingTaccomIn && ((WorldInfo.TimeSeconds - TaccomFadeStartTime) < TotalTaccomFadeTime) && (TaccomFadeStartTime > 0.f) )
	{
		if (TaccomFadeStartTime > WorldInfo.TimeSeconds)
		{
			TaccomFadeOpacity = 1.f;
		}
		else
		{
			TaccomFadeOpacity = 1.f - ((WorldInfo.TimeSeconds - TaccomFadeStartTime) / TotalTaccomFadeTime);
		}
			TacComEffect.bShowInGame = TRUE;
	}
	else
	{
		TacComEffect.bShowInGame = FALSE;
		TaccomFadeOpacity = 0.f;
	}
	// and set the scalar parm on the effect
	if (TacComEffect != None)
	{
		//`log(WorldInfo.TimeSeconds@`showvar(PC)@`showvar(TaccomFadeOpacity)@`showvar(TaccomEffect.bShowInGame)@`showvar(TaccomEffect)@`showvar(bInitialTaccomFadeIn));
		// if doing the initial fadein then skip the PP effect
		if (bInitialTaccomFadeIn)
		{
			MaterialInstanceConstant(TacComEffect.Material).SetScalarParameterValue('Opacity',0.f);
		}
		// otherwise update to the current opacity
		else
		{
			MaterialInstanceConstant(TacComEffect.Material).SetScalarParameterValue('Opacity',TaccomFadeOpacity);
			if (CurrSplitscreenType != eSST_None)
			{
				MaterialInstanceConstant(TacComEffect.Material).SetScalarParameterValue('RimAlpha',0.f);
			}
			else
			{
				MaterialInstanceConstant(TacComEffect.Material).SetScalarParameterValue('RimAlpha',1.f);
			}
		}
	}
	// if currently drawing taccom then draw the squad locators
	if ( TaccomFadeOpacity > 0.f )
	{

		PRI	= GearPRI(PC.PlayerReplicationInfo);
		PC.GetPlayerViewpoint(CameraLoc,CameraRot);

		foreach WorldInfo.AllPawns( class'GearPawn', OtherGP )
		{
			if( IsPawnValidForAssessMode( OtherGP, PRI, GP ) )
			{
				// if the player isn't currently visible
				if (!IsSquadMateVisible(OtherGP,CameraLoc,CameraRot,PC))
				{
					if ( ShouldDrawSquadMateIcon(OtherGP,PRI,GP) )
					{
						// scale the size slightly based on distance
						Distance = VSize2D(OtherGP.Location - GP.Location);
						ScaleModifier = Distance > 1000.f ? 1.f - (FClamp((Distance - 1000)/1000.f,0.f,1.f) * 0.35f) : 1.f;
						// reduce the size if in splitscreen
						if (CurrSplitscreenType != eSST_None)
						{
							ScaleModifier *= 0.68f;
						}
						// draw an icon to show their location;
						SquadMateIconPos = GetSquadMateIconPos(OtherGP,CameraLoc,CameraRot,ScaleModifier);
						DrawSquadmate( OtherGP, SquadMateIconPos.X, SquadMateIconPos.Y, ScaleModifier, 255*TaccomFadeOpacity);
					}
				}
				else if (ShouldDrawNameAboveHeadInTaccomWhenFaceIconIsShown( OtherGP, PRI, GP ))
				{
					DrawSquadMateNameAboveHead( OtherGP, CameraLoc, CameraRot, 255*TaccomFadeOpacity, SpecialIconToDrawAboveHead(OtherGP) );
				}
			}
		}

		if( !bInMultiplayerMode )
		{
			// Draw objectives
			if ( PC.ObjectiveMgr != None )
			{
				PC.ObjectiveMgr.DrawObjectives( self, PC, TaccomFadeOpacity );
			}
		}
	}
}

/** Draws names above heads when the player is spectating */
function DrawSpectatingNames(GearPC PC, GearPawn GP)
{
	local GearPawn OtherGP;
	local GearPRI PRI;
	local vector CameraLoc;
	local rotator CameraRot;

	if ( PC.IsSpectating() )
	{
		PRI	= GearPRI(PC.PlayerReplicationInfo);
		PC.GetPlayerViewpoint(CameraLoc,CameraRot);

		foreach WorldInfo.AllPawns( class'GearPawn', OtherGP )
		{
			if( IsPawnValidForAssessMode( OtherGP, PRI, GP ) )
			{
				// if the player isn't currently visible
				if ( IsSquadMateVisible(OtherGP,CameraLoc,CameraRot,PC) && ShouldDrawNameAboveHeadInTaccomWhenFaceIconIsShown( OtherGP, PRI, GP ) )
				{
					DrawSquadMateNameAboveHead( OtherGP, CameraLoc, CameraRot, 255 );
				}
			}
		}
	}
}

/** Special icon to draw above the name of the player in taccom */
function EGearSpecialIconType SpecialIconToDrawAboveHead(GearPawn PawnToTest)
{
	return eGSIT_None;
}

/** Should the player draw an icon to locate this pawn */
function bool ShouldDrawSquadMateIcon(GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn)
{
	// only ever show the icons for teammates (enemies only draw names when visible)
	return (PawnToTest.IsSameTeam(LocalPawn));
}

final function bool IsSquadMateVisible(GearPawn TestPawn, vector CamLoc, rotator CamRot, GearPC PC)
{
	local vector CamToPawn;
	CamToPawn = Normal(TestPawn.Location - CamLoc);
	return (CamToPawn dot vector(CamRot)) > 0.7f * (85.f/PC.FOVAngle) && TimeSince(TestPawn.LastRenderTime) < 0.5f && PC.LineOfSightTo(TestPawn);
}

/** Returns the canvasicon of the specialicontype passed in */
final function CanvasIcon GetSpecialCanvasIcon( EGearSpecialIconType IconType )
{
	local CanvasIcon ReturnIcon;

	switch ( IconType )
	{
		case eGSIT_Leader:
			ReturnIcon = LeaderIcon;
			break;
		case eGSIT_Flag:
			ReturnIcon = MeatflagIcon;
			break;
		case eGSIT_BuddyCOG:
			ReturnIcon = BuddyIconCOG;
			break;
		case eGSIT_BuddyLocust:
			ReturnIcon = BuddyIconLocust;
			break;
	}

	return ReturnIcon;
}

/** Draw the name of the player above their head in screenspace */
final function DrawSquadMateNameAboveHead( GearPawn SquadmatePawn, vector CameraLoc, rotator CameraRot, optional byte Alpha = 255, optional EGearSpecialIconType SpecialIconType = eGSIT_None )
{
	local float TextX, TextY;
	local vector PawnExtent, ScreenLoc, CameraDir, SquademateDir;
	local GearPC PC;
	local CanvasIcon SpecialIcon;
	local EGearHUDColor ColorType;

	PC = GearPC(PlayerOwner);
	if ( (SquadmatePawn != None) && (Pawn(SquadmatePawn.Base) == None) && SquadmatePawn.PlayerReplicationInfo != None && !SquadmatePawn.bIsHiddenByCamera && TimeSince(SquadmatePawn.LastRenderTime) < 2.f && PC.LineOfSightTo(SquadmatePawn) )
	{
		// Configure the color, alpha, font, and get the size of the string
		SetHUDDrawColor( IsCOG(SquadmatePawn.PlayerReplicationInfo) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED, Alpha );
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);
		Canvas.TextSize( SquadmatePawn.PlayerReplicationInfo.PlayerName, TextX, TextY );

		CameraDir = vector(CameraRot);
		SquademateDir = SquadmatePawn.Location - CameraLoc;
		if ( CameraDir Dot SquademateDir > 0 && SquadmatePawn.HeadBoneNames.Length > 0 )
		{
			PawnExtent = SquadmatePawn.Mesh.GetBoneLocation( SquadmatePawn.HeadBoneNames[0] );
			PawnExtent.Z += 30.0f;
			ScreenLoc = Canvas.Project( PawnExtent );
			ScreenLoc.X -= TextX / 2;
			ScreenLoc.Y -= TextY * 1.5f;

			// make sure this isn't clipping past the canvas (splitscreen)
			if (ScreenLoc.Y < 0 || ScreenLoc.Y > Canvas.ClipY - SafeZoneBottom)
			{
				return;
			}

			// Set the text position and draw
			Canvas.SetPos( ScreenLoc.X, ScreenLoc.Y );
			Canvas.DrawTextClipped( SquadmatePawn.PlayerReplicationInfo.PlayerName );

			if ( SpecialIconType != eGSIT_None )
			{
				SpecialIcon = GetSpecialCanvasIcon( SpecialIconType );
				if ( IsMeatflagPawn(SquadmatePawn) )
				{
					ColorType = eWARHUDCOLOR_WHITE;
				}
				else
				{
					ColorType = IsCOG(SquadmatePawn.PlayerReplicationInfo) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED;
				}
				SetHUDDrawColor( ColorType, Alpha );
				Canvas.DrawIcon( SpecialIcon, ScreenLoc.X + TextX/2 - SpecialIcon.UL/2, ScreenLoc.Y - SpecialIcon.VL, 1.0f );
			}
		}
	}
}

/** Whether the pawn is the meatflag */
function bool IsMeatflagPawn( GearPawn AGearPawn );

/** Whether we should draw a DBNO player's indicator or not */
final function bool ShouldDrawDBNOIndicator( GearPawn AWPawn )
{
	local GearPRI TestPRI, LocalPRI;
	if (AWPawn != None && !AWPawn.IsAHostage())
	{
		TestPRI = GearPRI(AWPawn.PlayerReplicationInfo);
		LocalPRI = GearPRI(PlayerOwner.PlayerReplicationInfo);
		if ((WorldInfo.GRI.IsMultiplayerGame() || (!WorldInfo.GRI.IsMultiplayerGame() && (AWPawn.IsA('GearPawn_COGMarcus') || AWPawn.IsA('GearPawn_COGDom'))) || (TestPRI != None && (TestPRI.bForceShowInTaccom || TestPRI.SquadName == LocalPRI.SquadName))) &&
			(AWPawn.IsDBNO() || (AWPawn.TimeOfRevival > 0 && AWPawn.TimeOfRevival > WorldInfo.TimeSeconds-2.f) || (AWPawn.IsInState('Dying') && (AWPawn.TimeOfDeath+2.f > WorldInfo.TimeSeconds))))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * This will draw the taccom images of your teammates that are down but not out.
 * In MP, we use this when ever your friend is down, we pop it up to let people know their pals
 * are down and hurting.
 **/
final function DrawDownedButNotOutTeammates(GearPC PC, GearPawn WP)
{
	local int Count;
	local GearPawn AWPawn;
	local GearPRI PRI;
	local byte Alpha;
	local int i;
	local float DBNODistance, DistFadePercent, PulseInterval;

	if ( PC.DBNOTeammates.Length == 0 )
		return;

	PRI = GearPRI(PC.PlayerReplicationInfo);

	if (PRI != None && PRI.Team != None)
	{
		for ( i=0; i<PC.DBNOTeammates.Length; i++ )
		{
			AWPawn = PC.DBNOTeammates[i];
			if( WP != AWPawn )
			{
				if( AWPawn.IsAHostage() )
				{
					`log(WorldInfo.TimeSeconds@"removing DBNO teammate hostage:"@`showvar(AWPawn));
					PC.DBNOTeammates.Remove(i--,1);
					continue;
				}
				if ( ShouldDrawDBNOIndicator(AWPawn) )
				{
					if ( AWPawn.IsInState('Dying') && (AWPawn.TimeOfDeath+1.5f < WorldInfo.TimeSeconds) )
					{
						Alpha = 255 * ((0.5f-(WorldInfo.TimeSeconds-AWPawn.TimeOfDeath-1.5f)) / 0.5f);
					}
					else if ( (AWPawn.TimeOfRevival > WorldInfo.TimeSeconds-2.f) && (AWPawn.TimeOfRevival < WorldInfo.TimeSeconds-1.5f) )
					{
						Alpha = 255 * ((0.5f-(WorldInfo.TimeSeconds-AWPawn.TimeOfRevival-1.5f)) / 0.5f);
					}
					else
					{
						Alpha = 255;
					}

					// Fade the icon depending on it's distance.
					DBNODistance = VSize( AWPawn.Location - WP.Location );
					if ( DBNODistance >= DBNOMaxFadeOpacityDistance )
					{
						DistFadePercent = DBNOMaxFadeOpacity;
					}
					else if ( DBNODistance > DBNOFullOpacityDistance )
					{
						DistFadePercent = 1.0f - ((DBNODistance - DBNOFullOpacityDistance) / (DBNOMaxFadeOpacityDistance - DBNOFullOpacityDistance));
						if ( DistFadePercent < DBNOMaxFadeOpacity )
						{
							DistFadePercent = DBNOMaxFadeOpacity;
						}
					}
					else
					{
						// add a slight pulsing for nearby players
						if (DBNODistance < 384.f)
						{
							PulseInterval = (WorldInfo.TimeSeconds - FFloor(WorldInfo.TimeSeconds));
							PulseInterval = 0.6f * (PulseInterval > 0.5 ? (1 - PulseInterval) : PulseInterval);
							DistFadePercent = 0.7 + PulseInterval;
						}
						else
						{
							DistFadePercent = 1.f;
						}
					}

					Alpha = byte(float(Alpha) * DistFadePercent);
					DrawSquadMate( AWPawn, Canvas.ClipX - SafeZoneRight - AssessSquadCoords[CurrSplitscreenType].Coords[Count].XFromTopRight, SafeZoneTop + AssessSquadCoords[CurrSplitscreenType].Coords[Count].YFromTopRight, (CurrSplitscreenType == eSST_None)?1.f:0.68f, Alpha, TRUE );
					Count++;
				}
				else
				{
					`log(WorldInfo.TimeSeconds@"removing DBNO teammate:"@`showvar(AWPawn)@`showvar(AWPawn.IsDBNO())@`showvar(AWPawn.Health));
					// remove from DBNO list
					PC.DBNOTeammates.Remove(i--, 1);
				}
			}
		}
	}
}

/**
 * Overridden to clear any transient data on player death.
 */
function PlayerOwnerDied()
{
	Super.PlayerOwnerDied();
	// clear any actions
	bUnfriendlySpotted = FALSE;
	ActiveAction.bActive = FALSE;
	ActiveAction.BlendPct = 0.f;
	PreviousAction.bActive = FALSE;
	PreviousAction.BlendPct = 0.f;
}

/** utility for setting HUDMessage "killer" and "victim" properties from the PRIs */
final function SetMessageKillerAndVictim(int HUDMessageIndex, PlayerReplicationInfo Killer, PlayerReplicationInfo Victim)
{
	if (Killer != None)
	{
		HUDMessages[HUDMessageIndex].KillerName = Killer.PlayerName$" ";
		HUDMessages[HUDMessageIndex].KillerTeamIndex = Killer.GetTeamNum();
		HUDMessages[HUDMessageIndex].bKillerDrawTeamColor = ShouldDrawTeamColor(Killer);
		HUDMessages[HUDMessageIndex].bKillerIsCOG = IsCOG(Killer);
	}
	if (Victim != None)
	{
		HUDMessages[HUDMessageIndex].VictimName = " "$Victim.PlayerName$" ";
		HUDMessages[HUDMessageIndex].VictimTeamIndex = Victim.GetTeamNum();
		HUDMessages[HUDMessageIndex].bVictimDrawTeamColor = ShouldDrawTeamColor(Victim);
		HUDMessages[HUDMessageIndex].bVictimIsCOG = IsCOG(Victim);
	}
}

/** @see AddTeamMessageData **/
final function AddDeathMessageData( PlayerReplicationInfo Killer, PlayerReplicationInfo Victim, class<GearDamageType> GearDamage, EGearDeathType DeathType, optional int PointsAwarded, optional bool bSkipBlood )
{
	local int index;

	if ( bInMultiplayerMode )
	{
		if ( Victim != None )
		{
			index = HUDMessages.length;
			HUDMessages.length = index + 1;

			if( ++CurrBloodIndex > (BLOOD_ICON_AMT-1) )
			{
				CurrBloodIndex = 0;
			}
			HUDMessages[index].BloodIconIndex = CurrBloodIndex;
			HUDMessages[index].DeathType = DeathType;
			SetMessageKillerAndVictim(index, Killer, Victim);
			HUDMessages[index].TimeRemaining = DeathDataTime;
			HUDMessages[index].Damage = GearDamage;
			if (GearDamage == class'GDT_ShieldBash')
			{
				HUDMessages[index].Damage = class'GDT_Melee';
			}
			HUDMessages[index].bShowBlood = !bSkipBlood;
			HUDMessages[index].bUseTeamColors = TRUE;
			if (PointsAwarded > 0)
			{
				HUDMessages[index].bUseMessageText = TRUE;
				HUDMessages[index].MessageText = "+"@PointsAwarded;
			}
			// always show player names for kills
			HUDMessages[index].bShowPlayerNames = TRUE;
		}
	}
}

/** @see AddDeathData **/
final function AddWeaponTakenMessage( PlayerReplicationInfo WeaponTaker, class<GearDamageType> GearDamage )
{
	local int index;

	if ( bInMultiplayerMode )
	{
		index = HUDMessages.length;
		HUDMessages.length = index + 1;

		SetMessageKillerAndVictim(index, WeaponTaker, None);
		HUDMessages[index].bShowPlayerNames = TRUE;
		HUDMessages[index].TimeRemaining = DeathDataTime;
		HUDMessages[index].Damage = GearDamage;
		if (GearDamage != None)
		{
			HUDMessages[index].Scale = GearDamage.default.IconScale;
		}
		HUDMessages[index].bShowBlood = FALSE;
		HUDMessages[index].bUseMessageText = TRUE;
		HUDMessages[index].MessageText = WeaponTakenString;
		HUDMessages[index].bUseTeamColors = TRUE;
	}
}

final function AddScoreGameMessage(int Id, int Pts, optional GearPRI Victim)
{
	local int index;
	if ( bInMultiplayerMode )
	{
		index = HUDMessages.length;
		HUDMessages.length = index + 1;

		SetMessageKillerAndVictim(index, PlayerOwner.PlayerReplicationInfo, Victim);
		HUDMessages[index].bShowPlayerNames = TRUE;
		HUDMessages[index].TimeRemaining = DeathDataTime;
		HUDMessages[index].Damage = None;
		HUDMessages[index].GameSpecificID = Id;
		HUDMessages[index].bShowBlood = FALSE;
		if (Pts > 0)
		{
			HUDMessages[index].bUseMessageText = TRUE;
			HUDMessages[index].MessageText = "+"@Pts;
		}
		HUDMessages[index].bUseTeamColors = TRUE;
	}
}

/** @see AddDeathData **/
final function AddAmmoMessage( PlayerReplicationInfo AmmoTaker, class<GearDamageType> GearDamage, int Amount )
{
	local int index;
	local string AmmoText;

	index = HUDMessages.length;
	HUDMessages.length = index + 1;

	SetMessageKillerAndVictim(index, AmmoTaker, None);
	HUDMessages[index].TimeRemaining = AmmoDataTime;
	HUDMessages[index].Damage = GearDamage;
	HUDMessages[index].bShowBlood = FALSE;
	HUDMessages[index].bUseMessageText = TRUE;
	if ( Amount > 0 )
	{
		AmmoText = "x " $ Amount;
	}
	HUDMessages[index].MessageText = AmmoText;
}

/** @see AddDeathData **/
final function AddPlayerJoinMessage( PlayerReplicationInfo Player, String JoinMessage )
{
	local int index;
	local GearGRI GGRI;

	GGRI = GearGRI(WorldInfo.GRI);

	if (!WorldInfo.static.IsMenuLevel())
	{
		// Added gamestatus check in MP because the server thinks the player left them game during travel
		if (!bInMultiplayerMode || (GGRI != None && GGRI.GameStatus >= GS_RoundInProgress))
		{
			index = HUDMessages.length;
			HUDMessages.length = index + 1;
			SetMessageKillerAndVictim(index, Player, None);
			HUDMessages[index].TimeRemaining = DeathDataTime;
			HUDMessages[index].Damage = None;
			HUDMessages[index].bShowBlood = FALSE;
			HUDMessages[index].bUseMessageText = TRUE;
			HUDMessages[index].MessageText = JoinMessage;
			HUDMessages[index].bAlwaysShow = TRUE;
		}
	}
}

/** @see AddDeathData **/
final function AddAnnexMessage( PlayerReplicationInfo Player, String AnnexMessage )
{
	local int index;

	index = HUDMessages.length;
	HUDMessages.length = index + 1;

	SetMessageKillerAndVictim(index, Player, None);
	HUDMessages[index].TimeRemaining = DeathDataTime;
	HUDMessages[index].Damage = None;
	HUDMessages[index].bShowBlood = FALSE;
	HUDMessages[index].bUseMessageText = TRUE;
	HUDMessages[index].MessageText = AnnexMessage;
	HUDMessages[index].bAlwaysShow = TRUE;
	HUDMessages[index].bUseTeamColors = TRUE;
}

/** Is the loading or checkpoint currently drawing? */
final function bool LoadingTextIsDrawing()
{
	local GearEngine Engine;

	Engine = GearEngine(GearPC(Owner).Player.Outer);

	if ( WorldInfo.bRequestedBlockOnAsyncLoading ||
		 (Engine != None && Engine.PendingCheckpointAction == Checkpoint_Save) ||
		 ((TimeOfLastCheckpoint + MinTimeBeforeFadeForCheckpoint + FadeTimeForCheckpoint > WorldInfo.TimeSeconds) && (TimeOfLastCheckpoint != -1)) )
	{
		return TRUE;
	}
	return FALSE;
}

/** Draw the loading text */
final function bool DrawLoadingText()
{
	local float FontCalcX, FontCalcY, SubTitleW, SubTitleH, StartY, DrawX, DrawY;
	local GearEngine Engine;
	local String StringToDraw;
	local byte Alpha;

	if ( LoadingTextIsDrawing() )
	{
		Alpha = 255;
		Engine = GearEngine(GearPC(Owner).Player.Outer);
		if ( Engine != None && Engine.PendingCheckpointAction == Checkpoint_Save )
		{
			TimeOfLastCheckpoint = WorldInfo.TimeSeconds;

			// only draw the checkpoint text if the user is not saving to disk; when saving to disk we show a UI scene with an
			// animated cog material and different text
			if ( !Engine.AreStorageWritesAllowed() )
			{
				StringToDraw = CheckpointReachedString;
			}
		}
		else if ( (TimeOfLastCheckpoint + MinTimeBeforeFadeForCheckpoint + FadeTimeForCheckpoint > WorldInfo.TimeSeconds) &&
			      (TimeOfLastCheckpoint != -1) )
		{
			// only draw the checkpoint text if the user is not saving to disk; when saving to disk we show a UI scene with an
			// animated cog material and different text
			if (WorldInfo.NetMode == NM_Client || !Engine.AreStorageWritesAllowed())
			{
				StringToDraw = CheckpointReachedString;
			}

			if ( TimeOfLastCheckpoint + MinTimeBeforeFadeForCheckpoint < WorldInfo.TimeSeconds )
			{
				Alpha = 255 * (1.0f - ((WorldInfo.TimeSeconds - TimeOfLastCheckpoint + MinTimeBeforeFadeForCheckpoint) / FadeTimeForCheckpoint));
			}
		}
		else
		{
			StringToDraw = LoadingString;
		}

		if ( StringToDraw != "" )
		{
			GetSubtitleSize( SubTitleW, SubTitleH );
			StartY = SafeZoneBottom + SubTitleH;

			Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro24);
			Canvas.TextSize( StringToDraw, FontCalcX, FontCalcY );
			DrawX = SafeZoneLeft;
			DrawY = Canvas.ClipY - StartY - FontCalcY;
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			DrawObjectiveBack(Texture2D'Warfare_HUD.WarfareHUD_main', 156, 29, 21, 19, DrawX-8, DrawY-2, FontCalcX+16, FontCalcY+4 );

			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
			Canvas.SetPos( DrawX, DrawY );
			Canvas.DrawText( StringToDraw );
		}

		return TRUE;
	}

	return FALSE;
}

/** Whether the PRI is COG or Locust */
final function bool IsCOG( PlayerReplicationInfo PRI )
{
	return (class'GearUISceneMP_Base'.static.GetRaceUsingPRIOnly(GearPRI(PRI)) == eGEARRACE_COG);
}

/** Whether we should draw the team color of the player */
function bool ShouldDrawTeamColor( PlayerReplicationInfo OtherPRI )
{
	return TRUE;
}

final function DrawHUDMessages(float DeltaTime)
{
	local float DrawXStart, DrawX, DrawY, XL, YL, TotalVerticalSpace, FontCalcX, FontCalcY, DamTypeHeight, TextYOffset, DrawOffsetY;
	local int index, maxMessages, lastMessageIndex, PlayerIndex;
	local byte Alpha;
    local CanvasIcon DeathIcon;
	local GearPC PC;

	PC = GearPC(PlayerOwner);

	// don't draw messages if we are displaying checkpoints
	if ( LoadingTextIsDrawing() || HUDMessages.Length == 0)
	{
		return;
	}

	// update death data
	for ( index = 0; index < HUDMessages.length; index++ )
	{
		HUDMessages[index].TimeRemaining -= DeltaTime;
		if ( HUDMessages[index].TimeRemaining <= 0.f )
		{
			HUDMessages.Remove(index--, 1);
		}
	}

	Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Euro20);
	Canvas.TextSize( "W", FontCalcX, FontCalcY );

	DrawX = SafeZoneLeft;
	DrawY = Canvas.ClipY - SafeZoneBottom - FontCalcY;
	if (PC.IsSpectating() && PC.IsSplitscreenPlayer(PlayerIndex) && PlayerIndex == 1)
	{
		DrawY -= 20.f;
	}
	DrawXStart = DrawX;

	// calculate how many death messages we can show
	TotalVerticalSpace = Canvas.ClipY - SafeZoneTop - SafeZoneBottom;
	if ( FontCalcY != 0.f )
	{
		maxMessages = (TotalVerticalSpace * HUDMessagesScreenPercent) / FontCalcY;
	}
	lastMessageIndex = (HUDMessages.length <= maxMessages) ? 0 : (HUDMessages.length - maxMessages);

	for( index = HUDMessages.length - 1; index >= lastMessageIndex; index-- )
	{
		DeathIcon.Texture = None;
		// see if we should show this message or not
		if ( (bShowHud && (bInMultiplayerMode || !GearPC(Owner).IsDead())) || HUDMessages[index].bAlwaysShow )
		{
			Alpha = byte((HUDMessages[index].TimeRemaining > DeathDataFadeTime) ? 255.f : 255.f * (HUDMessages[index].TimeRemaining/DeathDataFadeTime));
			if ( HUDMessages[index].Damage != None )
			{
				if (HUDMessages[index].DeathType == GDT_HEADSHOT)
				{
					DeathIcon = class'GearDamageType'.default.HeadshotIcon;
				}
				else
				{
					DeathIcon = HUDMessages[index].Damage.default.KilledByIcon;
				}
				DamTypeHeight = DeathIcon.VL * HUDMessages[index].Scale;
			}
			else if (HUDMessages[index].GameSpecificID != -1 && (HUDMessages[index].GameSpecificID < GameSpecificIcon.Length || HUDMessages[index].GameSpecificID == 69))
			{
				// hey sexy!
				if (HUDMessages[index].GameSpecificID == 69)
				{
					DeathIcon = RevivedIcon;
				}
				else
				{
					DeathIcon = GameSpecificIcon[HUDMessages[index].GameSpecificID];
				}
				DamTypeHeight = DeathIcon.VL;
			}
			else
			{
				DamTypeHeight = FontCalcY;
			}

			TextYOffset = fmax((DamTypeHeight/2 - FontCalcY/2), (FontCalcY/2 - DamTypeHeight/2));

			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );

			// draw the killer name
			if (HUDMessages[index].bShowPlayerNames)
			{
				if (HUDMessages[index].bUseTeamColors && HUDMessages[index].bKillerDrawTeamColor && HUDMessages[index].KillerTeamIndex != 255)
				{
					if (HUDMessages[index].bKillerIsCOG)
					{
						SetHUDDrawColor( eWARHUDCOLOR_TEAMBLUE, Alpha );
					}
					else
					{
						SetHUDDrawColor( eWARHUDCOLOR_RED, Alpha );
					}
				}
				else
				{
					//`log(`showvar(HUDMessages[index].bUseTeamColors)@`showvar(HUDMessages[index].bKillerDrawTeamColor)@`showvar(HUDMessages[index].KillerTeamIndex));
					SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
				}

				Canvas.TextSize( HUDMessages[index].KillerName, XL, YL );
				if ( HUDMessages[index].Damage != None )
				{
					DrawOffsetY = DeathIcon.VL * HUDMessages[index].Scale;
				}
				else
				{
					DrawOffsetY = YL;
				}
				Canvas.SetPos( DrawX,DrawY+TextYOffset-DrawOffsetY );
				Canvas.DrawText( HUDMessages[index].KillerName );

				SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
				DrawX += XL;
			}
			else
			{
				if ( HUDMessages[index].Damage != None )
				{
					DrawOffsetY = DeathIcon.VL * HUDMessages[index].Scale;
				}
				else
				{
					DrawOffsetY = FontCalcY;
				}
			}

			// draw backdrop blood
			if( HUDMessages[index].bShowBlood )
			{
				Canvas.SetPos( DrawX,DrawY-15-DrawOffsetY );

				// draw blood tile here
				Canvas.DrawIcon(BloodIcons[HUDMessages[index].BloodIconindex],DrawX,DrawY-15-DrawOffsetY);
			}

			if ( HUDMessages[index].Damage != None || DeathIcon.Texture != None )
			{
				// mega hack!
				Canvas.DrawIcon(DeathIcon,DrawX,DrawY-DrawOffsetY,HUDMessages[index].Scale);
				DrawX += DeathIcon.UL * HUDMessages[index].Scale;
			}

			// draw the victim name
			if (HUDMessages[index].bShowPlayerNames)
			{
				if (HUDMessages[index].bUseTeamColors && HUDMessages[index].bVictimDrawTeamColor && HUDMessages[index].VictimTeamIndex != 255)
				{
					if (HUDMessages[index].bVictimIsCOG)
					{
						SetHUDDrawColor( eWARHUDCOLOR_TEAMBLUE, Alpha );
					}
					else
					{
						SetHUDDrawColor( eWARHUDCOLOR_TEAMRED, Alpha );
					}
				}
				else
				{
					SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
				}

				Canvas.SetPos( DrawX,DrawY+TextYOffset-DrawOffsetY );
				Canvas.DrawText( HUDMessages[index].VictimName );
				if (HUDMessages[index].bUseMessageText)
				{
					Canvas.TextSize( HUDMessages[index].VictimName, XL, YL );
					DrawX += XL;
				}

				SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
			}

			if( HUDMessages[index].bUseMessageText )
			{
				Canvas.SetPos( DrawX,DrawY+TextYOffset-DrawOffsetY );
				Canvas.DrawText( HUDMessages[index].MessageText );
			}

			// move the Draw Pos to the next starting position
			DrawX = DrawXStart;
			DrawY -= FontCalcY * 1.5f;
		}
	}
}

function PlayCinematic()
{
	bPlayCinematic = TRUE;
	CinematicTextDisplayStartTime = WorldInfo.TimeSeconds;
}

function DrawTileSplit(Texture2D Tex, int TU, int TV, int TUL, int TVL, int X, int Y, int XL, int YL, int UL, int VL)
{
	// top lefth
	Canvas.SetPos(X,Y);
	Canvas.DrawTile(Tex,UL,VL,TU,TV,UL,VL);
	// top right
	Canvas.SetPos(X + XL - UL,Y);
	Canvas.DrawTile(Tex,UL,VL,TU+TUL-UL,TV,UL,VL);
	// bottom left
	Canvas.SetPos(X,Y + YL - VL);
	Canvas.DrawTile(Tex,UL,VL,TU,TV+TVL-VL,UL,VL);
	// bottom right
	Canvas.SetPos(X + XL - UL, Y + YL - VL);
	Canvas.DrawTile(Tex,UL,VL,TU+TUL-UL,TV+TVL-VL,UL,VL);
	// top
	Canvas.SetPos(X + UL,Y);
	Canvas.DrawTile(Tex,XL - (UL*2),VL,TU+UL,TV,TUL - (UL*2),VL);
	// bottom
	Canvas.SetPos(X + UL,Y + YL - VL);
	Canvas.DrawTile(Tex,XL - (UL*2),VL,TU+UL,TV+TVL-VL,TUL - (UL*2),TVL - VL);
	// middle
	Canvas.SetPos(X,Y + VL);
	//Canvas.DrawTile(Tex,XL,YL - (VL*2),TU+2,TV+VL,TUL-4,TVL - (VL*2));
	Canvas.DrawTile(Tex,XL,YL - (VL*2),TU,TV+VL,TUL,TVL - (VL*2));
}

function DrawObjectiveBack(Texture Tex, int TU, int TV, int TUL, int TVL, int X, int Y, int XL, int YL)
{
	local float ScaleY;
	ScaleY = float(YL) / float(TVL);

	// left
	Canvas.SetPos(X,Y);
	Canvas.DrawTile(Tex,TUL/2,TVL*ScaleY,TU,TV,TUL/2,TVL);
	// right
	Canvas.SetPos(X+XL-TUL/2,Y);
	Canvas.DrawTile(Tex,TUL/2,TVL*ScaleY,TU+TUL/2,TV,TUL/2,TVL);
	// middle
	Canvas.SetPos(X+TUL/2,Y);
	Canvas.DrawTile(Tex,XL-TUL+1,TVL*ScaleY,TU+TUL/2,TV,1,TVL);
}

/**
* Get the x position of the countdown timer in the hud - extracted from the UpdateCountdown() function
* so that the objective code can consider it to avoid overlapping.
*/
simulated function float GetCountdownXPosition()
{
	return (Canvas.ClipX/2 - GetCountdownWidth()/2);
}

/** Get the coundtown Y Position */
simulated function float GetCountdownYPosition()
{
	return SafeZoneTop;
}

simulated function float GetCountdownWidth()
{
	return 91.f;
}

simulated function float GetCountdownHeight()
{
	return 19.f;
}

/** Whether the countdown timer is drawing or not */
simulated function bool CountdownTimerIsShowing()
{
	return ( (CountdownEndTime > 0) || ((CountdownStopTime > 0) && (WorldInfo.TimeSeconds-CountdownStopTime < CountdownTotalFadeOutTime)) );
}

/** Create a time string */
simulated function String CreateTimeString( float TimeInSeconds )
{
	local float CurrTime, NumMinutes, NumSeconds;
	local String TimeString;

	CurrTime = TimeInSeconds;
	NumMinutes = int(CurrTime / 60.f);
	NumSeconds = int(CurrTime - NumMinutes*60.f);

	if ( NumMinutes <= 0 )
	{
		TimeString = "00:";
	}
	else if ( NumMinutes < 10.f )
	{
		TimeString = "0"$int(NumMinutes)$":";
	}
	else
	{
		TimeString = int(NumMinutes)$":";
	}

	if ( NumSeconds <= 0 )
	{
		TimeString = TimeString$"00";
	}
	else if ( NumSeconds < 10.f )
	{
		TimeString = TimeString$"0"$int(NumSeconds);
	}
	else
	{
		TimeString = TimeString$int(NumSeconds);
	}

	return TimeString;
}

// draw the countdown - return whether we drew the countdown or not
simulated function bool UpdateCountdown()
{
	local float TextWidth, TextHeight, FadePercent;
	local String TimeString;
	local GearGRI GGRI;
	local bool bDoingMPTimeExpiring;

	GGRI = GearGRI(WorldInfo.GRI);

	// See if we should start the countdown for an MP game expiring
	if ( bInMultiplayerMode &&
		 GGRI != None &&
		 !GGRI.bInfiniteRoundDuration &&
		 GGRI.RoundTime <= 60 )
	{
		if ( !CountdownTimerIsShowing() )
		{
			GearPC(PlayerOwner).StartCountdownTime( GGRI.RoundTime );
		}
		bDoingMPTimeExpiring = TRUE;
	}

	if ( CountdownTimerIsShowing() )
	{
		// Stop tutorial system if the timer just started
		if ( !bDrewCountdownLastFrame && GearGRI(WorldInfo.GRI).TrainingGroundsID > 0 )
		{
			GearPC(PlayerOwner).StopTutorialSystem();
		}
		bDrewCountdownLastFrame = TRUE;

		if ( bDoingMPTimeExpiring )
		{
			CountdownTimeToDraw = max(GGRI.RoundTime, 0.0f);
		}
		else if ( CountdownStopTime <= 0.0f )
		{
			CountdownTimeToDraw = max(CountdownEndTime - WorldInfo.TimeSeconds, 0.0f);
		}

		TimeString = CreateTimeString( CountdownTimeToDraw );

		if ( (WorldInfo.TimeSeconds - CountdownStartTime) < CountdownTotalFadeInTime )
		{
			FadePercent = (WorldInfo.TimeSeconds - CountdownStartTime) / CountdownTotalFadeInTime;
		}
		else if ( CountdownStopTime > 0.0f )
		{
			if ( (WorldInfo.TimeSeconds - CountdownStopTime) < CountdownTotalFadeOutTime )
			{
				FadePercent = 1.0f - ((WorldInfo.TimeSeconds - CountdownStopTime) / CountdownTotalFadeOutTime);
			}
			else
			{
				CountdownEndTime = 0.0f;
				CountdownStopTime = 0.0f;
				return FALSE;
			}
		}
		else
		{
			FadePercent = 1.0f;
		}

		SetHUDDrawColor( eWARHUDCOLOR_BLACK, byte(255.0f*FadePercent) );
		Canvas.SetPos( GetCountdownXPosition(), GetCountdownYPosition() );
		Canvas.DrawTile( CountdownBgdIcon.Texture, GetCountdownWidth(), GetCountdownHeight(), CountdownBgdIcon.U, CountdownBgdIcon.V, CountdownBgdIcon.UL, CountdownBgdIcon.VL );

		SetHUDDrawColor( eWARHUDCOLOR_WHITE, byte(255.0f*FadePercent) );
		Canvas.Font = Font'Warfare_HUD.WarfareHUD_font';
		Canvas.TextSize( TimeString, TextWidth, TextHeight );
		Canvas.SetPos( Canvas.ClipX/2 - TextWidth/2, SafeZoneTop + CountdownBgdIcon.VL/2 - TextHeight/2 + 1 );
		Canvas.DrawText( TimeString );

		return TRUE;
	}
	else if ( bDrewCountdownLastFrame )
	{
		// Restart the tutorial system if the timer just expired
		GearPC(PlayerOwner).RestartTutorialSystem( GearPC(PlayerOwner).bTutorialSystemWasOn );
		bDrewCountdownLastFrame = FALSE;
	}

	return FALSE;
}

/** Start the process of drawing the dramatic text */
simulated function StartDrawingDramaticText( String FirstString, String SecondString, float TotalDrawTime, float TotalFadeInTime, float TotalFadeOutTime, optional float StartDrawYPos = -1, optional float LineWaitTime = 1.0f, optional float LineFadeTime = 1.0f )
{
	if (WorldInfo.GRI.IsMultiPlayerGame() && GearGRI(WorldInfo.GRI).TrainingGroundsID < 0)
	{
		// Stop the tutorial system since the dramatic text will overlap it
		GearPC(PlayerOwner).StopTutorialSystem();
	}
	TotalDramaticStringDrawTime = TotalDrawTime;
	TotalDramaticStringFadeInTime = TotalFadeInTime;
	TotalDramaticStringFadeOutTime = TotalFadeOutTime;
	DramaticStringDrawStartTime = WorldInfo.TimeSeconds;
	DramaticString1 = FirstString;
	DramaticString2 = SecondString;
	DramaticTextDrawYStart = StartDrawYPos;
	DramaticLineWaitTime = LineWaitTime;
	DramaticLineFadeTime = LineFadeTime;
}

/** Stop the process of drawing the dramatic string */
simulated function StopDrawingDramaticText()
{
	TotalDramaticStringDrawTime = 0.0f;
	DramaticStringDrawStartTime = 0.0f;

	if (WorldInfo.GRI.IsMultiPlayerGame())
	{
		// Start the tutorial system back up again
		GearPC(PlayerOwner).RestartTutorialSystem( GearPC(PlayerOwner).bTutorialSystemWasOn );
	}
}

/** Whether the dramatic text is drawing or not */
simulated function bool IsDramaticTextDrawing()
{
	return (DramaticStringDrawStartTime > 0.0f);
}

/** Update and draw the dramatic text */
simulated function bool UpdateDramaticText()
{
	local float DrawX, DrawY, FirstStringTextWidth, FirstStringTextHeight, SecondStringTextWidth, SecondStringTextHeight, AlphaPercent, LineAlphaPercent;
	local byte Alpha, LineAlpha;
	local GearPC PC;
	local bool bIsSplitscreen;
	local bool bIsMultiplayer;

	PC = GearPC(PlayerOwner);
	bIsSplitscreen = PC.IsSplitscreenPlayer();
	bIsMultiplayer = WorldInfo.GRI.IsMultiPlayerGame();

	if ( DramaticStringDrawStartTime > 0.0f )
	{
		// Stop drawing it.
		if ( (WorldInfo.TimeSeconds - DramaticStringDrawStartTime) > TotalDramaticStringDrawTime )
		{
			StopDrawingDramaticText();
			return FALSE;
		}

		// Fading in text
		if ( (WorldInfo.TimeSeconds - DramaticStringDrawStartTime) < TotalDramaticStringFadeInTime )
		{
			AlphaPercent = (WorldInfo.TimeSeconds - DramaticStringDrawStartTime) / TotalDramaticStringFadeInTime;
		}
		// Fading out text
		else if ( (WorldInfo.TimeSeconds - DramaticStringDrawStartTime) > (TotalDramaticStringDrawTime - TotalDramaticStringFadeOutTime) )
		{
			AlphaPercent = (TotalDramaticStringDrawTime - (WorldInfo.TimeSeconds - DramaticStringDrawStartTime)) / TotalDramaticStringFadeOutTime;
		}
		else
		{
			AlphaPercent = 1.0f;
		}

		Alpha = byte(255.0f * AlphaPercent);

		// Fading in lines
		if ( ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) <= DramaticLineWaitTime) || ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) >= (TotalDramaticStringDrawTime - DramaticLineWaitTime)) )
		{
			LineAlphaPercent = 0.0f;
		}
		else if ( ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) > DramaticLineWaitTime) && ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) < (DramaticLineWaitTime + DramaticLineFadeTime)) )
		{
			LineAlphaPercent = (WorldInfo.TimeSeconds - DramaticStringDrawStartTime - DramaticLineWaitTime) / DramaticLineFadeTime;
		}
		// Fading out lines
		else if ( ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) > (TotalDramaticStringDrawTime - DramaticLineWaitTime - DramaticLineFadeTime)) && ((WorldInfo.TimeSeconds - DramaticStringDrawStartTime) < (TotalDramaticStringDrawTime - DramaticLineWaitTime)) )
		{
			LineAlphaPercent = (TotalDramaticStringDrawTime - (WorldInfo.TimeSeconds - DramaticStringDrawStartTime) - DramaticLineWaitTime) / DramaticLineFadeTime;
		}
		else
		{
			LineAlphaPercent = 1.0f;
		}

		LineAlpha = byte(255.0f * LineAlphaPercent);

		// Draw the first string with black backdrop
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Chrom24);
		Canvas.TextSize( DramaticString1, FirstStringTextWidth, FirstStringTextHeight );

		DrawY = (DramaticTextDrawYStart == -1) ? Canvas.ClipY - SafeZoneBottom - 150.0f : DramaticTextDrawYStart;
		DrawX = Canvas.ClipX/2 - FirstStringTextWidth/2;
		SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
		Canvas.SetPos( DrawX+3, DrawY );
		Canvas.DrawText( DramaticString1 );
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		Canvas.SetPos( DrawX, DrawY-3 );
		Canvas.DrawText( DramaticString1 );

		// Draw the left line
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, LineAlpha );
		Canvas.SetPos( DrawX - 512, DrawY-3+(FirstStringTextHeight/2)-8 );
		// Don't show the lines in splitscreen MP
		if (!bIsSplitscreen || !bIsMultiplayer)
		{
			Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_TitleLineBreak', 512, 16, 0, 0, 512, 16);
		}
		// Draw the right line
		Canvas.SetPos( DrawX + FirstStringTextWidth, DrawY-3+(FirstStringTextHeight/2)-8 );
		// Don't show the lines in splitscreen MP
		if (!bIsSplitscreen || !bIsMultiplayer)
		{
			Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_TitleLineBreak', 512, 16, 512, 0, -512, 16);
		}

		// Draw the second string with black backdrop if there is one
		if ( DramaticString2 != "" )
		{
			Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Chrom20);
			Canvas.TextSize( DramaticString2, SecondStringTextWidth, SecondStringTextHeight );
			DrawY += FirstStringTextHeight;
			DrawX = Canvas.ClipX/2 - SecondStringTextWidth/2;
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			Canvas.SetPos( DrawX+3, DrawY );
			Canvas.DrawText( DramaticString2 );
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
			Canvas.SetPos( DrawX, DrawY-3 );
			Canvas.DrawText( DramaticString2 );

			// Draw the bottom line
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, LineAlpha );
			Canvas.SetPos( Canvas.ClipX/2 - 512, DrawY+SecondStringTextHeight );
			// Don't show the lines in splitscreen MP
			if (!bIsSplitscreen || !bIsMultiplayer)
			{
				Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_TitleLineBreak', 512, 16, 0, 0, 512, 16);
			}
			Canvas.SetPos( Canvas.ClipX/2, DrawY+SecondStringTextHeight );
			// Don't show the lines in splitscreen MP
			if (!bIsSplitscreen || !bIsMultiplayer)
			{
				Canvas.DrawTile( Texture2D'Warfare_HUD.HUD_TitleLineBreak', 512, 16, 512, 0, -512, 16);
			}
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** Start the process of drawing the subtitle text (not actually subtitles... just the location where subtitle WOULD be)*/
simulated function StartDrawingSubtitleText( String StringToDraw, float TotalDrawTime, float TotalFadeInTime, float TotalFadeOutTime )
{
	TotalSubtitleStringDrawTime = TotalDrawTime;
	TotalSubtitleStringFadeInTime = TotalFadeInTime;
	TotalSubtitleStringFadeOutTime = TotalFadeOutTime;
	SubtitleStringDrawStartTime = WorldInfo.TimeSeconds;
	SubtitleString = StringToDraw;
}

/** Stop the process of drawing the subtitle string */
simulated function StopDrawingSubtitleText()
{
	TotalSubtitleStringDrawTime = 0.0f;
	SubtitleStringDrawStartTime = 0.0f;
}

/** Update and draw the subtitle text */
simulated function UpdateSubtitleText()
{
	local float DrawX, DrawY, StringTextWidth, StringTextHeight, AlphaPercent;
	local float SubTitleW, SubTitleH;
	local byte Alpha;

	if ( SubtitleStringDrawStartTime > 0.0f )
	{
		// Stop drawing it.
		if ( (WorldInfo.TimeSeconds - SubtitleStringDrawStartTime) > TotalSubtitleStringDrawTime )
		{
			StopDrawingSubtitleText();
			return;
		}

		// Fading in text
		if ( (WorldInfo.TimeSeconds - SubtitleStringDrawStartTime) < TotalSubtitleStringFadeInTime )
		{
			AlphaPercent = (WorldInfo.TimeSeconds - SubtitleStringDrawStartTime) / TotalSubtitleStringFadeInTime;
		}
		// Fading out text
		else if ( (WorldInfo.TimeSeconds - SubtitleStringDrawStartTime) > (TotalSubtitleStringDrawTime - TotalSubtitleStringFadeOutTime) )
		{
			AlphaPercent = (TotalSubtitleStringDrawTime - (WorldInfo.TimeSeconds - SubtitleStringDrawStartTime)) / TotalSubtitleStringFadeOutTime;
		}
		else
		{
			AlphaPercent = 1.0f;
		}

		Alpha = byte(255.0f * AlphaPercent);

		GetSubtitleSize( SubTitleW, SubTitleH );

		// Draw the first string with black backdrop
		Canvas.Font = class'Engine'.static.GetAdditionalFont(FONT_Chrom24);
		Canvas.TextSize( SubtitleString, StringTextWidth, StringTextHeight );
		DrawY = Canvas.ClipY - SafeZoneBottom - StringTextHeight;
		DrawX = Canvas.ClipX/2 - StringTextWidth/2;
		SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
		Canvas.SetPos( DrawX+3, DrawY );
		Canvas.DrawText( SubtitleString );
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		Canvas.SetPos( DrawX, DrawY-3 );
		Canvas.DrawText( SubtitleString );
	}
}

/**
 * Called by GearWeaponPickupFactory when pickup attempt fails because bCanPickupFactoryWeapons==FALSE
 */
function WeaponFactoryPickupFailed();

/**
 * Subfunction for scoreboard control in children
 * bUseAtoContinueVersion - whether to show the "Press A to Continue" string or not... if not
 * then the "B Back" string shows instead.
 */

function ToggleScoreboard(bool bOn);

/** Used to clear refs that would break GC */
function ClearScoreboardRef();

/**
 * Allows for easy opening of the EndRound/Match menu
 */

function SignalEndofRoundOrMatch(GearGRI GRI);
function SignalStartofRoundOrMatch(GearGRI GRI);

/** Helper function to draw a string with a black background */
simulated function DrawStringWithBackground( float DrawX, float DrawY, String TextToDisplay, EGearHUDColor TextColor = eWARHUDCOLOR_WHITE )
{
	local float BackgroundOffset, TextSizeX, TextSizeY, TextDrawWidth;

	BackgroundOffset = 2.0f;

	// Background
	Canvas.TextSize( TextToDisplay, TextSizeX, TextSizeY );
	TextDrawWidth = TextSizeX+16;
	SetHUDDrawColor( eWARHUDCOLOR_BLACK, 200 );
	DrawObjectiveBack( Texture2D'Warfare_HUD.WarfareHUD_main', 156, 29, 21, 19, DrawX-8, DrawY-BackgroundOffset, TextDrawWidth, TextSizeY+2*BackgroundOffset );

	// Text
	Canvas.SetPos(DrawX,DrawY);
	SetHUDDrawColor( TextColor );
	Canvas.DrawText( TextToDisplay );
}

/** Stub for Annex HUD */
function SetCommandPointIndicatorGlow( int NumPulses, float PulseLength )
{
}

/** Stub for Annex HUD */
function DoEndOfGameWarning( int ScoreDiff, int TeamIndex )
{
}

`define		SCENEFILTER_PausersOnly		0x00000004
/**
* Callback so the pause system can know when to unpause the game
*
* @return TRUE if the pause can be removed, FALSE otherwise
*/
function bool CanUnpauseFromUIOpen()
{
	local UIInteraction UIController;
	local bool bResult;

	bResult = TRUE;

	// normally, the UI would prevent the game from being unpaused if any scenes which have bPauseGameWhileActive=TRUE are active.  Since
	// we override the default pause handling in order to apply post-process effects, we need to represent the UI's unpause logic here.
	UIController = PlayerOwner.GetUIController();
	if ( UIController != None )
	{
		bResult = !UIController.SceneClient.IsUIActive(`SCENEFILTER_PausersOnly);
	}

	return bResult;
}

/** Whether the show the hud or not based on screens showing in front of it */
function bool HideHUDFromScenes()
{
	return (GameoverUISceneInstance != none ||
			PauseUISceneInstance != none);
}

/** Allow the HUD to close any scenes or cleanup any UI for when the pause screen is opened */
function CleanUIForPauseScreen();

/** Handles opening the pause screen */
function PauseGame( optional delegate<PlayerController.CanUnpause> CanUnpauseDelegate=CanUnpauseFromUIOpen )
{
	local GearPC MyGearPC;
	local delegate<PlayerController.CanUnpause> DefaultUnpauseDelegate;

	// never pause the game if we're in the menu level - this can happen if the player presses START to bypass the startup movie
	// since both the rendering and game thread will receive the input event
	if ( class'WorldInfo'.static.IsMenuLevel() )
	{
		return;
	}

	// If CurrentPlayerData is None then the PRI and GRI have not been replicated yet...
	MyGearPC = GearPC(PlayerOwner);
	if ( MyGearPC.GetCurrentPlayerData() == None )
	{
		// If replication hasn't happened yet, and this is not an MP game, do not bring up the menu.
		if ( WorldInfo.GRI == None || !WorldInfo.GRI.IsMultiPlayerGame() )
		{
			return;
		}
	}

	// Only open the pause screen if it hasn't been opened already and we aren't showing the gameover scene
	if ( PauseUISceneInstance == None && GameoverUISceneInstance == None )
	{
		// Clean up any UI before opening the pause screen
		CleanUIForPauseScreen();

		PauseUISceneInstance = GearUIScenePause_Base(MyGearPC.ClientOpenScene( GetPauseSceneReference() ));

		if ( PauseUISceneInstance != None )
		{
			GearPC(PlayerOwner).StartPostProcessOverride( eGPP_Pause, PauseUISceneInstance.GetSceneRenderMode() == SPLITRENDER_Fullscreen );

			// only server can pause.  Opening the pause scene should have already paused the game [because it has
			// bPauseGameWhileActive set] - this is only necessary in cases where we're adding additional CanUnpauseDelegate handlers
			// and the game is already paused.
			if ( WorldInfo.NetMode != NM_Client || WorldInfo.IsPlayingDemo() )
			{
				// but skip this step if we're already paused and the CanUnpauseDelegate is the GearPC's default one.
				DefaultUnpauseDelegate = MyGearPC.CanUnpause;
				if ( !MyGearPC.IsPaused() || CanUnpauseDelegate != DefaultUnpauseDelegate )
				{
					PlayerOwner.SetPause(TRUE,CanUnpauseDelegate);
				}
			}

			PauseUISceneInstance.OnSceneDeactivated = UnPauseGame;

			// Close scoreboard since pause screen is opened
			if ( (MyGearPC.MainPlayerInput != None) && MyGearPC.MainPlayerInput.IsButtonActive(GB_Back) )
			{
				MyGearPC.MainPlayerInput.ForceButtonRelease(GB_Back);
			}

			// turn subtitles off
			MyGearPC.SetShowSubtitles( FALSE );
		}
	}
}

function UnPauseGame( UIScene DeactivatedScene )
{
	local GearPC PC;

	GearUIScenePause_Base(DeactivatedScene).OnSceneDeactivatedCallback(DeactivatedScene);

	if ( PauseUISceneInstance != None )
	{
		if ( PauseUISceneInstance == DeactivatedScene || DeactivatedScene == None )
		{
			PauseUISceneInstance = None;
		}

		GearPC(PlayerOwner).StopPostProcessOverride( eGPP_Pause );

		// only server can unpause
		if (WorldInfo.NetMode != NM_Client)
		{
			PlayerOwner.SetPause(false);
			foreach WorldInfo.AllControllers(class'GearPC', PC)
			{
				if (LocalPlayer(PC.Player) == None)
				{
					PC.ClientUpdateHostPause(false);
				}
			}
		}

		// reset subtitles
		GearPC(PlayerOwner).RestoreShowSubtitles();
	}
}

/** Open the UI scene for a discoverable */
final function OpenDiscoverableScene()
{
	local GearPC MyGearPC;

	MyGearPC = GearPC(PlayerOwner);

	DiscoverUISceneInstance = GearUIScene_Discover(MyGearPC.ClientOpenScene( DiscoverUISceneReference ));
}

/** Close the UI scene for a discoverable */
final function CloseDiscoverableScene()
{
	local GearPC MyGearPC;

	MyGearPC = GearPC(PlayerOwner);

	if ( DiscoverUISceneInstance != None )
	{
		MyGearPC.ClientCloseScene( DiscoverUISceneInstance );
	}
}

/** Delegate for when the discoverable UI scene is closed */
final function OnDiscoverableSceneDeactivated( UIScene DeactivatedScene )
{
	DiscoverUISceneInstance = None;
}

/** shows the game over screen */
function ShowGameOverScreen()
{
	local GearPC MyGearPC;

	//@fixme - remove before ship, trying to track down errant failures
	ScriptTrace();

	// never pause the game if we're in the menu level - this can happen if the player presses START to bypass the startup movie
	// since both the rendering and game thread will receive the input event
	if ( class'WorldInfo'.static.IsMenuLevel() || PlayerOwner == None || !PlayerOwner.IsPrimaryPlayer() )
	{
		return;
	}

	// If CurrentPlayerData is None then the PRI and GRI have not been replicated yet...
	if ( GearPC(PlayerOwner).GetCurrentPlayerData() == None )
	{
		// If replication hasn't happened yet, and this is not an MP game, do not bring up the menu.
		if ( WorldInfo.GRI == None || !WorldInfo.GRI.IsMultiPlayerGame() )
		{
			return;
		}
	}

	ActiveAction.bActive = FALSE;

	// Only open the pause screen if it hasn't been opened already
	if ( GameoverUISceneInstance == None )
	{
		// Clean up any UI before opening the pause screen
		CleanUIForPauseScreen();

		MyGearPC = GearPC(PlayerOwner);
		GameoverUISceneInstance = GearUIScene_Base(MyGearPC.ClientOpenScene( GetGameoverSceneReference() ));

		if ( GameoverUISceneInstance != None )
		{
			// Setup the deactivate delegate so we can null the reference
			GameoverUISceneInstance.OnSceneDeactivated = OnGameOverSceneDeactivatedCallback;

			// only server can pause
			if (WorldInfo.NetMode != NM_Client)
			{
				PlayerOwner.SetPause(TRUE,CanUnpauseFromUIOpen);
			}

			// turn subtitles off
			GearPC(PlayerOwner).SetShowSubtitles( FALSE );
		}
	}
}

/**
 * Called when the gameover scene is closed so we can null the reference
 */
function OnGameOverSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	GearUIScenePause_Gameover(GameoverUISceneInstance).OnSceneDeactivatedCallback(DeactivatedScene);
	GameoverUISceneInstance = none;
}

/** Get the scene reference for gameover */
function GearUIScene_Base GetGameoverSceneReference()
{
	return GameoverUISceneReference;
}

/** Get the scene reference for pausing */
function GearUIScenePause_Base GetPauseSceneReference()
{
	return (WorldInfo.GRI.IsMultiPlayerGame() ? PauseMPUISceneReference : PauseCampaignUISceneReference);
}

/** Message from the game object that a leader has died */
function StartLeaderDiedMessage( GearPRI LeaderPRI );

/** Message from the game object that my buddy has died */
function StartBuddyDiedMessage( GearPRI BuddyPRI );

/** Allow HUD to handle a pawn being spotted when targetting */
function HandlePawnTargetted( GearPawn TargetPawn );

/** Function that can be overloaded by game-specific HUDs to draw icons above the heads of players */
function DrawSpecialPlayerIcons();

/** Determine if we should open or close the spectator screen */
function CheckSpectatorScene()
{
	local GearPC MyGearPC;
	local bool bWantsSpectatorScene;

	if (WorldInfo.GRI != None && GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress)
	{
		MyGearPC = GearPC(PlayerOwner);
		bWantsSpectatorScene = MyGearPC.WantsSpectatorUI();
		if (bWantsSpectatorScene && SpectatorUISceneInstance == None)
		{
			SpectatorUISceneInstance = GearUIScene_Base(MyGearPC.ClientOpenScene( SpectatorUISceneReference, 1 ));
			GearPC(PlayerOwner).RefreshAllSafeZoneViewports();
		}
		else if (!bWantsSpectatorScene && SpectatorUISceneInstance != None)
		{
			MyGearPC.ClientCloseScene( SpectatorUISceneInstance );
			SpectatorUISceneInstance = None;
			GearPC(PlayerOwner).RefreshAllSafeZoneViewports();
		}
	}
}

function CalculateDeadZone()
{
	local GameViewportClient VPClient;
	local bool bUseMaxSafeZone;
	local int LocalPlayerIndex;

	if (Owner == none)
	{
		`log("Owner isn't set to calculate DeadZone yet");
		return;
	}

	if (PlayerController(Owner).Player == none)
	{
		`log("Player isn't set to calculate DeadZone yet");
		return;
	}

	if (LocalPlayer(PlayerController(Owner).Player).ViewportClient == none)
	{
		`log("ViewportClient isn't set to calculate DeadZone yet");
		return;
	}

	if (Canvas == none)
	{
		`log("Canvas isn't set to calculate DeadZone yet");
		return;
	}

	// calculate the safezones and viewport centers
	VPClient = LocalPlayer(PlayerController(Owner).Player).ViewportClient;
	VPClient.UpdateActiveSplitscreenType();
	bUseMaxSafeZone = (VPClient.GetSplitscreenConfiguration() == eSST_None) ? FALSE : TRUE;

	// if no safe zone, zero all vars.
	if (VPClient.CalculateDeadZoneForAllSides( LocalPlayer(PlayerController(Owner).Player), Canvas, SafeZoneTop, SafeZoneBottomFromViewport, SafeZoneLeft, SafeZoneRight, bUseMaxSafeZone ) == false)
	{
		SafeZoneTop = 0.f;
		SafeZoneBottomFromViewport =  0.f;
		SafeZoneLeft = 0.f;
		SafeZoneRight = 0.f;
	}
	else
	{
		LocalPlayerIndex = VPClient.ConvertLocalPlayerToGamePlayerIndex(LocalPlayer(PlayerController(Owner).Player));
		if (LocalPlayerIndex > 0)
		{
			SafeZoneTop += 3.0f;
		}
	}

	SafeZoneBottomFromViewport = SafeZoneBottomFromViewport + (bUseMaxSafeZone?5.f:0.f);
	SafeZoneBottom = SafeZoneBottomFromViewport;
	bRefreshSafeZone = false;
}

function AttemptHordeModOpen();

defaultproperties
{

	HitLocatorMaterial=Material'Warfare_HUD.HUD_DamageDir_Mat'
	ActiveReloadPulseMaterial=Material'Warfare_HUD.HUD.M_HUD_Throb'
	SuperSweetReloadPulseMaterial=Material'Warfare_HUD.HUD.M_HUD_Throb'
	SniperOverlayMaterial=Material'Warfare_HUD.HUD_Sniper_overlay'

	WeaponInfoFrame=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=346,V=290,UL=163,VL=46)

	HealthIcons(0)=(Texture=Texture2D'Warfare_HUD.Warfare-HUD-health-texture2',U=0,V=0,UL=177,VL=158)
	HealthIconsXOffset(0)=0.f
	HealthIcons(1)=(Texture=Texture2D'Warfare_HUD.Warfare-HUD-health-texture2',U=177,V=0,UL=242,VL=158)
	HealthIconsXOffset(1)=7.f
	HealthIcons(2)=(Texture=Texture2D'Warfare_HUD.Warfare-HUD-health-texture2',U=177,V=158,UL=335,VL=158)
	HealthIconsXOffset(2)=15.f

	HealthIcons(3)=(Texture=Texture2D'Warfare_HUD.Warfare-HUD-health-texture2',U=0,V=316,UL=455,VL=196)
	HealthIconsXOffset(3)=-20.f

	DeathDataTime=5.5f
	DeathDataFadeTime=0.5f

	AmmoDataTime=5.5f
	AmmoDataFadeTime=0.5f
	LastWeaponInfoTime=-9999.f

	SquadBackgrounds(EASBT_Normal)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=398,V=397,UL=114,VL=114)
	SquadBackgrounds(EASBT_Dead)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=398,V=397,UL=114,VL=114)
	SquadBackgroundNamePlate=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=283,V=397,UL=114,VL=114)

	SquadDisplayData=(NameYFromCenter=8.f,OrderYFromCenter=23.f,HeadYFromTopOfBackground=6.f)

	AssessSquadCoords={(
		(Coords=((XFromTopRight=250,YFromTopRight=89),		// eSST_None
				 (XFromTopRight=139,YFromTopRight=197),
				 (XFromTopRight=84,YFromTopRight=337),
				 (XFromTopRight=139,YFromTopRight=478))),
		(Coords=((XFromTopRight=316,YFromTopRight=62),		// eSST_2P_HORIZONTAL
				 (XFromTopRight=204,YFromTopRight=89),
				 (XFromTopRight=105,YFromTopRight=141),
				 (XFromTopRight=55,YFromTopRight=232))),
		(Coords=((XFromTopRight=55,YFromTopRight=117),		// eSST_2P_VERTICAL
				 (XFromTopRight=55,YFromTopRight=220),
				 (XFromTopRight=55,YFromTopRight=323),
				 (XFromTopRight=55,YFromTopRight=426))),
		(Coords=((XFromTopRight=250,YFromTopRight=89),		// eSST_3P_FAVOR_TOP
				 (XFromTopRight=139,YFromTopRight=197),
				 (XFromTopRight=84,YFromTopRight=337),
				 (XFromTopRight=139,YFromTopRight=478))),
		(Coords=((XFromTopRight=250,YFromTopRight=89),		// eSST_3P_FAVOR_BOTTOM
				 (XFromTopRight=139,YFromTopRight=197),
				 (XFromTopRight=84,YFromTopRight=337),
				 (XFromTopRight=139,YFromTopRight=478))),
		(Coords=((XFromTopRight=250,YFromTopRight=89),		// eSST_4P
				 (XFromTopRight=139,YFromTopRight=197),
				 (XFromTopRight=84,YFromTopRight=337),
				 (XFromTopRight=139,YFromTopRight=478))),
	)}

	// @todo we need to support up to 7 places as omega man is 1 vs 7

	SquadStatusIcons(eSO_DEFENSIVE)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=158,V=152,UL=63,VL=32)
	SquadStatusIcons(eSO_ATTACK)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=158,V=119,UL=63,VL=32)
	SquadStatusIcons(eSO_DOWN)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=78,V=210,UL=42,VL=29)
	SquadStatusIcons(eSO_REVIVE)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=90,UL=38,VL=29)
	SquadStatusIcons(eSO_DEAD)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=156,V=2,UL=19,VL=26)

	SquadLocatorMaterial=Material'Warfare_HUD.HUD_LocationDir_Mat'

	PreviousFadeAlpha=0
	DesiredFadeAlpha=0
	FadeColor=(R=0,G=0,B=0)
	FadeAlphaDelay=0.f
	DesiredFadeAlphaTime=0.f
	FadeAlphaTime=0.f
	bDrawSniperZoom=FALSE


	BloodIcons(0)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=108,V=290,UL=59,VL=59)
	BloodIcons(1)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=167,V=290,UL=59,VL=59);
	BloodIcons(2)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=226,V=290,UL=59,VL=59);
	BloodIcons(3)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=285,V=290,UL=59,VL=59);

	CountdownBgdIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=164,V=198,UL=57,VL=18)

	DeadTaccomSplatMaterial=Material'Warfare_HUD.M_HUD_Taccom_Dead';
	ReviveTaccomIcon=(Texture=Texture2D'Warfare_HUD.HUD_Taccom_Revive',U=0,V=0,UL=256,VL=256)
	VoiceTaccomIcon=(Texture=Texture2D'UI_Art.FrontEnd.UI_Party_Chatbubble', U=0, V=0, UL=64, VL=32)
	VoiceTaccomIconPos[0]=64.0f
	VoiceTaccomIconPos[1]=62.0f

	MaxRednessOfDeathScreen = 1.f
	WeaponInfoScale=1.f
	CurrSplitscreenType=eSST_None

	WeaponSelect=(TotalNumBlinks=2,TotalBlinkTime=0.25f)

	TotalCrosshairFadeTime=0.25f
	TotalTaccomFadeTime=0.25f
	TotalCoopPauseFadeTime=0.25f
	TotalWeaponFadeTime=0.25f

	CountdownEndTime=0.f

	ARBarData={(
		BackGround=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=260,V=478,UL=200,VL=12),
		Flash=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=260,V=448,UL=200,VL=10),
		TickMark=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=447,V=406,UL=16,VL=33),
		SuccessRegion=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=442,V=386,UL=3,VL=6),
		PerfectGrade=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=450,V=387,UL=8,VL=16),
		GoodGrade=(Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=261,V=461,UL=96,VL=12),
		FlashDuration=1.0f,
		FadeDuration=0.5f,
	)}

	Begin Object Class=AudioComponent Name=TacComComp
	   SoundCue=SoundCue'Interface_Audio.Interface.TaccomLoopCue'
	End Object
	TacComLoop=TacComComp
	Components.Add(TacComComp);

	CountdownTotalFadeInTime=0.5f
	CountdownTotalFadeOutTime=1.0f
	NumSquadLocators=5

	LeaderIcon=(Texture=Texture2D'Warfare_HUD.HUD.UI_HUD_Leader',U=0,V=0,UL=32,VL=32)
	MeatflagIcon=(Texture=Texture2D'Warfare_HUD.UI_HUD_Meatflag',U=0,V=0,UL=32,VL=32)
	BuddyIconCOG=(Texture=Texture2D'Warfare_HUD.UI_HUD_WMCog',U=0,V=0,UL=64,VL=32)
	BuddyIconLocust=(Texture=Texture2D'Warfare_HUD.UI_HUD_WMLocust',U=0,V=0,UL=64,VL=32)

	bShowNamesWhenSpectating=TRUE

	MortarCrosshairTickIcon=(Texture=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Mortar_Tick',U=0,V=0,UL=128,VL=16)
	MortarCrosshairSaveIcon=(Texture=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Mortar_SavePoint',U=0,V=0,UL=128,VL=16)

	TimeOfLastCheckpoint=-1
	MinTimeBeforeFadeForCheckpoint=1.0f
	FadeTimeForCheckpoint=1.0f

	RevivedIcon=(Texture=Texture2D'Warfare_HUD.ScoreBoard.UI_MP_Icons',U=55,V=0,UL=25,VL=26)
}
