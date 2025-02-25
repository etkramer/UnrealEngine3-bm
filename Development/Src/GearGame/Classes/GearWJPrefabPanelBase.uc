/**
 * Base class for panels used in war journal pages which are instanced via UI prefabs.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWJPrefabPanelBase extends UIPanel
	abstract;

/** displays the friendly name of the item */
var	transient		UILabel					lblName;

/** the bar that is rendered over the item currently selected */
var	transient		UIImage					imgSelectionBar;

/** the image that displays the primary icon */
var	transient		UIImage					imgIcon;


var	transient		UIAnimationSeq			FadeIn_Selection;
var	transient		UIAnimationSeq			FadeOut_Selection;

/* == Delegates == */

/* == UnrealScript == */
/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	lblName = UILabel(FindChild('lblName',true));

	imgIcon = UIImage(FindChild('imgIcon',true));
	imgSelectionBar = UIImage(FindChild('imgSelectionBar',true));
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	// don't assign this delegate until PostInitialize is called or we'll try to change states for our children before they're
	// fully initialized.
	NotifyActiveStateChanged=OnStateChanged;
	imgSelectionBar.Add_UIAnimTrackCompletedHandler(SelectionBar_AnimationComplete);
}

/**
 * Wrapper for checking whether the child widget's enabled state should be toggled when the selection changes.
 */
function bool ShouldToggleWhenSelected( UIObject Child )
{
	return Child != imgSelectionBar;
}

function ToggleSelectedState( bool bIsSelected )
{
	local int ChildIndex;
	local int PlayerIndex;

	ShowSelectionBar(bIsSelected);

	PlayerIndex = GetPlayerOwnerIndex();
	for ( ChildIndex = 0; ChildIndex < Children.Length; ChildIndex++ )
	{
		if ( ShouldToggleWhenSelected(Children[ChildIndex]) )
		{
			Children[ChildIndex].SetEnabled(!bIsSelected, PlayerIndex);
		}
	}
}

/**
 * Accessor for setting the visiblity of the selection bar.
 */
function ShowSelectionBar( bool bVisible=true )
{
	local float InitialPosition;

	InitialPosition = -1;
	if ( bVisible )
	{
		if ( imgSelectionBar.IsAnimating('SelectionFadeOutAnim') )
		{
//			InitialPosition = 0.00001f;
			imgSelectionBar.StopUIAnimation('SelectionFadeOutAnim',,false);
		}

		imgSelectionBar.SetVisibility(true);
		imgSelectionBar.PlayUIAnimation('',FadeIn_Selection,/*UIANIMLOOP_MAX*/,/*1.f*/,InitialPosition,false);
	}
	else
	{
		if ( imgSelectionBar.IsAnimating('SelectionFadeInAnim') )
		{
//			InitialPosition = 0.00001f;
			imgSelectionBar.StopUIAnimation('SelectionFadeInAnim',,false);
		}

		imgSelectionBar.PlayUIAnimation('',FadeOut_Selection,/*UIANIMLOOP_MAX*/,/*1.f*/,InitialPosition,false);
	}
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called when a new UIState becomes the widget's currently active state, after all activation logic has occurred.
 *
 * @param	Sender					the widget that changed states.
 * @param	PlayerIndex				the index [into the GamePlayers array] for the player that activated this state.
 * @param	NewlyActiveState		the state that is now active
 * @param	PreviouslyActiveState	the state that used the be the widget's currently active state.
 */
function OnStateChanged( UIScreenObject Sender, int PlayerIndex, UIState NewlyActiveState, UIState PreviouslyActiveState )
{
	`log(`location @ `showobj(NewlyActiveState) @ "Sender:" $ PathName(Sender) @ `showvar(GetScene().IsFocused(PlayerIndex),SceneFocused),,'RON_DEBUG');
	if ( Sender == Self )
	{
		ToggleSelectedState(UIState_Focused(NewlyActiveState) != None);
	}
}
/**
 * Handler for this widget's OnProcessInputKey delegate.  Responsible for displaying this achievement's details when the correct
 * button is pressed.
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called AFTER kismet is given a chance to process the input, but BEFORE any native code processes the input.
 *
 * @param	EventParms	information about the input event, including the name of the input alias associated with the
 *						current key name (Tab, Space, etc.), event type (Pressed, Released, etc.) and modifier keys (Ctrl, Alt)
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool ProcessInputKey( const out SubscribedInputEventParameters EventParms )
{
	return EventParms.InputAliasName == 'ViewWJItemDetails';
}

/**
 * Handler for the completion of this panel's selection bar fade animations.
 *
 * @param	Sender			the widget executing the animation containing the track that just completed
 * @param	AnimName		the name of the animation sequence containing the track that completed.
 * @param	TrackTypeMask	a bitmask of EUIAnimType values indicating which animation tracks completed.  The value is generated by
 *							left shifting 1 by the value of the track type.  A value of 0 indicates that all tracks have completed (in
 *							other words, that the entire animation sequence is completed).
 */
function SelectionBar_AnimationComplete( UIScreenObject Sender, name AnimName, int TrackTypeMask )
{
	`log(`location @ `showobj(Sender) @ `showvar(AnimName),TrackTypeMask==0,'DevUIAnimation');
	if ( Sender == imgSelectionBar && TrackTypeMask == 0 && AnimName == 'SelectionFadeOutAnim' )
	{
		imgSelectionBar.SetVisibility(false);
	}
}

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	InitializeWidgetReferences();
	if ( !IsEditor() )
	{
		SetupCallbacks();
	}
}

/**
 * Generates a array of UI input aliases that this widget supports.
 *
 * @param	out_KeyNames	receives the list of input alias names supported by this widget
 */
event GetSupportedUIActionKeyNames( out array<Name> out_KeyNames )
{
	Super.GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem('ViewWJItemDetails');
}

DefaultProperties
{
	BackgroundImageComponent=None
	DefaultStates.Add(class'UIState_Focused')

	OnProcessInputKey=ProcessInputKey
	Begin Object Class=UIAnimationSeq Name=SelectionFadeInAnim
		SeqName=SelectionFadeInAnim
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.15,Data=(DestAsFloat=1.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.0,Data=(DestAsFloat=0.0)),(RemainingTime=0.3,Data=(DestAsFloat=1.0))))
	End Object
	FadeIn_Selection=SelectionFadeInAnim

	Begin Object Class=UIAnimationSeq Name=SelectionFadeOutAnim
		SeqName=SelectionFadeOutAnim
		Tracks(0)=(TrackType=EAT_Opacity,KeyFrames=((RemainingTime=0.15,Data=(DestAsFloat=0.0))))
		//Tracks(1)=(TrackType=EAT_PPBlurAmount,KeyFrames=((RemainingTime=0.2,Data=(DestAsFloat=0.0))))
	End Object
	FadeOut_Selection=SelectionFadeOutAnim
}
