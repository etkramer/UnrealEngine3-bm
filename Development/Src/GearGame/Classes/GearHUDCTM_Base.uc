/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDCTM_Base extends GearHUDMP_Base;

var CanvasIcon	ShieldIcon;

/** Materials for drawing the main scoreboard */
var Material MainScoreMaterialCOG;
var Material MainScoreMaterialLOC;
var Material ScoreInvertMaterial;
/** Material Instances for drawing the main scoreboard */
var MaterialInstanceConstant MainScoreMatInstanceCOG;
var MaterialInstanceConstant MainScoreMatInstanceLOC;
var MaterialInstanceConstant ScoreInvertMatInstance;

/** Materials for drawing the Command Point Indicator */
var Material CPIndicatorMaterialBG;
var Material CPIndicatorMaterialCOG;
var Material CPIndicatorMaterialLOC;
var Material CPIndicatorGlow;
var Material CPIndicatorWeapons;

/** Material Instance for drawing the glow around the victim's head in taccom */
var Material VictimLocatorHalo;

/** Material Instances for drawing the Command Point Indicator */
var MaterialInstanceConstant CPIndicatorMatInstanceBG;
var MaterialInstanceConstant CPIndicatorMatInstanceCOG;
var MaterialInstanceConstant CPIndicatorMatInstanceLOC;
var MaterialInstanceConstant CPIndicatorGlowMatInstance;
var MaterialInstanceConstant CPIndicatorWeaponsMatInstance;

/** Material Instance for drawing the glow around the victim's head in taccom */
var MaterialInstanceConstant VictimLocatorHaloMatInstance;

/** Variables for controlling the pulsing effect in the Command Point Indicator */
var int NumPulsesToDisplay;
var float PulseStartTime;
var float TotalPulseTime;

/** Variables for controlling the pulsing effect in the taccom for showing the person who has the victim */
var float VictimPulseStartTime;
var float VictimTotalPulseTime;

/** Played when one team is about to win */
var SoundCue AnnexSound_TeamAboutToWin[2];
/** Variables for controlling the pulsing effect in the Scoreboard */
var float ScorePulseStartTime;
var float ScoreTotalPulseTime;

/** Time to blend into showing the meatflag icon */
var const float MeatflagIconBlendInTime;
/** Time to blend out of showing the meatflag icon */
var const float MeatflagIconBlendOutTime;

/** Pawn to show an icon on */
var GearPawn PawnToDrawIconOn;

/** drawn on the compass when the meatflag is DBNO */
var CanvasIcon MeatflagDownIcon;

/** Initialize some data */
simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// initialize the scoreboard
	if( MainScoreMatInstanceCOG == None )
	{
		MainScoreMatInstanceCOG = new(outer) class'MaterialInstanceConstant';
		MainScoreMatInstanceCOG.SetParent( MainScoreMaterialCOG );
	}
	if( MainScoreMatInstanceLOC == None )
	{
		MainScoreMatInstanceLOC = new(outer) class'MaterialInstanceConstant';
		MainScoreMatInstanceLOC.SetParent( MainScoreMaterialLOC );
	}

	// initialize the score invert
	if ( ScoreInvertMatInstance == None )
	{
		ScoreInvertMatInstance = new(outer) class'MaterialInstanceConstant';
		ScoreInvertMatInstance.SetParent( ScoreInvertMaterial );
	}

	// initialize the CP indicator background
	if ( CPIndicatorMatInstanceBG == None )
	{
		CPIndicatorMatInstanceBG = new(outer) class'MaterialInstanceConstant';
		CPIndicatorMatInstanceBG.SetParent( CPIndicatorMaterialBG );
	}

	// initialize the CP indicator COG material
	if ( CPIndicatorMatInstanceCOG == None )
	{
		CPIndicatorMatInstanceCOG = new(outer) class'MaterialInstanceConstant';
		CPIndicatorMatInstanceCOG.SetParent( CPIndicatorMaterialCOG );
	}

	// initialize the CP indicator Locust material
	if ( CPIndicatorMatInstanceLOC == None )
	{
		CPIndicatorMatInstanceLOC = new(outer) class'MaterialInstanceConstant';
		CPIndicatorMatInstanceLOC.SetParent( CPIndicatorMaterialLOC );
	}

	// initialize the CP indicator weapons material
	if ( CPIndicatorGlowMatInstance == None )
	{
		CPIndicatorGlowMatInstance = new(outer) class'MaterialInstanceConstant';
		CPIndicatorGlowMatInstance.SetParent( CPIndicatorGlow );
	}

	// initialize the CP indicator weapons material
	if ( CPIndicatorWeaponsMatInstance == None )
	{
		CPIndicatorWeaponsMatInstance = new(outer) class'MaterialInstanceConstant';
		CPIndicatorWeaponsMatInstance.SetParent( CPIndicatorWeapons );
	}

	// initialize the victim halo
	if ( VictimLocatorHaloMatInstance == None )
	{
		VictimLocatorHaloMatInstance = new(outer) class'MaterialInstanceConstant';
		VictimLocatorHaloMatInstance.SetParent( VictimLocatorHalo );
	}
}

