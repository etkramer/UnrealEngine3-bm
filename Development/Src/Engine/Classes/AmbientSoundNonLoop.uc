//=============================================================================
// Version of AmbientSoundSimple that picks a random non-looping sound to play.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class AmbientSoundNonLoop extends AmbientSoundSimple
	native(Sound);

cpptext
{
public:
	// AActor interface.
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
}

defaultproperties
{
	Begin Object Name=DrawSoundRadius0
		SphereColor=(R=240,G=50,B=50)
	End Object

	Begin Object Name=AudioComponent0
		bShouldRemainActiveIfDropped=true
	End Object

	Begin Object Class=SoundNodeAmbientNonLoop Name=SoundNodeAmbientNonLoop0
	End Object
	SoundNodeInstance=SoundNodeAmbientNonLoop0
}
