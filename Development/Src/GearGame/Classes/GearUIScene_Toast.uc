/**
 * This is the scene that appears when the player unlocks an achievement or an achievement is updated, etc.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearUIScene_Toast extends GearUIScene_Base
	Config(inherit);

const TRUE_BYTE=1;
const FALSE_BYTE=0;

const NUM_SLOTS=3;

/** We will never allow more than 50 alerts to be queued up */
const MAX_QUEUE_SIZE = 50;

struct AlertPanel
{
	/** The panel that slides out; contains the title and subtext */
	var transient UIPanel Panel;
	/** The title that appears at the top */
	var transient UILabel Title;
	/** The description or alert (depends on the alert type) */
	var transient UILabel SubText;
	/** The progress bar if one exists */
	var transient UIProgressBar ProgressBar;
	/** The icon */
	var transient UIImage Icon;
};

/** The flash image used for screenshot notification, etc.  */
var transient UIImage ImgScreenFlash;

/** Various alert panels that do the sliding out and show the notification */
var transient AlertPanel ProgressPanels[NUM_SLOTS];
var transient AlertPanel UnlockPanels[NUM_SLOTS];
var transient AlertPanel FriendPanels[NUM_SLOTS];

/** Tracks which of the 3 notification slots are available */
var transient byte IsSlotAvailable[NUM_SLOTS];

/** How many of the slots are being used */
var transient int NumActiveSlots;


/** Queue of alerts to be displayed. */
var transient array<AlertEvent> AlertQueue;

/** The audio component used for playing a sample sound on volume change */
var transient AudioComponent MySoundAC;

/** Horrible hack to delay the toast by 1 tick right after the scene has been opened */
var transient bool bHorribileHackDelayByOneTick;



// - - Animations - -

/** Animation for flash white/black then fade out quickly */
var transient UIAnimationSeq Flash;

/** Animation for fading in */
var transient UIAnimationSeq FadeIn;

/** Animation for fading out */
var transient UIAnimationSeq FadeOut;

/** Pulsing animation for fading in, then fading out */
var transient UIAnimationSeq PulseFade;

/** Fade in, but start at opacity 0 */
var transient UIAnimationSeq ZeroOpacity;

/** Fade in, slide in, hang out, slide back out; for 1280x720 */
var transient UIAnimationSeq SlideIn_1280[3];

/** Fade in, slide in, hang out, slide back out; for 960x720 */
var transient UIAnimationSeq SlideIn_960[3];


/**
 * Add a new alert event at the head of the queue of alert events to process
 *
 * @param Event	The alert event to add
 */
