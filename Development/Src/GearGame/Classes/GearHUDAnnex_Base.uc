/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDAnnex_Base extends GearHUDMP_Base;

var localized string NumRespawnsString;

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
var Material CPIndicatorWeapons;
var Material CPIndicatorGlow;
/** Material Instances for drawing the Command Point Indicator */
var MaterialInstanceConstant CPIndicatorMatInstanceBG;
var MaterialInstanceConstant CPIndicatorMatInstanceCOG;
var MaterialInstanceConstant CPIndicatorMatInstanceLOC;
var MaterialInstanceConstant CPIndicatorWeaponsMatInstance;
var MaterialInstanceConstant CPIndicatorGlowMatInstance;

/** Variables for controlling the pulsing effect in the Command Point Indicator */
var int NumPulsesToDisplay;
var float PulseStartTime;
var float TotalPulseTime;

/** Played when a team receives a point and is about to win */
var SoundCue AnnexSound_TickFTW;
/** Played when one team is about to win */
var SoundCue AnnexSound_TeamAboutToWin[2];
/** Variables for controlling the pulsing effect in the Scoreboard */
var float ScorePulseStartTime;
var float ScoreTotalPulseTime;

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
	if ( CPIndicatorWeaponsMatInstance == None )
	{
		CPIndicatorWeaponsMatInstance = new(outer) class'MaterialInstanceConstant';
		CPIndicatorWeaponsMatInstance.SetParent( CPIndicatorWeapons );
	}

	// initialize the CP indicator weapons material
	if ( CPIndicatorGlowMatInstance == None )
	{
		CPIndicatorGlowMatInstance = new(outer) class'MaterialInstanceConstant';
		CPIndicatorGlowMatInstance.SetParent( CPIndicatorGlow );
	}
}

/** Reset the HUD */
function Reset()
{
	Super.Reset();

	NumPulsesToDisplay = 0;
	PulseStartTime = 0;
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

		// draw the main scoreboard
		DrawMainScoreBoard( DrawX, DrawY );

		// draw the command point indicator
		DrawCommandPointIndicator( DrawX, DrawY );
	}
}

/** Draws the main scoreboard for annex */
simulated function DrawMainScoreBoard( out float DrawX, out float DrawY )
{
	local MaterialInstanceConstant MainScoreMIC;
	local String ScoreString;
	local float LocalDrawX, LocalDrawY;
	local int ControlTeam;
	local GearGRI GGRI;
	local bool bHasCPControl;
	local int MyTeam;

	GGRI = GearGRI(WorldInfo.GRI);

	if (PlayerOwner.PlayerReplicationInfo != None && GGRI != None)
	{
		// default to team zero for players with no team (spectators, demo playback)
		MyTeam = (PlayerOwner.PlayerReplicationInfo.Team != None) ? PlayerOwner.PlayerReplicationInfo.Team.TeamIndex : 0;

		// See if one of the teams has control of the CP
		ControlTeam = GGRI.CPControlTeam;
		if ( GGRI.CPControlPct < 100.f )
		{
			ControlTeam = -1;
		}

		// Set the draw position for our scoreboard
		LocalDrawX = DrawX;
		LocalDrawY = DrawY;
		// Grab the scoreboard material instance for our scoreboard
		MainScoreMIC = (MyTeam == 0) ? MainScoreMatInstanceCOG : MainScoreMatInstanceLOC;
		// Grab the score string for our scoreboard
		ScoreString = (MyTeam == 0) ? String(int(GearGRI(WorldInfo.GRI).GameScore[0])) : String(int(GearGRI(WorldInfo.GRI).GameScore[1]));
		// See if our team has control of the command point
		bHasCPControl = (MyTeam == ControlTeam);
		// Draw our scoreboard
		DrawTeamScoreBoard( LocalDrawX, LocalDrawY, MainScoreMIC, ScoreString, bHasCPControl );

		// Set the draw position for the enemy scoreboard
		LocalDrawX = DrawX + 104.f;
		LocalDrawY = DrawY;
		// Grab the scoreboard material instance for the enemy scoreboard
		MainScoreMIC = (MyTeam == 1) ? MainScoreMatInstanceCOG : MainScoreMatInstanceLOC;
		// Grab the score string for the enemy scoreboard
		ScoreString = (MyTeam == 1) ? String(int(GearGRI(WorldInfo.GRI).GameScore[0])) : String(int(GearGRI(WorldInfo.GRI).GameScore[1]));
		// See if our team has control of the command point
		bHasCPControl = ((MyTeam != ControlTeam) && (ControlTeam != -1));
		// Draw the enemy scoreboard
		DrawTeamScoreBoard( LocalDrawX, LocalDrawY, MainScoreMIC, ScoreString, bHasCPControl );
	}
}

