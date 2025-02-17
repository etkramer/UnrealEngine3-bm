/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearHUDKTL extends GearHUDTDM;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Localized strings */
var localized string YouAreLeaderString;
var localized string IsLeaderString;
var localized string LeaderNowHighScorer;
var localized string LeaderNowKiller;
var localized string LeaderKilledEnemy;
var localized string LeaderKilledYour;
var localized string YouWereAssassinated;

/** Material Instance for drawing the glow around your leader's head in taccom */
var Material LeaderLocatorHalo;
/** Material Instance for drawing the glow around your leader's head in taccom */
var MaterialInstanceConstant LeaderLocatorHaloMatInstance;

/** Materials for drawing the Enemy Leader Indicator */
var Material EnemyIndicatorMaterialBG;
var Material EnemyIndicatorMaterialCOG;
var Material EnemyIndicatorMaterialLOC;
/** Material Instances for drawing the Enemy Leader Indicator */
var MaterialInstanceConstant EnemyIndicatorMatInstanceBG;
var MaterialInstanceConstant EnemyIndicatorMatInstanceCOG;
var MaterialInstanceConstant EnemyIndicatorMatInstanceLOC;

/** Reference to the leaders PRIs */
var GearPRI MyLeaderPRI;
var GearPRI EnemyLeaderPRI;

/** Variables for controlling the pulsing effect in the taccom for showing the leader */
var float LeaderPulseStartTime;
var float LeaderTotalPulseTime;

/** Played when my leader is killed **/
var SoundCue MyLeaderDiedSound;
/** Played when enemy leader is killed **/
var SoundCue EnemyLeaderDiedSound;
/** Audio component for leader died sounds */
var AudioComponent LeaderDiedAudioComponent;

/** Time to blend into showing the leader icons */
var const float LeaderIconBlendInTime;
/** Time to blend out of showing the leader icons */
var const float LeaderIconBlendOutTime;

/** Friendly leader to show an icon on */
var GearPawn LeaderToDrawIconOn_Friendly;
/** Enemy leader to show an icon on */
var GearPawn LeaderToDrawIconOn_Enemy;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Reset the HUD */
function Reset()
{
	Super.Reset();

	LeaderPulseStartTime = 0;
	UpdateLeaderSpotted_Friendly( None );
	UpdateLeaderSpotted_Enemy( None );
}

/** Overloaded to clear the spotted meatflag */
function EnableAssessMode()
{
	Super.EnableAssessMode();

	UpdateLeaderSpotted_Friendly( None );
	UpdateLeaderSpotted_Enemy( None );
}

/** Initialize some data */
simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// initialize the leader halo
	if ( LeaderLocatorHaloMatInstance == None )
	{
		LeaderLocatorHaloMatInstance = new(outer) class'MaterialInstanceConstant';
		LeaderLocatorHaloMatInstance.SetParent( LeaderLocatorHalo );
	}

	// initialize the leader BG
	if ( EnemyIndicatorMatInstanceBG == None )
	{
		EnemyIndicatorMatInstanceBG = new(outer) class'MaterialInstanceConstant';
		EnemyIndicatorMatInstanceBG.SetParent( EnemyIndicatorMaterialBG );
	}

	// initialize the enemy indicator COG material
	if ( EnemyIndicatorMatInstanceCOG == None )
	{
		EnemyIndicatorMatInstanceCOG = new(outer) class'MaterialInstanceConstant';
		EnemyIndicatorMatInstanceCOG.SetParent( EnemyIndicatorMaterialCOG );
	}

	// initialize the enemy indicator Locust material
	if ( EnemyIndicatorMatInstanceLOC == None )
	{
		EnemyIndicatorMatInstanceLOC = new(outer) class'MaterialInstanceConstant';
		EnemyIndicatorMatInstanceLOC.SetParent( EnemyIndicatorMaterialLOC );
	}

	// initialize the enemy indicator blood splat material
	if( DeadTaccomSplatMaterialConstant == None )
	{
		DeadTaccomSplatMaterialConstant = new(outer) class'MaterialInstanceConstant';
		DeadTaccomSplatMaterialConstant.SetParent( DeadTaccomSplatMaterial );
	}
}

