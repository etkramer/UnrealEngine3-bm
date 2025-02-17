/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class AudioDevice extends Subsystem
	config( engine )
	native( AudioDevice )
	transient;

/** 
 * Filled out with entries from DefaultEngine.ini
 */
enum ESoundClassName
{
	Master
};

enum ETTSSpeaker
{
	TTSSPEAKER_Paul,
	TTSSPEAKER_Harry,
	TTSSPEAKER_Frank,
	TTSSPEAKER_Dennis,
	TTSSPEAKER_Kit,
	TTSSPEAKER_Betty,
	TTSSPEAKER_Ursula,
	TTSSPEAKER_Rita,
	TTSSPEAKER_Wendy,
};

/** 
 * Defines the properties of the listener
 */
struct native Listener
{
	var const PortalVolume PortalVolume;
	var vector Location;
	var vector Up;
	var vector Right;
	var vector Front;
};

/** 
 * Structure for collating info about sound groups
 */
struct native AudioGroupInfo 
{
	var const int NumResident;
	var const int SizeResident;
	var const int NumRealTime;
	var const int SizeRealTime;
};

/**
 * Structure containing configurable properties of a sound group.
 */
struct native SoundGroupProperties
{
	/** Volume multiplier. */
	var() float Volume;
	/** Pitch multiplier. */
	var() float Pitch;
	/** Voice center channel volume - Not a multiplier (no propagation)	*/
	var() float VoiceCenterChannelVolume;
	/** Radio volume multiplier - Not a multiplier (no propagation) */
	var() float VoiceRadioVolume;

	/** Sound mode voice - whether to apply audio effects */
	var() bool bApplyEffects;
	/** Whether to artificially prioritise the component to play */
	var() bool bAlwaysPlay;
	/** Whether or not this sound plays when the game is paused in the UI */
	var() bool bIsUISound;
	/** Whether or not this is music (propagates only if parent is TRUE) */
	var() bool bIsMusic;
	/** Whether or not this sound group is excluded from reverb EQ */
	var() bool bNoReverb;
	/** Whether or not to bleed stereo effects to the rear speakers */
	var() bool bBleedStereo;

	structdefaultproperties
	{
		Volume=1
		Pitch=1
		VoiceCenterChannelVolume=0
		VoiceRadioVolume=0
		bApplyEffects=FALSE
		bAlwaysPlay=FALSE
		bIsUISound=FALSE
		bIsMusic=FALSE
		bNoReverb=FALSE
		bBleedStereo=FALSE
	}

	structcpptext
	{
		/** Interpolate the data in sound groups */
		void Interpolate( FLOAT InterpValue, FSoundGroupProperties& Start, FSoundGroupProperties& End );
	}
};

/**
 * Structure containing information about a sound group.
 */
struct native SoundGroup
{
	/** Configurable properties like volume and priority. */
	var() SoundGroupProperties	Properties;
	/** Name of this sound group. */
	var() name					GroupName;
	/** Array of names of child sound groups. Empty for leaf groups. */
	var() array<name>			ChildGroupNames;
};

/** The maximum number of concurrent audible sounds */
var		config const	int							MaxChannels;
/** The amount of memory to reserve for always resident sounds */
var		config const	int							CommonAudioPoolSize;
/** Use effects such as reverb, low pass filter and EQ */
var		config const	bool						UseEffectsProcessing;

/** Pointer to permanent memory allocation stack. */
var		native const	pointer						CommonAudioPool;
/** Available size in permanent memory stack */
var		native const	int							CommonAudioPoolFreeBytes;

var		transient const	array<AudioComponent>		AudioComponents;
var		native const	array<pointer>				Sources{FSoundSource};
var		native const	array<pointer>				FreeSources{FSoundSource};
var		native const	Map_Mirror					WaveInstanceSourceMap{TMap<FWaveInstance*, FSoundSource*>};

var		native const	bool						bGameWasTicking;

var		native const	array<Listener>				Listeners;
var		native const	QWORD						CurrentTick;

/** Map from name to the sound group index - used to index the following 4 arrays */
var		native const	Map_Mirror					NameToSoundGroupIndexMap{TMap<FName, INT>};