/** Calculate the opacity of the scoreboard */
simulated function float CalculateTeamScoreboardOpacity( bool bHasCPControl )
{
	local float MinOpacityNormal, MaxOpacityNormal, MinOpacityPulse, MaxOpacityPulse, NewOpacity;

	MinOpacityNormal = 0.4f;
	MaxOpacityNormal = 0.85f;
	MinOpacityPulse = 0.4f;
	MaxOpacityPulse = 1.0f;

	if ( !bHasCPControl )
	{
		return MinOpacityNormal;
	}

	if ( ScorePulseStartTime + ScoreTotalPulseTime < WorldInfo.TimeSeconds )
	{
		ScorePulseStartTime = 0.f;
		ScoreTotalPulseTime = 0.f;
		return MaxOpacityNormal;
	}

	if ( (WorldInfo.TimeSeconds - ScorePulseStartTime) < (0.2f * ScoreTotalPulseTime) )
	{
		NewOpacity = MaxOpacityNormal + (MaxOpacityPulse - MaxOpacityNormal) * ((WorldInfo.TimeSeconds - ScorePulseStartTime) / (0.2f * ScoreTotalPulseTime));
	}
	else if ( (WorldInfo.TimeSeconds - ScorePulseStartTime) < (0.6f * ScoreTotalPulseTime) )
	{
		NewOpacity = MaxOpacityPulse - (MaxOpacityPulse - MinOpacityPulse) * ((WorldInfo.TimeSeconds - ScorePulseStartTime - 0.2f*ScoreTotalPulseTime) / (0.4f * ScoreTotalPulseTime));
	}
	else
	{
		NewOpacity = MinOpacityPulse + (MaxOpacityNormal - MinOpacityPulse) * ((WorldInfo.TimeSeconds - ScorePulseStartTime - 0.6f*ScoreTotalPulseTime) / (0.4f * ScoreTotalPulseTime));
	}

	return NewOpacity;
}

