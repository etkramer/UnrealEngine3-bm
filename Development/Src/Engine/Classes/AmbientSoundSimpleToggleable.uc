class AmbientSoundSimpleToggleable extends AmbientSoundSimple
    placeable;

struct CheckpointRecord
{
    var bool bCurrentlyPlaying;

    structdefaultproperties
    {
        bCurrentlyPlaying=false
    }
};

var repnotify bool bCurrentlyPlaying;

simulated event PostBeginPlay()
{
    super(Actor).PostBeginPlay();
    bCurrentlyPlaying = AudioComponent.bAutoPlay;
    //return;    
}

simulated event ReplicatedEvent(name VarName)
{
    // End:0x36
    if(VarName == 'bCurrentlyPlaying')
    {
        // End:0x29
        if(bCurrentlyPlaying)
        {
            StartPlaying();            
        }
        else
        {
            StopPlaying();
        }        
    }
    else
    {
        super(Actor).ReplicatedEvent(VarName);
    }
    //return;    
}

simulated function StartPlaying()
{
    AudioComponent.Play();
    bCurrentlyPlaying = true;
    //return;    
}

simulated function StopPlaying()
{
    AudioComponent.Stop();
    bCurrentlyPlaying = false;
    //return;    
}

simulated function OnToggle(SeqAct_Toggle Action)
{
    // End:0x67
    if(Action.InputLinks[0].bHasImpulse || Action.InputLinks[2].bHasImpulse && !AudioComponent.bWasPlaying)
    {
        StartPlaying();        
    }
    else
    {
        StopPlaying();
    }
    ForceNetRelevant();
    //return;    
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
    Record.bCurrentlyPlaying = bCurrentlyPlaying;
    //return;    
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
    bCurrentlyPlaying = Record.bCurrentlyPlaying;
    // End:0x2E
    if(bCurrentlyPlaying)
    {
        StartPlaying();        
    }
    else
    {
        StopPlaying();
    }
    //return;    
}

defaultproperties
{
    // Reference: SoundCue'Default__AmbientSoundSimpleToggleable.SoundCue0'
    // Archetype: SoundCue'Default__AmbientSoundSimple.SoundCue0'
    begin object name="SoundCue0"
    end object
    SoundCueInstance=SoundCue0
    // Reference: SoundNodeAmbient'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0'
    // Archetype: SoundNodeAmbient'Default__AmbientSoundSimple.SoundNodeAmbient0'
    begin object name="SoundNodeAmbient0"
        MinRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionMinRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionMinRadius'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionMinRadius'
        begin object name="DistributionMinRadius"
        end object
        Distribution=DistributionMinRadius)
        MaxRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionMaxRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionMaxRadius'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionMaxRadius'
        begin object name="DistributionMaxRadius"
        end object
        Distribution=DistributionMaxRadius)
        LPFMinRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionLPFMinRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionLPFMinRadius'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionLPFMinRadius'
        begin object name="DistributionLPFMinRadius"
        end object
        Distribution=DistributionLPFMinRadius)
        LPFMaxRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionLPFMaxRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionLPFMaxRadius'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionLPFMaxRadius'
        begin object name="DistributionLPFMaxRadius"
        end object
        Distribution=DistributionLPFMaxRadius)
        VolumeModulation=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionVolume'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionVolume'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionVolume'
        begin object name="DistributionVolume"
        end object
        Distribution=DistributionVolume)
        PitchModulation=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimpleToggleable.SoundNodeAmbient0.DistributionPitch'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionPitch'
        // Archetype: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionPitch'
        begin object name="DistributionPitch"
        end object
        Distribution=DistributionPitch)
    end object
    SoundNodeInstance=SoundNodeAmbient0
    bAutoPlay=false
    // Reference: AudioComponent'Default__AmbientSoundSimpleToggleable.AudioComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'AudioComponent0'
    // Archetype: AudioComponent'Default__AmbientSoundSimple.AudioComponent0'
    begin object name="AudioComponent0"
        PreviewSoundRadius=none
    end object
    AudioComponent=AudioComponent0
    bStatic=false
    bNoDelete=true
    Components[0]=none
    Components[1]=none
    Components[2]=AudioComponent0
}
