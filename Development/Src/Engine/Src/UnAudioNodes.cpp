/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h" 
#include "EngineAudioDeviceClasses.h"
#include "EngineSoundClasses.h"
#include "UnSubtitleManager.h"

// Some predefined FNames
static FName SpokenTextFName = FName( TEXT( "SpokenText" ) );
static FName TTSSpeakerFName = FName( TEXT( "TTSSpeaker" ) );
static FName bUseTTSFName = FName( TEXT( "bUseTTS" ) );
static FName CompressionQualityFName = FName( TEXT( "CompressionQuality" ) );

//@warning: DO NOT access variables declared via DECLARE_SOUNDNODE_ELEMENT* after calls to ParseNodes as they
//@warning: might cause the array to be realloc'ed and hence will point to garbage data till the declaration.

/*-----------------------------------------------------------------------------
	USoundCue implementation.
-----------------------------------------------------------------------------*/

/**
 * Recursively finds all attenuation nodes in a sound node tree
 */
void USoundCue::RecursiveFindAttenuation( USoundNode* Node, TArray<USoundNodeAttenuation*> &OutAttens )
{
	if( Node )
	{
		// Record the node if it is attenuation
		if( Node->IsA( USoundNodeAttenuation::StaticClass() ) )
		{
			OutAttens.AddUniqueItem( static_cast<USoundNodeAttenuation*>( Node ) );
		}

		// Recurse.
		const INT MaxChildNodes = Node->GetMaxChildNodes();
		for( INT ChildIndex = 0 ; ChildIndex < Node->ChildNodes.Num() && ( ChildIndex < MaxChildNodes || MaxChildNodes == -1 ); ++ChildIndex )
		{
			RecursiveFindAttenuation( Node->ChildNodes( ChildIndex ), OutAttens );
		}
	}
}

/**
 * Recursively finds all waves in a sound node tree.
 */
void USoundCue::RecursiveFindWaves( USoundNode* Node, TArray<USoundNodeWave*> &OutWaves )
{
	if( Node )
	{
		// Record the node if it is a wave.
		if( Node->IsA( USoundNodeWave::StaticClass() ) )
		{
			OutWaves.AddUniqueItem( static_cast<USoundNodeWave*>( Node ) );
		}

		// Recurse.
		const INT MaxChildNodes = Node->GetMaxChildNodes();
		for( INT ChildIndex = 0 ; ChildIndex < Node->ChildNodes.Num() && ( ChildIndex < MaxChildNodes || MaxChildNodes == -1 ); ++ChildIndex )
		{
			RecursiveFindWaves( Node->ChildNodes( ChildIndex ), OutWaves );
		}
	}
}

/**
 * Returns a description of this object that can be used as extra information in list views.
 */
FString USoundCue::GetDesc( void )
{
	FString Description = TEXT( "" );

	// Display duration
	if( GetCueDuration() < INDEFINITELY_LOOPING_DURATION )
	{
		Description = FString::Printf( TEXT( "%3.2fs" ), GetCueDuration() );
	}
	else
	{
		Description = TEXT( "Forever" );
	}

	// Display group
	Description += TEXT( " [" );
	Description += TEXT("Ungrouped");
	Description += TEXT( "]" );

	return Description;
}

/** 
 * Returns detailed info to populate listview columns
 */
FString USoundCue::GetDetailedDescription( INT InIndex )
{
	FString Description = TEXT( "" );
	switch( InIndex )
	{
	case 0:
		Description = TEXT("Ungrouped");
		break;
	case 3:
		if( GetCueDuration() < INDEFINITELY_LOOPING_DURATION )
		{
			Description = FString::Printf( TEXT( "%2.2f Sec" ), GetCueDuration() );
		}
		else
		{
			Description = TEXT( "Forever" );
		}
		break;
	case 7:
		TArray<USoundNodeWave*> Waves;

		RecursiveFindWaves( FirstNode, Waves );

		Description = TEXT( "<no subtitles>" );
		if( Waves.Num() > 0 )
		{
			USoundNodeWave* Wave = Waves( 0 );
			if( Wave->Subtitles.Num() > 0 )
			{
				const FSubtitleCue& Cue = Wave->Subtitles(0);
				Description = FString::Printf( TEXT( "%c \"%s\"" ), Waves.Num() > 1 ? TEXT( '*' ) : TEXT( ' ' ), *Cue.Text );
			}
		}
		break;
	}

	return( Description );
}

/**
 * @return		Sum of the size of waves referenced by this cue.
 */
INT USoundCue::GetResourceSize( void )
{
	TArray<USoundNodeWave*> Waves;
	RecursiveFindWaves( FirstNode, Waves );

	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
	for( INT WaveIndex = 0; WaveIndex < Waves.Num(); ++WaveIndex )
	{
		ResourceSize += Waves( WaveIndex )->GetResourceSize();
	}

	return( ResourceSize );
}

/**
 *	@param		Platform		EPlatformType indicating the platform of interest...
 *
 *	@return		Sum of the size of waves referenced by this cue for the given platform.
 */
INT USoundCue::GetResourceSize( UE3::EPlatformType Platform )
{
	TArray<USoundNodeWave*> Waves;
	RecursiveFindWaves(FirstNode, Waves);

	FArchiveCountMem CountBytesSize(this);
	INT ResourceSize = CountBytesSize.GetNum();
	for (INT WaveIndex = 0; WaveIndex < Waves.Num(); ++WaveIndex)
	{
		ResourceSize += Waves(WaveIndex)->GetResourceSize(Platform);
	}

	return ResourceSize;
}

/** 
 * Standard Serialize function
 */
void USoundCue::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	Ar << EditorData;
	// Empty when loading and we don't care about saving it again, like e.g. a client.
	if( Ar.IsLoading() && ( !GIsEditor && !GIsUCC ) )
	{
		EditorData.Empty();
	}
}

/**
 * Strip Editor-only data when cooking for console platforms.
 */
void USoundCue::StripData( UE3::EPlatformType TargetPlatform )
{
	Super::StripData( TargetPlatform );
	if( ( TargetPlatform & UE3::PLATFORM_Console ) || GIsCookingForDemo )
	{
		EditorData.Empty();
	}
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void USoundCue::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	MarkPackageDirty();
	// MFricker - TTP 89318
	// GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
}

/*-----------------------------------------------------------------------------
	USoundNode implementation.
-----------------------------------------------------------------------------*/

void USoundNode::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	for( INT i = 0; i < ChildNodes.Num() && i < GetMaxChildNodes(); i++ )
	{
		if( ChildNodes( i ) )
		{
			ChildNodes( i )->ParseNodes( AudioDevice, this, i, AudioComponent, WaveInstances );
		}
	}
}

void USoundNode::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );
	INT MaxChildNodes = GetMaxChildNodes();
	for( INT i = 0; i < ChildNodes.Num() && ( i < MaxChildNodes || MaxChildNodes == -1 ); i++ )
	{
		if( ChildNodes( i ) )
		{
			ChildNodes( i )->GetNodes( AudioComponent, SoundNodes );
		}
	}
}

void USoundNode::GetAllNodes( TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );
	INT MaxChildNodes = GetMaxChildNodes();
	for( INT i = 0; i < ChildNodes.Num() && ( i < MaxChildNodes || MaxChildNodes == -1 ); i++ )
	{
		if( ChildNodes( i ) )
		{
			ChildNodes( i )->GetAllNodes( SoundNodes );
		}
	}
}

/**
 * Helper function to reset bFinished on wave instances this node has been notified of being finished.
 *
 * @param	AudioComponent	Audio component this node is used in.
 */
void USoundNode::ResetWaveInstances( UAudioComponent* AudioComponent )
{
	// Find all wave instances associated with this node in the passed in audio component.
	TArray<FWaveInstance*> ComponentWaveInstances;
	AudioComponent->SoundNodeResetWaveMap.MultiFind( this, ComponentWaveInstances );

	// Reset bFinished on wave instances found.
	for( INT InstanceIndex = 0; InstanceIndex < ComponentWaveInstances.Num(); InstanceIndex++ )
	{
		FWaveInstance* WaveInstance = ComponentWaveInstances( InstanceIndex );
		WaveInstance->bIsStarted = FALSE;
		WaveInstance->bIsFinished = FALSE;
	}

	// Empty list.
	AudioComponent->SoundNodeResetWaveMap.Remove( this );
}

/**
 * Called by the Sound Cue Editor for nodes which allow children.  The default behaviour is to
 * attach a single connector. Derived classes can override to eg add multiple connectors.
 */
void USoundNode::CreateStartingConnectors( void )
{
	InsertChildNode( ChildNodes.Num() );
}

void USoundNode::InsertChildNode( INT Index )
{
	check( Index >= 0 && Index <= ChildNodes.Num() );
	ChildNodes.InsertZeroed( Index );
}

void USoundNode::RemoveChildNode( INT Index )
{
	check( Index >= 0 && Index < ChildNodes.Num() );
	ChildNodes.Remove( Index );
}

FLOAT USoundNode::GetDuration( void )
{
	// Iterate over children and return maximum length of any of them
	FLOAT MaxDuration = 0.0f;
	for( INT i = 0; i < ChildNodes.Num(); i++ )
	{
		if( ChildNodes( i ) )
		{
			MaxDuration = ::Max( ChildNodes( i )->GetDuration(), MaxDuration );
		}
	}
	return MaxDuration;
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void USoundNode::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	MarkPackageDirty();

	// MFricker - TTP 89318
	// GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
}

IMPLEMENT_CLASS(USoundNode);


/*-----------------------------------------------------------------------------
	USoundNodeMixer implementation.
-----------------------------------------------------------------------------*/

