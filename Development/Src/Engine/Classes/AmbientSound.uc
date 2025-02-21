//=============================================================================
// Ambient sound, sits there and emits its sound.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class AmbientSound extends Keypoint
	native(Sound);

/** Should the audio component automatically play on load? */
var() bool bAutoPlay;

/** Is the audio component currently playing? */
var private bool bIsPlaying;

/** Audio component to play */
var(Audio) editconst const AudioComponent AudioComponent;

cpptext
{
public:
	// AActor interface.
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();

protected:
	/**
	 * Starts audio playback if wanted.
	 */
	virtual void UpdateComponentsInternal(UBOOL bCollisionUpdate = FALSE);
public:
}

defaultproperties
{
	Begin Object NAME=Sprite
		Sprite=Texture2D'EditorResources.S_Ambient'
	End Object

	Begin Object Class=DrawSoundRadiusComponent Name=DrawSoundRadius0
		SphereColor=(R=50,G=50,B=240)
	End Object
	Components.Add(DrawSoundRadius0)
	
	Begin Object Class=AudioComponent Name=AudioComponent0
		PreviewSoundRadius=DrawSoundRadius0
		bAutoPlay=false
		bStopWhenOwnerDestroyed=true
		bShouldRemainActiveIfDropped=true
	End Object
	AudioComponent=AudioComponent0
	Components.Add(AudioComponent0)

	bAutoPlay=TRUE
	
	RemoteRole=ROLE_None
}