/** Reset the HUD */
function Reset()
{
	Super.Reset();

	NumPulsesToDisplay = 0;
	PulseStartTime = 0;
	VictimPulseStartTime = 0;
	UpdateMeatflagSpotted( None );
}

/** Overloaded to clear the spotted meatflag */
function EnableAssessMode()
{
	Super.EnableAssessMode();

	UpdateMeatflagSpotted( None );
}

/** Overloaded Draw call */
function DrawHUD()
{
	local float DrawX, DrawY;

	Super.DrawHUD();

	if ( bShowHud &&
		!HideHUDFromScenes() &&
		(GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress) )
	{
		DrawX = SafeZoneLeft;
		DrawY = SafeZoneTop;

		// draw the command point indicator
		DrawCommandPointIndicator( DrawX, DrawY );
	}
}

/** Returns the type of special icon to draw in the squadmate UI */
function EGearSpecialIconType GetSpecialIconType( GearPawn PawnToTest )
{
	if( PawnToTest.IsAKidnapper() && IsMeatflagPawn(PawnToTest.InteractionPawn) )
	{
		return eGSIT_Flag;
	}

	return eGSIT_None;
}

/** Set the glow material to start pulsing */
function SetCommandPointIndicatorGlow( int NumPulses, float PulseLength )
{
	NumPulsesToDisplay = NumPulses;
	PulseStartTime = WorldInfo.TimeSeconds;
	TotalPulseTime = PulseLength;
}

/**
 * Draws the Command Point indicator which tells the player what/where the current command point is
 * NOTE: DrawX and DrawY is the center point of where this will draw.
 */
