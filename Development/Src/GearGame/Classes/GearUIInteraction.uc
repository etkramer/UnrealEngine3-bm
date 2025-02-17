/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearUIInteraction extends UIInteraction
	within GearGameViewportClient
	native(UIPrivate);

/** the name of the movie to show while loading */
const	LOADING_MOVIE	= "LoadingMovie";

/**
 * Controls whether we're allowed to enter attract mode.  Disabled once the user advances past the title scene.
 */
var								transient	bool	bAttractModeAllowed;

/**
 * TRUE if we are currently playing the attract mode movie.
 */
var	const	private{private}	transient	bool	bAttractMoviePlaying;

/**
 * The approx. number of seconds since we last receieved input.  Only updated when bAttractModeAllowed is TRUE.
 */
var	const	private{private}	transient	float	IdleSeconds;


/**
 * Once IdleSeconds is greater than this value, attract mode is activated [if bAttractModeAllowed==TRUE].
 */
var	const						config		float	MaxIdleSeconds;


cpptext
{
	/* === UGearUIInteraction interface === */
	/**
	 * Begin playing the attract-mode movie.
	 */
	void BeginAttractMovie();

	/**
	 * Stop playing the attract mode movie.
	 */
	void EndAttractMovie();

	/**
	 * @return	TRUE if the attract mode movie is currently playing
	 */
	UBOOL IsAttractMoviePlaying() const
	{
		return bAttractMoviePlaying;
	}

	/* === UUIInteraction interface === */
	/**
	 * Called once a frame to update the interaction's state.
	 *
	 * @param	DeltaTime - The time since the last frame.
	 */
	virtual void Tick( FLOAT DeltaTime );

	/**
	 * Check a key event received by the viewport.
	 *
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @param	bGamepad - input came from gamepad (ie xbox controller)
	 *
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed=1.f,UBOOL bGamepad=FALSE);

	/**
	 * Check an axis movement received by the viewport.
	 *
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Key - The name of the axis which moved.
	 * @param	Delta - The axis movement delta.
	 * @param	DeltaTime - The time since the last axis update.
	 *
	 * @return	True to consume the axis movement, false to pass it on.
	 */
	virtual UBOOL InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	/**
	 * Check a character input received by the viewport.
	 *
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Character - The character.
	 *
	 * @return	True to consume the character, false to pass it on.
	 */
	virtual UBOOL InputChar(INT ControllerId,TCHAR Character);
}

/**
 * @return	TRUE if mature language is supported by the current language/region/etc.
 */
native final function bool IsMatureLanguageSupported() const;

defaultproperties
{
	SceneClientClass=class'GearGame.GearGameUISceneClient'
}

