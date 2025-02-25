/*===========================================================================
    C++ class definitions exported from UnrealScript.
    This is automatically generated by the tools.
    DO NOT modify this manually! Edit the corresponding .uc files instead!
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
===========================================================================*/
#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,4)
#endif


// Split enums from the rest of the header so they can be included earlier
// than the rest of the header file by including this file twice with different
// #define wrappers. See Engine.h and look at EngineClasses.h for an example.
#if !NO_ENUMS && !defined(NAMES_ONLY)

#ifndef INCLUDED_ENGINE_SOUND_ENUMS
#define INCLUDED_ENGINE_SOUND_ENUMS 1

enum ESoundDistanceCalc
{
    SOUNDDISTANCE_Normal    =0,
    SOUNDDISTANCE_InfiniteXYPlane=1,
    SOUNDDISTANCE_InfiniteXZPlane=2,
    SOUNDDISTANCE_InfiniteYZPlane=3,
    SOUNDDISTANCE_MAX       =4,
};
enum SoundDistanceModel
{
    ATTENUATION_Linear      =0,
    ATTENUATION_Logarithmic =1,
    ATTENUATION_Inverse     =2,
    ATTENUATION_LogReverse  =3,
    ATTENUATION_NaturalSound=4,
    ATTENUATION_MAX         =5,
};
enum EDecompressionType
{
    DTYPE_Setup             =0,
    DTYPE_Invalid           =1,
    DTYPE_Preview           =2,
    DTYPE_Native            =3,
    DTYPE_RealTime          =4,
    DTYPE_MAX               =5,
};

#endif // !INCLUDED_ENGINE_SOUND_ENUMS
#endif // !NO_ENUMS

#if !ENUMS_ONLY

#ifndef NAMES_ONLY
#define AUTOGENERATE_NAME(name) extern FName ENGINE_##name;
#define AUTOGENERATE_FUNCTION(cls,idx,name)
#endif


#ifndef NAMES_ONLY

#ifndef INCLUDED_ENGINE_SOUND_CLASSES
#define INCLUDED_ENGINE_SOUND_CLASSES 1

class AAmbientSound : public AKeypoint
{
public:
    //## BEGIN PROPS AmbientSound
    BITFIELD bAutoPlay:1;
    BITFIELD bIsPlaying:1;
    class UAudioComponent* AudioComponent;
    //## END PROPS AmbientSound

    DECLARE_CLASS(AAmbientSound,AKeypoint,0,Engine)
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
};

class AAmbientSoundMovable : public AAmbientSound
{
public:
    //## BEGIN PROPS AmbientSoundMovable
    //## END PROPS AmbientSoundMovable

    DECLARE_CLASS(AAmbientSoundMovable,AAmbientSound,0,Engine)
    NO_DEFAULT_CONSTRUCTOR(AAmbientSoundMovable)
};

class AAmbientSoundSimple : public AAmbientSound
{
public:
    //## BEGIN PROPS AmbientSoundSimple
    class USoundNodeAmbient* AmbientProperties;
    class USoundCue* SoundCueInstance;
    class USoundNodeAmbient* SoundNodeInstance;
    //## END PROPS AmbientSoundSimple

    DECLARE_CLASS(AAmbientSoundSimple,AAmbientSound,0,Engine)
	/**
	 * Helper function used to sync up instantiated objects.
	 */
	void SyncUpInstantiatedObjects();

	/**
	 * Called from within SpawnActor, calling SyncUpInstantiatedObjects.
	 */
	virtual void Spawned();

	/**
	 * Called when after .t3d import of this actor (paste, duplicate or .t3d import),
	 * calling SyncUpInstantiatedObjects.
	 */
	virtual void PostEditImport();

	/**
	 * Used to temporarily clear references for duplication.
	 *
	 * @param PropertyThatWillChange	property that will change
	 */
	virtual void PreEditChange(UProperty* PropertyThatWillChange);

	/**
	 * Used to reset audio component when AmbientProperties change
	 *
	 * @param PropertyThatChanged	property that changed
	 */
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	virtual void EditorApplyScale(const FVector& DeltaScale, const FMatrix& ScaleMatrix, const FVector* PivotLocation, UBOOL bAltDown, UBOOL bShiftDown, UBOOL bCtrlDown);
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
};