simulated function DrawCommandPointIndicator( out float DrawX, out float DrawY )
{
	local MaterialInstanceConstant CPControllerMIC;
	local float LocalDrawX, LocalDrawY, CPIWidth, CPIHeight, Scale, GlowOpacity, PulseScale, IconScale, WeapScale;
	local CanvasIcon Icon;
	local GearGRI GGRI;
	local Actor ActorToIndicate;
	local byte Alpha;
	local GearPC MyGearPC;
	local GearPawn MyGearPawn;
	local bool bShowDesination;
	local CanvasIcon WeaponIcon;

	GGRI = GearGRI(WorldInfo.GRI);
	Alpha = 255 * (1.0f - TaccomFadeOpacity);

	if ( (PlayerOwner.PlayerReplicationInfo != None) && (PlayerOwner.PlayerReplicationInfo.Team != None) && (GGRI != None) )
	{
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		Scale = 0.65f;
		CPIWidth = 128.f * Scale;
		CPIHeight = 128.f * Scale;

		// Original Y Position + Height of the Scoreboard texture + an additional spacing offset.
		DrawY = SafeZoneTop + (CPIHeight/2) + 16.f;
		DrawX = SafeZoneLeft + (CPIWidth/2) + 16.f;

		// Grab references to the PC and Pawn
		MyGearPC = GearPC(PlayerOwner);
		MyGearPawn = GearPawn(MyGearPC.Pawn);

		// Figure out whether we should draw the destination or the meatflag
		if ( GGRI.CommandPoint != None &&
			 GGRI.CPControlPct >= 100.0f &&
			 MyGearPawn != None &&
			 MyGearPawn.IsAKidnapper() &&
			 GGRI.MeatflagPawn == MyGearPawn.InteractionPawn )
		{
			bShowDesination = true;
		}

		if ( bShowDesination )
		{
			ActorToIndicate = GGRI.CommandPoint;
		}
		else if ( GGRI.MeatflagPawn != None )
		{
			ActorToIndicate = GGRI.MeatflagPawn;
		}
		else
		{
			return;
		}

		// Draw the background
		LocalDrawX = DrawX - CPIWidth/2;
		LocalDrawY = DrawY - CPIHeight/2;
		Canvas.SetPos( LocalDrawX, LocalDrawY );
		CPIndicatorMatInstanceBG.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
		Canvas.DrawMaterialTile( CPIndicatorMatInstanceBG, CPIWidth, CPIHeight );

		// Draw the CP's controlling team's material
		if ( (GGRI.CommandPoint != None) && (GGRI.CPControlTeam != -1) )
		{
			CPControllerMIC = (GGRI.CPControlTeam == 0) ? CPIndicatorMatInstanceCOG : CPIndicatorMatInstanceLOC;
			Canvas.SetPos( LocalDrawX, LocalDrawY );
			CPControllerMIC.SetScalarParameterValue( 'SymbolMasking', 2.f - (2.f * (float(GGRI.CPControlPct)/100.0f)) );
			if ( GGRI.CPControlTeam == 0 )
			{
				CPControllerMIC.SetVectorParameterValue( 'SymbolColorCOG', MakeLinearColor(139.f/255.f, 255.f/255.f, 255.f/255.f, 0.4f) );
			}
			else
			{
				CPControllerMIC.SetVectorParameterValue( 'SymbolColorLOC', MakeLinearColor(205.f/255.f, 22.f/255.f, 22.f/255.f, 0.4f) );
			}
			CPControllerMIC.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
			Canvas.DrawMaterialTile( CPControllerMIC, CPIWidth, CPIHeight );
		}

		// Draw the location pointer
		DrawSquadLocator( ActorToIndicate, LocalDrawX + CPIWidth/2, LocalDrawY + CPIHeight/2, Scale, Alpha );

		// Draw the glow
		if ( NumPulsesToDisplay > 0 )
		{
			// See if we just finished a pulse
			if ( (PulseStartTime + TotalPulseTime) < WorldInfo.TimeSeconds )
			{
				NumPulsesToDisplay--;
				PulseStartTime = WorldInfo.TimeSeconds;
			}
			// else draw the pulse
			else
			{
				PulseScale = 1.2f;

				if ( (WorldInfo.TimeSeconds - PulseStartTime) < (TotalPulseTime / 2) )
				{
					GlowOpacity = (WorldInfo.TimeSeconds - PulseStartTime) / (TotalPulseTime / 2);
				}
				else
				{
					GlowOpacity = 1.f - ((WorldInfo.TimeSeconds - PulseStartTime - (TotalPulseTime / 2)) / (TotalPulseTime / 2));
				}
				CPIndicatorGlowMatInstance.SetScalarParameterValue( 'GlowOpacity', GlowOpacity );
				LocalDrawX = DrawX - CPIWidth*PulseScale/2;
				LocalDrawY = DrawY - CPIHeight*PulseScale/2;
				Canvas.SetPos( LocalDrawX, LocalDrawY );
				CPIndicatorGlowMatInstance.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
				Canvas.DrawMaterialTile( CPIndicatorGlowMatInstance, CPIWidth*PulseScale, CPIHeight*PulseScale );
			}
		}

		// If we're drawing the destination, show the weapon icon
		if ( bShowDesination )
		{
			// Draw the weapon of the CP
			if ( (GGRI.CommandPoint.WeaponPickupClass != None || GGRI.CommandPoint.PlaceholderClass != None) && !GGRI.CommandPoint.bHidden )
			{
				WeapScale = 1.f * Scale;
				if (GGRI.CommandPoint.WeaponPickupClass != None)
				{
					if (ClassIsChildOf(GGRI.CommandPoint.WeaponPickupClass,class'GearWeap_HeavyBase'))
					{
						WeapScale += 0.25f;
					}
					WeaponIcon = GGRI.CommandPoint.WeaponPickupClass.default.AnnexWeaponIcon;
				}
				else if (class<GearShield>(GGRI.CommandPoint.PlaceholderClass) != None)
				{
					WeapScale += 0.25f;
					WeaponIcon = ShieldIcon;
				}
				LocalDrawX = DrawX;
				LocalDrawY = DrawY + 8;
				Canvas.SetPos( LocalDrawX - (WeaponIcon.UL*WeapScale/2), LocalDrawY - (WeaponIcon.VL*WeapScale/2) - 8 );
				CPIndicatorWeaponsMatInstance.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
				Canvas.DrawMaterialTile( CPIndicatorWeaponsMatInstance, WeaponIcon.UL*WeapScale, WeaponIcon.VL*WeapScale, WeaponIcon.U/256.f, WeaponIcon.V/512.f, WeaponIcon.UL/256.f, WeaponIcon.VL/512.f );
			}
		}
		// If we're drawing the meatflag, show his face
		else if ( GGRI.MeatflagPawn != None )
		{
			Icon = GGRI.MeatflagPawn.HeadIcon;
			IconScale = 1.25f * Scale;
			LocalDrawX = DrawX - (Icon.UL*IconScale/2);
			LocalDrawY = DrawY - (Icon.VL*IconScale*0.75f);
			Canvas.DrawIcon( Icon, LocalDrawX, LocalDrawY, IconScale );

			// Draw the leader icon
			if (GGRI.MeatflagPawn.IsDBNO() || GGRI.MeatflagPawn.IsDoingSpecialMove(SM_StumbleGoDown))
			{
				LocalDrawX = DrawX - MeatflagDownIcon.UL*IconScale/2;
				LocalDrawY = DrawY + (Icon.VL*IconScale*0.25f);
				Canvas.DrawIcon( MeatflagDownIcon, LocalDrawX, LocalDrawY, IconScale );
			}
			else
			{
				LocalDrawX = DrawX - MeatflagIcon.UL*IconScale/2;
				LocalDrawY = DrawY + (Icon.VL*IconScale*0.25f);
				Canvas.DrawIcon( MeatflagIcon, LocalDrawX, LocalDrawY, IconScale );
			}
		}
	}

	DrawY += CPIHeight + 22.f;
}