/** Overloaded Draw call */
function DrawHUD()
{
	SetTeamLeaderPRIs();

	Super.DrawHUD();

	if (bShowHud &&
		!HideHUDFromScenes() &&
		(GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress) )
	{
		// draw the enemy leader UI
		CheckForEnemyLeader();
	}
}

/** Returns the PRIleader of the team of the HUD player */
final function SetTeamLeaderPRIs()
{
	local GearGRI GGRI;
	local GearPC MyGearPC;
	local GearPRI MyPRI, CurrPRI;
	local int PRIIdx;

	MyGearPC = GearPC(PlayerOwner);
	MyPRI = GearPRI(MyGearPC.PlayerReplicationInfo);
	GGRI = GearGRI(WorldInfo.GRI);

	// Find the leader PRIs
	if ( (GGRI != None) && (MyPRI != None) )
	{
		for ( PRIIdx = 0; PRIIdx < GGRI.PRIArray.length; PRIIdx++ )
		{
			CurrPRI = GearPRI(GGRI.PRIArray[PRIIdx]);
			if ( (CurrPRI != None) && CurrPRI.bIsLeader )
			{
				if ( MyPRI.GetTeamNum() == CurrPRI.GetTeamNum() )
				{
					MyLeaderPRI = CurrPRI;
				}
				else
				{
					EnemyLeaderPRI = CurrPRI;
				}
			}
		}
	}
}

/** See if there's an enemy leader to draw on the HUD and call the function to do so */
final function CheckForEnemyLeader()
{
	local GearPawn AWPawn;
	local GearPRI MyPRI, EnemyPRI;
	local GearPC MyGearPC;

	MyGearPC = GearPC(PlayerOwner);
	MyPRI = GearPRI(MyGearPC.PlayerReplicationInfo);

	// Look for the leader pawn and draw the UI for him
	foreach WorldInfo.AllPawns( class'GearPawn', AWPawn )
	{
		EnemyPRI = GearPRI(AWPawn.PlayerReplicationInfo);
		if ( (EnemyPRI != None) && EnemyPRI.bIsLeader && (MyPRI.GetTeamNum() != EnemyPRI.GetTeamNum()) )
		{
			DrawEnemyLeader( AWPawn, EnemyPRI );
			return;
		}
	}

	// The leader has no pawn so he must be dead, so don't pass in the pawn
	DrawEnemyLeader( None, EnemyLeaderPRI );
}