class AAmbientSoundNonLoop : public AAmbientSoundSimple
{
public:
    //## BEGIN PROPS AmbientSoundNonLoop
    //## END PROPS AmbientSoundNonLoop

    DECLARE_CLASS(AAmbientSoundNonLoop,AAmbientSoundSimple,0,Engine)
public:
	// AActor interface.
	/**
	 * Function that gets called from within Map_Check to allow this actor to check itself
	 * for any potential errors and register them with map check dialog.
	 */
	virtual void CheckForErrors();
};

class UDistributionFloatSoundParameter : public UDistributionFloatParameterBase
{
public:
    //## BEGIN PROPS DistributionFloatSoundParameter
    //## END PROPS DistributionFloatSoundParameter

    DECLARE_CLASS(UDistributionFloatSoundParameter,UDistributionFloatParameterBase,0,Engine)
	virtual UBOOL GetParamValue(UObject* Data, FName ParamName, FLOAT& OutFloat);
};

struct FAudioEQEffect
{
    DOUBLE RootTime;
    FLOAT HFFrequency;
    FLOAT HFGain;
    FLOAT MFCutoffFrequency;
    FLOAT MFBandwidthFrequency;
    FLOAT MFGain;
    FLOAT LFFrequency;
    FLOAT LFGain;

		// Cannot use strcutdefaultproperties here as this class is a member of a native class
		FAudioEQEffect( void ) :
			RootTime( 0.0 ),
			HFFrequency( 12000.0f ),
			HFGain( 1.0f ),
			MFCutoffFrequency( 8000.0f ),
			MFBandwidthFrequency( 1000.0f ),
			MFGain( 1.0f ),
			LFFrequency( 2000.0f ),
			LFGain( 1.0f )
		{
		}

		/** Interpolates between Start and Ed reverb effect settings */
		void Interpolate( FLOAT InterpValue, FAudioEQEffect& Start, FAudioEQEffect& End );
	
};

struct FSoundClassAdjuster
{
    BYTE SoundClassName;
    FName SoundClass;
    FLOAT VolumeAdjuster;
    FLOAT PitchAdjuster;

    /** Constructors */
    FSoundClassAdjuster() {}
    FSoundClassAdjuster(EEventParm)
    {
        appMemzero(this, sizeof(FSoundClassAdjuster));
    }
};

class USoundMode : public UObject
{
public:
    //## BEGIN PROPS SoundMode
    BITFIELD bApplyEQ:1;
    struct FAudioEQEffect EQSettings;
    TArrayNoInit<struct FSoundClassAdjuster> SoundClassEffects;
    FLOAT InitialDelay;
    FLOAT Duration;
    FLOAT FadeInTime;
    FLOAT FadeOutTime;
    //## END PROPS SoundMode

    DECLARE_CLASS(USoundMode,UObject,0,Engine)
	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/**
	 * Returns a description of this object that can be used as extra information in list views.
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/** 
	 * Populate the enum using the serialised fname
	 */
	void Fixup( void );

	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );
};

class USoundNode : public UObject
{
public:
    //## BEGIN PROPS SoundNode
    INT NodeUpdateHint;
    TArrayNoInit<class USoundNode*> ChildNodes;
    //## END PROPS SoundNode

