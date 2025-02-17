//------------------------------------------------------------------------------
// Manages audio data for analysis purposes.  Note that this is not a general
// purpose audio container.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudio_H__
#define FxAudio_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxNamedObject.h"
#include "FxArchive.h"
#include "FxAudioFile.h"

#ifndef __UNREAL__
	// FxSampleBuffer objects are compressed and uncompressed during 
	// serialization to save memory.
	#include "zlib.h"
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

// These constants define the required audio format for FxAudio.
static const FxSize FxDefaultNumChannels   = 1;
static const FxSize FxDefaultBitsPerSample = 16;
static const FxSize FxDefaultSampleRate    = 16000;

// A sample buffer.
template<typename SampleType,
         FxSize NumChannels   = FxDefaultNumChannels,
         FxSize BitsPerSample = FxDefaultBitsPerSample,
         FxSize SampleRate    = FxDefaultSampleRate> 
class FxSampleBuffer
{
public:
	typedef SampleType sample_type;

	// Constructors.
	FxSampleBuffer();
	// FxSampleBuffer assumes ownership of the buffer pointer 
	// and will free it internally (buffer must be created with FxAlloc()).
	explicit FxSampleBuffer( SampleType* buffer, FxSize numSamples );
	// Copy constructor.
	FxSampleBuffer( const FxSampleBuffer& other );
	// Assignment operator.
	FxSampleBuffer& operator=( const FxSampleBuffer& other );
	// Destructor.
	~FxSampleBuffer();

	// Sets the data in the sample buffer.
	// FxSampleBuffer assumes ownership of the buffer pointer 
	// and will free it internally (buffer must be created with FxAlloc()).
	void SetBuffer( SampleType* buffer, FxSize numSamples );

	// Returns a const pointer to the sample buffer.
	FX_INLINE const SampleType* GetSamples( void ) const { return _buffer; }
	// Returns the number of samples in the buffer.
	FX_INLINE FxSize GetNumSamples( void ) const { return _numSamples; }
	// Some helper functions to tell clients about the format of the data.
	FX_INLINE FxSize GetNumChannels( void ) const { return NumChannels; }
	FX_INLINE FxSize GetBitsPerSample( void ) const { return BitsPerSample; }
	FX_INLINE FxSize GetSampleRate( void ) const { return SampleRate; }
	// Returns the duration of the sample buffer in seconds.
	FX_INLINE FxDReal GetDuration( void ) const 
	{ 
		return static_cast<FxDReal>(GetNumSamples()) / 
			   static_cast<FxDReal>(GetSampleRate());
	}

	// Sample buffer serialization helper.  This is specialized in 
	// FxAudio.cpp only for FxRawSampleBuffer.  All other sample buffer types
	// will result in a compile time error and this is by design.  Note that it
	// is only safe to pass FxSampleBuffer objects to this function.
	template<class SampleBufferType> friend 
	void FX_CALL FxSampleBufferSerializationHelper( SampleBufferType& sampleBuffer, FxArchive& arc );
	// Serialization.
	void SerializeSampleBuffer( FxArchive& arc );

protected:
	SampleType* _buffer;
	FxSize      _numSamples;

	// Frees the data in the sample buffer.
	void _freeBuffer( void );
};

// A raw byte sample buffer.
typedef FxSampleBuffer<FxByte, FxDefaultNumChannels, FxDefaultBitsPerSample, FxDefaultSampleRate> FxRawSampleBuffer;

// A double precision floating point sample buffer.
class FxDRealSampleBuffer : public FxSampleBuffer<FxDReal, FxDefaultNumChannels, FxDefaultBitsPerSample, FxDefaultSampleRate>
{
public:
	// Returns the sample at index or 0.0 if there is no valid sample buffer.
	FX_INLINE FxDReal GetSample( FxSize index ) const
	{
		return _buffer ? _buffer[index] : 0.0;
	}
};

// Calculates the size in bytes required for the given sample buffer type.
template<class SampleBufferType>
FX_INLINE FxSize FX_CALL FxCalcRequiredBufferBytes( const SampleBufferType& sampleBuffer )
{
	return ((sampleBuffer.GetNumSamples() * sampleBuffer.GetBitsPerSample()) / FX_CHAR_BIT);
}

// Returns the size (in bytes) of the sample buffer.  This is specialized only 
// for FxRawSampleBuffer. 
template<class SampleBufferType> 
FxSize FX_CALL FxGetBufferSize( const SampleBufferType& sampleBuffer )
{
	return (sizeof(SampleBufferType::sample_type)*sampleBuffer.GetNumSamples());
}