void USoundNodeMixer::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	// A mixer cannot use seamless looping as it waits for the longest one to finish before it "finishes".
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	for( INT ChildNodeIndex = 0; ChildNodeIndex < ChildNodes.Num(); ChildNodeIndex++ )
	{
		if( ChildNodes(ChildNodeIndex) )
		{
			FAudioComponentSavedState SavedState;
			SavedState.Set( AudioComponent );
			AudioComponent->CurrentVolume *= InputVolume( ChildNodeIndex );
			ChildNodes(ChildNodeIndex)->ParseNodes( AudioDevice, this, ChildNodeIndex, AudioComponent, WaveInstances );
			SavedState.Restore( AudioComponent );
		}
	}
}

/**
 * Mixers have two connectors by default.
 */
void USoundNodeMixer::CreateStartingConnectors( void )
{
	// Mixers default with two connectors.
	InsertChildNode( ChildNodes.Num() );
	InsertChildNode( ChildNodes.Num() );
}

/**
 * Overloaded to add an entry to InputVolume.
 */
void USoundNodeMixer::InsertChildNode( INT Index )
{
	Super::InsertChildNode( Index );
	InputVolume.Insert( Index );
	InputVolume( Index ) = 1.0f;
}

/**
 * Overloaded to remove an entry from InputVolume.
 */
void USoundNodeMixer::RemoveChildNode( INT Index )
{
	Super::RemoveChildNode( Index );
	InputVolume.Remove( Index );
}

IMPLEMENT_CLASS(USoundNodeMixer);


/*-----------------------------------------------------------------------------
USoundNodeDistanceCrossFade implementation.
-----------------------------------------------------------------------------*/

FLOAT USoundNodeDistanceCrossFade::MaxAudibleDistance( FLOAT CurrentMaxDistance )
{
	FLOAT Retval = 0;

	for( INT ChildNodeIndex=0; ChildNodeIndex<ChildNodes.Num(); ChildNodeIndex++ )
	{
		FLOAT FadeInDistanceMin = 0.0f;
		FLOAT FadeInDistanceMax = 0.0f;
		CrossFadeInput( ChildNodeIndex ).FadeInDistance.GetOutRange( FadeInDistanceMin, FadeInDistanceMax);

		FLOAT FadeOutDistanceMin = 0.0f;
		FLOAT FadeOutDistanceMax = 0.0f;
		CrossFadeInput( ChildNodeIndex ).FadeOutDistance.GetOutRange( FadeOutDistanceMin, FadeOutDistanceMax);

		if( FadeInDistanceMax > Retval )
		{
			Retval = FadeInDistanceMax;
		}

		if( FadeOutDistanceMax > Retval )
		{
			Retval = FadeOutDistanceMax;
		}
	}

	return Retval;
}


void USoundNodeDistanceCrossFade::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	// A "mixer type" cannot use seamless looping as it waits for the longest path to finish before it "finishes".
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	for( INT ChildNodeIndex = 0; ChildNodeIndex < ChildNodes.Num(); ChildNodeIndex++ )
	{
		if( ChildNodes( ChildNodeIndex ) != NULL )
		{
			FAudioComponentSavedState SavedState;
			// save our state for this path through the SoundCue (i.e. at the end we need to remove any modifications parsing the path did so we can parse the next input with a clean slate)
			SavedState.Set( AudioComponent );

			// get the various distances for this input so we can fade in/out the volume correctly
			FLOAT FadeInDistanceMin = 0.0f;
			FLOAT FadeInDistanceMax = 0.0f;
			CrossFadeInput( ChildNodeIndex ).FadeInDistance.GetOutRange( FadeInDistanceMin, FadeInDistanceMax );

			FLOAT FadeOutDistanceMin = 0.0f;
			FLOAT FadeOutDistanceMax = 0.0f;
			CrossFadeInput( ChildNodeIndex ).FadeOutDistance.GetOutRange( FadeOutDistanceMin, FadeOutDistanceMax );

			// watch out here.  If one is playing the sound on the PlayerController then this will not update correctly as PlayerControllers don't move in normal play
			const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );

			// determine the volume amount we should set the component to before "playing" 
			FLOAT VolumeToSet = 1.0f;
			//debugf( TEXT("  USoundNodeDistanceCrossFade.  Distance: %f ChildNodeIndex: %d CurrLoc: %s  ListenerLoc: %s"), Distance, ChildNodeIndex, *AudioComponent->CurrentLocation.ToString(), *AudioComponent->Listener->Location.ToString() );

			// Ignore distance calculations for preview components as they are undefined
			if( AudioComponent->bPreviewComponent )
			{
				VolumeToSet = CrossFadeInput( ChildNodeIndex ).Volume;
			}
			else if( ( Distance >= FadeInDistanceMin ) && ( Distance <= FadeInDistanceMax ) )
			{
				VolumeToSet = CrossFadeInput( ChildNodeIndex ).Volume * ( 0.0f + ( Distance - FadeInDistanceMin ) / ( FadeInDistanceMax - FadeInDistanceMin ) );
				//debugf( TEXT("     FadeIn.  Distance: %f,  VolumeToSet: %f"), Distance, VolumeToSet );
			}
			// else if we are inside the FadeOut edge
			else if( ( Distance >= FadeOutDistanceMin ) && ( Distance <= FadeOutDistanceMax ) )
			{
				VolumeToSet = CrossFadeInput( ChildNodeIndex ).Volume * ( 1.0f - ( Distance - FadeOutDistanceMin ) / ( FadeOutDistanceMax - FadeOutDistanceMin ) );
				//debugf( TEXT("     FadeOut.  Distance: %f,  VolumeToSet: %f"), Distance, VolumeToSet );
			}
			// else we are in between the fading edges of the CrossFaded sound and we should play the
			// sound at the CrossFadeInput's specified volume
			else if( ( Distance >= FadeInDistanceMax ) && ( Distance <= FadeOutDistanceMin ) )
			{
				VolumeToSet = CrossFadeInput( ChildNodeIndex ).Volume;
				//debugf( TEXT("     In Between.  Distance: %f,  VolumeToSet: %f"), Distance, VolumeToSet );
			}
			// else we are outside of the range of this CrossFadeInput and should not play anything
			else
			{
				//debugf( TEXT("     OUTSIDE!!!" ));
				VolumeToSet = 0; //CrossFadeInput( ChildNodeIndex ).Volume;
			}

			AudioComponent->CurrentVolume *= VolumeToSet;

			// "play" the rest of the tree
			ChildNodes( ChildNodeIndex )->ParseNodes( AudioDevice, this, ChildNodeIndex, AudioComponent, WaveInstances );

			SavedState.Restore( AudioComponent );
		}
	}
}

/**
 *  USoundNodeDistanceCrossFades have two connectors by default.
 **/
void USoundNodeDistanceCrossFade::CreateStartingConnectors()
{
	// Mixers default with two connectors.
	InsertChildNode( ChildNodes.Num() );
	InsertChildNode( ChildNodes.Num() );
}


void USoundNodeDistanceCrossFade::InsertChildNode( INT Index )
{
	Super::InsertChildNode( Index );
	CrossFadeInput.InsertZeroed( Index );

	CrossFadeInput(Index).FadeInDistance.Distribution = ConstructObject<UDistributionFloatUniform>(UDistributionFloatUniform::StaticClass(), this );

	CrossFadeInput(Index).FadeOutDistance.Distribution = ConstructObject<UDistributionFloatUniform>(UDistributionFloatUniform::StaticClass(), this );

	CrossFadeInput(Index).Volume = 1.0f;
}


void USoundNodeDistanceCrossFade::RemoveChildNode( INT Index )
{
	Super::RemoveChildNode( Index );
	CrossFadeInput.Remove( Index );
}

IMPLEMENT_CLASS(USoundNodeDistanceCrossFade);


/*-----------------------------------------------------------------------------
	USoundNodeWave implementation.
-----------------------------------------------------------------------------*/

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT USoundNodeWave::GetResourceSize( void )
{
	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
#if _WINDOWS
		if( DecompressionType == DTYPE_Native )
		{
			ResourceSize += SampleDataSize; 
		}
		else
		{
			ResourceSize += CompressedPCData.GetBulkDataSize(); 
		}
#elif PS3
		ResourceSize += CompressedPS3Data.GetBulkDataSize(); 
#elif _XBOX
		ResourceSize += CompressedXbox360Data.GetBulkDataSize();
#endif
	return ResourceSize;
}

/**
 *	@param		Platform		EPlatformType indicating the platform of interest...
 *
 *	@return		Sum of the size of waves referenced by this cue for the given platform.
 */
INT USoundNodeWave::GetResourceSize( UE3::EPlatformType Platform )
{
	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();

	if ((Platform & UE3::PLATFORM_PC) != 0)
	{
		if( DecompressionType == DTYPE_Native )
		{
			ResourceSize += SampleDataSize; 
		}
		else
		{
			ResourceSize += CompressedPCData.GetBulkDataSize(); 
		}
	}
	else
	if ((Platform & UE3::PLATFORM_PS3) != 0)
	{
		ResourceSize += CompressedPS3Data.GetBulkDataSize(); 
	}
	else 
	if ((Platform & UE3::PLATFORM_Xenon) != 0)
	{
		ResourceSize += CompressedXbox360Data.GetBulkDataSize();
	}

	return ResourceSize;
}

/** 
 * Returns the name of the exporter factory used to export this object
 * Used when multiple factories have the same extension
 */
FName USoundNodeWave::GetExporterName( void )
{
	if( ChannelOffsets.Num() > 0 && ChannelSizes.Num() > 0 )
	{
		return( FName( TEXT( "SoundSurroundExporterWAV" ) ) );
	}

	return( FName( TEXT( "SoundExporterWAV" ) ) );
}

/** 
 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
 */
