/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_PlaySound extends SeqAct_Latent
	native(Sequence);

cpptext
{
	void Activated();
	UBOOL UpdateOp(FLOAT deltaTime);
	void DeActivated();
	/** stops the sound on all targets */
	void Stop();
	virtual void CleanUp();

	void DrawTitleBar(FCanvas* Canvas, UBOOL bSelected, UBOOL bMouseOver, const FIntPoint& Pos, const FIntPoint& Size);
};


/** Sound cue to play on the targeted actor(s) */
var() SoundCue PlaySound;

/** Additional dead space to append to SoundDuration */
var() float ExtraDelay;

/** Remaining duration of sound, for timing activation of 'Finished' output */
var transient float SoundDuration;

/** Time taken for sound to fade in when action is activated. */
var()	float	FadeInTime;

/** Time take for sound to fade out when Stop input is fired. */
var()	float	FadeOutTime;

/** Volume multiplier propagated to audio component */
var()	float	VolumeMultiplier;

/** Pitch multiplier propagated to audio component */
var()	float	PitchMultiplier;

/** TRUE to suppress display of any subtitles the soundcue may have.  FALSE for normal subtitle behavior. */
var()	bool	bSuppressSubtitles;

// BM1
var() bool bLockVariableLinks;

/** Was this sound stopped? */
var transient bool bStopped;

// BM1
var int NumberOfParameters;
var array<float> StoredFloat;
var array<string> ParamNameList;

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
	return Super.GetObjClassVersion() + 2;
}

defaultproperties
{
	VolumeMultiplier=1
	PitchMultiplier=1
	ObjName="Play Sound"
	ObjCategory="Sound"

	InputLinks(0)=(LinkDesc="Play")
	InputLInks(1)=(LinkDesc="Stop")

	OutputLinks(0)=(LinkDesc="Out")
	OutputLinks(1)=(LinkDesc="Finished")
	OutputLinks(2)=(LinkDesc="Stopped")
}