    DECLARE_ABSTRACT_CLASS(USoundNode,UObject,0,Engine)
	// USoundNode interface.
	
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance ) {}

	/**
	 * Returns whether the node is finished after having been notified of buffer
	 * being finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	TRUE if finished, FALSE otherwise.
	 */
	virtual UBOOL IsFinished( class UAudioComponent* /*Unused*/) { return TRUE; }

	/** 
	 * Returns the maximum distance this sound can be heard from.
	 */
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return CurrentMaxDistance; }
	
	/** 
	 * Returns the maximum duration this sound node will play for. 
	 */
	virtual FLOAT GetDuration();

	/** 
	 * Returns whether the sound is looping indefinitely or not.
	 */
	virtual UBOOL IsLoopingIndefinitely( void ) { return( FALSE ); }

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void GetAllNodes( TArray<USoundNode*>& SoundNodes ); // Like above but returns ALL (not just active) nodes.

	virtual INT GetMaxChildNodes() { return 1; }

	// Tool drawing
	virtual void DrawSoundNode(FCanvas* Canvas, const struct FSoundNodeEditorData& EdData, UBOOL bSelected);
	virtual FIntPoint GetConnectionLocation(FCanvas* Canvas, INT ConnType, INT ConnIndex, const struct FSoundNodeEditorData& EdData);

	/**
	 * Helper function to reset bFinished on wave instances this node has been notified of being finished.
	 *
	 * @param	AudioComponent	Audio component this node is used in.
	 */
	void ResetWaveInstances( UAudioComponent* AudioComponent );

	// Editor interface.

	/** Get the name of the class used to help out when handling events in UnrealEd.
	 * @return	String name of the helper class.
	 */
	virtual const FString	GetEdHelperClassName() const
	{
		return FString( TEXT("UnrealEd.SoundNodeHelper") );
	}

	/**
	 * Called by the Sound Cue Editor for nodes which allow children.  The default behaviour is to
	 * attach a single connector. Dervied classes can override to eg add multiple connectors.
	 */
	virtual void CreateStartingConnectors();
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );
	
	/**
	 * Called when a property value from a member struct or array has been changed in the editor.
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );	
};

class USoundNodeAttenuation : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeAttenuation
    BITFIELD bAttenuate:1;
    BITFIELD bSpatialize:1;
    BITFIELD bAttenuateWithLowPassFilter:1;
    BYTE DistanceModel GCC_BITFIELD_MAGIC;
    BYTE DistanceType;
    struct FRawDistributionFloat MinRadius;
    struct FRawDistributionFloat MaxRadius;
    FLOAT dBAttenuationAtMax;
    struct FRawDistributionFloat LPFMinRadius;
    struct FRawDistributionFloat LPFMaxRadius;
    //## END PROPS SoundNodeAttenuation

    DECLARE_CLASS(USoundNodeAttenuation,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return ::Max<FLOAT>(CurrentMaxDistance,MaxRadius.GetValue(0.9f)); }
};

class USoundNodeAmbient : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeAmbient
    BYTE DistanceModel;
    struct FRawDistributionFloat MinRadius;
    struct FRawDistributionFloat MaxRadius;
    struct FRawDistributionFloat LPFMinRadius;
    struct FRawDistributionFloat LPFMaxRadius;
    BITFIELD bSpatialize:1;
    BITFIELD bAttenuate:1;
    BITFIELD bAttenuateWithLowPassFilter:1;
    class USoundNodeWave* Wave;
    struct FRawDistributionFloat VolumeModulation;
    struct FRawDistributionFloat PitchModulation;
    //## END PROPS SoundNodeAmbient

    DECLARE_CLASS(USoundNodeAmbient,USoundNode,0,Engine)
	// USoundNode interface.
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void GetAllNodes( TArray<USoundNode*>& SoundNodes ); // Like above but returns ALL (not just active) nodes.
	virtual FLOAT MaxAudibleDistance(FLOAT CurrentMaxDistance) { return ::Max<FLOAT>(CurrentMaxDistance,MaxRadius.GetValue(0.9f)); }
	virtual INT GetMaxChildNodes() { return 0; }
		
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/**
	 * We're looping indefinitely so we're never finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	FALSE
	 */
	virtual UBOOL IsFinished( class UAudioComponent* /*Unused*/ ) 
	{ 
		return( FALSE ); 
	}

	/** 
	 * Gets the time in seconds of ambient sound - in this case INDEFINITELY_LOOPING_DURATION
	 */	
	virtual FLOAT GetDuration( void )
	{
		return( INDEFINITELY_LOOPING_DURATION );
	}
};

struct FAmbientSoundSlot
{
    class USoundNodeWave* Wave;
    FLOAT PitchScale;
    FLOAT VolumeScale;
    FLOAT Weight;

    /** Constructors */
    FAmbientSoundSlot() {}
    FAmbientSoundSlot(EEventParm)
    {
        appMemzero(this, sizeof(FAmbientSoundSlot));
    }
};

class USoundNodeAmbientNonLoop : public USoundNodeAmbient
{
public:
    //## BEGIN PROPS SoundNodeAmbientNonLoop
    struct FRawDistributionFloat DelayTime;
    TArrayNoInit<struct FAmbientSoundSlot> SoundSlots;
    //## END PROPS SoundNodeAmbientNonLoop