FString USoundNodeWave::GetDesc( void )
{
	FString Channels;

	if( NumChannels == 0 )
	{
		Channels = TEXT( "Unconverted" );
	}
	else if( ChannelSizes.Num() == 0 )
	{
		Channels = ( NumChannels == 1 ) ? TEXT( "Mono" ) : TEXT( "Stereo" );
	}
	else
	{
		Channels = FString::Printf( TEXT( "%d Channels" ), NumChannels );
	}	

	FString Description = FString::Printf( TEXT( "%3.2fs %s" ), GetDuration(), *Channels );
	return Description;
}

/**
 * Returns a detailed description for populating listview columns
 */
FString USoundNodeWave::GetDetailedDescription( INT InIndex )
{
	FString Description = TEXT( "" );
	switch( InIndex )
	{
	case 0:
		if( NumChannels == 0 )
		{
			Description = TEXT( "Unconverted" );
		}
		else if( ChannelSizes.Num() == 0 )
		{
			Description = ( NumChannels == 1 ) ? TEXT( "Mono" ) : TEXT( "Stereo" );
		}
		else
		{
			Description = FString::Printf( TEXT( "%d Channels" ), NumChannels );
		}
		break;
	case 1:
		if( SampleRate != 0 )
		{
			Description = FString::Printf( TEXT( "%d Hz" ), SampleRate );
		}
		break;
	case 2:
		Description = FString::Printf( TEXT( "%d pct" ), CompressionQuality );
		break;
	case 3:
		Description = FString::Printf( TEXT( "%2.2f Sec" ), GetDuration() );
		break;
	case 4:
		Description = FString::Printf( TEXT( "%.2f Kb Ogg" ), CompressedPCData.GetBulkDataSize() / 1024.0f );
		break;
	case 5:
		Description = FString::Printf( TEXT( "%.2f Kb XMA" ), CompressedXbox360Data.GetBulkDataSize() / 1024.0f );
		break;
	case 6:
		Description = FString::Printf( TEXT( "%.2f Kb PS3" ), CompressedPS3Data.GetBulkDataSize() / 1024.0f );
		break;
	case 7:
		if( Subtitles.Num() > 0 )
		{
			Description = FString::Printf( TEXT( "\"%s\"" ), *Subtitles( 0 ).Text );
		}
		else
		{
			Description = TEXT( "<no subtitles>" );
		}
		break;
	}
	return( Description );
}

void USoundNodeWave::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	
	RawData.Serialize( Ar, this );
	
	CompressedPCData.Serialize( Ar, this );
	CompressedXbox360Data.Serialize( Ar, this );
	CompressedPS3Data.Serialize( Ar, this );

	if( Ar.Ver() < VER_MP3ENC_TO_MSENC )
	{
		CompressedPS3Data.RemoveBulkData();
	}

	extern UBOOL GForceSoundRecook;
	if( Ar.IsLoading() && GForceSoundRecook )
	{
		CompressedPCData.RemoveBulkData();
		CompressedPS3Data.RemoveBulkData();
		CompressedXbox360Data.RemoveBulkData();
	}
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void USoundNodeWave::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData( TargetPlatform );

	// Remove original wav data
	RawData.RemoveBulkData();

	ChannelOffsets.Empty();
	ChannelSizes.Empty();
}

/** 
* Removes the bulk data from any USoundNodeWave objects that were loaded 
*/
void appSoundNodeRemoveBulkData()
{
	for( TObjectIterator<USoundNodeWave> It; It; ++It )
	{
		USoundNodeWave* SoundNodeWave = *It;
		SoundNodeWave->CompressedPS3Data.RemoveBulkData();
		SoundNodeWave->CompressedPCData.RemoveBulkData();
		SoundNodeWave->CompressedXbox360Data.RemoveBulkData();
	}
}

/**
 * Outside the Editor, uploads resource to audio device and performs general PostLoad work.
 *
 * This function is being called after all objects referenced by this object have been serialized.
 */
extern INT Localization_GetLanguageExtensionIndex(const TCHAR* Ext);

void USoundNodeWave::PostLoad( void )
{
	Super::PostLoad();

	// Only the cooked version will have the LocalizedSubtitles array filled in...
	if( LocalizedSubtitles.Num() > 0 )
	{
		INT LangIndex = Localization_GetLanguageExtensionIndex( UObject::GetLanguage() );

		if( GUseFullLocalizationFiles == FALSE )
		{
			// Copy the proper one into the 'real' properties, then clear the array.
			FLocalizedSubtitle& LocSubtitle = LocalizedSubtitles( LangIndex );
			// Copy it into the appropriate slot
			bManualWordWrap = LocSubtitle.bManualWordWrap;
			bMature = LocSubtitle.bMature;
			Subtitles = LocSubtitle.Subtitles;
		}
		// Clear the array - no sense storing what we don't need.
		LocalizedSubtitles.Empty();

		// Only INT has mature lines
		if( LangIndex != 0 )
		{
			bMature = FALSE;
		}
	}

	// We don't precache default objects and we don't precache in the Editor as the latter will
	// most likely cause us to run out of memory.
	if( !GIsEditor && !IsTemplate( RF_ClassDefaultObject ) && GEngine && GEngine->Client )
	{
		UAudioDevice* AudioDevice = GEngine->Client->GetAudioDevice();
		if( AudioDevice )
		{
#if WITH_TTS
			// Create the TTS PCM data if necessary
			if( bUseTTS && ( ResourceID == 0 ) && ( RawPCMData == NULL ) )
			{
				AudioDevice->TextToSpeech->CreatePCMData( this );
			}
#endif // WITH_TTS
			// Upload the data to the hardware
			AudioDevice->Precache( this );
		}
		// remove bulk data if no AudioDevice is used and no sounds were initialized
		else if( GIsGame )
		{
			CompressedPS3Data.RemoveBulkData();
			CompressedPCData.RemoveBulkData();
			CompressedXbox360Data.RemoveBulkData();
		}
	}

	INC_FLOAT_STAT_BY( STAT_AudioBufferTime, Duration );
	INC_FLOAT_STAT_BY( STAT_AudioBufferTimeChannels, NumChannels * Duration );
}

/** 
 * Copy the compressed audio data from the bulk data
 */
void USoundNodeWave::InitAudioResource( FByteBulkData& CompressedData )
{
	if( !ResourceSize )
	{
		// Grab the compressed vorbis data from the bulk data
		ResourceSize = CompressedData.GetBulkDataSize();
		if( ResourceSize > 0 )
		{
			CompressedData.GetCopy( &ResourceData, TRUE );
		}
	}
}

/** 
 * Remove the compressed audio data associated with the passed in wave
 */
void USoundNodeWave::RemoveAudioResource( void )
{
	if( ResourceData )
	{
		appFree( ( void* )ResourceData );
		ResourceSize = 0;
		ResourceData = NULL;
	}
}

/**
 * Returns whether this wave file is a localized resource.
 *
 * @return TRUE if it is a localized resource, FALSE otherwise.
 */
UBOOL USoundNodeWave::IsLocalizedResource( void )
{
	FString FullPathName;
	UBOOL bIsLocalised = FALSE;

	if( GPackageFileCache->FindPackageFile( *GetOutermost()->GetPathName(), NULL, FullPathName ) )
	{
		FString BasePath = TEXT( "Content\\Sounds\\" );

		// Package is localised if it resides in Content\Sounds\XXX (where XXX = 3 letter language code)
		INT Offset = FullPathName.InStr( BasePath, FALSE, TRUE );
		if( Offset >= 0 )
		{
			FString LangRelPath = FullPathName.Right( FullPathName.Len() - ( Offset + BasePath.Len() ) );
			if( LangRelPath[3] == '\\' )
			{
				bIsLocalised = TRUE;
			}
		}
	}

	return( Super::IsLocalizedResource() || Subtitles.Num() > 0 || bIsLocalised );
}

/** 
 * Invalidate compressed data
 */
void USoundNodeWave::PostEditChange( UProperty* PropertyThatChanged )
{	
#if _WINDOWS
	// Clear out the generated PCM data if any of the TTS fields are changed
	if( PropertyThatChanged == NULL
		|| PropertyThatChanged->GetFName() == SpokenTextFName
		|| PropertyThatChanged->GetFName() == TTSSpeakerFName
		|| PropertyThatChanged->GetFName() == bUseTTSFName )
	{
		UAudioDevice* AudioDevice = NULL;

		// Stop all sounds in case any SoundNodeWaves are actively being played
		if( GEngine && GEngine->Client )
		{
			AudioDevice = GEngine->Client->GetAudioDevice();
			if( AudioDevice )
			{
				AudioDevice->StopAllSounds( TRUE );
			}
		}

		// Free up all old resources
		FreeResources();

		// Generate new resources
		if( AudioDevice )
		{
			if( bUseTTS )
			{
#if WITH_TTS
				AudioDevice->TextToSpeech->CreatePCMData( this );
#endif // WITH_TTS
			}
			else
			{
				ValidateData();
			}
		
			MarkPackageDirty();

			// MFricker - TTP 89318
			// GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
			// GCallbackEvent->Send( CALLBACK_ObjectPropertyChanged );
		}
	}

	// Regenerate on save any compressed sound formats
	if( PropertyThatChanged && PropertyThatChanged->GetFName() == CompressionQualityFName )
	{
		CompressedPCData.RemoveBulkData();
		CompressedXbox360Data.RemoveBulkData();
		CompressedPS3Data.RemoveBulkData();

		NumChannels = 0;

		MarkPackageDirty();

		// MFricker - TTP 89318
		// GCallbackEvent->Send( CALLBACK_RefreshEditor_GenericBrowser );
	}
#endif
}

/** 
 * Frees up all the resources allocated in this class
 */