/** Draw the enemy leader */
final function DrawEnemyLeader( GearPawn LeaderPawn, GearPRI LeaderPRI )
{
	local MaterialInstanceConstant TeamBackGroundMaterial;
	local float DrawX, DrawY, LocalDrawX, LocalDrawY, CPIWidth, CPIHeight, Scale, IconScale, Alpha;
	local CanvasIcon Icon;

	Alpha = 255 * (1.0f - TaccomFadeOpacity);
	SetHUDDrawColor( eWARHUDCOLOR_WHITE, Alpha );
	Scale = 0.65f;
	CPIWidth = 128.f * Scale;
	CPIHeight = 128.f * Scale;

	DrawY = SafeZoneTop + CPIHeight/2;
	DrawX = SafeZoneLeft + CPIWidth/2;

	// Draw the background
	LocalDrawX = DrawX - CPIWidth/2;
	LocalDrawY = DrawY - CPIHeight/2;
	Canvas.SetPos( LocalDrawX, LocalDrawY );
	EnemyIndicatorMatInstanceBG.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
	Canvas.DrawMaterialTile( EnemyIndicatorMatInstanceBG, CPIWidth, CPIHeight );

	// Draw the CP's controlling team's material
	TeamBackGroundMaterial = (LeaderPRI.GetTeamNum() == 0) ? EnemyIndicatorMatInstanceCOG : EnemyIndicatorMatInstanceLOC;
	Canvas.SetPos( LocalDrawX, LocalDrawY );
	if ( LeaderPRI.GetTeamNum() == 0 )
	{
		TeamBackGroundMaterial.SetVectorParameterValue( 'SymbolColorCOG', MakeLinearColor(139.f/255.f, 255.f/255.f, 255.f/255.f, 0.4f) );
	}
	else
	{
		TeamBackGroundMaterial.SetVectorParameterValue( 'SymbolColorLOC', MakeLinearColor(205.f/255.f, 22.f/255.f, 22.f/255.f, 0.4f) );
	}
	TeamBackGroundMaterial.SetScalarParameterValue( 'Opacity', 1.f - TaccomFadeOpacity );
	Canvas.DrawMaterialTile( TeamBackGroundMaterial, CPIWidth, CPIHeight );

	// Draw the location pointer
	if ( (LeaderPawn != None) && (LeaderPawn.Health > 0 || LeaderPawn.IsDBNO()) )
	{
		DrawSquadLocator( LeaderPawn, LocalDrawX + CPIWidth/2, LocalDrawY + CPIHeight/2, Scale, Alpha );
	}

	// Draw the enemy face
	Icon = (LeaderPRI.GetTeamNum() == 0) ? class'GearPawn_COGHoffmanMP'.default.HeadIcon : class'GearPawn_LocustSkorgeMP'.default.HeadIcon;
	IconScale = 1.25f * Scale;
	LocalDrawX = DrawX - (Icon.UL*IconScale/2);
	LocalDrawY = DrawY - (Icon.VL*IconScale*0.75f);
	Canvas.DrawIcon( Icon, LocalDrawX, LocalDrawY, IconScale );

	// Draw the leader icon
	LocalDrawX = DrawX - LeaderIcon.UL*IconScale/2;
	LocalDrawY = DrawY + (Icon.VL*IconScale*0.25f);
	Canvas.DrawIcon( LeaderIcon, LocalDrawX, LocalDrawY, IconScale );

	// Draw the blood splat if the leader is dead
	if ( (LeaderPawn == None) || ((LeaderPawn.Health <= 0) && !LeaderPawn.IsDBNO()) )
	{
		LocalDrawX = DrawX - 128.f*Scale;
		LocalDrawY = DrawY - 256.f*Scale;
		Canvas.SetPos( DrawX, DrawY );
		Alpha = 255.0f;
		DeadTaccomSplatMaterialConstant.SetScalarParameterValue( 'TaccomDead_Fade', 2.0f - (2.0f * (1.0f-TaccomFadeOpacity)) );
		Canvas.SetPos( LocalDrawX, LocalDrawY );
		Canvas.DrawMaterialTile( DeadTaccomSplatMaterialConstant, 256.f*Scale, 512.f*Scale );
	}
}

/** Returns the type of special icon to draw in the squadmate UI */
function EGearSpecialIconType GetSpecialIconType( GearPawn PawnToTest )
{
	if ( GearPRI(PawnToTest.PlayerReplicationInfo).bIsLeader )
	{
		return eGSIT_Leader;
	}

	return eGSIT_None;
}

/** Returns the respawn time interval for the game */
simulated function int GetRespawnTimeInterval()
{
	return class'GearGameKTL'.default.RespawnTimeInterval;
}

