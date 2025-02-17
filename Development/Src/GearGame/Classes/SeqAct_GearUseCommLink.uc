/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_GearUseCommLink extends SeqAct_Latent
	native(Sequence);

cpptext
{
	void Activated();
	UBOOL UpdateOp(FLOAT DeltaTime);
	void DeActivated();
};

/** true if this action was stopped via the stop input */
var protected bool bStopped;

/** true if the target actually started the commlink stance */
var protected bool bStarted;

/** True is the SingleLineSound has begun to play. */
var protected bool bSingleLineSoundIsPlaying;

/** true if the SingleLineSound finished*/
var protected bool bFinished;


/**
 * If this is None, the Target will be put into commlink mode until
 * the Stop input is activated.
 * If a valid cue is provided, the Target will be put into commlink mode
 * only for the duration of that sound.
 */
var() SoundCue SingleLineSound;

/** time remaining on the playing SingleLineSound, if specified */
var	float SoundDuration;

/** if true, target will go into commlink mode, with the hand-to-ear headset anim */
var() bool bUseCommLink;

var() protected const bool	bPlayerCanAbort;

var protected transient bool bAbortedByPlayer;

/** Lines that Marcus can say when he interrupts another character speaking. */
var() protected const array<SoundCue>		AbortLines;

/** How long to wait before action is abortable. */
var() protected const float AbortabilityDelay;
var protected transient float AbortabilityDelayRemaining;



/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 3;
}

defaultproperties
{
	bCallHandler=FALSE
	bUseCommLink=TRUE
	bPlayerCanAbort=TRUE
	AbortabilityDelay=1.f

	ObjName="Use CommLink"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Started")
	OutputLinks(2)=(LinkDesc="Stopped")
	OutputLinks(3)=(LinkDesc="SLS Finished")
	OutputLinks(4)=(LinkDesc="Player Aborted")

	AbortLines(0)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_AlrightWeGetIt_Medium02Cue'
	AbortLines(1)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_Enough_Medium02Cue'
	AbortLines(2)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_GiveItARest_Medium02Cue'
	AbortLines(3)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_ThatsEnough_Medium01Cue'
	AbortLines(4)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_GuysShutUp_Loud01Cue'
	AbortLines(5)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_HeyQuitYerYappin_Loud01Cue'
	AbortLines(6)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_LetsGoGuys_Medium02Cue'
	AbortLines(7)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_NotNow_Medium01Cue'
	AbortLines(8)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_NotNow_Medium02Cue'
	AbortLines(9)=SoundCue'Human_Marcus_Chatter_Cue.SilencingSquad.MarcusChatter_ShutIt_Medium01Cue'
}
