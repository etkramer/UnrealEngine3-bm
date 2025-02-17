//------------------------------------------------------------------------------
// An audio file.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAudioFile_H__
#define FxAudioFile_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

class FxAudio;

class FxAudioFile
{
public:
	FxAudioFile();
	// Construct and load an audio file from disc.
    FxAudioFile( const FxString& filename );
	// Construct an audio file from an FxAudio object for saving to 
	// disc (for debugging only).
	FxAudioFile( FxAudio* pAudio );
	virtual ~FxAudioFile();

	static void Startup( void );
	static void Shutdown( void );
	static FxString GetVersion( void );
	static FxArray<FxString> GetSupportedExtensions( void );
	static FxArray<FxString> GetDescriptions( void );

	FxBool Load( void );
	FxBool Save( const FxString& filename );

	const FxString& GetFilename( void ) const;
	FxBool IsValid( void ) const;
	FxSize GetBitsPerSample( void ) const;
	FxSize GetSampleRate( void ) const;
	FxSize GetNumChannels( void ) const;
	FxSize GetNumSamples( void ) const;
	FxByte* GetSampleBuffer( void ) const;

	void SetBitsPerSample( FxInt32 bitsPerSample );
	void SetSampleRate( FxInt32 sampleRate );
	void SetNumChannels( FxInt32 numChannels );
	void SetNumSamples( FxInt32 numSamples );
	void SetSampleBuffer( FxByte* pSampleBuffer );

protected:
	FxString _filename;
	FxBool	 _isValid;
	FxSize   _bitsPerSample;
	FxSize	 _sampleRate; 
	FxSize	 _numChannels; 
	FxSize	 _numSamples;
    FxByte*  _pSampleBuffer;
};

} // namespace Face

} // namespace OC3Ent

#endif