/** Message from the game object that a leader has died */
function StartLeaderDiedMessage( GearPRI LeaderPRI )
{
	local String Title, Message;
	local GearPC MyGearPC;
	local GearPRI MyPRI;
	local SoundCue SoundCueToPlay;

	MyGearPC = GearPC(PlayerOwner);
	MyPRI = GearPRI(MyGearPC.PlayerReplicationInfo);

	if ( MyPRI != None )
	{
		// Figure out if it was me, my leader or the enemy leader
		if ( MyPRI == LeaderPRI )
		{
			Title = YouWereAssassinated;
			SoundCueToPlay = MyLeaderDiedSound;
		}
		else if ( MyPRI.GetTeamNum() == LeaderPRI.GetTeamNum() )
		{
			Title = LeaderKilledYour;
			SoundCueToPlay = MyLeaderDiedSound;
			MyGearPC.AttemptTrainingGroundTutorial(GEARTUT_TRAIN_LeDed, eGEARTRAIN_Respawn);
		}
		else
		{
			Title = LeaderKilledEnemy;
			SoundCueToPlay = EnemyLeaderDiedSound;
		}

		// Get the name
		Message = LeaderPRI.PlayerName;

		StartDrawingDramaticText( Title, Message, 8.0f, 0.25f, 2.0f, GetCountdownYPosition() + GetCountdownHeight() + GearPC(PlayerOwner).NumPixelsToMoveDownForDramaText(), 0.25f, 2.0f );

		if ( LeaderDiedAudioComponent != None && LeaderDiedAudioComponent.IsPlaying() )
		{
			LeaderDiedAudioComponent.Stop();
		}
		LeaderDiedAudioComponent = CreateAudioComponent(SoundCueToPlay, true, true);
	}
}


/** Start the begin round screen */
function SignalStartofRoundOrMatch(GearGRI CurGRI)
{
	// Don't show if there's a network error
	if (WorldInfo.Game != None && WorldInfo.Game.bHasNetworkError)
	{
		return;
	}

	DrawStartRound();
}

/** Overload to stop leader died sound */
function SignalEndofRoundOrMatch(GearGRI CurGRI)
{
	Super.SignalEndofRoundOrMatch(GRI);

	if ( LeaderDiedAudioComponent != None && LeaderDiedAudioComponent.IsPlaying() )
	{
		LeaderDiedAudioComponent.Stop();
	}
}

/** Start wave screen */
final function DrawStartRound()
{
	local String Title;//, Message;
	local GearPC MyGearPC;
	local GearPRI MyPRI;
	//local GearGRI GGRI;

	SetTeamLeaderPRIs();

	MyGearPC = GearPC(PlayerOwner);
	MyPRI = GearPRI(MyGearPC.PlayerReplicationInfo);
	//GGRI = GearGRI(WorldInfo.GRI);

	if ( MyPRI != None )
	{
		// Figure out the title - who is the leader
		if ( MyPRI.bIsLeader )
		{
			Title = YouAreLeaderString;
		}
		else
		{
			Title = Repl(IsLeaderString, "*t", MyLeaderPRI.PlayerName);
		}

		// Figure out the reason for the leader being the leader
		/*
		if ( (GGRI != None) && (GGRI.RoundCount > 0) )
		{
			if ( MyLeaderPRI.GetTeamNum() == 0 && MyLeaderPRI == GGRI.KillerOfLocustLeaderPRI )
			{
				Message = LeaderNowKiller;
			}
			else if ( MyLeaderPRI.GetTeamNum() == 1 && MyLeaderPRI == GGRI.KillerOfCOGLeaderPRI )
			{
				Message = LeaderNowKiller;
			}
			else
			{
				Message = LeaderNowHighScorer;
			}
		}
		*/

		StartDrawingDramaticText( Title, "", 8.0f, 2.0f, 2.0f, GetCountdownYPosition() + GetCountdownHeight() + GearPC(PlayerOwner).NumPixelsToMoveDownForDramaText(), 2.0f, 2.0f );
	}
}

/** Whether to draw the leader icon or not */
function EGearSpecialIconType SpecialIconToDrawInWeaponIndicator( GearPC PC, bool bIsMainWeaponIndicator )
{
	return ((bIsMainWeaponIndicator && GearPRI(Pawn(PlayerOwner.ViewTarget).PlayerReplicationInfo).bIsLeader) ? eGSIT_Leader : eGSIT_None);
}