    DECLARE_CLASS(USoundNodeAmbientNonLoop,USoundNodeAmbient,0,Engine)
	// USoundNode interface.
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void GetAllNodes( TArray<USoundNode*>& SoundNodes ); // Like above but returns ALL (not just active) nodes.

	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/** 
	 * Pick which slot to play next. 
	 */
	INT PickNextSlot( void );

	/** 
	 * Gets the time in seconds of ambient sound - in this case INDEFINITELY_LOOPING_DURATION
	 */	
	virtual FLOAT GetDuration( void )
	{
		return( INDEFINITELY_LOOPING_DURATION );
	}
};

class USoundNodeConcatenator : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeConcatenator
    TArrayNoInit<FLOAT> InputVolume;
    //## END PROPS SoundNodeConcatenator

    DECLARE_CLASS(USoundNodeConcatenator,USoundNode,0,Engine)
	// USoundNode interface.
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes() { return -1; }	

	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/**
	 * Returns whether the node is finished after having been notified of buffer
	 * being finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	TRUE if finished, FALSE otherwise.
	 */
	UBOOL IsFinished( class UAudioComponent* AudioComponent );

	/**
	 * Returns the maximum duration this sound node will play for. 
	 * 
	 * @return maximum duration this sound will play for
	 */
	virtual FLOAT GetDuration();

	/**
	 * Concatenators have two connectors by default.
	 */
	virtual void CreateStartingConnectors();

	/**
	 * Overloaded to add an entry to InputVolume.
	 */
	virtual void InsertChildNode( INT Index );

	/**
	 * Overloaded to remove an entry from InputVolume.
	 */
	virtual void RemoveChildNode( INT Index );
};

class USoundNodeDelay : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeDelay
    struct FRawDistributionFloat DelayDuration;
    //## END PROPS SoundNodeDelay

    DECLARE_CLASS(USoundNodeDelay,USoundNode,0,Engine)
	// USoundNode interface.
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	/**
	 * Returns the maximum duration this sound node will play for. 
	 * 
	 * @return maximum duration this sound will play for
	 */
	virtual FLOAT GetDuration();
};

struct FDistanceDatum
{
    struct FRawDistributionFloat FadeInDistance;
    struct FRawDistributionFloat FadeOutDistance;
    FLOAT Volume;

    /** Constructors */
    FDistanceDatum() {}
    FDistanceDatum(EEventParm)
    {
        appMemzero(this, sizeof(FDistanceDatum));
    }
};

class USoundNodeDistanceCrossFade : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeDistanceCrossFade
    TArrayNoInit<struct FDistanceDatum> CrossFadeInput;
    //## END PROPS SoundNodeDistanceCrossFade

    DECLARE_CLASS(USoundNodeDistanceCrossFade,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes() { return -1; }

	/**
	 * DistanceCrossFades have two connectors by default.
	 */
	virtual void CreateStartingConnectors();
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );

	virtual FLOAT MaxAudibleDistance( FLOAT CurrentMaxDistance );
};

class USoundNodeLooping : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeLooping
    BITFIELD bLoopIndefinitely:1;
    struct FRawDistributionFloat LoopCount;
    //## END PROPS SoundNodeLooping

    DECLARE_CLASS(USoundNodeLooping,USoundNode,0,Engine)
	/**
	 * Notifies the sound node that a wave instance in its subtree has finished.
	 *
	 * @param WaveInstance	WaveInstance that was finished 
	 */
	virtual void NotifyWaveInstanceFinished( struct FWaveInstance* WaveInstance );

	/**
	 * Returns whether the node is finished after having been notified of buffer
	 * being finished.
	 *
	 * @param	AudioComponent	Audio component containing payload data
	 * @return	TRUE if finished, FALSE otherwise.
	 */
	virtual UBOOL IsFinished( class UAudioComponent* AudioComponent );

	/** 
	 * Returns the maximum distance this sound can be heard from. Very large for looping sounds as the
	 * player can move into the hearable range during a loop.
	 */
	virtual FLOAT MaxAudibleDistance( FLOAT CurrentMaxDistance ) { return WORLD_MAX; }
	
	/** 
	 * Returns the duration of the sound accounting for the number of loops.
	 */
	virtual FLOAT GetDuration();

	/** 
	 * Returns whether the sound is looping indefinitely or not.
	 */
	virtual UBOOL IsLoopingIndefinitely( void );

	/** 
	 * Process this node in the sound cue
	 */
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
};