void USoundNodeWave::FreeResources( void )
{
	// Housekeeping of stats
	DEC_FLOAT_STAT_BY( STAT_AudioBufferTime, Duration );
	DEC_FLOAT_STAT_BY( STAT_AudioBufferTimeChannels, NumChannels * Duration );

	// GEngine is NULL during script compilation and GEngine->Client and its audio device might be
	// destroyed first during the exit purge.
	if( GEngine && !GExitPurge )
	{
		// Notify the audio device to free the bulk data associated with this wave.
		UAudioDevice* AudioDevice = GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
		if( AudioDevice )
		{
			AudioDevice->FreeResource( this );
		}
	}

	NumChannels = 0;
	SampleRate = 0;
	Duration = 0.0f;
	ResourceID = 0;
	bDynamicResource = FALSE;
	bOneTimeUse = FALSE;
}

/**
 * Returns whether the resource is ready to have finish destroy routed.
 *
 * @return	TRUE if ready for deletion, FALSE otherwise.
 */
UBOOL USoundNodeWave::IsReadyForFinishDestroy( void )
{
	// Wait till vorbis decompression finishes before deleting resource.
	return( ( VorbisDecompressor == NULL ) || VorbisDecompressor->IsDone() );
}

/**
 * Frees the sound resource data.
 */
void USoundNodeWave::FinishDestroy( void )
{
	FreeResources();

	Super::FinishDestroy();
}

void USoundNodeWave::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	AudioComponent->CurrentVolume *= Volume;
	AudioComponent->CurrentPitch *= Pitch;

	// See whether this SoundNodeWave already has a WaveInstance associated with it. Note that we also have to handle
	// the same SoundNodeWave being used multiple times inside the SoundCue which is why we're using a GUID.
	FWaveInstance* WaveInstance = NULL;
	QWORD ParentGUID = ( ( QWORD )( ( Parent ? Parent->GetIndex() : 0xFFFFFFFF ) ) << 32 ) | ( ( DWORD )ChildIndex );
	
	for( INT WaveIndex = 0; WaveIndex < AudioComponent->WaveInstances.Num(); WaveIndex++ )
	{
		FWaveInstance* ExistingWaveInstance = AudioComponent->WaveInstances( WaveIndex );
		if( ExistingWaveInstance->WaveData == this && ExistingWaveInstance->ParentGUID == ParentGUID )
		{
			WaveInstance = ExistingWaveInstance;
			break;
		}
	}

#if WITH_TTS
	// Expand the TTS data if required
	if( bUseTTS && ( ResourceID == 0 ) && ( RawPCMData == NULL ) )
	{
		AudioDevice->TextToSpeech->CreatePCMData( this );
	}
#endif // WITH_TTS

	// Create a new WaveInstance if this SoundNodeWave doesn't already have one associated with it.
	if( WaveInstance == NULL )
	{
		WaveInstance = new FWaveInstance( AudioComponent );
		WaveInstance->ParentGUID = ParentGUID;
		AudioComponent->WaveInstances.AddItem( WaveInstance );

		// Add in the subtitle
		if( !AudioComponent->bSuppressSubtitles )
		{
			// Subtitles are hashed based on the associated sound (wave instance).
			if (Subtitles.Num() > 0)
			{
				FSubtitleManager::GetSubtitleManager()->QueueSubtitles( ( PTRINT )WaveInstance, AudioComponent->SubtitlePriority, bManualWordWrap, Duration, Subtitles );
			}
		}
	}

	// Check for finished paths.
	if( !WaveInstance->bIsFinished )
	{
		// Propagate properties and add WaveInstance to outgoing array of FWaveInstances.
		WaveInstance->Volume = AudioComponent->CurrentVolume;
		WaveInstance->VolumeMultiplier = AudioComponent->CurrentVolumeMultiplier;
		WaveInstance->PlayPriority = AudioComponent->CurrentVolume + ( AudioComponent->bAlwaysPlay ? 1.0f : 0.0f ) + AudioComponent->CurrentVoiceRadioVolume;
		WaveInstance->Pitch = AudioComponent->CurrentPitch * AudioComponent->CurrentPitchMultiplier;
		WaveInstance->HighFrequencyGain = AudioComponent->CurrentHighFrequencyGain;
		WaveInstance->VoiceCenterChannelVolume = AudioComponent->CurrentVoiceCenterChannelVolume;
		WaveInstance->VoiceRadioVolume = AudioComponent->CurrentVoiceRadioVolume;

		WaveInstance->bApplyEffects = AudioComponent->bApplyEffects;
		WaveInstance->bAlwaysPlay = AudioComponent->bAlwaysPlay;
		WaveInstance->bIsUISound = AudioComponent->bIsUISound;
		WaveInstance->bIsMusic = AudioComponent->bIsMusic;
		WaveInstance->bNoReverb = AudioComponent->bNoReverb;
		WaveInstance->bBleedStereo = AudioComponent->bBleedStereo;

		WaveInstance->Location = AudioComponent->CurrentLocation;
		WaveInstance->bIsStarted = TRUE;
		WaveInstance->bAlreadyNotifiedHook = FALSE;
		WaveInstance->bUseSpatialization = AudioComponent->CurrentUseSpatialization;
		WaveInstance->WaveData = this;
		WaveInstance->NotifyBufferFinishedHook = AudioComponent->CurrentNotifyBufferFinishedHook;

		WaveInstance->LoopingMode = LOOP_Never;
		if( AudioComponent->CurrentUseSeamlessLooping )
		{
			WaveInstance->LoopingMode = LOOP_WithNotification;
		}

		// Don't add wave instances that are not going to be played at this point.
		if( WaveInstance->PlayPriority > KINDA_SMALL_NUMBER )
		{
			WaveInstances.AddItem( WaveInstance );
		}

		// We're still alive.
		AudioComponent->bFinished = FALSE;

		// Sanity check
		if( NumChannels > 2 && WaveInstance->bUseSpatialization )
		{
			debugf( NAME_Warning, TEXT( "Spatialisation on stereo and multichannel sounds is not supported (%s)" ), *GetName() );
		}
	}
}

FLOAT USoundNodeWave::GetDuration( void )
{
	// Just return the duration of this sound data.
	return( Duration );
}

IMPLEMENT_CLASS(USoundNodeWave);

/*-----------------------------------------------------------------------------
	USoundNodeAttenuation implementation.
-----------------------------------------------------------------------------*/

void USoundNodeAttenuation::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	if( AudioComponent->bAllowSpatialization )
	{
		RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) );
		DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMinRadius );
		DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMaxRadius );
		DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMinRadius );
		DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMaxRadius );

		if( *RequiresInitialization )
		{
			UsedMinRadius = MinRadius.GetValue( 0.0f, AudioComponent );
			UsedMaxRadius = MaxRadius.GetValue( 0.0f, AudioComponent );
			UsedLPFMinRadius = LPFMinRadius.GetValue( 0.0f, AudioComponent );
			UsedLPFMaxRadius = LPFMaxRadius.GetValue( 0.0f, AudioComponent );
			*RequiresInitialization = 0;
		}

		// Calculate distance from source to listener
		FLOAT Distance = 0.0f;
		switch( DistanceType )
		{
		case SOUNDDISTANCE_Normal:
		default:
			Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
			break;

		case SOUNDDISTANCE_InfiniteXYPlane:
			Distance = fabsf( AudioComponent->CurrentLocation.Z - AudioComponent->Listener->Location.Z );
			break;

		case SOUNDDISTANCE_InfiniteXZPlane:
			Distance = fabsf( AudioComponent->CurrentLocation.Y - AudioComponent->Listener->Location.Y );
			break;

		case SOUNDDISTANCE_InfiniteYZPlane:
			Distance = fabsf( AudioComponent->CurrentLocation.X - AudioComponent->Listener->Location.X );
			break;
		}

		// Attenuate the volume based on the model
		if( bAttenuate )
		{
			FLOAT Constant;
			
			if( Distance >= UsedMaxRadius )
			{
				AudioComponent->CurrentVolume = 0.0f;
			}
			// UsedMinRadius is the point at which to start attenuating
			else if( Distance > UsedMinRadius )
			{
				// determine which AttenuationModel to use here
				if( DistanceModel == ATTENUATION_Linear )
				{
					AudioComponent->CurrentVolume *= 1.0f - ( Distance - UsedMinRadius ) / ( UsedMaxRadius - UsedMinRadius );
				}
				else if( DistanceModel == ATTENUATION_Logarithmic )
				{
					if( UsedMinRadius == 0.0f )
					{
						Constant = 0.25f;
					}
					else
					{
						Constant = -1.0f / appLoge( UsedMinRadius / UsedMaxRadius );
					}
					AudioComponent->CurrentVolume *= Min( Constant * -appLoge( Distance / UsedMaxRadius ), 1.0f );
				}
				else if( DistanceModel == ATTENUATION_Inverse )
				{
					if( UsedMinRadius == 0.0f )
					{
						Constant = 1.0f;
					}
					else
					{
						Constant = UsedMaxRadius / UsedMinRadius;
					}
					AudioComponent->CurrentVolume *= Min( Constant * ( 0.02f / ( Distance / UsedMaxRadius ) ), 1.0f );
				}
				else if( DistanceModel == ATTENUATION_LogReverse )
				{
					if( UsedMinRadius == 0.0f )
					{
						Constant = 0.25f;
					}
					else
					{
						Constant = -1.0f / appLoge( UsedMinRadius / UsedMaxRadius );
					}
					AudioComponent->CurrentVolume *= Max( 1.0f - Constant * appLoge( 1.0f / ( 1.0f - ( Distance / UsedMaxRadius ) ) ), 0.0f );
				}
				else if( DistanceModel == ATTENUATION_NaturalSound )
				{
					check( dBAttenuationAtMax <= 0.0f );
					float DistanceInsideMinMax = ( Distance - UsedMinRadius ) / ( UsedMaxRadius - UsedMinRadius );
					AudioComponent->CurrentVolume *= appPow( 10.0f, ( DistanceInsideMinMax * dBAttenuationAtMax ) / 20.0f );
				}
			}
		}

		// Attenuate with the low pass filter if necessary
		if( bAttenuateWithLowPassFilter )
		{
			if( Distance >= UsedLPFMaxRadius )
			{
				AudioComponent->CurrentHighFrequencyGain = 0.0f;
			}
			// UsedLPFMinRadius is the point at which to start applying the low pass filter
			else if( Distance > UsedLPFMinRadius )
			{
				AudioComponent->CurrentHighFrequencyGain *= 1.0f - ( ( Distance - UsedLPFMinRadius ) / ( UsedLPFMaxRadius - UsedLPFMinRadius ) );
			}
		}

		AudioComponent->CurrentUseSpatialization |= bSpatialize;
	}
	else
	{
		AudioComponent->CurrentUseSpatialization = FALSE;
	}

	Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
}