// Specialization of FxGetBufferSize() for FxByte.
template<> 
FX_INLINE FxSize FX_CALL FxGetBufferSize( const FxRawSampleBuffer& sampleBuffer )
{
	return FxCalcRequiredBufferBytes(sampleBuffer);
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::FxSampleBuffer()
	: _buffer(NULL)
	, _numSamples(0)
{
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::FxSampleBuffer( SampleType* buffer, FxSize numSamples )
	: _buffer(buffer)
	, _numSamples(numSamples)
{
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::FxSampleBuffer( const FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>& other )
	: _buffer(NULL)
	, _numSamples(other._numSamples)
{
	FxSize bufferSize = FxGetBufferSize(*this);
	if( _numSamples > 0 && bufferSize > 0 )
	{
		_buffer = static_cast<SampleType*>(FxAlloc(bufferSize, "FxSampleBuffer"));
		if( _buffer && other._buffer )
		{
			FxMemcpy(_buffer, other._buffer, bufferSize);
		}
	}
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>& 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::operator=( const FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>& other )
{
	if( this == &other ) return *this;
	_freeBuffer();
	_numSamples = other._numSamples;
	FxSize bufferSize = FxGetBufferSize(*this);
	if( _numSamples > 0 && bufferSize > 0 )
	{
		_buffer = static_cast<SampleType*>(FxAlloc(bufferSize, "FxSampleBuffer"));
		if( _buffer && other._buffer )
		{
			FxMemcpy(_buffer, other._buffer, bufferSize);
		}
	}
	return *this;
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::~FxSampleBuffer()
{
	_freeBuffer();
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
void FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::SetBuffer( SampleType* buffer, FxSize numSamples )
{
	_freeBuffer();
	_buffer     = buffer;
	_numSamples = numSamples;
}

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
void FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::_freeBuffer( void )
{
	if( _buffer )
	{
		FxFree(_buffer, FxGetBufferSize(*this));
	}
	_buffer     = NULL;
	_numSamples = 0;
}

// The FxRawSampleBuffer specialization is declared here but actually defined 
// in FxAudio.cpp.
template<> extern
void FX_CALL FxSampleBufferSerializationHelper( FxRawSampleBuffer& sampleBuffer, FxArchive& arc );

template<typename SampleType, FxSize NumChannels, FxSize BitsPerSample, FxSize SampleRate> 
void FxSampleBuffer<SampleType, NumChannels, BitsPerSample, SampleRate>
::SerializeSampleBuffer( FxArchive& arc )
{
	FxSampleBufferSerializationHelper(*this, arc);
}

// Audio data for analysis.  FxAudio objects are named and can reside
// in FxArchive objects.
class FxAudio : public FxNamedObject, public FxDRealSampleBuffer
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAudio, FxNamedObject);
public:
	// Constructors.
	FxAudio();
	FxAudio( FxSize bitsPerSample, FxSize sampleRate, 
			 FxSize numChannels, FxSize numSamples,
		     const FxByte* rawSampleBuffer );
	FxAudio( const FxString& filename );
	FxAudio( const FxAudioFile& audioFile );
	// Copy constructor.
	FxAudio( const FxAudio& other );
	// Assignment operator.
	FxAudio& operator=( const FxAudio& other );
	// Destructor.
	virtual ~FxAudio();

#ifdef __UNREAL__
	/**
	* Creates an FxAudio object from the passed in USoundNodeWave object.
	*
	* @param SoundNodeWave	Wave file to create FxAudio object from
	* @return FxAudio object created from passed in SoundNodeWave or NULL if an error occurred
	*/
	static FxAudio* CreateFromSoundNodeWave( class USoundNodeWave* SoundNodeWave );
#endif

	// Export the raw sample buffer.
	// This routine will (possibly) produce a clipped sample buffer.
	FxRawSampleBuffer ExportRawSampleBuffer( void ) const;

	// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	void _createSampleBuffer( const FxByte* pSampleBuffer, FxSize numSamples, 
							  FxSize sampleRate, FxSize numChannels );
	void _resample( FxSize sampleRate );
};

// This is required for backwards compatibility reasons.
class FxDigitalAudio
{
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxDigitalAudio, FxNamedObject);
};

} // namespace Face

} // namespace OC3Ent

#endif