/** Whether to draw the face icon when in taccom or not */
function bool IsPawnValidForAssessMode( GearPawn PawnToTest, GearPRI LocalPRI, GearPawn LocalPawn )
{
	// if not the same pawn and the pawn is alive/dbno
	if (PawnToTest != LocalPawn && (PawnToTest.Health > 0 || ShouldDrawDBNOIndicator(PawnToTest)))
	{
		// if spectating then we want to view everyone
		if (PlayerOwner.IsSpectating())
		{
			return TRUE;
		}
		else if ( LocalPawn != None )
		{
			// otherwise only show friendly players
			return (LocalPawn.IsSameTeam(PawnToTest) || GearPRI(PawnToTest.PlayerReplicationInfo).bIsLeader);
		}
	}
	return FALSE;
}

/** Special icon to draw above the name of the player in taccom */
function EGearSpecialIconType SpecialIconToDrawAboveHead(GearPawn PawnToTest)
{
	return (GearPRI(PawnToTest.PlayerReplicationInfo).bIsLeader ? eGSIT_Leader : eGSIT_None);
}

// Draw a glow ring around the background (used in Meatflag)
function DrawBackgroundRing( GearPawn AGearPawn, float CenterXCoord, float CenterYCoord, float Scale )
{
	local float PulseScale, MatWidth, MatHeight, LocalDrawX, LocalDrawY, GlowOpacity;
	local LinearColor GlowColor;

	// Draw the glow
	if ( GearPRI(AGearPawn.PlayerReplicationInfo).bIsLeader )
	{
		// See if we just finished a pulse
		if ( (LeaderPulseStartTime + LeaderTotalPulseTime) < WorldInfo.TimeSeconds )
		{
			LeaderPulseStartTime = WorldInfo.TimeSeconds;
		}
		// else draw the pulse
		else
		{
			PulseScale = 1.2f;

			if ( (WorldInfo.TimeSeconds - LeaderPulseStartTime) < (LeaderTotalPulseTime / 2) )
			{
				GlowOpacity = (WorldInfo.TimeSeconds - LeaderPulseStartTime) / (LeaderTotalPulseTime / 2);
			}
			else
			{
				GlowOpacity = 1.f - ((WorldInfo.TimeSeconds - LeaderPulseStartTime - (LeaderTotalPulseTime / 2)) / (LeaderTotalPulseTime / 2));
			}

			LeaderLocatorHaloMatInstance.SetScalarParameterValue( 'GlowOpacity', GlowOpacity );

			GlowColor = (AGearPawn.GetTeamNum() == 0) ? MakeLinearColor(0.44f, 0.9f, 0.95f, 1.0f) : MakeLinearColor(1.0f, 0.09f, 0.09f, 1.0f);

			LeaderLocatorHaloMatInstance.SetVectorParameterValue( 'Color', GlowColor );

			MatWidth = 128.f * Scale;
			MatHeight = 128.f * Scale;

			LocalDrawX = CenterXCoord - MatWidth*PulseScale/2;
			LocalDrawY = CenterYCoord - MatHeight*PulseScale/2;
			Canvas.SetPos( LocalDrawX, LocalDrawY );
			LeaderLocatorHaloMatInstance.SetScalarParameterValue( 'Opacity', TaccomFadeOpacity );
			Canvas.DrawMaterialTile( LeaderLocatorHaloMatInstance, MatWidth*PulseScale, MatHeight*PulseScale );
		}
	}
}

/** Allow HUD to handle a pawn being spotted when targetting */
function HandlePawnTargetted( GearPawn TargetPawn )
{
	local GearPRI TargetPRI;
	local GearPawn MyGearPawn;

	MyGearPawn = GearPawn(GearPC(PlayerOwner).Pawn);

	if ( MyGearPawn != None )
	{
		// Check for leader
		TargetPRI = GearPRI(TargetPawn.PlayerReplicationInfo);
		if ( (TargetPRI != None) && TargetPRI.bIsLeader )
		{
			if ( WorldInfo.GRI.OnSameTeam(MyGearPawn, TargetPawn) )
			{
				UpdateLeaderSpotted_Friendly(TargetPawn);
			}
			else
			{
				UpdateLeaderSpotted_Enemy(TargetPawn);
			}
		}
	}
}