/** Draw the main scoreboard for a specific team */
simulated function DrawTeamScoreBoard( out float DrawX, out float DrawY, MaterialInstanceConstant MainScoreMIC, String ScoreString, bool bHasCPControl )
{
	local float LocalDrawX, LocalDrawY, TextWidth, TextHeight, SBOpacity;
	local byte Alpha;

	LocalDrawX = DrawX;
	LocalDrawY = DrawY;
	Alpha = 255 * (1.0f - TaccomFadeOpacity);

	// Draw the scoreboard
	SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
	Canvas.SetPos( LocalDrawX, LocalDrawY );
	SBOpacity = CalculateTeamScoreboardOpacity( bHasCPControl );
	MainScoreMIC.SetScalarParameterValue( (MainScoreMIC==MainScoreMatInstanceCOG)?'ScoreBoardOpacityCOG':'ScoreBoardOpacityLOC', SBOpacity );
	MainScoreMIC.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
	Canvas.DrawMaterialTile( MainScoreMIC, 128, 128 );

	// Draw the score invert material if this team has the command point
	//if ( bHasCPControl )
	//{
		//SetHUDDrawColor( eWARHUDCOLOR_WHITE );
		//Canvas.SetPos( LocalDrawX + 10.f, LocalDrawY + 30.f );
		//Canvas.DrawMaterialTile( ScoreInvertMatInstance, 128, 32 );
	//}

	// Draw the score
	if ( bHasCPControl )
	{
		SetHUDDrawColor( (MainScoreMIC == MainScoreMatInstanceCOG) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED, Alpha );
	}
	else
	{
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
	}
	Canvas.Font = class'Engine'.static.GetAdditionalFont( FONT_Euro24 );
	Canvas.TextSize( ScoreString, TextWidth, TextHeight );
	LocalDrawX = DrawX + 42.f - (TextWidth/2);
	LocalDrawY = DrawY + 43.f - (TextHeight/2);
	Canvas.SetPos( LocalDrawX, LocalDrawY );
	Canvas.DrawText( ScoreString );
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
	local float LocalDrawX, LocalDrawY, CPIWidth, CPIHeight, WeapScale, TextWidth, TextHeight, Scale, GlowOpacity, PulseScale;
	local GearGRI GGRI;
	local CanvasIcon WeaponIcon;
	local String PointsString;
	local byte Alpha;

	GGRI = GearGRI(WorldInfo.GRI);
	Alpha = 255 * (1.0f - TaccomFadeOpacity);

	if (PlayerOwner.PlayerReplicationInfo != None && GGRI != None)
	{
		SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
		Scale = 0.65f;
		CPIWidth = 128.f * Scale;
		CPIHeight = 128.f * Scale;

		// Original Y Position + Height of the Scoreboard texture + an additional spacing offset.
		DrawY = SafeZoneTop + 128.f + 10.f*Scale;
		DrawX = SafeZoneLeft + 93.5;

		// Draw the background
		LocalDrawX = DrawX - CPIWidth/2;
		LocalDrawY = DrawY - CPIHeight/2;
		Canvas.SetPos( LocalDrawX, LocalDrawY );
		CPIndicatorMatInstanceBG.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
		Canvas.DrawMaterialTile( CPIndicatorMatInstanceBG, CPIWidth, CPIHeight );

		if ( GGRI.CommandPoint != None )
		{
			// Draw the CP's controlling team's material
			if ( GGRI.CPControlTeam != -1 )
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
			DrawSquadLocator( GGRI.CommandPoint, LocalDrawX + CPIWidth/2, LocalDrawY + CPIHeight/2, Scale, Alpha );

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
				LocalDrawY = DrawY;
				if (GGRI.bAnnexIsKOTHRules)
				{
					LocalDrawY += 8;
				}
				Canvas.SetPos( LocalDrawX - (WeaponIcon.UL*WeapScale/2), LocalDrawY - (WeaponIcon.VL*WeapScale/2) - 8 );
				CPIndicatorWeaponsMatInstance.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
				Canvas.DrawMaterialTile( CPIndicatorWeaponsMatInstance, WeaponIcon.UL*WeapScale, WeaponIcon.VL*WeapScale, WeaponIcon.U/256.f, WeaponIcon.V/512.f, WeaponIcon.UL/256.f, WeaponIcon.VL/512.f );
			}

			// Draw the amount of points left, with a black border around the text
			if( GearGRI(WorldInfo.GRI).CPResourceLeft < 0 )
			{
				PointsString = "";
			}
			else
			{
				PointsString = "" $ GearGRI(WorldInfo.GRI).CPResourceLeft;
			}


			Canvas.Font = class'Engine'.static.GetAdditionalFont( FONT_Euro20 );
			Canvas.TextSize( PointsString, TextWidth, TextHeight );

			// set the location of the text
			LocalDrawX = DrawX - TextWidth/2;
			LocalDrawY = DrawY + CPIHeight/2 - TextHeight*1.5f;

			// first draw the black backgound behind the text
			SetHUDDrawColor( eWARHUDCOLOR_BLACK, Alpha );
			Canvas.SetPos( LocalDrawX-1, LocalDrawY-1 );
			Canvas.DrawText( PointsString );
			Canvas.SetPos( LocalDrawX-1, LocalDrawY+1 );
			Canvas.DrawText( PointsString );
			Canvas.SetPos( LocalDrawX+1, LocalDrawY+1 );
			Canvas.DrawText( PointsString );
			Canvas.SetPos( LocalDrawX+1, LocalDrawY-1 );
			Canvas.DrawText( PointsString );
			// now draw the string in white
			SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
			Canvas.SetPos( LocalDrawX, LocalDrawY );
			Canvas.DrawText( PointsString );
		}
	}

	DrawY += CPIHeight + 22.f;
}

/** Returns the respawn time interval for the game */
simulated function int GetRespawnTimeInterval()
{
	return class'GearGameAnnex_Base'.default.RespawnTimeInterval;
}

/** Play a sound and have the scoreboard pulse for an end of game warning */
function DoEndOfGameWarning( int ScoreDiff, int TeamIndex )
{
	// Play last 10 second sound
	if ( (ScoreDiff <= 5) || (ScoreDiff == 15) )
	{
		PlaySound( AnnexSound_TeamAboutToWin[TeamIndex], true );
	}
	// Play other sound
	else
	{
		PlaySound( AnnexSound_TeamAboutToWin[TeamIndex], true );
		//PlaySound( AnnexSound_TickFTW, true );
	}

	// start the pulse
	ScorePulseStartTime = WorldInfo.TimeSeconds;
	ScoreTotalPulseTime = 1.0f;
}

/** Defaults properties */
defaultproperties
{
	NumPulsesToDisplay=1000;
	PulseStartTime=100000000;
	TotalPulseTime=1.f;

	ShieldIcon=(U=128,V=68,UL=128,VL=40)
}