IMPLEMENT_CLASS(USoundNodeAttenuation);

/*-----------------------------------------------------------------------------
	USoundNodeLooping implementation.
-----------------------------------------------------------------------------*/

// Value used to indicate that loop has finished.
#define LOOP_FINISHED 0

void USoundNodeLooping::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) + sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, UsedLoopCount );
	DECLARE_SOUNDNODE_ELEMENT( INT, FinishedCount );
		
	if( *RequiresInitialization )
	{
		UsedLoopCount = appTrunc( LoopCount.GetValue( 0.0f, AudioComponent ) );
		FinishedCount = 0;

		*RequiresInitialization = 0;
	}

	if( bLoopIndefinitely || UsedLoopCount > LOOP_FINISHED )
	{
		AudioComponent->CurrentNotifyBufferFinishedHook = this;
		AudioComponent->CurrentUseSeamlessLooping = TRUE;
	}
	
	Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
}

/**
 * Returns whether the node is finished after having been notified of buffer being finished.
 *
 * @param	AudioComponent	Audio component containing payload data
 * @return	TRUE if finished, FALSE otherwise.
 */
UBOOL USoundNodeLooping::IsFinished( UAudioComponent* AudioComponent ) 
{ 
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( INT ) + sizeof( INT ) );
	DECLARE_SOUNDNODE_ELEMENT( INT, UsedLoopCount );
	DECLARE_SOUNDNODE_ELEMENT( INT, FinishedCount );

	check( *RequiresInitialization == 0 );

	// Sounds that loop indefinitely can never be finished
	if( bLoopIndefinitely )
	{
		return( FALSE );
	}

	// The -1 is a bit unintuitive as a loop count of 1 means playing the sound twice
	// so NotifyWaveInstanceFinished checks for it being >= 0 as a value of 0 means
	// that we should play it once more.
	return( UsedLoopCount == LOOP_FINISHED );  
}

/**
 * Notifies the sound node that a wave instance in its subtree has finished.
 *
 * @param WaveInstance	WaveInstance that was finished 
 */
void USoundNodeLooping::NotifyWaveInstanceFinished( FWaveInstance* WaveInstance )
{
	UAudioComponent* AudioComponent = WaveInstance->AudioComponent;
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( INT ) + sizeof( INT ) );
	DECLARE_SOUNDNODE_ELEMENT( INT, UsedLoopCount );
	DECLARE_SOUNDNODE_ELEMENT( INT, FinishedCount );

	check( *RequiresInitialization == 0 );
	
	//debugf( TEXT( "NotifyWaveInstanceFinished: %d" ), UsedLoopCount );

	// Maintain loop count
	if( bLoopIndefinitely || UsedLoopCount > LOOP_FINISHED )
	{
		// Add to map of nodes that might need to have bFinished reset.
		AudioComponent->SoundNodeResetWaveMap.AddUnique( this, WaveInstance );

		// Figure out how many child wave nodes are in subtree *and* currently playing
		INT NumChildNodes = 0;
		for( INT InstanceIndex = 0; InstanceIndex < AudioComponent->WaveInstances.Num(); InstanceIndex++ )
		{
			FWaveInstance* ComponentWaveInstance = AudioComponent->WaveInstances( InstanceIndex );
			if( ComponentWaveInstance->bIsStarted && ComponentWaveInstance->bIsFinished )
			{
				NumChildNodes++;
			}
		}

		// Wait till all leaves are finished.
		if( ++FinishedCount == NumChildNodes )
		{
			FinishedCount = 0;

			// Only decrease loop count if all leaves are finished.
			UsedLoopCount--;

			// Retrieve all child nodes.
			TArray<USoundNode*> AllChildNodes;
			GetAllNodes( AllChildNodes );

			// GetAllNodes includes current node so we have to start at Index 1.
			for( INT NodeIndex = 1; NodeIndex < AllChildNodes.Num(); NodeIndex++ )
			{
				// Reset all child nodes so they are initialized again.
				USoundNode* ChildNode = AllChildNodes( NodeIndex );
				UINT* Offset = AudioComponent->SoundNodeOffsetMap.Find( ChildNode );
				if( Offset )
				{
					UBOOL* bRequiresInitialization = ( UBOOL* )&AudioComponent->SoundNodeData( *Offset );
					*bRequiresInitialization = TRUE;
				}
			}
		
			// Reset wave instances that notified us of completion.
			ResetWaveInstances( AudioComponent );
		}
	}
}

/** 
 * Returns whether the sound is looping indefinitely or not.
 */
UBOOL USoundNodeLooping::IsLoopingIndefinitely( void )
{
	return( bLoopIndefinitely );
}

/** 
 * Returns the maximum duration of the owning sound cue correctly accounting for loop count
 */
FLOAT USoundNodeLooping::GetDuration( void )
{
	// Sounds that loop forever return a long time
	if( bLoopIndefinitely )
	{
		return( INDEFINITELY_LOOPING_DURATION );
	}

	// Get length of child node
	FLOAT ChildDuration = 0.0f;
	if( ChildNodes( 0 ) )
	{
		ChildDuration = ChildNodes( 0 )->GetDuration();
	}

	// Get the maximum number of time that we are going to want to loop it
	FLOAT MinLoopCount, MaxLoopCount;
	LoopCount.GetOutRange( MinLoopCount, MaxLoopCount );

	// Return these multiplied together after taking into account that a loop count of 0 means it still is 
	// being played at least once.
	return( ChildDuration * ( MaxLoopCount + 1 ) );
}

IMPLEMENT_CLASS(USoundNodeLooping);


/*-----------------------------------------------------------------------------
	USoundNodeOscillator implementation.
-----------------------------------------------------------------------------*/

void USoundNodeOscillator::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(FLOAT) + sizeof(FLOAT) + sizeof(FLOAT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedAmplitude );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedFrequency );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedOffset );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedCenter );

	if( *RequiresInitialization )
	{
		UsedAmplitude	= Amplitude.GetValue(0.f, AudioComponent);
		UsedFrequency	= Frequency.GetValue(0.f, AudioComponent);
		UsedOffset		= Offset.GetValue(0.f, AudioComponent);
		UsedCenter		= Center.GetValue(0.f, AudioComponent);

		*RequiresInitialization = 0;
	}

	const FLOAT ModulationFactor = UsedCenter + UsedAmplitude * appSin( UsedOffset + UsedFrequency * AudioComponent->PlaybackTime * PI );
	if( bModulateVolume )
	{
		AudioComponent->CurrentVolume *= ModulationFactor;
	}
	if( bModulatePitch )
	{
		AudioComponent->CurrentPitch *= ModulationFactor;
	}

	Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
}

IMPLEMENT_CLASS(USoundNodeOscillator);


/*-----------------------------------------------------------------------------
	USoundNodeModulator implementation.
-----------------------------------------------------------------------------*/

void USoundNodeModulator::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(FLOAT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedVolumeModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedPitchModulation );

	if( *RequiresInitialization )
	{
		UsedVolumeModulation		= VolumeModulation.GetValue(0.f, AudioComponent);
		UsedPitchModulation			= PitchModulation.GetValue(0.f, AudioComponent);
		*RequiresInitialization		= 0;
	}
	
	AudioComponent->CurrentVolume	*= UsedVolumeModulation;
	AudioComponent->CurrentPitch	*= UsedPitchModulation;

	Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
}

IMPLEMENT_CLASS(USoundNodeModulator);

/*-----------------------------------------------------------------------------
	USoundNodeModulatorContinuous implementation.
-----------------------------------------------------------------------------*/

void USoundNodeModulatorContinuous::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(FLOAT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedVolumeModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedPitchModulation );

	UsedVolumeModulation		= VolumeModulation.GetValue(AudioComponent->PlaybackTime, AudioComponent);
	UsedPitchModulation			= PitchModulation.GetValue(AudioComponent->PlaybackTime, AudioComponent);

	AudioComponent->CurrentVolume	*= UsedVolumeModulation;
	AudioComponent->CurrentPitch	*= UsedPitchModulation;

	Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
}

IMPLEMENT_CLASS(USoundNodeModulatorContinuous);


/*-----------------------------------------------------------------------------
	USoundNodeAmbient implementation.
-----------------------------------------------------------------------------*/

