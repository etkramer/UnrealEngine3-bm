/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_StreamInTextures extends SeqAct_Latent
	native(Sequence);

cpptext
{
	void Activated();
	UBOOL UpdateOp(FLOAT deltaTime);
	void DeActivated();
	virtual void PostLoad();

	virtual FString GetDisplayTitle() const;
};

/** Whether we should stream in textures based on location or usage. If TRUE, textures surrounding the attached actors will start to stream in. If FALSE, textures used by the attached actors will start to stream in. */
var()	bool	bLocationBased;

/** Number of seconds to force the streaming system to stream in all of the target's textures. If zero, it will continue streaming until there's an impulse to Stop. */
var()	float	Seconds;

/** Is this streaming currently active? */
var const bool	bStreamingActive;

/** Timestamp for when we should stop the forced texture streaming. */
var const float StopTimestamp;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Stream In Textures"
	ObjCategory="Actor"
	Seconds=15.0
	bLocationBased=false
	bStreamingActive=false
	StopTimestamp=0.0
	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")
}