/** Returns the respawn time interval for the game */
simulated function int GetRespawnTimeInterval()
{
	return class'GearGameCTM_Base'.default.RespawnTimeInterval;
}

/** Play a sound and have the scoreboard pulse for an end of game warning */
function DoEndOfGameWarning( int ScoreDiff, int TeamIndex )
{
	// Play last 10 second sound
	PlaySound( AnnexSound_TeamAboutToWin[TeamIndex], true );

	// start the pulse
	ScorePulseStartTime = WorldInfo.TimeSeconds;
	ScoreTotalPulseTime = 1.0f;
}

/** Whether to draw the face icon when in taccom or not */
function bool IsPawnValidForAssessMode( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	local GearPawn MeatflagPawn;

	if ( WorldInfo.GRI != None )
	{
		MeatflagPawn = GearGRI(WorldInfo.GRI).MeatflagPawn;
	}

	if( Super.IsPawnValidForAssessMode(PawnToTest, LocalPRI, LocalPawn) ||
		(IsMeatflagPawn(PawnToTest) && !PawnToTest.IsAHostage()) ||
		((MeatflagPawn != None) && MeatflagPawn.IsAHostage() && (MeatflagPawn.InteractionPawn == PawnToTest)) )
	{
		return true;
	}

	return false;
}

/** Whether to draw the name above a player's head or not in taccom */
function bool ShouldDrawNameAboveHeadInTaccom( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	if( IsMeatflagPawn(PawnToTest) )
	{
		return false;
	}

	return Super.ShouldDrawNameAboveHeadInTaccom( PawnToTest, LocalPRI, LocalPawn );
}

/** Override for drawing the victim */
function EAssessSquadStatus GetSquadmateStatus( GearPawn AGearPawn )
{
	if( IsMeatflagPawn(AGearPawn) )
	{
		return EASS_Normal;
	}

	return Super.GetSquadmateStatus( AGearPawn );
}

/** Override for drawing the victim */
function float CalculateRevivePercentAlive( GearPawn AGearPawn )
{
	if( IsMeatflagPawn(AGearPawn) )
	{
		return 1.0f;
	}

	return Super.CalculateRevivePercentAlive( AGearPawn );
}