class USoundNodeMature : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeMature
    //## END PROPS SoundNodeMature

    DECLARE_CLASS(USoundNodeMature,USoundNode,0,Engine)
	// USoundNode interface.
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes();
};

class USoundNodeMixer : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeMixer
    TArrayNoInit<FLOAT> InputVolume;
    //## END PROPS SoundNodeMixer

    DECLARE_CLASS(USoundNodeMixer,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	virtual INT GetMaxChildNodes() { return -1; }

	/**
	 * Mixers have two connectors by default.
	 */
	virtual void CreateStartingConnectors();

	/**
	 * Overloaded to add an entry to InputVolume.
	 */
	virtual void InsertChildNode( INT Index );

	/**
	 * Overloaded to remove an entry from InputVolume.
	 */
	virtual void RemoveChildNode( INT Index );
};

class USoundNodeModulator : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeModulator
    struct FRawDistributionFloat VolumeModulation;
    struct FRawDistributionFloat PitchModulation;
    //## END PROPS SoundNodeModulator

    DECLARE_CLASS(USoundNodeModulator,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
};

class USoundNodeModulatorContinuous : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeModulatorContinuous
    struct FRawDistributionFloat VolumeModulation;
    struct FRawDistributionFloat PitchModulation;
    //## END PROPS SoundNodeModulatorContinuous

    DECLARE_CLASS(USoundNodeModulatorContinuous,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
};

class USoundNodeOscillator : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeOscillator
    struct FRawDistributionFloat Amplitude;
    struct FRawDistributionFloat Frequency;
    struct FRawDistributionFloat Offset;
    struct FRawDistributionFloat Center;
    BITFIELD bModulatePitch:1;
    BITFIELD bModulateVolume:1;
    //## END PROPS SoundNodeOscillator

    DECLARE_CLASS(USoundNodeOscillator,USoundNode,0,Engine)
	// USoundNode interface.

	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
};

class USoundNodeRandom : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeRandom
    TArrayNoInit<FLOAT> Weights;
    BITFIELD bRandomizeWithoutReplacement:1;
    TArrayNoInit<UBOOL> HasBeenUsed;
    INT NumRandomUsed;
    //## END PROPS SoundNodeRandom

    DECLARE_CLASS(USoundNodeRandom,USoundNode,0,Engine)
	// USoundNode interface.
	
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );

	virtual INT GetMaxChildNodes() { return -1; }
	
	// Editor interface.
	
	virtual void InsertChildNode( INT Index );
	virtual void RemoveChildNode( INT Index );
	
	// USoundNodeRandom interface
	void FixWeightsArray();
	void FixHasBeenUsedArray();


	/**
	 * Called by the Sound Cue Editor for nodes which allow children.  The default behaviour is to
	 * attach a single connector. Dervied classes can override to e.g. add multiple connectors.
	 */
	virtual void CreateStartingConnectors();
};

struct FSubtitleCue
{
    FStringNoInit Text;
    FLOAT Time;
    FStringNoInit TaggedText;

    /** Constructors */
    FSubtitleCue() {}
    FSubtitleCue(EEventParm)
    {
        appMemzero(this, sizeof(FSubtitleCue));
    }
};

struct FLocalizedSubtitle
{
    TArrayNoInit<struct FSubtitleCue> Subtitles;
    BITFIELD bMature:1;
    BITFIELD bManualWordWrap:1;

    /** Constructors */
    FLocalizedSubtitle() {}
    FLocalizedSubtitle(EEventParm)
    {
        appMemzero(this, sizeof(FLocalizedSubtitle));
    }
};