void USoundNodeAmbient::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedVolumeModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedPitchModulation );

	if( *RequiresInitialization )
	{
		UsedMinRadius = MinRadius.GetValue( 0.0f, AudioComponent );
		UsedMaxRadius = MaxRadius.GetValue( 0.0f, AudioComponent );
		UsedLPFMinRadius = LPFMinRadius.GetValue( 0.0f, AudioComponent );
		UsedLPFMaxRadius = LPFMaxRadius.GetValue( 0.0f, AudioComponent );
		UsedVolumeModulation = VolumeModulation.GetValue( 0.0f, AudioComponent );
		UsedPitchModulation = PitchModulation.GetValue( 0.0f, AudioComponent );
		*RequiresInitialization = 0;
	}

	if( bAttenuate )
	{
		const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
		if( Distance >= UsedMaxRadius )
		{
			AudioComponent->CurrentVolume = 0.0f;
		}
		else if( Distance > UsedMinRadius )
		{
			AudioComponent->CurrentVolume *= 1.0f - ( Distance - UsedMinRadius ) / ( UsedMaxRadius - UsedMinRadius );
		}
	}

	// Default to no low pass filter
	if( bAttenuateWithLowPassFilter )
	{
		const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
		if( Distance >= UsedLPFMaxRadius )
		{
			AudioComponent->CurrentHighFrequencyGain = 0.0f;
		}
		// UsedLPFMinRadius is the point at which to start applying the low pass filter
		else if( Distance > UsedLPFMinRadius )
		{
			AudioComponent->CurrentHighFrequencyGain *= 1.0f - ( ( Distance - UsedLPFMinRadius ) / ( UsedLPFMaxRadius - UsedLPFMinRadius ) );
		}
	}

	AudioComponent->CurrentUseSpatialization |= bSpatialize;
	AudioComponent->CurrentVolume *= UsedVolumeModulation;
	AudioComponent->CurrentPitch *= UsedPitchModulation;

	// Make sure we're getting notified when the sounds stops.
	AudioComponent->CurrentNotifyBufferFinishedHook = this;

	// Parse wave node.
	if( Wave )
	{
		const INT PreviousNum = WaveInstances.Num();
		Wave->ParseNodes( AudioDevice, this, 0, AudioComponent, WaveInstances );

		// Mark wave instance associated with this "Wave" to loop indefinitely.
		for( INT i=PreviousNum; i<WaveInstances.Num(); i++ )
		{
			FWaveInstance* WaveInstance = WaveInstances(i);
			WaveInstance->LoopingMode = LOOP_Forever;
		}
	}
}

void USoundNodeAmbient::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );
	if( Wave )
	{
		Wave->GetNodes( AudioComponent, SoundNodes );
	}
}

void USoundNodeAmbient::GetAllNodes( TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );
	if( Wave )
	{
		Wave->GetAllNodes( SoundNodes );
	}
}

/**
 * Notifies the sound node that a wave instance in its subtree has finished.
 *
 * @param WaveInstance	WaveInstance that was finished 
 */
void USoundNodeAmbient::NotifyWaveInstanceFinished( FWaveInstance* WaveInstance ) 
{
	// Mark wave instance associated with wave as not yet finished.
	WaveInstance->bIsStarted = TRUE;
	WaveInstance->bIsFinished = FALSE;
}

IMPLEMENT_CLASS( USoundNodeAmbient );


/*-----------------------------------------------------------------------------
	USoundNodeAmbientNonLoop implementation.
-----------------------------------------------------------------------------*/

void USoundNodeAmbientNonLoop::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( INT ) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedVolumeModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedPitchModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, NextSoundTime );
	DECLARE_SOUNDNODE_ELEMENT( INT, SlotIndex );

	if( *RequiresInitialization )
	{
		UsedMinRadius = MinRadius.GetValue( 0.0f, AudioComponent );
		UsedMaxRadius = MaxRadius.GetValue( 0.0f, AudioComponent );
		UsedLPFMinRadius = LPFMinRadius.GetValue( 0.0f, AudioComponent );
		UsedLPFMaxRadius = LPFMaxRadius.GetValue( 0.0f, AudioComponent );
		UsedVolumeModulation = VolumeModulation.GetValue( 0.0f, AudioComponent );
		UsedPitchModulation = PitchModulation.GetValue( 0.0f, AudioComponent );
		NextSoundTime = AudioComponent->PlaybackTime + DelayTime.GetValue( 0.0f, AudioComponent );

		SlotIndex = PickNextSlot();

		*RequiresInitialization = 0;
	}

	if( bAttenuate )
	{
		const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
		if( Distance >= UsedMaxRadius )
		{
			AudioComponent->CurrentVolume = 0.f;
		}
		else if( Distance > UsedMinRadius )
		{
			AudioComponent->CurrentVolume *= 1.f - (Distance - UsedMinRadius) / (UsedMaxRadius - UsedMinRadius);
		}
	}

	// Default to no low pass filter
	if( bAttenuateWithLowPassFilter )
	{
		const FLOAT Distance = FDist( AudioComponent->CurrentLocation, AudioComponent->Listener->Location );
		if( Distance >= UsedLPFMaxRadius )
		{
			AudioComponent->CurrentHighFrequencyGain = 0.0f;
		}
		// UsedLPFMinRadius is the point at which to start applying the low pass filter
		else if( Distance > UsedLPFMinRadius )
		{
			AudioComponent->CurrentHighFrequencyGain = 1.0f - ( ( Distance - UsedLPFMinRadius ) / ( UsedLPFMaxRadius - UsedLPFMinRadius ) );
		}
	}

	AudioComponent->CurrentUseSpatialization |= bSpatialize;
	AudioComponent->CurrentVolume *= UsedVolumeModulation;
	AudioComponent->CurrentPitch *= UsedPitchModulation;

	// Apply per-slot volume and pitch modifiers.
	if( SlotIndex < SoundSlots.Num() )
	{
		AudioComponent->CurrentVolume *= SoundSlots(SlotIndex).VolumeScale;
		AudioComponent->CurrentPitch *= SoundSlots(SlotIndex).PitchScale;
	}

	// Make sure we're getting notified when the sounds stops.
	AudioComponent->CurrentNotifyBufferFinishedHook = this;

	// Never finished.
	AudioComponent->bFinished = FALSE;

	// If we should currently be playing a sound, parse the current slot
	if(AudioComponent->PlaybackTime >= NextSoundTime)
	{
		// If slot index is in range and and a valid wave.
		if( SlotIndex < SoundSlots.Num() )
		{
			// If slot has a wave, start playing it
			if( SoundSlots(SlotIndex).Wave != NULL )
			{
				SoundSlots(SlotIndex).Wave->ParseNodes( AudioDevice, this, SlotIndex, AudioComponent, WaveInstances );
			}
			// If it doesn't - move the NextSoundTime on again and pick a different slot right now.
			else
			{
				NextSoundTime = AudioComponent->PlaybackTime + DelayTime.GetValue(0.f, AudioComponent);
				SlotIndex = PickNextSlot();
			}
		}
	}
}

void USoundNodeAmbientNonLoop::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );

	for(INT i=0; i<SoundSlots.Num(); i++)
	{
		if( SoundSlots(i).Wave )
		{
			SoundSlots(i).Wave->GetNodes( AudioComponent, SoundNodes );
		}
	}
}

void USoundNodeAmbientNonLoop::GetAllNodes( TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );

	for(INT i=0; i<SoundSlots.Num(); i++)
	{
		if( SoundSlots(i).Wave )
		{
			SoundSlots(i).Wave->GetAllNodes( SoundNodes );
		}
	}
}

/**
 * Notifies the sound node that a wave instance in its subtree has finished.
 *
 * @param WaveInstance	WaveInstance that was finished 
 */
void USoundNodeAmbientNonLoop::NotifyWaveInstanceFinished( FWaveInstance* WaveInstance ) 
{
	UAudioComponent* AudioComponent = WaveInstance->AudioComponent;
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( FLOAT ) + sizeof( INT ) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMinRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedLPFMaxRadius );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedVolumeModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, UsedPitchModulation );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, NextSoundTime );
	DECLARE_SOUNDNODE_ELEMENT( INT, SlotIndex );

	// Choose the various parameters again.
	UsedMinRadius = MinRadius.GetValue( 0.0f, AudioComponent );
	UsedMaxRadius = MaxRadius.GetValue( 0.0f, AudioComponent );
	UsedLPFMinRadius = LPFMinRadius.GetValue( 0.0f, AudioComponent );
	UsedLPFMaxRadius = LPFMaxRadius.GetValue( 0.0f, AudioComponent );
	UsedVolumeModulation = VolumeModulation.GetValue( 0.0f, AudioComponent );
	UsedPitchModulation	= PitchModulation.GetValue( 0.0f, AudioComponent );

	// Pick next time to play a sound
	NextSoundTime = AudioComponent->PlaybackTime + DelayTime.GetValue( 0.0f, AudioComponent );

	// Allow wave instance to be played again the next iteration.
	WaveInstance->bIsStarted = TRUE;
	WaveInstance->bIsFinished = FALSE;

	// Pick next slot to play
	SlotIndex = PickNextSlot();
}

INT USoundNodeAmbientNonLoop::PickNextSlot()
{
	// Handle case of empty array
	if(SoundSlots.Num() == 0)
	{
		return 0;
	}

	// First determine to sum total of all weights.
	FLOAT TotalWeight = 0.f;
	for(INT i=0; i<SoundSlots.Num(); i++)
	{
		TotalWeight += SoundSlots(i).Weight;
	}

	// The pick the weight we want.
	FLOAT ChosenWeight = appFrand() * TotalWeight;

	// Then count back through accumulating weights until we pass it.
	FLOAT TestWeight = 0.f;
	for(INT i=0; i<SoundSlots.Num(); i++)
	{
		TestWeight += SoundSlots(i).Weight;
		if(TestWeight >= ChosenWeight)
		{
			return i;
		}
	}

	// Handle edge case - use last slot.
	return SoundSlots.Num()-1;
}


IMPLEMENT_CLASS(USoundNodeAmbientNonLoop);

/*-----------------------------------------------------------------------------
	USoundNodeWaveParam implementation
-----------------------------------------------------------------------------*/

/** 
 * Gets the time in seconds of the associated sound data
 */	