// Draw a glow ring around the background (used in Meatflag)
function DrawBackgroundRing( GearPawn AGearPawn, float CenterXCoord, float CenterYCoord, float Scale )
{
	local float PulseScale, MatWidth, MatHeight, LocalDrawX, LocalDrawY, GlowOpacity;
	local LinearColor GlowColor;
	local bool bIsVictim, bIsVictimKidnapper;

	bIsVictimKidnapper = (AGearPawn.IsAKidnapper() && IsMeatflagPawn(AGearPawn.InteractionPawn));
	if( IsMeatflagPawn(AGearPawn) )
	{
		bIsVictim = TRUE;
	}

	// Draw the glow
	if ( bIsVictimKidnapper || bIsVictim )
	{
		// See if we just finished a pulse
		if ( (VictimPulseStartTime + VictimTotalPulseTime) < WorldInfo.TimeSeconds )
		{
			VictimPulseStartTime = WorldInfo.TimeSeconds;
		}
		// else draw the pulse
		else
		{
			PulseScale = 1.2f;

			if ( (WorldInfo.TimeSeconds - VictimPulseStartTime) < (VictimTotalPulseTime / 2) )
			{
				GlowOpacity = (WorldInfo.TimeSeconds - VictimPulseStartTime) / (VictimTotalPulseTime / 2);
			}
			else
			{
				GlowOpacity = 1.f - ((WorldInfo.TimeSeconds - VictimPulseStartTime - (VictimTotalPulseTime / 2)) / (VictimTotalPulseTime / 2));
			}

			VictimLocatorHaloMatInstance.SetScalarParameterValue( 'GlowOpacity', GlowOpacity );

			if ( bIsVictim )
			{
				GlowColor = MakeLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
			}
			else
			{
				GlowColor = (GetGRI().CPControlTeam == 0) ? MakeLinearColor(0.44f, 0.9f, 0.95f, 1.0f) : MakeLinearColor(1.0f, 0.09f, 0.09f, 1.0f);
			}

			VictimLocatorHaloMatInstance.SetVectorParameterValue( 'Color', GlowColor );

			MatWidth = 128.f * Scale;
			MatHeight = 128.f * Scale;

			LocalDrawX = CenterXCoord - MatWidth*PulseScale/2;
			LocalDrawY = CenterYCoord - MatHeight*PulseScale/2;
			Canvas.SetPos( LocalDrawX, LocalDrawY );
			VictimLocatorHaloMatInstance.SetScalarParameterValue( 'Opacity', TaccomFadeOpacity );
			Canvas.DrawMaterialTile( VictimLocatorHaloMatInstance, MatWidth*PulseScale, MatHeight*PulseScale );
		}
	}
}

/** Whether to draw the leader icon or not */
function EGearSpecialIconType SpecialIconToDrawInWeaponIndicator( GearPC PC, bool bIsMainWeaponIndicator )
{
	local GearPawn MeatflagWP;

	if ( bIsMainWeaponIndicator && (WorldInfo.GRI != None) && (PC.MyGearPawn != None) )
	{
		MeatflagWP = GearGRI(WorldInfo.GRI).MeatflagPawn;
		if ( MeatflagWP != None )
		{
			if ( MeatflagWP.IsAHostage() && (MeatflagWP.InteractionPawn == PC.MyGearPawn) )
			{
				return eGSIT_Flag;
			}
		}
	}

	return eGSIT_None;
}

/** Special icon to draw above the name of the player in taccom */
function EGearSpecialIconType SpecialIconToDrawAboveHead(GearPawn PawnToTest)
{
	local GearPawn MeatflagWP;

	if ( WorldInfo.GRI != None )
	{
		MeatflagWP = GearGRI(WorldInfo.GRI).MeatflagPawn;
		if ( MeatflagWP != None )
		{
			if ( (MeatflagWP.IsAHostage() && (MeatflagWP.InteractionPawn == PawnToTest)) || IsMeatflagPawn(PawnToTest) )
			{
				return eGSIT_Flag;
			}
		}
	}

	return eGSIT_None;
}

/** Allow HUD to handle a pawn being spotted when targetting */
function HandlePawnTargetted( GearPawn TargetPawn )
{
	local GearPawn MeatflagWP;

	// Check for meatflag/meatflag carrier
	MeatflagWP = GearGRI(WorldInfo.GRI).MeatflagPawn;
	if ( MeatflagWP != None )
	{
		// If the meatflag is kidnapped we want to have the icon placed above the dude who is kidnapping him
		if ( MeatflagWP.IsAHostage() )
		{
			if ( (MeatflagWP.InteractionPawn == TargetPawn) || (MeatflagWP == TargetPawn) )
			{
				UpdateMeatflagSpotted( MeatflagWP.InteractionPawn );
			}
		}
		// Else put the icon above the meatflag itself
		else if ( MeatflagWP == TargetPawn )
		{
			UpdateMeatflagSpotted( TargetPawn );
		}
	}
}

