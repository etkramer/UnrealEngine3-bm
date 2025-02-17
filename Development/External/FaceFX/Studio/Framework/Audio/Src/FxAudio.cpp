//------------------------------------------------------------------------------
// Manages audio data for analysis purposes.  Note that this is not a general
// purpose audio container.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAudio.h"
#include "FxMath.h"
#include "FxMemFnCallback.h"
#include "FxStudioApp.h"
#include "FxProgressDialog.h"

#ifdef __UNREAL__
	// In Unreal, the audio is not stored in the session files so there is no
// need to include the compression code.
#include "UnrealEd.h"
	#include "EngineSoundClasses.h"
#endif // __UNREAL__

#include "libresample.h"

#ifndef __UNREAL__
	#pragma comment(lib, "libresample.lib")
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

// Specialization of FxSampleBufferSerializationHelper() for FxRawSampleBuffer.
template<> void FX_CALL 
FxSampleBufferSerializationHelper( FxRawSampleBuffer& sampleBuffer, FxArchive& arc )
{
	if( arc.IsLoading() )
	{
		sampleBuffer._freeBuffer();
	}

	arc << sampleBuffer._numSamples;

	FxSize bufferSize = FxGetBufferSize(sampleBuffer);

	// Note: uLongf is a zlib type.
	if( arc.IsSaving() )
	{
#ifndef __UNREAL__
		// Compress the exported sample buffer.  zlib requires the initial
		// compressed buffer size to be at least 0.1% larger than the original
		// buffer size plus 12 bytes, so I chose 10% larger to be safe.
		FxSize compressedSize = static_cast<uLongf>(bufferSize + 12 + (0.1f * bufferSize));
		FxByte* compressedSampleBuffer = static_cast<FxByte*>(FxAlloc(sizeof(FxByte)*(compressedSize), "FxRawSampleBufferCompressed"));
		uLongf compressedSampleBufferSize = static_cast<uLongf>(compressedSize);
		compress2(compressedSampleBuffer, &compressedSampleBufferSize, sampleBuffer.GetSamples(), bufferSize, Z_BEST_COMPRESSION);
#else
		FxSize compressedSize = 0;
		FxByte* compressedSampleBuffer = static_cast<FxByte*>(FxAlloc(sizeof(FxByte)*(compressedSize), "FxRawSampleBufferCompressed"));
#endif // __UNREAL__

		// Write out the compressed size and the compressed buffer.
		arc << compressedSize;
		arc.SerializePODArray(compressedSampleBuffer, compressedSize);

		FxFree(compressedSampleBuffer, compressedSize);
		compressedSampleBuffer = NULL;
		compressedSize = 0;
	}
	else
	{
		// Read in the compressed size and the compressed buffer.
		FxSize compressedSize = 0;
		arc << compressedSize;
		FxByte* compressedSampleBuffer = static_cast<FxByte*>(FxAlloc(sizeof(FxByte)*(compressedSize), "FxRawSampleBufferCompressed"));
		arc.SerializePODArray(compressedSampleBuffer, compressedSize);

#ifndef __UNREAL__
		// Uncompress.
		sampleBuffer._buffer = static_cast<FxByte*>(FxAlloc(bufferSize, "FxRawSampleBuffer"));
		uLongf uncompressedSampleBufferSize = static_cast<uLongf>(bufferSize);
		uncompress(sampleBuffer._buffer, &uncompressedSampleBufferSize, compressedSampleBuffer, compressedSize);
#else
		sampleBuffer._numSamples = 0;
#endif // __UNREAL__
		FxFree(compressedSampleBuffer, compressedSize);
		compressedSampleBuffer = NULL;
		compressedSize = 0;
	}
}

#define kCurrentFxAudioVersion 1

FX_IMPLEMENT_CLASS(FxAudio,kCurrentFxAudioVersion,FxNamedObject);

FxAudio::FxAudio()
{
}

FxAudio::FxAudio( FxSize bitsPerSample, FxSize sampleRate, 
				  FxSize numChannels, FxSize numSamples,
				  const FxByte* rawSampleBuffer )
{
	if( GetBitsPerSample() == bitsPerSample &&
		sampleRate >= GetSampleRate()       &&
		numSamples > 0                      &&
		rawSampleBuffer )
	{
		_createSampleBuffer(rawSampleBuffer, numSamples, sampleRate, numChannels);
	}
}

FxAudio::FxAudio( const FxString& filename )
{
	FxAudioFile audioFile(filename);
	if( audioFile.Load() && audioFile.IsValid() )
	{
		if( GetBitsPerSample() == audioFile.GetBitsPerSample() &&
			audioFile.GetSampleRate() >= GetSampleRate()       &&
			audioFile.GetNumSamples() > 0                      &&
			audioFile.GetSampleBuffer() )
		{
			_createSampleBuffer(audioFile.GetSampleBuffer(), 
				                audioFile.GetNumSamples(), 
								audioFile.GetSampleRate(),
								audioFile.GetNumChannels());
		}
	}
}