class USoundNodeWave : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeWave
    INT CompressionQuality;
    BITFIELD bForceRealtimeDecompression:1;
    BITFIELD bDynamicResource:1;
    BITFIELD bOneTimeUse:1;
    BITFIELD bUseTTS:1;
    BITFIELD bLinearRollOff:1;
    BITFIELD bMature:1;
    BITFIELD bManualWordWrap:1;
    BITFIELD bDataIsStreamed:1;
    BYTE TTSSpeaker GCC_BITFIELD_MAGIC;
    BYTE DecompressionType;
    FStringNoInit SpokenText;
    FLOAT VolumeDB;
    FLOAT MinRange;
    FLOAT MaxRange;
    INT DialogPriority;
    INT DialogMode;
    FLOAT Volume;
    FLOAT Pitch;
    FLOAT Duration;
    INT NumChannels;
    INT SampleRate;
    INT SampleDataSize;
    TArrayNoInit<INT> ChannelOffsets;
    TArrayNoInit<INT> ChannelSizes;
    FByteBulkData RawData;
    SWORD* RawPCMData;
    FAsyncVorbisDecompress* VorbisDecompressor;
    TArrayNoInit<BYTE> PCMData;
    FByteBulkData CompressedPCData;
    FByteBulkData CompressedXbox360Data;
    FByteBulkData CompressedPS3Data;
    INT ResourceID;
    INT ResourceSize;
    void* ResourceData;
    TArrayNoInit<struct FSubtitleCue> Subtitles;
    FStringNoInit Comment;
    FStringNoInit SourceFilePath;
    FStringNoInit SourceFileTimestamp;
    INT FMODResourceSize;
    FStringNoInit Effect;
    TArrayNoInit<struct FLocalizedSubtitle> LocalizedSubtitles;
    FName StreamFilenameURL;
    FStringNoInit FaceFXGroupName;
    FStringNoInit FaceFXAnimName;
    //## END PROPS SoundNodeWave

    DECLARE_CLASS(USoundNodeWave,USoundNode,0,Engine)
	/** UObject interface. */
	virtual void Serialize( FArchive& Ar );

	/** 
	 * Frees up all the resources allocated in this class
	 */
	void FreeResources( void );

	/**
	 * Returns whether the resource is ready to have finish destroy routed.
	 *
	 * @return	TRUE if ready for deletion, FALSE otherwise.
	 */
	UBOOL IsReadyForFinishDestroy();

	/**
	 * Frees the sound resource data.
	 */
	virtual void FinishDestroy( void );

	/**
	 * Outside the Editor, uploads resource to audio device and performs general PostLoad work.
	 *
	 * This function is being called after all objects referenced by this object have been serialized.
	 */
	virtual void PostLoad( void );

	/** 
	 * Invalidate compressed data
	 */
	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/** 
	 * Copy the compressed audio data from the bulk data
	 */
	void InitAudioResource( FByteBulkData& CompressedData );

	/** 
	 * Remove the compressed audio data associated with the passed in wave
	 */
	void RemoveAudioResource( void );

	/** 
	 * USoundNode interface.
	 */
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	
	/** 
	 * 
	 */
	virtual INT GetMaxChildNodes( void ) 
	{ 
		return( 0 );  
	}
	
	/** 
	 * Gets the time in seconds of the associated sound data
	 */	
	virtual FLOAT GetDuration( void );
	
	/** 
	 * Prints the subtitle associated with the SoundNodeWave to the console
	 */
	void LogSubtitle( FOutputDevice& Ar );

	/** 
	 * Get the name of the class used to help out when handling events in UnrealEd.
	 * @return	String name of the helper class.
	 */
	virtual const FString GetEdHelperClassName( void ) const
	{
		return FString( TEXT( "UnrealEd.SoundNodeWaveHelper" ) );
	}

	/**
	 * Returns whether this wave file is a localized resource.
	 *
	 * @return TRUE if it is a localized resource, FALSE otherwise.
	 */
	virtual UBOOL IsLocalizedResource( void );

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize( void );

	/**
	 *	@param		Platform		EPlatformType indicating the platform of interest...
	 *
	 *	@return		Sum of the size of waves referenced by this cue for the given platform.
	 */
	virtual INT GetResourceSize( UE3::EPlatformType Platform );

	/** 
	 * Returns the name of the exporter factory used to export this object
	 * Used when multiple factories have the same extension
	 */
	virtual FName GetExporterName( void );

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc( void );

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );

	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData( UE3::EPlatformType TargetPlatform );
	
	/** 
	 * Makes sure ogg vorbis data is available for this sound node by converting on demand
	 */
	UBOOL ValidateData( void );	
};

class USoundNodeWaveParam : public USoundNode
{
public:
    //## BEGIN PROPS SoundNodeWaveParam
    FName WaveParameterName;
    //## END PROPS SoundNodeWaveParam

    DECLARE_CLASS(USoundNodeWaveParam,USoundNode,0,Engine)
	/** 
	 * Return the maximum number of child nodes; normally 0 to 2
	 */
	virtual INT GetMaxChildNodes()
	{
		return( 1 );
	}
	