FLOAT USoundNodeWaveParam::GetDuration( void )
{
	debugf( NAME_Warning, TEXT( "Set the duration of the owning sound cue. There is no way to calculate it from here." ) );
	return( 0.0f );
}

/** 
 * USoundNode interface.
 */
void USoundNodeWaveParam::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	USoundNodeWave* NewWave = NULL;
	AudioComponent->GetWaveParameter( WaveParameterName, NewWave );
	if( NewWave != NULL )
	{
		NewWave->ParseNodes( AudioDevice, this, -1, AudioComponent, WaveInstances );
	}
	else
	{
		// use the default node linked to us, if any
		Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
	}
}

/** 
 *
 */
void USoundNodeWaveParam::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	SoundNodes.AddItem( this );
	USoundNodeWave* NewWave = NULL;
	AudioComponent->GetWaveParameter( WaveParameterName, NewWave );
	if( NewWave != NULL )
	{
		NewWave->GetNodes( AudioComponent, SoundNodes );
	}
	else
	{
		// use the default node linked to us, if any
		Super::GetNodes( AudioComponent, SoundNodes );
	}
}

IMPLEMENT_CLASS(USoundNodeWaveParam);

/*-----------------------------------------------------------------------------
    USoundNodeRandom implementation.
-----------------------------------------------------------------------------*/

void USoundNodeRandom::FixWeightsArray()
{
	// If weights and children got out of sync, we fix it first.
	if(Weights.Num() < ChildNodes.Num())
	{
		Weights.AddZeroed( ChildNodes.Num() - Weights.Num() );
	}
	else if(Weights.Num() > ChildNodes.Num())
	{
		const INT NumToRemove = Weights.Num() - ChildNodes.Num();
		Weights.Remove( Weights.Num() - NumToRemove, NumToRemove );
	}
}

void USoundNodeRandom::FixHasBeenUsedArray()
{
	// If HasBeenUsed and children got out of sync, we fix it first.
	if(HasBeenUsed.Num() < ChildNodes.Num())
	{
		HasBeenUsed.AddZeroed( ChildNodes.Num() - HasBeenUsed.Num() );
	}
	else if(HasBeenUsed.Num() > ChildNodes.Num())
	{
		const INT NumToRemove = HasBeenUsed.Num() - ChildNodes.Num();
		HasBeenUsed.Remove( HasBeenUsed.Num() - NumToRemove, NumToRemove );
	}
}

void USoundNodeRandom::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	// A random sound node cannot use seamless looping as it might switch to another wave the next iteration.
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	if( bRandomizeWithoutReplacement == TRUE )
	{
		FixHasBeenUsedArray();  // for now prob need this until resave packages has occurred
	}

	// Pick a random child node and save the index.
	if( *RequiresInitialization )
	{
		NodeIndex		= 0;	
		FLOAT WeightSum = 0;

		// only calculate the weights that have not been used and use that set for the random choice
		for( INT i=0; i<Weights.Num(); i++ )
		{
			if( ( bRandomizeWithoutReplacement == TRUE ) 
				&& ( HasBeenUsed(i) != TRUE )
				)
			{
				WeightSum += Weights(i);
			}
			else if( bRandomizeWithoutReplacement == FALSE ) 
			{
				WeightSum += Weights(i);
			}
		}

		FLOAT Weight = appFrand() * WeightSum;
		for( INT i=0; i<ChildNodes.Num() && i<Weights.Num(); i++ )
		{

			if( ( bRandomizeWithoutReplacement == TRUE )
				&& ( Weights(i) >= Weight )
				&& ( HasBeenUsed(i) != TRUE )
				)
			{
				HasBeenUsed(i) = TRUE;
				NumRandomUsed++; // we played a sound so increment how many sounds we have played	

				NodeIndex = i;
				break;

			}
			else if( ( bRandomizeWithoutReplacement == FALSE )
				&& ( Weights(i) >= Weight )
				)
			{
				NodeIndex = i;
				break;
			}
			else
			{
				Weight -= Weights(i);
			}
		}

		*RequiresInitialization = 0;
	}

	// check to see if we have used up our random sounds
	if( ( bRandomizeWithoutReplacement == TRUE )
		&& ( HasBeenUsed.Num() > 0 )
		&& ( NumRandomUsed >= HasBeenUsed.Num()  )
		)
	{
		// reset all of the children nodes
		for( INT i = 0; i < HasBeenUsed.Num(); ++i )
		{
			HasBeenUsed(i) = FALSE;
		}

		// set the node that has JUST played to be TRUE so we don't repeat it
		HasBeenUsed(NodeIndex) = TRUE;
		NumRandomUsed = 1;
	}

	// "play" the sound node that was selected
	if( NodeIndex < ChildNodes.Num() && ChildNodes(NodeIndex) )
	{
		ChildNodes(NodeIndex)->ParseNodes( AudioDevice, this, NodeIndex, AudioComponent, WaveInstances );	
	}
}

void USoundNodeRandom::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	if( !*RequiresInitialization )
	{
		SoundNodes.AddItem( this );
		if( NodeIndex < ChildNodes.Num() && ChildNodes(NodeIndex) )
		{
			ChildNodes(NodeIndex)->GetNodes( AudioComponent, SoundNodes );	
		}
	}
}

/**
 * Called by the Sound Cue Editor for nodes which allow children.  The default behavior is to
 * attach a single connector. Derived classes can override to e.g. add multiple connectors.
 **/
void USoundNodeRandom::CreateStartingConnectors()
{
	// Random Sound Nodes default with two connectors.
	InsertChildNode( ChildNodes.Num() );
	InsertChildNode( ChildNodes.Num() );
}

void USoundNodeRandom::InsertChildNode( INT Index )
{
	FixWeightsArray();
	FixHasBeenUsedArray();

	check( Index >= 0 && Index <= Weights.Num() );
	check( ChildNodes.Num() == Weights.Num() );

	Weights.Insert( Index );
	Weights(Index) = 1.0f;

	HasBeenUsed.Insert( Index );
	HasBeenUsed( Index ) = FALSE;

	Super::InsertChildNode( Index );
}

void USoundNodeRandom::RemoveChildNode( INT Index )
{
	FixWeightsArray();
	FixHasBeenUsedArray();

	check( Index >= 0 && Index < Weights.Num() );
	check( ChildNodes.Num() == Weights.Num() );

	Weights.Remove( Index );
	HasBeenUsed.Remove( Index );

	Super::RemoveChildNode( Index );
}

IMPLEMENT_CLASS(USoundNodeRandom);


/*-----------------------------------------------------------------------------
         USoundNodeDelay implementation.
-----------------------------------------------------------------------------*/

/**
 * The SoundNodeDelay will delay randomly from min to max seconds.
 *
 * Once the delay has passed it will then tell all of its children nods
 * to go ahead and be "parsed" / "play"
 *
 **/
void USoundNodeDelay::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(FLOAT) + sizeof(FLOAT) );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, ActualDelay );
	DECLARE_SOUNDNODE_ELEMENT( FLOAT, StartOfDelay );

	// A delay node cannot use seamless looping as it introduces a delay.
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	// Check to see if this is the first time through.
	if( *RequiresInitialization )
	{
		ActualDelay	 = DelayDuration.GetValue(0.f, AudioComponent);
		StartOfDelay = GWorld->GetTimeSeconds();

		*RequiresInitialization = FALSE;
	}

	FLOAT TimeSpentWaiting = GWorld->GetTimeSeconds() - StartOfDelay;

	// If we have not waited long enough then just keep waiting.
	if( TimeSpentWaiting < ActualDelay )
	{
		// We're not finished even though we might not have any wave instances in flight.
		AudioComponent->bFinished = FALSE;
	}
	// Go ahead and play the sound.
	else
	{
		Super::ParseNodes( AudioDevice, Parent, ChildIndex, AudioComponent, WaveInstances );
	}
}

/**
 * Returns the maximum duration this sound node will play for. 
 * 
 * @return maximum duration this sound will play for
 */
FLOAT USoundNodeDelay::GetDuration( void )
{
	// Get length of child node.
	FLOAT ChildDuration = 0.f;
	if( ChildNodes(0) )
	{
		ChildDuration = ChildNodes(0)->GetDuration();
	}

	// Find out the maximum delay.
	FLOAT MinDelay, MaxDelay;
	DelayDuration.GetOutRange( MinDelay, MaxDelay );

	// And return the two together.
	return ChildDuration + MaxDelay;
}

IMPLEMENT_CLASS(USoundNodeDelay);


/*-----------------------------------------------------------------------------
         USoundNodeConcatenator implementation.
-----------------------------------------------------------------------------*/

/**
 * Returns whether the node is finished after having been notified of buffer
 * being finished.
 *
 * @param	AudioComponent	Audio component containing payload data
 * @return	TRUE if finished, FALSE otherwise.
 */
UBOOL USoundNodeConcatenator::IsFinished( UAudioComponent* AudioComponent ) 
{ 
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );
	check(*RequiresInitialization == 0 );

	return NodeIndex >= ChildNodes.Num() ? TRUE : FALSE; 
}

/**
 * Notifies the sound node that a wave instance in its subtree has finished.
 *
 * @param WaveInstance	WaveInstance that was finished 
 */
void USoundNodeConcatenator::NotifyWaveInstanceFinished( FWaveInstance* WaveInstance )
{
	UAudioComponent* AudioComponent = WaveInstance->AudioComponent;
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );
	check(*RequiresInitialization == 0 );
	
	// Allow wave instance to be played again the next iteration.
	WaveInstance->bIsStarted = TRUE;
	WaveInstance->bIsFinished = FALSE;

	// Advance index.
	NodeIndex++;
}

/**
 * Returns the maximum duration this sound node will play for. 
 * 
 * @return maximum duration this sound will play for
 */