/** The sound group constants that we are interpolating from */
var		native const	array<SoundGroup>			SourceSoundGroups;
/** The current state of sound group constants */
var		native const	array<SoundGroup>			CurrentSoundGroups;
/** The sound group constants that we are interpolating to */
var		native const	array<SoundGroup>			DestinationSoundGroups;

/** Array of sound groups read from ini file */
var()	config			array<SoundGroup>			SoundGroups;

/** Array of used sound modes */
var		native const	Map_Mirror					SoundModes{TMap<FName, class USoundMode*>};

/** Interface to audio effects processing */
var		native const	pointer						Effects{class FAudioEffectsManager};

var		native const	SoundMode					CurrentMode;
var		native const	double						SoundModeStartTime;
var		native const	double						SoundModeFadeInStartTime;
var		native const	double						SoundModeFadeInEndTime;
var		native const	double						SoundModeEndTime;

/** An AudioComponent to play test sounds on */
var			   const	AudioComponent				TestAudioComponent;

/** Interface to text to speech processor */
var		native const	pointer						TextToSpeech{class FTextToSpeech};

/** Apply a low pass filter to all sound sources for testing */
var		native const	bool						bTestLowPassFilter;
/** Disable any active low pass filter effects */
var		native const	bool						bDisableLowPassFilter;
/** Enable EQ effects for testing */
var		native const	bool						bTestEQFilter;
/** Disable any active EQ effects */
var		native const	bool						bDisableEQFilter;
/** Enable radio filter for testing */
var		native const	bool						bTestRadioFilter;
/** Disable any active radio filters */
var		native const	bool						bDisableRadioFilter;

/** transient master volume multiplier that can be modified at runtime without affecting user settings
 * automatically reset to 1.0 on level change
 */
var transient float TransientMasterVolume;