FxAudio::FxAudio( const FxAudioFile& audioFile )
{
	if( audioFile.IsValid() )
	{
		if( GetBitsPerSample() == audioFile.GetBitsPerSample() &&
			audioFile.GetSampleRate() >= GetSampleRate()       &&
			audioFile.GetNumSamples() > 0                      &&
			audioFile.GetSampleBuffer() )
		{
			_createSampleBuffer(audioFile.GetSampleBuffer(), 
				                audioFile.GetNumSamples(), 
				                audioFile.GetSampleRate(),
				                audioFile.GetNumChannels());
		}
	}
}

FxAudio::FxAudio( const FxAudio& other )
	: Super(other)
	, FxDRealSampleBuffer(other)
{
}

FxAudio& FxAudio::operator=( const FxAudio& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	FxDRealSampleBuffer::operator=(other);
	return *this;
}

FxAudio::~FxAudio()
{
}

#ifdef __UNREAL__
/**
* Creates an FxAudio object from the passed in USoundNodeWave object.
*
* @param SoundNodeWave	Wave file to create FxAudio object from
* @return FxAudio object created from passed in SoundNodeWave or NULL if an error occurred
*/
FxAudio* FxAudio::CreateFromSoundNodeWave( USoundNodeWave* SoundNodeWave )
{
	FxAudio* AnalysisAudio = NULL;
	if( SoundNodeWave )
	{
		FWaveModInfo WaveInfo;
		if( WaveInfo.ReadWaveInfo((BYTE*)SoundNodeWave->RawData.Lock(LOCK_READ_ONLY),SoundNodeWave->RawData.GetBulkDataSize()) )
		{
			AnalysisAudio = new FxAudio(
				static_cast<FxSize>(*WaveInfo.pBitsPerSample), 
				static_cast<FxSize>(*WaveInfo.pSamplesPerSec),
				static_cast<FxSize>(*WaveInfo.pChannels), 
				static_cast<FxSize>(((WaveInfo.SampleDataSize * FX_CHAR_BIT) / *WaveInfo.pBitsPerSample)) / static_cast<FxSize>(*WaveInfo.pChannels), 
				static_cast<const FxByte*>(WaveInfo.SampleDataStart));
		}
		SoundNodeWave->RawData.Unlock();
	}
	return AnalysisAudio;
}
#endif

FxRawSampleBuffer FxAudio::ExportRawSampleBuffer( void ) const
{
	if( _buffer )
	{
		FxSize bufferSize = FxCalcRequiredBufferBytes(*this);
		FxByte* buffer = static_cast<FxByte*>(FxAlloc(bufferSize, "RawExportSampleBuffer"));
		if( buffer )
		{
			FxInt16* pWords = reinterpret_cast<FxInt16*>(buffer);

			for( FxSize i = 0; i < _numSamples; ++i )
			{
				pWords[i] = static_cast<FxInt16>(FxNearestInteger(FxClamp(static_cast<FxDReal>(SHRT_MIN), _buffer[i], static_cast<FxDReal>(SHRT_MAX))));
			}
			// Return value optimization.
			return (FxRawSampleBuffer(buffer, _numSamples));
		}
	}
	// Return value optimization.
	return (FxRawSampleBuffer(NULL, 0));
}

void FxAudio::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxAudio");

	// If loading a version 0 object, eat some data that is no longer needed.
	if( arc.IsLoading() && 0 == version )
	{
		FxBool   dummyIsValid       = FxFalse;
		FxUInt32 dummyBitsPerSample = 0;
		FxUInt32 dummySampleRate    = 0;
		FxUInt32 dummyChannelCount  = 0;

		arc << dummyIsValid << dummyBitsPerSample << dummySampleRate 
			<< dummyChannelCount;
	}

	FxRawSampleBuffer rawSampleBuffer = ExportRawSampleBuffer();
	rawSampleBuffer.SerializeSampleBuffer(arc);

	if( arc.IsLoading() )
	{
		_createSampleBuffer(rawSampleBuffer.GetSamples(), rawSampleBuffer.GetNumSamples(), GetSampleRate(), GetNumChannels());
	}
}

void FxAudio::_createSampleBuffer( const FxByte* buffer, FxSize numSamples, FxSize sampleRate, FxSize numChannels )
{
	if( numSamples > 0 && buffer )
	{
		FxDReal* realBuffer = static_cast<FxDReal*>(FxAlloc(sizeof(FxDReal)*(numSamples), "FxAudioSampleBuffer"));

		const FxInt16* pWords = reinterpret_cast<const FxInt16*>(buffer);

		for( FxUInt32 i = 0; i < numSamples; ++i )
		{
			// Only read one channel if there are multiple channels.  In stereo
			// audio this is the left channel.
			realBuffer[i] = pWords[i * numChannels];
		}
		// Set the sample buffer data.
		SetBuffer(realBuffer, numSamples);
		// Resample if required.
		_resample(sampleRate);
	}
}