	/** 
	 * Gets the time in seconds of the associated sound data
	 */	
	virtual FLOAT GetDuration( void );
	
	/** 
	 * USoundNode interface.
	 */
	virtual void ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, class UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances );
	
	/** 
	 *
	 */
	virtual void GetNodes( class UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes );
};

#endif // !INCLUDED_ENGINE_SOUND_CLASSES
#endif // !NAMES_ONLY


#ifndef NAMES_ONLY
#undef AUTOGENERATE_NAME
#undef AUTOGENERATE_FUNCTION
#endif

#ifdef STATIC_LINKING_MOJO
#ifndef ENGINE_SOUND_NATIVE_DEFS
#define ENGINE_SOUND_NATIVE_DEFS

DECLARE_NATIVE_TYPE(Engine,AAmbientSound);
DECLARE_NATIVE_TYPE(Engine,AAmbientSoundMovable);
DECLARE_NATIVE_TYPE(Engine,AAmbientSoundNonLoop);
DECLARE_NATIVE_TYPE(Engine,AAmbientSoundSimple);
DECLARE_NATIVE_TYPE(Engine,UDistributionFloatSoundParameter);
DECLARE_NATIVE_TYPE(Engine,USoundMode);
DECLARE_NATIVE_TYPE(Engine,USoundNode);
DECLARE_NATIVE_TYPE(Engine,USoundNodeAmbient);
DECLARE_NATIVE_TYPE(Engine,USoundNodeAmbientNonLoop);
DECLARE_NATIVE_TYPE(Engine,USoundNodeAttenuation);
DECLARE_NATIVE_TYPE(Engine,USoundNodeConcatenator);
DECLARE_NATIVE_TYPE(Engine,USoundNodeDelay);
DECLARE_NATIVE_TYPE(Engine,USoundNodeDistanceCrossFade);
DECLARE_NATIVE_TYPE(Engine,USoundNodeLooping);
DECLARE_NATIVE_TYPE(Engine,USoundNodeMature);
DECLARE_NATIVE_TYPE(Engine,USoundNodeMixer);
DECLARE_NATIVE_TYPE(Engine,USoundNodeModulator);
DECLARE_NATIVE_TYPE(Engine,USoundNodeModulatorContinuous);
DECLARE_NATIVE_TYPE(Engine,USoundNodeOscillator);
DECLARE_NATIVE_TYPE(Engine,USoundNodeRandom);
DECLARE_NATIVE_TYPE(Engine,USoundNodeWave);
DECLARE_NATIVE_TYPE(Engine,USoundNodeWaveParam);

#define AUTO_INITIALIZE_REGISTRANTS_ENGINE_SOUND \
	AAmbientSound::StaticClass(); \
	AAmbientSoundMovable::StaticClass(); \
	AAmbientSoundNonLoop::StaticClass(); \
	AAmbientSoundSimple::StaticClass(); \
	UDistributionFloatSoundParameter::StaticClass(); \
	UMixBin::StaticClass(); \
	URFMODSound::StaticClass(); \
	USoundMode::StaticClass(); \
	USoundNode::StaticClass(); \
	USoundNodeAmbient::StaticClass(); \
	USoundNodeAmbientNonLoop::StaticClass(); \
	USoundNodeAttenuation::StaticClass(); \
	USoundNodeConcatenator::StaticClass(); \
	USoundNodeDelay::StaticClass(); \
	USoundNodeDistanceCrossFade::StaticClass(); \
	USoundNodeLooping::StaticClass(); \
	USoundNodeMature::StaticClass(); \
	USoundNodeMixer::StaticClass(); \
	USoundNodeModulator::StaticClass(); \
	USoundNodeModulatorContinuous::StaticClass(); \
	USoundNodeOscillator::StaticClass(); \
	USoundNodeRandom::StaticClass(); \
	USoundNodeWave::StaticClass(); \
	USoundNodeWaveParam::StaticClass(); \

#endif // ENGINE_SOUND_NATIVE_DEFS

#ifdef NATIVES_ONLY
#endif // NATIVES_ONLY
#endif // STATIC_LINKING_MOJO