/** Start/update spotting an enemy leader */
function UpdateMeatflagSpotted( GearPawn SpottedPawn )
{
	PawnToDrawIconOn = SpottedPawn;

	// Clear the timers if the Leader to draw is null
	if ( PawnToDrawIconOn == None )
	{
		ClearTimer( 'OnDoneBlendingMeatflagIconIn' );
		ClearTimer( 'OnDoneBlendingMeatflagIconOut' );
	}
	// Restart the fadeout timer
	else if ( IsTimerActive('OnDoneBlendingMeatflagIconOut') )
	{
		SetTimer( MeatflagIconBlendOutTime, FALSE, nameof(OnDoneBlendingMeatflagIconOut) );
	}
	// Start the blend-in timer if it hasn't been started already
	else if ( !IsTimerActive('OnDoneBlendingMeatflagIconIn') )
	{
		SetTimer( MeatflagIconBlendInTime, FALSE, nameof(OnDoneBlendingMeatflagIconIn) );
	}
}

/** Function called when we are done blending meatflag icon in */
function OnDoneBlendingMeatflagIconIn()
{
	SetTimer( MeatflagIconBlendOutTime, FALSE, nameof(OnDoneBlendingMeatflagIconOut) );
}

/** Function called when we are done blending the enemy leader icon out */
function OnDoneBlendingMeatflagIconOut()
{
	PawnToDrawIconOn = None;
}

/** Function that can be overloaded by game-specific HUDs to draw icons above the heads of players */
function DrawSpecialPlayerIcons()
{
	local float AlphaPercent, TimerCount;
	local EGearHUDColor ColorType;

	if ( (PawnToDrawIconOn != None) && (WorldInfo.GRI != None) )
	{
		TimerCount = GetTimerCount( 'OnDoneBlendingMeatflagIconIn' );
		if ( TimerCount > -1 )
		{
			AlphaPercent = TimerCount / MeatflagIconBlendInTime;
		}
		else
		{
			TimerCount = GetTimerCount( 'OnDoneBlendingMeatflagIconOut' );
			if ( TimerCount > -1 )
			{
				AlphaPercent = 1.0f - (TimerCount / MeatflagIconBlendOutTime);
			}
		}

		if ( GearGRI(WorldInfo.GRI).MeatflagPawn == PawnToDrawIconOn )
		{
			ColorType = eWARHUDCOLOR_WHITE;
		}
		else
		{
			ColorType = (PawnToDrawIconOn.GetTeamNum() == 0) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED;
		}

		DrawMeatflagIconAboveHead( PawnToDrawIconOn, ColorType, 255*AlphaPercent );
	}
}

/** Draw the leader icon above the head of the leader */
final function DrawMeatflagIconAboveHead( GearPawn PawnToDraw, EGearHUDColor ColorType, byte Alpha )
{
	local vector PawnExtent, ScreenLoc, CameraDir, LeaderDir;
	local GearPC PC;
	local vector CameraLoc;
	local rotator CameraRot;
	local float IconScale;

	PC = GearPC(PlayerOwner);

	PC.GetPlayerViewpoint( CameraLoc, CameraRot );

	if ( IsSquadMateVisible(PawnToDraw, CameraLoc, CameraRot, PC) )
	{
		if ( (PawnToDraw != None) && !PawnToDraw.bIsHiddenByCamera && TimeSince(PawnToDraw.LastRenderTime) < 2.f && PC.LineOfSightTo(PawnToDraw) )
		{
			CameraDir = vector(CameraRot);
			LeaderDir = PawnToDraw.Location - CameraLoc;
			if ( CameraDir Dot LeaderDir > 0 )
			{
				PawnExtent = PawnToDraw.Mesh.GetBoneLocation( PawnToDraw.HeadBoneNames[0] );
				PawnExtent.Z += 30.0f;
				ScreenLoc = Canvas.Project( PawnExtent );
				IconScale = 1.0f;
				ScreenLoc.X -= MeatflagIcon.UL*IconScale / 2;
				ScreenLoc.Y -= MeatflagIcon.VL*IconScale;

				// Set the text position and draw
				SetHUDDrawColor( ColorType, Alpha );
				Canvas.DrawIcon( MeatflagIcon, ScreenLoc.X, ScreenLoc.Y, IconScale );
			}
		}
	}
}

/** Defaults properties */
defaultproperties
{
	NumPulsesToDisplay=1000;
	PulseStartTime=100000000;
	TotalPulseTime=1.f;
	VictimTotalPulseTime=1.0f;

	MeatflagIconBlendInTime=0.25f
	MeatflagIconBlendOutTime=2.0f

	ShieldIcon=(U=128,V=68,UL=128,VL=40)

	GameSpecificIcon(0)=(Texture=Texture2D'Warfare_HUD.ScoreBoard.UI_MP_Icons',U=207,V=0,UL=25,VL=26)
}
