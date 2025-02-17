/*=============================================================================
	AudioDevice.h: Native AudioDevice calls
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

protected:
	friend class FSoundSource;

	/** Constructor */
	UAudioDevice( void ) 
	{
	}

	/**
	 * Add any referenced objects that shouldn't be garbage collected
	 */
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );

	/**
	 * Complete the destruction of this class
	 */
	virtual void FinishDestroy( void );

	/**
	 * Handle pausing/unpausing of sources when entering or leaving pause mode
	 */
	void HandlePause( UBOOL bGameTicking );

	/**
	 * Stop sources that are no longer audible
	 */
	void StopSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex );

	/**
	 * Start and/or update any sources that have a high enough priority to play
	 */
	void StartSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex, UBOOL bGameTicking );

	/**
	 * Lists all the loaded sounds and their memory footprint
	 */
	virtual void ListSounds( const TCHAR* Cmd, FOutputDevice& Ar )
	{
	}

	/**
	 * Sets the 'pause' state of sounds which are always loaded.
	 *
	 * @param	bPaused			Pause sounds if TRUE, play paused sounds if FALSE.
	 */
	virtual void PauseAlwaysLoadedSounds(UBOOL bPaused)
	{
	}

	/**
	 * Check for errors and output a human readable string
	 */
	virtual UBOOL ValidateAPICall( const TCHAR* Function, INT ErrorCode )
	{
		return( TRUE );
	}

	/**
	 * Lists all the playing waveinstances and their associated source
	 */
	void ListWaves( FOutputDevice& Ar );

	/**
	 * Lists a summary of loaded sound collated by class
	 */
	void ListSoundClasses( FOutputDevice& Ar );

	/**
	 * Set up the sound class hierarchy
	 */
	void InitSoundClasses( void );

	/**
	 * Load up all requested sound modes
	 */
	void InitSoundModes( void );

	/**
	 * Parses the sound classes and propagates multiplicative properties down the tree.
	 */
	void ParseSoundClasses( void );

	/**
	 * Construct the CurrentSoundClassProperties map
	 *
	 * This contains the original sound class properties propagated properly, and all adjustments due to the sound mode
	 */
	void GetCurrentSoundClassState( void );

	/**
	 * Gets the parameters for EQ based on settings and time
	 */
	void Interpolate( FLOAT InterpValue, FSoundClassProperties& Current, FSoundClassProperties& Start, FSoundClassProperties& End );

	/**
	 * Set the mode for altering sound class properties
	 */
	void ApplySoundMode( USoundMode* NewMode );

	/**
	 * PostEditChange
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/**
	 * Serialize
	 */
	virtual void Serialize( FArchive& Ar );

	/**
	 * Platform dependent call to init effect data on a sound source
	 */
	void* InitEffect( FSoundSource* Source );

	/**
 	 * Platform dependent call to update the sound output with new parameters
	 */
	void* UpdateEffect( FSoundSource* Source );

	/**
	 * Platform dependent call to destroy any effect related data
	 */
	void DestroyEffect( FSoundSource* Source );

	/**
	 * Return the pointer to the sound effects handler
	 */
	class FAudioEffectsManager* GetEffects( void )
	{
		return( Effects );
	}

	/**
	 * Checks to see if the low pass filter has been disabled
	 */
	UBOOL IsDisabledLowPassFilter( void ) const
	{
		return( bDisableLowPassFilter );
	}

	/**
	 * Checks to see if the EQ filter has been disabled
	 */
	UBOOL IsDisabledEQFilter( void ) const
	{
		return( bDisableEQFilter );
	}

	/** Internal */
	void SortWaveInstances( INT MaxChannels );

	/**
	 * Internal helper function used by ParseSoundClasses to traverse the tree.
	 *
	 * @param CurrentClass			Subtree to deal with
	 * @param ParentProperties		Propagated properties of parent node
	 */
	void RecurseIntoSoundClasses( FSoundClass* CurrentClass, FSoundClassProperties* ParentProperties );

public:
	/**
	 * Basic initialisation of the platform agnostic layer of the audio system
	 */
	virtual UBOOL Init( void );

	/**
	 * The audio system's main "Tick" function
	 */
	virtual void Update( UBOOL bGameTicking );

	/**
	 * Iterate over the active AudioComponents for wave instances that could be playing
	 */
	INT GetSortedActiveWaveInstances( TArray<FWaveInstance*>& WaveInstances, UBOOL bGameTicking );

	/**
	 * Exec
	 */
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar = *GLog );

	/**
	 * Stop all the audio components and sources attached to the scene. NULL scene means all components.
	 */
	virtual void Flush( FSceneInterface* Scene );

	/**
	 * Precaches the passed in sound node wave object.
	 *
	 * @param	SoundNodeWave	Resource to be precached.
	 */
	virtual void Precache( USoundNodeWave* SoundNodeWave )
	{
	}

	/**
	 * Frees the bulk resource data assocated with this SoundNodeWave.
	 *
	 * @param	SoundNodeWave	wave object to free associated bulk data
	 */
	virtual void FreeResource( USoundNodeWave* SoundNodeWave )
	{
	}

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param bShouldStopUISounds If TRUE, this function will stop UI sounds as well
	 */
	virtual void StopAllSounds( UBOOL bShouldStopUISounds = FALSE );

	/**
	 * Sets the listener's location and orientation for the viewport
	 */
	void SetListener( INT ViewportIndex, INT MaxViewportIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front );

	/**
	 * Pushes the specified reverb settings onto the reverb settings stack.
	 *
	 * @param	ReverbSettings		The reverb settings to use.
	 */
	void SetReverbSettings( const FReverbSettings& ReverbSettings );

	/**
	 * Creates an audio component to handle playing a sound cue
	 */
	static UAudioComponent* CreateComponent( USoundCue* SoundCue, FSceneInterface* Scene, AActor* Actor = NULL, UBOOL Play = TRUE, UBOOL bStopWhenOwnerDestroyed = FALSE, FVector* Location = NULL );

	/**
	 * Adds a component to the audio device
	 */
	void AddComponent( UAudioComponent* AudioComponent );

	/**
	 * Removes an attached audio component
	 */
	void RemoveComponent( UAudioComponent* AudioComponent );

	/** 
	 * Gets a summary of loaded sound collated by class
	 */
	void GetSoundClassInfo( TMap<FName, FAudioClassInfo>& AudioClassInfos );

	/**
	 * Returns the sound class properties associates with a sound class taking into account
	 * the class tree.
	 *
	 * @param	SoundClassName	name of sound class to retrieve properties from
	 * @return	sound class properties if a sound class with name SoundClassName exists, NULL otherwise
	 */
	FSoundClassProperties* GetSoundClassProperties( FName SoundClassName );

	/**
	 * Updates sound class volumes
	 */
	void SetClassVolume( FName Class, FLOAT Volume );

	/**
	 * Checks to see if a coordinate is within a distance of any listener
	 */
	UBOOL LocationIsAudible( FVector Location, FLOAT MaxDistance );

	/**
	 * Add a newly created sound mode to the base set
 	 */
	void AddMode( USoundMode* Mode );

	/**
	 * Creates a soundcue and PCM data for a the text in SpokenText
	 */ 
	USoundCue* CreateTTSSoundCue( const FString& SpokenText, ETTSSpeaker Speaker );

	friend class FAudioEffectsManager;
	friend class FALAudioEffectsManager;
	friend class FXAudio2EffectsManager;
	friend class FPS3AudioEffectsManager;

	friend class FALSoundSource;
	friend class FXAudio2SoundSource;
	friend class FPS3SoundSource;