#ifdef VERIFY_CLASS_SIZES
VERIFY_CLASS_OFFSET_NODIE(A,AmbientSound,AudioComponent)
VERIFY_CLASS_SIZE_NODIE(AAmbientSound)
VERIFY_CLASS_SIZE_NODIE(AAmbientSoundMovable)
VERIFY_CLASS_SIZE_NODIE(AAmbientSoundNonLoop)
VERIFY_CLASS_OFFSET_NODIE(A,AmbientSoundSimple,AmbientProperties)
VERIFY_CLASS_OFFSET_NODIE(A,AmbientSoundSimple,SoundNodeInstance)
VERIFY_CLASS_SIZE_NODIE(AAmbientSoundSimple)
VERIFY_CLASS_SIZE_NODIE(UDistributionFloatSoundParameter)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,Version)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,MixBinName)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,FMODSound)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,CategoryCount)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,List)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,CueCount)
VERIFY_CLASS_OFFSET_NODIE(U,MixBin,CueList)
VERIFY_CLASS_SIZE_NODIE(UMixBin)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,SourceFilePath)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FileType)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,cues)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,cueNames)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FMODRawData)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,RawDataSize)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,eventProject)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,refCueList)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,importTime)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,ReadVersion)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,bankNames)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FMODXboxRawData)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FMODPS3RawData)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,CategoryCount)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,categoryList)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,Flags)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagMasterCategoryList)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagFullyLoaded)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagIsDirty)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagRegistered)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagCanStream)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagPS3RSX)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,flagMemoryStream)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,NumberOfPCStreams)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,NumberOfX360Streams)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,NumberOfPS3Streams)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FSBBankData)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FSBBankDataSize)
VERIFY_CLASS_OFFSET_NODIE(U,RFMODSound,FSBBankSound)
VERIFY_CLASS_SIZE_NODIE(URFMODSound)
VERIFY_CLASS_OFFSET_NODIE(U,SoundMode,EQSettings)
VERIFY_CLASS_OFFSET_NODIE(U,SoundMode,FadeOutTime)
VERIFY_CLASS_SIZE_NODIE(USoundMode)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNode,NodeUpdateHint)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNode,ChildNodes)
VERIFY_CLASS_SIZE_NODIE(USoundNode)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAmbient,DistanceModel)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAmbient,PitchModulation)
VERIFY_CLASS_SIZE_NODIE(USoundNodeAmbient)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAmbientNonLoop,DelayTime)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAmbientNonLoop,SoundSlots)
VERIFY_CLASS_SIZE_NODIE(USoundNodeAmbientNonLoop)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAttenuation,DistanceModel)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeAttenuation,LPFMaxRadius)
VERIFY_CLASS_SIZE_NODIE(USoundNodeAttenuation)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeConcatenator,InputVolume)
VERIFY_CLASS_SIZE_NODIE(USoundNodeConcatenator)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeDelay,DelayDuration)
VERIFY_CLASS_SIZE_NODIE(USoundNodeDelay)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeDistanceCrossFade,CrossFadeInput)
VERIFY_CLASS_SIZE_NODIE(USoundNodeDistanceCrossFade)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeLooping,LoopCount)
VERIFY_CLASS_SIZE_NODIE(USoundNodeLooping)
VERIFY_CLASS_SIZE_NODIE(USoundNodeMature)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeMixer,InputVolume)
VERIFY_CLASS_SIZE_NODIE(USoundNodeMixer)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeModulator,VolumeModulation)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeModulator,PitchModulation)
VERIFY_CLASS_SIZE_NODIE(USoundNodeModulator)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeModulatorContinuous,VolumeModulation)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeModulatorContinuous,PitchModulation)
VERIFY_CLASS_SIZE_NODIE(USoundNodeModulatorContinuous)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeOscillator,Amplitude)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeOscillator,Center)
VERIFY_CLASS_SIZE_NODIE(USoundNodeOscillator)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeRandom,Weights)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeRandom,NumRandomUsed)
VERIFY_CLASS_SIZE_NODIE(USoundNodeRandom)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeWave,CompressionQuality)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeWave,FaceFXAnimName)
VERIFY_CLASS_SIZE_NODIE(USoundNodeWave)
VERIFY_CLASS_OFFSET_NODIE(U,SoundNodeWaveParam,WaveParameterName)
VERIFY_CLASS_SIZE_NODIE(USoundNodeWaveParam)
#endif // VERIFY_CLASS_SIZES
#endif // !ENUMS_ONLY

#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif
