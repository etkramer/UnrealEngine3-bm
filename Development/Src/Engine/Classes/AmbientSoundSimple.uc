class AmbientSoundSimple extends AmbientSound
    native(Sound)
    placeable;

var() editconst editinline SoundNodeAmbient AmbientProperties;
var const export editinline SoundCue SoundCueInstance;
var const export editinline SoundNodeAmbient SoundNodeInstance;

cpptext
{
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
}

defaultproperties
{
    // Reference: SoundCue'Default__AmbientSoundSimple.SoundCue0'
    begin object name="SoundCue0" class=Class'SoundCue'
    end object
    SoundCueInstance=SoundCue0
    // Reference: SoundNodeAmbient'Default__AmbientSoundSimple.SoundNodeAmbient0'
    begin object name="SoundNodeAmbient0" class=Class'SoundNodeAmbient'
        MinRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionMinRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionMinRadius'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionMinRadius'
        begin object name="DistributionMinRadius"
        end object
        Distribution=DistributionMinRadius)
        MaxRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionMaxRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionMaxRadius'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionMaxRadius'
        begin object name="DistributionMaxRadius"
        end object
        Distribution=DistributionMaxRadius)
        LPFMinRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionLPFMinRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionLPFMinRadius'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionLPFMinRadius'
        begin object name="DistributionLPFMinRadius"
        end object
        Distribution=DistributionLPFMinRadius)
        LPFMaxRadius=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionLPFMaxRadius'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionLPFMaxRadius'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionLPFMaxRadius'
        begin object name="DistributionLPFMaxRadius"
        end object
        Distribution=DistributionLPFMaxRadius)
        VolumeModulation=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionVolume'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionVolume'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionVolume'
        begin object name="DistributionVolume"
        end object
        Distribution=DistributionVolume)
        PitchModulation=(Distribution=// Reference: DistributionFloatUniform'Default__AmbientSoundSimple.SoundNodeAmbient0.DistributionPitch'
        // TemplateOwnerClass: none
        // TemplateOwnerName: 'DistributionPitch'
        // Archetype: DistributionFloatUniform'Default__SoundNodeAmbient.DistributionPitch'
        begin object name="DistributionPitch"
        end object
        Distribution=DistributionPitch)
    end object
    SoundNodeInstance=SoundNodeAmbient0
    // Reference: AudioComponent'Default__AmbientSoundSimple.AudioComponent0'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'AudioComponent0'
    // Archetype: AudioComponent'Default__AmbientSound.AudioComponent0'
    begin object name="AudioComponent0"
        PreviewSoundRadius=none
    end object
    AudioComponent=AudioComponent0
    Components[0]=none
    Components[1]=none
    Components[2]=AudioComponent0
}
