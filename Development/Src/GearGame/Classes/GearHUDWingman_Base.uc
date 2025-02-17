/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearHUDWingman_Base extends GearHUDMP_Base
	abstract;


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** String to display when your buddy has died */
var localized string BuddyDiedString;

/** Material Instance for drawing the glow around your buddy's head in taccom */
var Material BuddyLocatorHalo;
/** Material Instance for drawing the glow around your buddy's head in taccom */
var MaterialInstanceConstant BuddyLocatorHaloMatInstance;

/** Materials for drawing the Buddy Indicator */
var Material BuddyIndicatorMaterialBG;
var Material BuddyIndicatorMaterialCOG;
var Material BuddyIndicatorMaterialLOC;
/** Material Instances for drawing the Buddy Indicator */
var MaterialInstanceConstant BuddyIndicatorMatInstanceBG;
var MaterialInstanceConstant BuddyIndicatorMatInstanceCOG;
var MaterialInstanceConstant BuddyIndicatorMatInstanceLOC;

/** Variables for controlling the pulsing effect in the taccom for showing the buddy */
var float BuddyPulseStartTime;
var float BuddyTotalPulseTime;

/** Played when my buddy is killed **/
var SoundCue MyBuddyDiedSound;
/** Audio component for buddy died sounds */
var AudioComponent BuddyDiedAudioComponent;

/** Time to blend into showing the buddy icons */
var const float BuddyIconBlendInTime;
/** Time to blend out of showing the buddy icons */
var const float BuddyIconBlendOutTime;

/** Reference to my Buddy's PRI */
var GearPRI MyBuddyPRI;
/** Reference to my Buddy's pawn */
var GearPawn MyBuddyPawn;

/** Buddy spotted by targetting */
var GearPawn BuddyToDrawIconOn;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Reset the HUD */
function Reset()
{
	Super.Reset();

	BuddyPulseStartTime = 0;
	UpdateBuddySpotted( None );
}

/** Overloaded to clear the spotted meatflag */
function EnableAssessMode()
{
	Super.EnableAssessMode();

	UpdateBuddySpotted( None );
}

/** Initialize some data */
simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	// initialize the buddy halo
	if ( BuddyLocatorHaloMatInstance == None )
	{
		BuddyLocatorHaloMatInstance = new(outer) class'MaterialInstanceConstant';
		BuddyLocatorHaloMatInstance.SetParent( BuddyLocatorHalo );
	}

	// initialize the buddy BG
	if ( BuddyIndicatorMatInstanceBG == None )
	{
		BuddyIndicatorMatInstanceBG = new(outer) class'MaterialInstanceConstant';
		BuddyIndicatorMatInstanceBG.SetParent( BuddyIndicatorMaterialBG );
	}

	// initialize the buddy indicator COG material
	if ( BuddyIndicatorMatInstanceCOG == None )
	{
		BuddyIndicatorMatInstanceCOG = new(outer) class'MaterialInstanceConstant';
		BuddyIndicatorMatInstanceCOG.SetParent( BuddyIndicatorMaterialCOG );
	}

	// initialize the buddy indicator Locust material
	if ( BuddyIndicatorMatInstanceLOC == None )
	{
		BuddyIndicatorMatInstanceLOC = new(outer) class'MaterialInstanceConstant';
		BuddyIndicatorMatInstanceLOC.SetParent( BuddyIndicatorMaterialLOC );
	}

	// initialize the buddy indicator blood splat material
	if( DeadTaccomSplatMaterialConstant == None )
	{
		DeadTaccomSplatMaterialConstant = new(outer) class'MaterialInstanceConstant';
		DeadTaccomSplatMaterialConstant.SetParent( DeadTaccomSplatMaterial );
	}
}

/** Overloaded Draw call */
function DrawHUD()
{
	local GearPawn AWPawn;
	local GearPRI MyGearPRI;
	local GearPC MyGearPC;
	local int PRIIdx;

	Super.DrawHUD();

	if (bShowHud &&
		!HideHUDFromScenes() &&
		(WorldInfo.GRI != None) &&
		(GearGRI(WorldInfo.GRI).GameStatus == GS_RoundInProgress) )
	{
		MyGearPC = GearPC(PlayerOwner);
		MyGearPRI = GearPRI(MyGearPC.PlayerReplicationInfo);
		MyBuddyPawn = None;
		MyBuddyPRI = None;

		// Look for my buddy's pawn
		foreach WorldInfo.AllPawns( class'GearPawn', AWPawn )
		{
			if ( (MyGearPRI != AWPawn.PlayerReplicationInfo) && (MyGearPRI.GetTeamNum() == AWPawn.PlayerReplicationInfo.GetTeamNum()) )
			{
				MyBuddyPawn = AWPawn;
				MyBuddyPRI = GearPRI(AWPawn.PlayerReplicationInfo);
				break;
			}
		}

		// Didn't find the pawn and pri so lets look for the pri in the priarray (dude might be dead)
		if ( MyBuddyPRI == None )
		{
			for ( PRIIdx = 0; PRIIdx < WorldInfo.GRI.PRIArray.length; PRIIdx++ )
			{
				if ( (WorldInfo.GRI.PRIArray[PRIIdx] != MyGearPRI) &&
					 (WorldInfo.GRI.PRIArray[PRIIdx].GetTeamNum() == MyGearPRI.GetTeamNum()) )
				{
					MyBuddyPRI = GearPRI(WorldInfo.GRI.PRIArray[PRIIdx]);
					break;
				}
			}
		}

		// If there is no buddy PRI that means you don't have one
		if ( MyBuddyPRI != None && MyBuddyPawn != None &&
			(MyBuddyPawn.TimeOfDeath == 0.0 || WorldInfo.TimeSeconds - MyBuddyPawn.TimeOfDeath < 4.0) )
		{
			DrawBuddy();
		}
	}
}

/** Allow HUD to handle a pawn being spotted when targetting */
function HandlePawnTargetted( GearPawn TargetPawn )
{
	local GearPawn MyGearPawn;

	MyGearPawn = GearPawn(GearPC(PlayerOwner).Pawn);

	if ( MyGearPawn != None )
	{
		// Check for buddy
		if ( WorldInfo.GRI.OnSameTeam(MyGearPawn, TargetPawn) )
		{
			UpdateBuddySpotted(TargetPawn);
		}
	}
}

/** Start/update spotting a friendly leader */
function UpdateBuddySpotted( GearPawn SpottedPawn )
{
	BuddyToDrawIconOn = SpottedPawn;

	// Clear the timers if the Leader to draw is null
	if ( BuddyToDrawIconOn == None )
	{
		ClearTimer( 'OnDoneBlendingBuddyIconIn' );
		ClearTimer( 'OnDoneBlendingBuddyIconOut' );
	}
	// Restart the fadeout timer
	else if ( IsTimerActive('OnDoneBlendingBuddyIconOut') )
	{
		SetTimer( BuddyIconBlendOutTime, FALSE, nameof(OnDoneBlendingBuddyIconOut) );
	}
	// Start the blend-in timer if it hasn't been started already
	else if ( !IsTimerActive('OnDoneBlendingBuddyIconIn') )
	{
		SetTimer( BuddyIconBlendInTime, FALSE, nameof(OnDoneBlendingBuddyIconIn) );
	}
}

/** Function called when we are done blending the friendly leader icon in */
function OnDoneBlendingBuddyIconIn()
{
	SetTimer( BuddyIconBlendOutTime, FALSE, nameof(OnDoneBlendingBuddyIconOut) );
}

/** Function called when we are done blending the friendly leader icon out */
function OnDoneBlendingBuddyIconOut()
{
	BuddyToDrawIconOn = None;
}

/** Function that can be overloaded by game-specific HUDs to draw icons above the heads of players */
function DrawSpecialPlayerIcons()
{
	local float AlphaPercent, TimerCount;
	local EGearHUDColor ColorType;

	if ( (BuddyToDrawIconOn != None) && (WorldInfo.GRI != None) )
	{
		TimerCount = GetTimerCount( 'OnDoneBlendingBuddyIconIn' );
		if ( TimerCount > -1 )
		{
			AlphaPercent = TimerCount / BuddyIconBlendInTime;
		}
		else
		{
			TimerCount = GetTimerCount( 'OnDoneBlendingBuddyIconOut' );
			if ( TimerCount > -1 )
			{
				AlphaPercent = 1.0f - (TimerCount / BuddyIconBlendOutTime);
			}
		}

		ColorType = IsCOG(BuddyToDrawIconOn.PlayerReplicationInfo) ? eWARHUDCOLOR_TEAMBLUE : eWARHUDCOLOR_TEAMRED;

		DrawBuddyIconAboveHead( BuddyToDrawIconOn, ColorType, 255*AlphaPercent );
	}
}

/** Draw the buddy icon above the head of the buddy */
final function DrawBuddyIconAboveHead( GearPawn PawnToDraw, EGearHUDColor ColorType, byte Alpha )
{
	local vector PawnExtent, ScreenLoc, CameraDir, BuddyDir;
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
			BuddyDir = PawnToDraw.Location - CameraLoc;
			if ( CameraDir Dot BuddyDir > 0 )
			{
				PawnExtent = PawnToDraw.Mesh.GetBoneLocation( PawnToDraw.HeadBoneNames[0] );
				PawnExtent.Z += 30.0f;
				ScreenLoc = Canvas.Project( PawnExtent );
				IconScale = 1.0f;
				ScreenLoc.X -= MeatflagIcon.UL*IconScale / 2;
				ScreenLoc.Y -= MeatflagIcon.VL*IconScale;

				// Set the text position and draw
				SetHUDDrawColor( ColorType, Alpha );
				Canvas.DrawIcon( IsCOG(PawnToDraw.PlayerReplicationInfo) ? BuddyIconCOG : BuddyIconLocust, ScreenLoc.X, ScreenLoc.Y, IconScale );
			}
		}
	}
}

/** Draw your teammate on the HUD */
final function DrawBuddy()
{
	local MaterialInstanceConstant TeamBackGroundMaterial;
	local float DrawX, DrawY, LocalDrawX, LocalDrawY, CPIWidth, CPIHeight, Scale, IconScale, Alpha;
	local CanvasIcon Icon, BuddyIcon;

	Alpha = 255 * (1.0f - TaccomFadeOpacity);
	if (MyBuddyPawn.TimeOfDeath > 0.0 && WorldInfo.TimeSeconds - MyBuddyPawn.TimeOfDeath > 3.0)
	{
		Alpha *= 1.0 - (WorldInfo.TimeSeconds - MyBuddyPawn.TimeOfDeath - 3.0);
	}
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
	BuddyIndicatorMatInstanceBG.SetScalarParameterValue('Opacity', Alpha / 255.0);
	Canvas.DrawMaterialTile( BuddyIndicatorMatInstanceBG, CPIWidth, CPIHeight );

	// Draw the team's material
	TeamBackGroundMaterial = IsCOG(MyBuddyPRI) ? BuddyIndicatorMatInstanceCOG : BuddyIndicatorMatInstanceLOC;
	Canvas.SetPos( LocalDrawX, LocalDrawY );
	if ( IsCOG(MyBuddyPRI) )
	{
		TeamBackGroundMaterial.SetVectorParameterValue( 'SymbolColorCOG', MakeLinearColor(139.f/255.f, 255.f/255.f, 255.f/255.f, 0.4f) );
	}
	else
	{
		TeamBackGroundMaterial.SetVectorParameterValue( 'SymbolColorLOC', MakeLinearColor(205.f/255.f, 22.f/255.f, 22.f/255.f, 0.4f) );
	}
	TeamBackGroundMaterial.SetScalarParameterValue('Opacity', Alpha / 255.0);
	Canvas.DrawMaterialTile( TeamBackGroundMaterial, CPIWidth, CPIHeight );

	// Draw the location pointer
	if (MyBuddyPawn.Health > 0 || MyBuddyPawn.IsDBNO())
	{
		DrawSquadLocator( MyBuddyPawn, LocalDrawX + CPIWidth/2, LocalDrawY + CPIHeight/2, Scale, Alpha );
	}

	// Draw buddy face
	Icon = MyBuddyPRI.PawnClass.default.HeadIcon;
	IconScale = 1.25f * Scale;
	LocalDrawX = DrawX - (Icon.UL*IconScale/2);
	LocalDrawY = DrawY - (Icon.VL*IconScale*0.75f);
	Canvas.DrawIcon( Icon, LocalDrawX, LocalDrawY, IconScale );

	// Draw the buddy icon
	BuddyIcon = IsCOG(MyBuddyPRI) ? BuddyIconCOG : BuddyIconLocust;
	LocalDrawX = DrawX - BuddyIcon.UL*IconScale/2;
	LocalDrawY = DrawY + (Icon.VL*IconScale*0.25f);
	Canvas.DrawIcon( BuddyIcon, LocalDrawX, LocalDrawY, IconScale );

	// Draw the blood splat if the leader is dead
	if (MyBuddyPawn.Health <= 0 && !MyBuddyPawn.IsDBNO())
	{
		LocalDrawX = DrawX - 128.f*Scale;
		LocalDrawY = DrawY - 256.f*Scale;
		Canvas.SetPos( DrawX, DrawY );
		DeadTaccomSplatMaterialConstant.SetScalarParameterValue( 'TaccomDead_Fade', 2.0f - (2.0f * (Alpha / 255.0)) );
		Canvas.SetPos( LocalDrawX, LocalDrawY );
		Canvas.DrawMaterialTile( DeadTaccomSplatMaterialConstant, 256.f*Scale, 512.f*Scale );
	}
}

/** Message from the game object that my buddy has died */
function StartBuddyDiedMessage( GearPRI BuddyPRI )
{
	StartDrawingDramaticText( BuddyDiedString, BuddyPRI.PlayerName, 8.0f, 0.25f, 2.0f, GetCountdownYPosition() + GetCountdownHeight() + GearPC(PlayerOwner).NumPixelsToMoveDownForDramaText(), 0.25f, 2.0f );

	if ( BuddyDiedAudioComponent != None && BuddyDiedAudioComponent.IsPlaying() )
	{
		BuddyDiedAudioComponent.Stop();
	}
	BuddyDiedAudioComponent = CreateAudioComponent(MyBuddyDiedSound, true, true);
}

/** Special icon to draw above the name of the player in taccom */
function EGearSpecialIconType SpecialIconToDrawAboveHead(GearPawn PawnToTest)
{
	if ( PawnToTest.PlayerReplicationInfo != None )
	{
		return (IsCOG(PawnToTest.PlayerReplicationInfo) ? eGSIT_BuddyCOG : eGSIT_BuddyLocust);
	}

	return eGSIT_None;
}

// Draw a glow ring around the background
function DrawBackgroundRing( GearPawn AGearPawn, float CenterXCoord, float CenterYCoord, float Scale )
{
	local float PulseScale, MatWidth, MatHeight, LocalDrawX, LocalDrawY, GlowOpacity;
	local LinearColor GlowColor;

	// See if we just finished a pulse
	if ( (BuddyPulseStartTime + BuddyTotalPulseTime) < WorldInfo.TimeSeconds )
	{
		BuddyPulseStartTime = WorldInfo.TimeSeconds;
	}
	// else draw the pulse
	else
	{
		PulseScale = 1.2f;

		if ( (WorldInfo.TimeSeconds - BuddyPulseStartTime) < (BuddyTotalPulseTime / 2) )
		{
			GlowOpacity = (WorldInfo.TimeSeconds - BuddyPulseStartTime) / (BuddyTotalPulseTime / 2);
		}
		else
		{
			GlowOpacity = 1.f - ((WorldInfo.TimeSeconds - BuddyPulseStartTime - (BuddyTotalPulseTime / 2)) / (BuddyTotalPulseTime / 2));
		}

		BuddyLocatorHaloMatInstance.SetScalarParameterValue( 'GlowOpacity', GlowOpacity );

		GlowColor = IsCOG(AGearPawn.PlayerReplicationInfo) ? MakeLinearColor(0.44f, 0.9f, 0.95f, 1.0f) : MakeLinearColor(1.0f, 0.09f, 0.09f, 1.0f);

		BuddyLocatorHaloMatInstance.SetVectorParameterValue( 'Color', GlowColor );

		MatWidth = 128.f * Scale;
		MatHeight = 128.f * Scale;

		LocalDrawX = CenterXCoord - MatWidth*PulseScale/2;
		LocalDrawY = CenterYCoord - MatHeight*PulseScale/2;
		Canvas.SetPos( LocalDrawX, LocalDrawY );
		BuddyLocatorHaloMatInstance.SetScalarParameterValue( 'Opacity', TaccomFadeOpacity );
		Canvas.DrawMaterialTile( BuddyLocatorHaloMatInstance, MatWidth*PulseScale, MatHeight*PulseScale );
	}
}

/** Whether we should draw the team color of the player */
function bool ShouldDrawTeamColor( PlayerReplicationInfo OtherPRI )
{
	if ( PlayerOwner.PlayerReplicationInfo != None && OtherPRI != None )
	{
		if ( PlayerOwner.PlayerReplicationInfo.GetTeamNum() == OtherPRI.GetTeamNum() )
		{
			return true;
		}
	}

	return false;
}

defaultproperties
{
	BuddyTotalPulseTime=1.0f;
	BuddyIconBlendInTime=0.25f
	BuddyIconBlendOutTime=2.0f
}