function PushAlert( const out AlertEvent Event )
{
	if ( Event.Type == eALERT_Screenshot )
	{
		// We can process screenshots right away; just flash the screen white
		ImgScreenFlash.SetVisibility(true);
		ImgScreenFlash.PlayUIAnimation( '', Flash, /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
		StartSound( GetGearPlayerOwner(), "Interface_Audio.Menu.CameraClick01Cue" );
	}
	else if (AlertQueue.Length < MAX_QUEUE_SIZE)
	{
		AlertQueue.AddItem(Event);
		//`Log("Queued: " $ string(Event.type) $ "  Title: " $ Event.Title $ "; " $ string(AlertQueue.Length) $ " in Queue"  );
	}
}

/**
 * Get the oldest alert event in the queue to process. Remove that alert event.
 *
 * @param	OutEvent	The alert event to populate with the value from the tail of the queue.
 *
 * @return	True if the alert event was successfully removed; false if the queue is empty.
 */
function bool PopAlert( out AlertEvent OutEvent )
{
	if ( AlertQueue.Length > 0 )
	{
		OutEvent = AlertQueue[0];
		AlertQueue.Remove(0, 1);
		//`Log("Removed event;" $ string(AlertQueue.Length) $ " in Queue");
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Test if there are any alert events that need displaying.
 *
 * @return	True if there are alert events waiting to be processed.
 */
function bool AreAlertsQueued()
{
	return AlertQueue.Length > 0;
}





/**
 * Called after this screen object's children have been initialized
 * Overloaded to set the deactivated callback
 */
event PostInitialize()
{
	// Initialize references to widgets
	InitializeWidgetReferences();

	Super.PostInitialize();

	bCloseOnLevelChange = false;
}

/** Initialize references to widgets */
function InitializeWidgetReferences()
{
	local int i;
	local float InitOpacity;

	ImgScreenFlash = UIImage(FindChild('imgScreenFlash', TRUE));

	UnlockPanels[0]=( BuildPanel(eALERT_Unlock, 1) );
	UnlockPanels[1]=( BuildPanel(eALERT_Unlock, 2) );
	UnlockPanels[2]=( BuildPanel(eALERT_Unlock, 3) );

	ProgressPanels[0]=( BuildPanel(eALERT_Progress, 1) );
	ProgressPanels[1]=( BuildPanel(eALERT_Progress, 2) );
	ProgressPanels[2]=( BuildPanel(eALERT_Progress, 3) );

	FriendPanels[0]=( BuildPanel(eALERT_FriendAlert, 1) );
	FriendPanels[1]=( BuildPanel(eALERT_FriendAlert, 2) );
	FriendPanels[2]=( BuildPanel(eALERT_FriendAlert, 3) );



	InitOpacity = (IsEditor()) ? 1.0 : 0.0;
	for (i=0; i<NUM_SLOTS; i++)
	{
		UnlockPanels[i].Panel.Opacity = InitOpacity;
		ProgressPanels[i].Panel.Opacity = InitOpacity;
		FriendPanels[i].Panel.Opacity = InitOpacity;
	}

	ImgScreenFlash.SetVisibility(FALSE);

}


/** Hack around the Unused */
function AlertPanel GetUnlockAlertPanel( int Id )
{
	return UnlockPanels[Id];
}


/**
 * Populate an AlertPanel to point at widgets existing in the scene.
 *
 * @param	AlertType	Used to determine the panel names to pull out;
 *                          only cares about ePROG_Unlockable vs not.
 * @param	Id
 */
function AlertPanel BuildPanel( EAlertType AlertType , int Id )
{
	// pnlProgress01-03
	// pnlUnlock01-03
	//
	// Progress widgets:
	// lblTitle - Achievement Name
	// lblProgress - ### / ###
	// imgIcon - achievement icon. Currently has a default achievement icon assigned to it from WarfareHUD.
	// UIProgressBar_Achievement
	//

	// pnlInvite 01 02 03
	// lblDescribe
	// imgIcon
	// lblTitle
	// imgToastBG

	// Unlock widgets:
	// 	lblTitle - simple "new unlock!" added loc string.
	//	lblDescribe - what you just unlocked. Keep as brief as possible.
	//	imgIcon - does not need to change. One icon for unlock types.

	local AlertPanel Panel;
	local string WidgetName;

	if (AlertType == eALERT_Unlock)
	{
		WidgetName ="pnlUnlock0" $ Id;
		Panel.Panel = UIPanel(FindChild( Name(WidgetName), TRUE));
		Panel.SubText = UILabel(Panel.Panel.FindChild('lblDescribe', TRUE));
	}
	else if (AlertType == eALERT_FriendAlert)
	{
		WidgetName ="pnlInvite0" $ Id;
		Panel.Panel = UIPanel(FindChild( Name(WidgetName), TRUE));
		Panel.SubText = UILabel(Panel.Panel.FindChild('lblDescribe', TRUE));
	}
	else
	{
		WidgetName ="pnlProgress0" $ Id;
		Panel.Panel = UIPanel(FindChild( Name(WidgetName), TRUE));
		Panel.SubText = UILabel(Panel.Panel.FindChild('lblProgress', TRUE));
		Panel.ProgressBar = UIProgressBar(Panel.Panel.FindChild('UIProgressBar_Achievement', TRUE));
	}

	Panel.Icon = UIImage(Panel.Panel.FindChild('imgIcon', TRUE));
	Panel.Title = UILabel(Panel.Panel.FindChild('lblTitle', TRUE));

	return Panel;
}

/**
 * Handler for the completion of this scene's opening animation...
 *
 * @warning - if you override this in a child class, keep in mind that this function will not be called if the scene has no opening animation.
 */
function OnOpenAnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.OnOpenAnimationComplete(Sender, AnimName, TrackTypeMask);
	OnGearUISceneTick = ToastTick;
}

/**
 * Notification that one or more tracks in an animation sequence have completed.
 *
 * @param	Sender				the widget that completed animating.
 * @param	AnimName			the name of the animation sequence that completed.
 * @param	TypeMask			a bitmask indicating which animation tracks completed.  It is generated by left shifting 1 by the
 *								values of the EUIAnimType enum.
 *								A value of 0 indicates that all tracks in the animation sequence have finished.
 */
event UIAnimationEnded( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	Super.UIAnimationEnded( Sender, AnimName, TrackTypeMask );


	//`Log( "Terminated animation " $ AnimName $ " TrackType " $ string(TrackTypeMask) );
	if (TrackTypeMask == 0)
	{
		if ( AnimName=='SlideIn_1280_0' || AnimName=='SlideIn_960_0' )
		{
			IsSlotAvailable[0] = TRUE_BYTE;
			NumActiveSlots--;
		}
		else if ( AnimName=='SlideIn_1280_1' || AnimName=='SlideIn_960_1' )
		{
			IsSlotAvailable[1] = TRUE_BYTE;
			NumActiveSlots--;
		}
		else if ( AnimName=='SlideIn_1280_2' || AnimName=='SlideIn_960_2' )
		{
			IsSlotAvailable[2] = TRUE_BYTE;
			NumActiveSlots--;
		}

		//if ( Sender == ImgScreenFlash )
		//{
		//	ImgScreenFlash.SetVisibility(FALSE);
		//}
	}

}

/**
 * Set the widget to a specified image
 *
 * @param Image image to modify
 * @param Path	path of image to show
 */
function SetImage( UIImage Image, string Path )
{
	local Texture2D MyTexture;

	if ( Left(Path, 1) == "<" )
	{
		//Image.SetValue(None);
		Image.SetDataStoreBinding(Path);
	}
	else
	{
		MyTexture = Texture2D(DynamicLoadObject(Path, class'Texture2D'));
		if ( MyTexture != None )
		{
			Image.ImageComponent.SetImage( MyTexture );
		}
	}
}


/**
 * Workaround for bogus warning	TTP#100386 Script compiler: unused local warning when local is used.
 */
function TTP100386Workaround(const out AlertPanel Evt)
{
	//`log("Workaround. Please fix TTP#100386.") ;
}

/**
 * Tries to find an available slot. Plays the notification in that slot if that is possible.
 *
 * @param Evt	An AlertEvent about which to notify the player
 *
 * @return true if we played the animation, false if we couldn't find a slot
 */
function bool PlayNotification( const out AlertEvent Evt )
{
	local int Slot;
	local AlertPanel MyPanel;
	local string DefaultIcon;

	//HACK
	TTP100386Workaround(MyPanel);

	for (Slot=0; Slot<NUM_SLOTS; Slot++)
	{
		//`Log( "Trying Slot: " $ string(slot) );
		if ( IsSlotAvailable[Slot] == TRUE_BYTE )
		{
			//`Log("Playing Slot: " $ string(slot));
			IsSlotAvailable[Slot] = FALSE_BYTE;

			if ( Evt.Type == eALERT_Unlock )
			{
				MyPanel = UnlockPanels[Slot];
				DefaultIcon = "Warfare_HUD.HUD_Unlocked";
				StartSound( GetGearPlayerOwner(), "Interface_Audio.Menu.MenuUnlockablesCue" );
			}
			else if ( Evt.Type == eALERT_FriendAlert )
			{
				MyPanel = FriendPanels[Slot];
				DefaultIcon = "Warfare_HUD.HUD_InviteSent";
			}
			else
			{
				MyPanel = ProgressPanels[Slot];
				if (Evt.PercentComplete >= 0)
				{
					MyPanel.ProgressBar.SetValue(Evt.PercentComplete, true);
					MyPanel.ProgressBar.SetVisibility(true);
				}
				else
				{
					MyPanel.ProgressBar.SetVisibility(false);
				}
				DefaultIcon = "UI_Art.FrontEnd.T_UI_DefaultGamerPic";
				StartSound( GetGearPlayerOwner(), "Interface_Audio.Menu.G2UI_VoteOpenCue" );
			}

			MyPanel.Title.SetValue(Evt.Title);
			MyPanel.SubText.SetValue(Evt.SubText);

			//`Log("Custom alert icon is" $ Evt.CustomIcon);

			if (Evt.CustomIcon != "")
			{
				SetImage(MyPanel.Icon, Evt.CustomIcon);
			}
			else
			{
				SetImage(MyPanel.Icon, DefaultIcon);
			}


			//@todo ronp - why not use this [more reliable?] check?
			//if ( GetAspectRatio() > ASPECTRATIO_Normal )
			// If we're running a res higher than 960x720 (supposedly 1280x820) then play the animation for a wide screen
			MyPanel.Panel.Opacity = 1.0f;
			if (CurrentViewportSize.X > 960)
			{
				MyPanel.Panel.PlayUIAnimation( '', SlideIn_1280[Slot], /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
			}
			else
			{
				MyPanel.Panel.PlayUIAnimation( '', SlideIn_960[Slot], /*OverrideLoopMode*/,/*PlaybackRate*/, /*InitialPosition*/, FALSE );
			}
			NumActiveSlots++;
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Handler for the OnGearUISceneTick delegate.
 *
 * @param DeltaTime	time since last tick.
 */
function ToastTick(float DeltaTime)
{
	local AlertEvent EventToProcess;

	if (NUM_SLOTS > NumActiveSlots && !bHorribileHackDelayByOneTick)
	{
		if (AreAlertsQueued())
		{
			PopAlert(EventToProcess);
			if( !PlayNotification(EventToProcess) )
			{
				PushAlert(EventToProcess);
			}
		}
	}

	if (bHorribileHackDelayByOneTick)
	{
		bHorribileHackDelayByOneTick = FALSE;
	}
}


/**
 * Play a sound.
 *
 * @param MyGearPC	the Player Controller which owns the Audio Component
 * @param SoundCuePath	the path to which sound to play
 */
function StartSound(GearPC MyGearPC, string SoundCuePath)
{
	local SoundCue MusicCue;

	StopSound();

	MusicCue = SoundCue(DynamicLoadObject(SoundCuePath, class'SoundCue'));
	if (MusicCue != None)
	{
		MySoundAC = MyGearPC.CreateAudioComponent(MusicCue, false, true);
		if (MySoundAC != none)
		{
			MySoundAC.bAllowSpatialization = false;
			MySoundAC.bAutoDestroy = true;
			MySoundAC.Play();
		}
	}
}

/** Stop the sound */
function StopSound()
{
	if (MySoundAC != none)
	{
		MySoundAC.FadeOut(0.5f, 0.0f);
		MySoundAC = none;
	}
}


DefaultProperties
{

	/** Animation tracks */

	//BREAKS STUFF: Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1300.0,Y=495.0,Z=0.0))),(RemainingTime=0.1,Data=(DestAsVector=(X=1100.0,Y=495.0,Z=0.0))),(RemainingTime=0.55,Data=(DestAsVector=(X=878.0,Y=495.0,Z=0.0)))))

	/** The three animation tracks for 1280x720 */
	//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1300.0,Y=495.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=950.0,Y=495.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=878.0,Y=495.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=878.0,Y=495.0,Z=0.0)))))
	Begin Object Class=UIAnimationSeq Name=SlideIn_1280_Template_0
		SeqName=SlideIn_1280_0
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1300.0)),(RemainingTime=0.15,Data=(DestAsFloat=950.0)),(RemainingTime=0.35,Data=(DestAsFloat=878.0)),(RemainingTime=6.0,Data=(DestAsFloat=878.0)),(RemainingTime=0.2,Data=(DestAsFloat=1300.0))))
		//(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1300.0,Y=495.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=950.0,Y=495.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=878.0,Y=495.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=878.0,Y=495.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1300.0,Y=495.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_1280(0) = SlideIn_1280_Template_0

	Begin Object Class=UIAnimationSeq Name=SlideIn_1280_Template_1
		SeqName=SlideIn_1280_1
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1300.0)),(RemainingTime=0.15,Data=(DestAsFloat=950.0)),(RemainingTime=0.35,Data=(DestAsFloat=878.0)),(RemainingTime=6.0,Data=(DestAsFloat=878.0)),(RemainingTime=0.2,Data=(DestAsFloat=1300.0))))
		//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1300.0,Y=440.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=950.0,Y=440.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=878.0,Y=440.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=878.0,Y=440.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1300.0,Y=440.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_1280(1) = SlideIn_1280_Template_1

	Begin Object Class=UIAnimationSeq Name=SlideIn_1280_Template_2
		SeqName=SlideIn_1280_2
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1300.0)),(RemainingTime=0.15,Data=(DestAsFloat=950.0)),(RemainingTime=0.35,Data=(DestAsFloat=878.0)),(RemainingTime=6.0,Data=(DestAsFloat=878.0)),(RemainingTime=0.2,Data=(DestAsFloat=1300.0))))
		//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1300.0,Y=385.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=950.0,Y=385.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=878.0,Y=385.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=878.0,Y=385.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1300.0,Y=385.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_1280(2) = SlideIn_1280_Template_2

	/** The three animation tracks for 960x720 */
	Begin Object Class=UIAnimationSeq Name=SlideIn_960_Template_0
		SeqName=SlideIn_960_0
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1000)),(RemainingTime=0.15,Data=(DestAsFloat=630.0)),(RemainingTime=0.35,Data=(DestAsFloat=558)),(RemainingTime=6.0,Data=(DestAsFloat=558.0)),(RemainingTime=0.2,Data=(DestAsFloat=1000.0))))
		//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1000.0,Y=495.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=630.0,Y=495.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=558.0,Y=495.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=558.0,Y=495.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1000.0,Y=495.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_960(0) = SlideIn_960_Template_0

	Begin Object Class=UIAnimationSeq Name=SlideIn_960_Template_1
		SeqName=SlideIn_960_1
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1000)),(RemainingTime=0.15,Data=(DestAsFloat=630.0)),(RemainingTime=0.35,Data=(DestAsFloat=558)),(RemainingTime=6.0,Data=(DestAsFloat=558.0)),(RemainingTime=0.2,Data=(DestAsFloat=1000.0))))
		//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1000.0,Y=440.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=630.0,Y=440.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=558.0,Y=440.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=558.0,Y=440.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1000.0,Y=440.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_960(1) = SlideIn_960_Template_1

	Begin Object Class=UIAnimationSeq Name=SlideIn_960_Template_2
		SeqName=SlideIn_960_2
		Tracks(0)=(TrackType=EAT_Left,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=1000)),(RemainingTime=0.15,Data=(DestAsFloat=630.0)),(RemainingTime=0.35,Data=(DestAsFloat=558)),(RemainingTime=6.0,Data=(DestAsFloat=558.0)),(RemainingTime=0.2,Data=(DestAsFloat=1000.0))))
		//Tracks(0)=(TrackType=EAT_Position,KeyFrames=((RemainingTime=0.0,Data=(DestAsVector=(X=1000.0,Y=385.0,Z=0.0))),(RemainingTime=0.15,Data=(DestAsVector=(X=630.0,Y=385.0,Z=0.0))),(RemainingTime=0.35,Data=(DestAsVector=(X=558.0,Y=385.0,Z=0.0))),(RemainingTime=4.0,Data=(DestAsVector=(X=558.0,Y=385.0,Z=0.0))),(RemainingTime=0.2,Data=(DestAsVector=(X=1000.0,Y=385.0,Z=0.0)))))
		Tracks(1)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.35,Data=(DestAsFloat=1.0))))
	End Object
	SlideIn_960(2) = SlideIn_960_Template_2

	Begin Object Class=UIAnimationSeq Name=Flash_Template
		SeqName=Flash
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.75)), (RemainingTime=0.2,Data=(DestAsFloat=0.3)), (RemainingTime=0.55,Data=(DestAsFloat=0.0))))
	End Object
	Flash = Flash_Template

	//bCaptureMatchedInput=FALSE

	IsSlotAvailable(0)=TRUE_BYTE
	IsSlotAvailable(1)=TRUE_BYTE
	IsSlotAvailable(2)=TRUE_BYTE

	NumActiveSlots=0

	Begin Object Name=SceneEventComponent
		DisabledEventAliases.Add(CloseScene)
	End Object

	SceneStackPriority=GEAR_SCENE_PRIORITY_NOTIFICATION
	bExemptFromAutoClose=true
	bCloseOnLevelChange=false
	SceneSkin=None
	bHorribileHackDelayByOneTick=true
}
