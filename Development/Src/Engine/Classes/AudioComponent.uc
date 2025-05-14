class AudioComponent extends ActorComponent
    native
    editinlinenew
    collapsecategories
    noexport;

struct native AudioComponentParam
{
    var() name ParamName;
    var() float FloatParam;
    var() float SeekSpeed;
    var() float Velocity;
    var bool bSetValue;
    var() SoundNodeWave WaveParam;

    structdefaultproperties
    {
        ParamName="None"
        FloatParam=0.0000000
        SeekSpeed=0.0000000
        Velocity=0.0000000
        bSetValue=false
        WaveParam=none
    }
};

var() SoundCue SoundCue;
var native const SoundNode CueFirstNode;
var() editinline array<editinline AudioComponentParam> InstanceParameters;
var bool bUseOwnerLocation;
var bool bAutoPlay;
var bool bAutoDestroy;
var bool bStopWhenOwnerDestroyed;
var bool bShouldRemainActiveIfDropped;
var bool bWasOccluded;
var transient bool bSuppressSubtitles;
var transient bool bWasPlaying;
var bool bAllowSpatialization;
var transient bool bFinished;
var transient bool bPreviewComponent;
var transient bool bIgnoreForFlushing;
var transient bool bApplyEffects;
var transient bool bAlwaysPlay;
var transient bool bIsUISound;
var transient bool bIsMusic;
var transient bool bNoReverb;
var transient bool bFMODGenerated;
var transient bool bIsPaused;
var transient bool bForcePauseUpdate;
var transient bool bPlaying;
var transient float SubPriDistance;
var duplicatetransient native const array<Pointer> WaveInstances{struct FWaveInstance};
var duplicatetransient native const array<byte> SoundNodeData;
var duplicatetransient native const map{USoundNode*,UINT} SoundNodeOffsetMap;
var duplicatetransient native const MultiMap_Mirror SoundNodeResetWaveMap{TMultiMap<USoundNode*,FWaveInstance*>};
var duplicatetransient native const MultiMap_Mirror SoundNodeFMODMap{TMultiMap<USoundNode*,UINT>};
var duplicatetransient native const Pointer Listener{struct FListener};
var duplicatetransient native const float PlaybackTime;
var duplicatetransient native const PortalVolume PortalVolume;
var duplicatetransient native Vector Location;
var duplicatetransient native const Vector ComponentLocation;
var const transient Actor LastOwner;
var native float SubtitlePriority;
var native const SoundNode CurrentNotifyBufferFinishedHook;
var native const Vector CurrentLocation;
var native const float CurrentVolume;
var native const float CurrentPitch;
var native const int CurrentUseSpatialization;
var native const int CurrentUseSeamlessLooping;
var native const float CurrentVolumeMultiplier;
var native const float CurrentPitchMultiplier;
var() float VolumeMultiplier;
var() float PitchMultiplier;
var native const bool bIgnorePitch;
var transient bool bEmoteCue;
var float OcclusionCheckInterval;
var transient float LastOcclusionCheckTime;
var const export editinline DrawSoundRadiusComponent PreviewSoundRadius;
//var delegate<OnAudioFinished> __OnAudioFinished__Delegate;
//var delegate<OnAudioMarker> __OnAudioMarker__Delegate;

// Export UAudioComponent::execPlay(FFrame&, void* const)
native final function Play();

// Export UAudioComponent::execStop(FFrame&, void* const)
native final function Stop();

// Export UAudioComponent::execKeyOff(FFrame&, void* const)
native final function KeyOff();

// Export UAudioComponent::execKeyOffOnMarker(FFrame&, void* const)
native final function KeyOffOnMarker();

// Export UAudioComponent::execIsPlaying(FFrame&, void* const)
native final function bool IsPlaying();

// Export UAudioComponent::execIsPaused(FFrame&, void* const)
native final function bool IsPaused();

// Export UAudioComponent::execPause(FFrame&, void* const)
native final function Pause(bool bDoPause);

// Export UAudioComponent::execGetMinDistance(FFrame&, void* const)
native final function float GetMinDistance();

// Export UAudioComponent::execGetMaxDistance(FFrame&, void* const)
native final function float GetMaxDistance();

// Export UAudioComponent::execSetFloatParameter(FFrame&, void* const)
native final function SetFloatParameter(name InName, float InFloat);

// Export UAudioComponent::execSetSeekSpeed(FFrame&, void* const)
native final function SetSeekSpeed(name InName, float InFloat);

// Export UAudioComponent::execSetVelocity(FFrame&, void* const)
native final function SetVelocity(name InName, float InFloat);

// Export UAudioComponent::execSetWaveParameter(FFrame&, void* const)
native final function SetWaveParameter(name InName, SoundNodeWave InWave);

// Export UAudioComponent::execResetToDefaults(FFrame&, void* const)
native final function ResetToDefaults();

// Export UAudioComponent::execIsLooping(FFrame&, void* const)
native final function bool IsLooping();

delegate OnAudioFinished(AudioComponent AC)
{
    //return;    
}

delegate OnAudioMarker(AudioComponent AC, string MarkerName)
{
    //return;    
}

// Export UAudioComponent::execGetAverageVolume(FFrame&, void* const)
native final function float GetAverageVolume();

defaultproperties
{
    bUseOwnerLocation=true
    bAllowSpatialization=true
    VolumeMultiplier=1.0000000
    PitchMultiplier=1.0000000
}
