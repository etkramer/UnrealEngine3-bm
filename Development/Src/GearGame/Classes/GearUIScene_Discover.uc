/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUIScene_Discover extends GearUIScene_Base
	Config(UI);

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Label for displaying the title of the discoverable */
var transient UILabel TitleLabel;

/** Label for displaying the body of the discoverable */
var transient UILabel BodyLabel;

/** defines the animation for fading in the labels */
var	const	UIAnimationSeq	FadeInSequence;

/** Image of the background */
var transient UIImage BackgroundImage;

/** Whether we've paused the game from this scene or not */
var transient bool bHavePausedGame;

/** The stinger to play when this scene is opening */
var SoundCue PickupStingerCueResource;

/** The audio component used when looping the music during pause */
var transient AudioComponent PickupStingerAC;

/** Whether we are closing the scene (used for unpause detection) */
var transient bool bIsClosingScene;

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
* Called after this screen object's children have been initialized
* Overloaded to initialized the references to this scene's widgets
*/
event PostInitialize()
{
	InitializeWidgetReferences();
	OnGearUISceneTick=None;			// this was previously set in code (not defprops) so make sure to clear ref in case it got serialized.

	Super.PostInitialize();
}

/** Initialize the widget references */
final function InitializeWidgetReferences()
{
	// Find the labels
	TitleLabel = UILabel(FindChild('lblTitle', true));
	BodyLabel = UILabel(FindChild('lblBody', true));
	BackgroundImage = UIImage(FindChild('imgCollectible', true));
}

/** Set the string labels for the discoverable */
final function FindAndSetStrings()
{
	local string TitleString, BodyString, ImagePath;
	local GearPC MyGearPC;
	local GearUIDataStore_GameResource GameResourceDS;

	MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());

	// Locate the string values
	GameResourceDS = GetGameResourceDataStore();
	if ( MyGearPC != None && GameResourceDS != None )
	{
		if ( !GameResourceDS.GetDiscoverableDisplayStrings(MyGearPC.DiscoverType, TitleString, BodyString) )
		{
			`log(`location @ "failed to resolve title or description for discoverable" @ `showenum(EGearDiscoverableType,MyGearPC.DiscoverType));
		}

		if ( !GameResourceDS.GetDiscoverableBackgroundPath(MyGearPC.DiscoverType, ImagePath) )
		{
			`log(`location @ "failed to resolve background image pathname for discoverable" @ `showenum(EGearDiscoverableType,MyGearPC.DiscoverType));
		}
		else
		{
			BackgroundImage.SetDataStoreBinding(ImagePath);
		}
	}

	// Set the string values
	TitleLabel.SetDataStoreBinding( TitleString );
	BodyLabel.SetDataStoreBinding( BodyString );
}

/* == Delegate handlers == */
/** Called when the scene is activated so we can turn the music on and set the time for text fade */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local GearPC MyGearPC;

	MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
	if ( bInitialActivation && (MyGearPC != None) && !IsEditor() )
	{
		FindAndSetStrings();
		BodyLabel.Add_UIAnimTrackCompletedHandler(OnLabelFadeInComplete);

		TitleLabel.PlayUIAnimation('', FadeInSequence);
		BodyLabel.PlayUIAnimation('', FadeInSequence);

		// Play COGTag sound
		MyGearPC.PlaySound(PickupStingerCueResource, true);

		// If we are going to pause the game start the music
		if (ShouldPauseGame())
		{
			StartMusic(MyGearPC);
		}
	}
}

/** Called when the scene is closed so we can stop the music */
function OnSceneDeactivatedCallback( UIScene DeactivatedScene )
{
	local GearPC MyGearPC;

	MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
	if ( MyGearPC != None )
	{
		MyGearPC.DiscoverableSceneClosed( 0.f );
	}

	bIsClosingScene = true;

	// stop the music
	StopMusic();
	// Unpause game
	UnpauseGame();
}

/** Callback the server uses to determine if the unpause can happen */
function bool CanUnpause()
{
	return (bIsClosingScene || !ShouldPauseGame());
}

/** Whether we should pause the game or not */
function bool ShouldPauseGame()
{
	local WorldInfo WI;
	local GearPC PC;

	WI = GetWorldInfo();

	// Pause the game if there are no networked players
	if (WI.NetMode < NM_Client)
	{
		foreach WI.AllControllers(class'GearPC', PC)
		{
			if (!PC.IsLocalPlayerController())
			{
				// Found a networked player, bail
				return false;
			}
		}
		// I am the server and never found a networked player
		return true;
	}
	// I'm a networked client
	return false;
}

/** Pause the game */
function PauseGame()
{
	local GearPC MyGearPC;

	if (!bHavePausedGame)
	{
		MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
		MyGearPC.SetPause(true, CanUnpause);
	}
}

/** Unpause the game */
function UnpauseGame()
{
	local GearPC MyGearPC;

	if (bHavePausedGame)
	{
		MyGearPC = GetGearPlayerOwner(GetBestPlayerIndex());
		MyGearPC.SetPause(false);
	}
}

/** Start the background music */
function StartMusic(GearPC MyGearPC)
{
	local SoundCue MusicCue;

	MusicCue = SoundCue(FindObject("Interface_Audio.Menu.MenuPauseLoopCue", class'SoundCue'));
	if (MusicCue != None)
	{
		PickupStingerAC = MyGearPC.CreateAudioComponent(MusicCue, false, true);
		if (PickupStingerAC != none)
		{
			PickupStingerAC.bAllowSpatialization = false;
			PickupStingerAC.bAutoDestroy = true;
			PickupStingerAC.Play();
		}
	}
}

/** Stop the background music */
function StopMusic()
{
	if (PickupStingerAC != none)
	{
		PickupStingerAC.FadeOut(1.0f, 0.0f);
		PickupStingerAC = none;
	}
}

/**
 * Handler for the completion of the labels' fade-in animation...
 */
function OnLabelFadeInComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	`log(`location @ `showobj(Sender) @ `showvar(AnimName),TrackTypeMask==0,'DevUIAnimation');
	if ( TrackTypeMask == 0 && AnimName == 'FadeInText' )
	{
		// Pause game
		if (ShouldPauseGame())
		{
			PauseGame();
		}

		Sender.Remove_UIAnimTrackCompletedHandler(OnLabelFadeInComplete);
	}
}

/**
 * Callback function when the scene gets input
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInput( out InputEventParameters EventParms )
{
	local GearPC MyGearPC;

	// Must be able to pause the game when looking at a discoverable
	MyGearPC = GetGearPlayerOwner(EventParms.PlayerIndex);
	if ( EventParms.InputKeyName == 'XboxTypeS_Start' )
	{
		if ( (MyGearPC != None) && (EventParms.EventType == IE_Released) )
		{
			MyGearPC.ConditionalPauseGame();
		}

		return TRUE;
	}

	return FALSE;
}

/** Check for needing to unpause the game due to a networked JIP */
function UpdateCallback(float DeltaTime)
{
	// If we should not be pausing the game make sure we aren't pausing it and
	// that we are not playing music
	if (GetWorldInfo().NetMode < NM_Client &&
		!ShouldPauseGame())
	{
		StopMusic();
		UnpauseGame();
	}
}


defaultproperties
{
	OnRawInputKey=ProcessInput
	OnSceneActivated=OnSceneActivatedCallback
	OnSceneDeactivated=OnSceneDeactivatedCallback
	OnGearUISceneTick=UpdateCallback
	bCaptureMatchedInput=true
	PickupStingerCueResource=SoundCue'Interface_Audio.Objectives.CogTagsCue'

	SceneStackPriority=GEAR_SCENE_PRIORITY_DISCOVERABLES

	/** For the first time the scene is open. */
	Begin Object Class=UIAnimationSeq Name=FadeTextIn
		SeqName=FadeInText
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.4f,Data=(DestAsFloat=1.0))))
	End Object
	FadeInSequence=FadeTextIn
}