FLOAT USoundNodeConcatenator::GetDuration()
{
	// Sum up length of child nodes.
	FLOAT Duration = 0.f;
	for( INT ChildNodeIndex=0; ChildNodeIndex<ChildNodes.Num(); ChildNodeIndex++ )
	{
		USoundNode* ChildNode = ChildNodes(ChildNodeIndex);
		if( ChildNode )
		{
			Duration += ChildNode->GetDuration();
		}
	}
	return Duration;
}

/**
 * Concatenators have two connectors by default.
 */
void USoundNodeConcatenator::CreateStartingConnectors()
{
	// Concatenators default to two two connectors.
	InsertChildNode( ChildNodes.Num() );
	InsertChildNode( ChildNodes.Num() );
}

/**
 * Overloaded to add an entry to InputVolume.
 */
void USoundNodeConcatenator::InsertChildNode( INT Index )
{
	Super::InsertChildNode( Index );
	InputVolume.Insert( Index );
	InputVolume( Index ) = 1.0f;
}

/**
 * Overloaded to remove an entry from InputVolume.
 */
void USoundNodeConcatenator::RemoveChildNode( INT Index )
{
	Super::RemoveChildNode( Index );
	InputVolume.Remove( Index );
}

void USoundNodeConcatenator::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	// Start from the beginning.
	if( *RequiresInitialization )
	{
		NodeIndex = 0;	
		*RequiresInitialization = FALSE;
	}

	// Play the current node.
	if( NodeIndex < ChildNodes.Num() )
	{
		// A concatenator cannot use seamless looping as it will switch to another wave once the current one is finished.
		AudioComponent->CurrentUseSeamlessLooping = FALSE;

		// Don't set notification hook for the last entry so hooks higher up the chain get called.
		if( NodeIndex < ChildNodes.Num()-1 )
		{
			AudioComponent->CurrentNotifyBufferFinishedHook = this;
		}

		// Play currently active node.
		USoundNode* ChildNode = ChildNodes(NodeIndex);
		if( ChildNode )
		{
			FAudioComponentSavedState SavedState;
			SavedState.Set( AudioComponent );
			AudioComponent->CurrentVolume *= InputVolume( NodeIndex );
			ChildNode->ParseNodes( AudioDevice, this, NodeIndex, AudioComponent, WaveInstances );
			SavedState.Restore( AudioComponent );
		}
	}
}

void USoundNodeConcatenator::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	if( !*RequiresInitialization )
	{
		SoundNodes.AddItem( this );
		if( NodeIndex < ChildNodes.Num() )
		{
			USoundNode* ChildNode = ChildNodes(NodeIndex);
			if( ChildNode )
			{
				ChildNode->GetNodes( AudioComponent, SoundNodes );	
			}
		}
	}
}

IMPLEMENT_CLASS(USoundNodeConcatenator);

/*-----------------------------------------------------------------------------
	USoundCue implementation.
-----------------------------------------------------------------------------*/

/** 
 * Calculate the maximum audible distance accounting for every node
 */
void USoundCue::CalculateMaxAudibleDistance( void )
{
}

UBOOL USoundCue::IsAudible( const FVector &SourceLocation, const FVector &ListenerLocation, AActor* SourceActor, INT& bIsOccluded, UBOOL bCheckOcclusion )
{
	//@fixme - naive implementation, needs to be optimized
	// for now, check max audible distance, and also if looping
	CalculateMaxAudibleDistance();

	// Account for any portals
	FVector ModifiedSourceLocation = GWorld->GetWorldInfo()->RemapLocationThroughPortals( SourceLocation, ListenerLocation );

	// Can't line check through portals
	if( bCheckOcclusion && ( ModifiedSourceLocation == SourceLocation ) )
	{
		// simple trace occlusion check - reduce max audible distance if occluded
		FCheckResult Hit( 1.0f );
		GWorld->SingleLineCheck( Hit, SourceActor, ListenerLocation, ModifiedSourceLocation, TRACE_World | TRACE_StopAtAnyHit );
		if( Hit.Time < 1.0f )
		{
			bIsOccluded = 1;
		}
		else
		{
			bIsOccluded = 0;
		}
	}

	return TRUE;
}

UBOOL USoundCue::IsAudibleSimple( FVector* Location )
{
	// No location means a range check is meaningless
	if( !Location )
	{
		return( TRUE );
	}

	// No audio device means no listeners to check against
	if( !GEngine || !GEngine->Client || !GEngine->Client->GetAudioDevice() )
	{
		return( TRUE );
	}

	// Listener position could change before long sounds finish
	if( GetCueDuration() > 1.0f )
	{
		return( TRUE );
	}

	// Calculate the MaxAudibleDistance from all the nodes
	CalculateMaxAudibleDistance();

	// Is this SourceActor within the MaxAudibleDistance of any of the listeners?
	UBOOL IsAudible = GEngine->Client->GetAudioDevice()->LocationIsAudible( *Location, WORLD_MAX );

	return( IsAudible );
}

FLOAT USoundCue::GetCueDuration( void )
{
	// Always recalc the duration when in the editor as it could change
	if( (GIsEditor && !GIsGame) || Duration == 0.0f )
	{
		if( FirstNode )
		{
			Duration = FirstNode->GetDuration();
		}
	}
	
	return( Duration );
}

IMPLEMENT_CLASS(USoundCue);

/*-----------------------------------------------------------------------------
	Parameter-based distributions
-----------------------------------------------------------------------------*/

UBOOL UDistributionFloatSoundParameter::GetParamValue(UObject* Data, FName ParamName, FLOAT& OutFloat)
{
	UBOOL bFoundParam = false;

	UAudioComponent* AudioComp = Cast<UAudioComponent>(Data);
	if(AudioComp)
	{
		bFoundParam = AudioComp->GetFloatParameter(ParameterName, OutFloat);
	}

	return bFoundParam;
}

IMPLEMENT_CLASS(UDistributionFloatSoundParameter);




/*-----------------------------------------------------------------------------
       USoundNodeMature implementation.
-----------------------------------------------------------------------------*/

INT USoundNodeMature::GetMaxChildNodes()
{
	// This node can have any number of children.
	return -1;
}

void USoundNodeMature::GetNodes( UAudioComponent* AudioComponent, TArray<USoundNode*>& SoundNodes )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof(INT) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	if( !*RequiresInitialization )
	{
		SoundNodes.AddItem( this );
		if( NodeIndex < ChildNodes.Num() && ChildNodes(NodeIndex) )
		{
			ChildNodes(NodeIndex)->GetNodes( AudioComponent, SoundNodes );	
		}
	}
}

/** 
 * This SoundNode uses UEngine::bAllowMatureLanguage to determine whether child nodes
 * that have USoundNodeWave::bMature=TRUE should be played.
 */
void USoundNodeMature::ParseNodes( UAudioDevice* AudioDevice, USoundNode* Parent, INT ChildIndex, UAudioComponent* AudioComponent, TArray<FWaveInstance*>& WaveInstances )
{
	RETRIEVE_SOUNDNODE_PAYLOAD( sizeof( INT ) );
	DECLARE_SOUNDNODE_ELEMENT( INT, NodeIndex );

	// A random sound node cannot use seamless looping as it might switch to another wave the next iteration.
	AudioComponent->CurrentUseSeamlessLooping = FALSE;

	// Pick a random child node and save the index.
	if( *RequiresInitialization )
	{
		*RequiresInitialization = 0;

		// Make a list of mature and non-mature child nodes.
		TArray<INT> MatureChildNodes;
		MatureChildNodes.Empty( ChildNodes.Num() );

		TArray<INT> NonMatureChildNodes;
		NonMatureChildNodes.Empty( ChildNodes.Num() );

		for( INT i = 0; i < ChildNodes.Num() ; ++i )
		{
			if( ChildNodes( i ) ) 
			{
				if( ChildNodes( i )->IsA( USoundNodeWave::StaticClass() ) )
				{
					USoundNodeWave* Wave = static_cast<USoundNodeWave*>( ChildNodes( i ) );
					if( Wave->bMature )
					{
						MatureChildNodes.AddItem( i );
					}
					else
					{
						NonMatureChildNodes.AddItem( i );
					}
				}
				else
				{
					debugf( NAME_Warning, TEXT( "SoundNodeMature(%s) has a child which is not a sound node wave" ), *GetPathName() );
				}
			}
		}

		// Select a child node.
		NodeIndex = -1;
		if( GEngine->bAllowMatureLanguage )
		{
			// If mature language is allowed, prefer a mature node.
			if( MatureChildNodes.Num() > 0 )
			{
				NodeIndex = MatureChildNodes( 0 );
			}
			else if( NonMatureChildNodes.Num() > 0 )
			{
				NodeIndex = NonMatureChildNodes( 0 );
			}
		}
		else
		{
			// If mature language is not allowed, prefer a non-mature node.
			if( NonMatureChildNodes.Num() > 0 )
			{
				NodeIndex = NonMatureChildNodes( 0 );
			}
			else
			{
				debugf( NAME_Warning, TEXT( "SoundNodeMature(%s): GEngine->bAllowMatureLanguage is FALSE, no non-mature child sound exists" ), *GetPathName() );
			}
		}
	}

	// "play" the sound node that was selected
	if( NodeIndex >= 0 && NodeIndex < ChildNodes.Num() && ChildNodes( NodeIndex ) )
	{
		ChildNodes( NodeIndex )->ParseNodes( AudioDevice, this, NodeIndex, AudioComponent, WaveInstances );	
	}
}

IMPLEMENT_CLASS( USoundNodeMature );

/**  Hash function. Needed to avoid UObject v FResource ambiguity due to multiple inheritance */
DWORD GetTypeHash( const class USoundNodeWave* A )
{
	return A ? A->GetIndex() : 0;
}

// end