void FxAudio::_resample( FxSize sampleRate )
{
	if( _buffer && _numSamples > 0 )
	{
		if( sampleRate > GetSampleRate() )
		{
			FxReal* sourceBuffer = static_cast<FxReal*>(FxAlloc(_numSamples*sizeof(FxReal), "FxAudioResampleSourceBufferTemp"));
			for( FxSize i = 0; i < _numSamples; ++i )
			{
				sourceBuffer[i] = _buffer[i];
			}

			FxReal ratio = static_cast<FxReal>(GetSampleRate()) / static_cast<FxReal>(sampleRate);

			FxInt32 outputBufferLength = (_numSamples * ratio + 1000);

			FxReal* outputBuffer = static_cast<FxReal*>(FxAlloc(outputBufferLength*sizeof(FxReal), "FxAudioResampleOutputBufferTemp"));
			FxMemset(outputBuffer, 0, outputBufferLength*sizeof(FxReal));

			FxDReal* doubleOutputBuffer = static_cast<FxDReal*>(FxAlloc(outputBufferLength*sizeof(FxDReal), "FxAudioResampleDoubleOutputBufferTemp"));
			FxMemset(doubleOutputBuffer, 0, outputBufferLength*sizeof(FxDReal));

			void* pHandle = resample_open(1, ratio, ratio);

			FxSize  sourceBufferPos        = 0;
			FxSize  numOutputSamples       = 0;
			FxInt32 blockSize              = 8192;
			FxInt32 processedSourceSamples = 0;

			// Increase progress callback for each old sample used.
			FxReal percentComplete = 0.0f;
			FxReal percentStepPerSample = 1.0f / static_cast<FxReal>(_numSamples);
			FxBool bIsComplete = FxFalse;

			FxProgressDialog progress(_("Resampling..."), FxStudioApp::GetMainWindow());
			FxFunctor* progressCallback = new FxProgressCallback<FxProgressDialog>(&progress, &FxProgressDialog::Update);

			while( sourceBufferPos < _numSamples )
			{
				if( sourceBufferPos + blockSize > _numSamples )
				{
					blockSize = _numSamples - sourceBufferPos;
				}

				processedSourceSamples = 0;

				FxInt32 numSamplesProduced =  resample_process(pHandle, 
															   ratio, 
															   sourceBuffer + sourceBufferPos, 
															   blockSize, 
															   (sourceBufferPos + blockSize >= _numSamples),
															   &processedSourceSamples, 
														       outputBuffer, 
															   outputBufferLength);

				sourceBufferPos += processedSourceSamples;

				for( FxInt32 i = 0; i < numSamplesProduced; ++i )
				{
					doubleOutputBuffer[i + numOutputSamples] = outputBuffer[i];
				}

				numOutputSamples += numSamplesProduced;

				percentComplete += percentStepPerSample * processedSourceSamples;
				if( progressCallback )
				{
					if( percentComplete >= 1.0f && !bIsComplete )
					{
						(*progressCallback)(percentComplete);
						bIsComplete = FxTrue;
					}
					else if( !bIsComplete )
					{
						(*progressCallback)(percentComplete);
					}
				}
			}

			FxFree(sourceBuffer, _numSamples*sizeof(FxReal));
			FxFree(outputBuffer, outputBufferLength*sizeof(FxReal));

			FxDReal* newBuffer = static_cast<FxDReal*>(FxAlloc(numOutputSamples*sizeof(FxDReal), "ResampledFxSampleBuffer"));
			FxMemcpy(newBuffer, doubleOutputBuffer, numOutputSamples*sizeof(FxDReal));

			FxFree(doubleOutputBuffer, outputBufferLength*sizeof(FxDReal));

			SetBuffer(newBuffer, numOutputSamples);

			resample_close(pHandle);

			if( progressCallback )
			{
				if( percentComplete < 1.0f && !bIsComplete )
				{
					(*progressCallback)(1.0f);
					bIsComplete = FxTrue;
				}
			}
			progress.Destroy();
		}
	}
}

// This is required for backwards compatibility reasons.
FxClass* FxDigitalAudio::_pClassDesc = NULL; 
FxClass* FX_CALL FxDigitalAudio::AllocateStaticClassFxDigitalAudio( void ) 
{ 
	return new FxClass("FxDigitalAudio",0,FxNamedObject::StaticClass(), 
		sizeof(FxDigitalAudio),&FxAudio::ConstructObject); 
}

} // namespace Face

} // namespace OC3Ent