/** Start/update spotting a friendly leader */
function UpdateLeaderSpotted_Friendly( GearPawn SpottedPawn )
{
	LeaderToDrawIconOn_Friendly = SpottedPawn;

	// Clear the timers if the Leader to draw is null
	if ( LeaderToDrawIconOn_Friendly == None )
	{
		ClearTimer( 'OnDoneBlendingFriendlyLeaderIconIn' );
		ClearTimer( 'OnDoneBlendingFriendlyLeaderIconOut' );
	}
	// Restart the fadeout timer
	else if ( IsTimerActive('OnDoneBlendingFriendlyLeaderIconOut') )
	{
		SetTimer( LeaderIconBlendOutTime, FALSE, nameof(OnDoneBlendingFriendlyLeaderIconOut) );
	}
	// Start the blend-in timer if it hasn't been started already
	else if ( !IsTimerActive('OnDoneBlendingFriendlyLeaderIconIn') )
	{
		SetTimer( LeaderIconBlendInTime, FALSE, nameof(OnDoneBlendingFriendlyLeaderIconIn) );
	}
}

/** Function called when we are done blending the friendly leader icon in */
function OnDoneBlendingFriendlyLeaderIconIn()
{
	SetTimer( LeaderIconBlendOutTime, FALSE, nameof(OnDoneBlendingFriendlyLeaderIconOut) );
}

/** Function called when we are done blending the friendly leader icon out */
function OnDoneBlendingFriendlyLeaderIconOut()
{
	LeaderToDrawIconOn_Friendly = None;
}

/** Start/update spotting an enemy leader */
function UpdateLeaderSpotted_Enemy( GearPawn SpottedPawn )
{
	LeaderToDrawIconOn_Enemy = SpottedPawn;

	// Clear the timers if the Leader to draw is null
	if ( LeaderToDrawIconOn_Enemy == None )
	{
		ClearTimer( 'OnDoneBlendingEnemyLeaderIconIn' );
		ClearTimer( 'OnDoneBlendingEnemyLeaderIconOut' );
	}
	// Restart the fadeout timer
	else if ( IsTimerActive('OnDoneBlendingEnemyLeaderIconOut') )
	{
		SetTimer( LeaderIconBlendOutTime, FALSE, nameof(OnDoneBlendingEnemyLeaderIconOut) );
	}
	// Start the blend-in timer if it hasn't been started already
	else if ( !IsTimerActive('OnDoneBlendingEnemyLeaderIconIn') )
	{
		SetTimer( LeaderIconBlendInTime, FALSE, nameof(OnDoneBlendingEnemyLeaderIconIn) );
	}
}

/** Function called when we are done blending the enemy leader icon in */
function OnDoneBlendingEnemyLeaderIconIn()
{
	SetTimer( LeaderIconBlendOutTime, FALSE, nameof(OnDoneBlendingEnemyLeaderIconOut) );
}

/** Function called when we are done blending the enemy leader icon out */
function OnDoneBlendingEnemyLeaderIconOut()
{
	LeaderToDrawIconOn_Enemy = None;
}