cpptext
{
	friend class FSoundSource;

	/** Constructor */
	UAudioDevice( void ) 
	{
	}

	/**
	 * Basic initialisation of the platform agnostic layer of the audio system
	 */
	virtual UBOOL Init( void );

	/**
	 * Stop all the audio components and sources attached to the scene. NULL scene means all components.
	 */
	virtual void Flush( FSceneInterface* Scene );

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
	 * Iterate over the active AudioComponents for wave instances that could be playing
	 */
	INT GetSortedActiveWaveInstances( TArray<FWaveInstance*>& WaveInstances, UBOOL bGameTicking );

	/**
	 * Stop sources that are no longer audible
	 */
	void StopSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex );

	/**
	 * Start and/or update any sources that have a high enough priority to play
	 */
	void StartSources( TArray<FWaveInstance*>& WaveInstances, INT FirstActiveIndex, UBOOL bGameTicking );

	/**
	 * The audio system's main "Tick" function
	 */
	virtual void Update( UBOOL bGameTicking );

	/**
	 * Sets the listener's location and orientation for the viewport
	 */
	void SetListener( INT ViewportIndex, INT MaxViewportIndex, const FVector& Location, const FVector& Up, const FVector& Right, const FVector& Front );

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param bShouldStopUISounds If TRUE, this function will stop UI sounds as well
	 */
	virtual void StopAllSounds( UBOOL bShouldStopUISounds = FALSE );

	/**
	 * Pushes the specified reverb settings onto the reverb settings stack.
	 *
	 * @param	ReverbSettings		The reverb settings to use.
	 */
	void SetReverbSettings( const FReverbSettings& ReverbSettings );

	/**
	 * Frees the bulk resource data assocated with this SoundNodeWave.
	 *
	 * @param	SoundNodeWave	wave object to free associated bulk data
	 */
	virtual void FreeResource( USoundNodeWave* SoundNodeWave )
	{
	}

	/**
	 * Precaches the passed in sound node wave object.
	 *
	 * @param	SoundNodeWave	Resource to be precached.
	 */
	virtual void Precache( USoundNodeWave* SoundNodeWave )
	{
	}

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
	 * Gets a summary of loaded sound collated by group
	 */
	void GetSoundGroupInfo( TMap<FName, FAudioGroupInfo>& AudioGroupInfos );
	
	/**
	 * Lists all the playing waveinstances and their associated source
	 */
	void ListWaves( FOutputDevice& Ar );
	
	/**
	 * Lists a summary of loaded sound collated by group
	 */
	void ListSoundGroups( FOutputDevice& Ar );

	/**
	 * Set up the sound group hierarchy
	 */
	void InitSoundClasses( void );

	/**
	 * Add a newly created sound mode to the base set
	 */
	void AddMode( USoundMode* Mode );

	/**
	 * Load up all requested sound modes
	 */
	void InitSoundModes( void );

	/**
	 * Returns the sound group properties associates with a sound group taking into account
	 * the group tree.
	 *
	 * @param	SoundGroupName	name of sound group to retrieve properties from
	 * @return	sound group properties if a sound group with name SoundGroupName exists, NULL otherwise
	 */
	FSoundGroupProperties* GetSoundGroupProperties( FName SoundGroupName );

	/**
	 * Parses the sound groups and propagates multiplicative properties down the tree.
	 */
	void ParseSoundClasses( void );

	/**
	 * Construct the CurrentSoundGroupProperties map
	 *
	 * This contains the original sound group properties propagated properly, and all adjustments due to the sound mode
	 */
	void GetCurrentSoundGroupState( void );

	/**
	 * Gets the parameters for EQ based on settings and time
	 */
	void Interpolate( FLOAT InterpValue, FSoundGroupProperties& Current, FSoundGroupProperties& Start, FSoundGroupProperties& End );

	/**
	 * Set the mode for altering sound group properties
	 */
	void ApplySoundMode( USoundMode* NewMode );

	/**
	 * Adds a component to the audio device
	 */
	void AddComponent( UAudioComponent* AudioComponent );

	/**
	 * Removes an attached audio component
	 */
	void RemoveComponent( UAudioComponent* AudioComponent );

	/**
	 * Creates an audio component to handle playing a sound cue
	 */
	static UAudioComponent* CreateComponent( USoundCue* SoundCue, FSceneInterface* Scene, AActor* Actor = NULL, UBOOL Play = TRUE, UBOOL bStopWhenOwnerDestroyed = FALSE, FVector* Location = NULL );

	/**
	 * Exec
	 */
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar = *GLog );

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
	 * Updates sound group volumes
	 */
	void SetGroupVolume( FName Group, FLOAT Volume );

	/**
	 * Return the pointer to the sound effects handler
	 */
	class FAudioEffectsManager* GetEffects( void )
	{
		return( Effects );
	}

	/**
	 * Checks to see if a coordinate is within a distance of any listener
	 */
	UBOOL LocationIsAudible( FVector Location, FLOAT MaxDistance );

	/**
	 * Creates a soundcue and PCM data for a the text in SpokenText
	 */ 
	USoundCue* CreateTTSSoundCue( const FString& SpokenText, ETTSSpeaker Speaker );

	/**
	 * Checks to see if the low pass filter is being tested
	 */
	UBOOL IsTestingLowPassFilter( void ) const
	{
		return( bTestLowPassFilter );
	}

	/**
	 * Checks to see if the low pass filter has been disabled
	 */
	UBOOL IsDisabledLowPassFilter( void ) const
	{
		return( bDisableLowPassFilter );
	}

	/**
	 * Checks to see if the EQ filter is being tested
	 */
	UBOOL IsTestingEQFilter( void ) const
	{
		return( bTestEQFilter );
	}

	/**
	 * Checks to see if the EQ filter has been disabled
	 */
	UBOOL IsDisabledEQFilter( void ) const
	{
		return( bDisableEQFilter );
	}

	/**
	 * Checks to see if the EQ filter is being tested
	 */
	UBOOL IsTestingRadioFilter( void ) const
	{
		return( bTestRadioFilter );
	}

	/**
	 * Checks to see if the EQ filter has been disabled
	 */
	UBOOL IsDisabledRadioFilter( void ) const
	{
		return( bDisableRadioFilter );
	}

protected:
	/** Internal */
	void SortWaveInstances( INT MaxChannels );

	/**
	 * Internal helper function used by ParseSoundGroups to traverse the tree.
	 *
	 * @param CurrentGroup			Subtree to deal with
	 * @param ParentProperties		Propagated properties of parent node
	 */
	void RecurseIntoSoundGroups( FSoundGroup* CurrentGroup, FSoundGroupProperties* ParentProperties );
}

/**
 * Sets a new sound mode and applies it to all appropriate sound groups
 */
native final function SetSoundMode( name NewMode );

defaultproperties
{
	TransientMasterVolume=1.0
}