/** Function that can be overloaded by game-specific HUDs to draw icons above the heads of players */
function DrawSpecialPlayerIcons()
{
	local float AlphaPercent, TimerCount;

	// Draw my leader
	if ( LeaderToDrawIconOn_Friendly != None )
	{
		TimerCount = GetTimerCount( 'OnDoneBlendingFriendlyLeaderIconIn' );
		if ( TimerCount > -1 )
		{
			AlphaPercent = TimerCount / LeaderIconBlendInTime;
		}
		else
		{
			TimerCount = GetTimerCount( 'OnDoneBlendingFriendlyLeaderIconOut' );
			if ( TimerCount > -1 )
			{
				AlphaPercent = 1.0f - (TimerCount / LeaderIconBlendOutTime);
			}
		}

		DrawLeaderIconAboveHead( LeaderToDrawIconOn_Friendly, (LeaderToDrawIconOn_Friendly.GetTeamNum() == 0) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED, 255*AlphaPercent );
	}

	// Draw enemy leader
	if ( LeaderToDrawIconOn_Enemy != None )
	{
		TimerCount = GetTimerCount( 'OnDoneBlendingEnemyLeaderIconIn' );
		if ( TimerCount > -1 )
		{
			AlphaPercent = TimerCount / LeaderIconBlendInTime;
		}
		else
		{
			TimerCount = GetTimerCount( 'OnDoneBlendingEnemyLeaderIconOut' );
			if ( TimerCount > -1 )
			{
				AlphaPercent = 1.0f - (TimerCount / LeaderIconBlendOutTime);
			}
		}

		DrawLeaderIconAboveHead( LeaderToDrawIconOn_Enemy, (LeaderToDrawIconOn_Enemy.GetTeamNum() == 0) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED, 255*AlphaPercent );
	}
}

/** Draw the leader icon above the head of the leader */
final function DrawLeaderIconAboveHead( GearPawn LeaderToDraw, EGearHUDColor ColorType, byte Alpha )
{
	local vector PawnExtent, ScreenLoc, CameraDir, LeaderDir;
	local GearPC PC;
	local vector CameraLoc;
	local rotator CameraRot;
	local float IconScale;

	PC = GearPC(PlayerOwner);

	PC.GetPlayerViewpoint( CameraLoc, CameraRot );

	if ( IsSquadMateVisible(LeaderToDraw, CameraLoc, CameraRot, PC) )
	{
		if ( (LeaderToDraw != None) && !LeaderToDraw.bIsHiddenByCamera && TimeSince(LeaderToDraw.LastRenderTime) < 2.f && PC.LineOfSightTo(LeaderToDraw) )
		{
			CameraDir = vector(CameraRot);
			LeaderDir = LeaderToDraw.Location - CameraLoc;
			if ( CameraDir Dot LeaderDir > 0 )
			{
				PawnExtent = LeaderToDraw.Mesh.GetBoneLocation( LeaderToDraw.HeadBoneNames[0] );
				PawnExtent.Z += 30.0f;
				ScreenLoc = Canvas.Project( PawnExtent );
				IconScale = 1.0f;
				ScreenLoc.X -= LeaderIcon.UL*IconScale / 2;
				ScreenLoc.Y -= LeaderIcon.VL*IconScale;

				// Set the text position and draw
				SetHUDDrawColor( ColorType, Alpha );
				Canvas.DrawIcon( LeaderIcon, ScreenLoc.X, ScreenLoc.Y, IconScale );
			}
		}
	}
}

defaultproperties
{
	ScoreBoardUISceneReference=GearUIScene_Scoreboard'UI_Scenes_MP.UI_Scoreboard_Warzone'
	EndRoundUISceneReference=GearUISceneMP_Base'UI_Scenes_MP.UI_EndOfRound'
	SpectatorUISceneReference=GearUIScene_Base'UI_Scenes_MP.UI_Game_Spectator'
	PauseMPUISceneReference=GearUIScenePause_MP'UI_Scenes_MP.UI_Pause_MP'

	LeaderLocatorHalo=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialGlow'
	EnemyIndicatorMaterialBG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialBG'
	EnemyIndicatorMaterialCOG=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialCOG'
	EnemyIndicatorMaterialLOC=Material'Gear_Annex_Effects.HUD.M_HUD_Annex_RadialLOC'

	LeaderTotalPulseTime=1.0f;
 	MyLeaderDiedSound=SoundCue'Music_Stingers.stinger_kryll02Cue'
 	EnemyLeaderDiedSound=SoundCue'Music_Stingers.stinger_music02Cue'

	LeaderIconBlendInTime=0.25f
	LeaderIconBlendOutTime=2.0f
}
