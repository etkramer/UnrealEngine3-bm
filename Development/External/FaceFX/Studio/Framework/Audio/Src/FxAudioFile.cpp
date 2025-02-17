//------------------------------------------------------------------------------
// An audio file.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAudioFile.h"
#include "FxAudio.h"

#ifdef __UNREAL__
	#include "UnrealEd.h"
#endif // __UNREAL__

#ifndef __UNREAL__
	#include "FxConsole.h"
	#include "sndfile.h"
	#pragma comment(lib, "libsndfile-1.lib")
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

#ifndef __UNREAL__
	void __FxAudioFile_print_libsndfile_log( SNDFILE* pFile )
	{
		FxChar logInfo[4096] = {0};
		sf_command(pFile, SFC_GET_LOG_INFO, logInfo, 4096);
		FxChar toPrint[5008] = "libsndfile: ";
		FxSize j = 12;
		for( FxSize i = 0; i < 4096; ++i )
		{
			toPrint[j++] = logInfo[i];
			if( logInfo[i] == '\n' )
			{
				toPrint[j] = '\0';
				FxMsg(toPrint);
				j = 12;
			}
		}
	}
#endif // __UNREAL__

FxAudioFile::FxAudioFile()
	: _filename("") 
    , _isValid(FxFalse)
	, _bitsPerSample(0)
	, _sampleRate(0)
	, _numChannels(0) 
	, _numSamples(0)
	, _pSampleBuffer(NULL)
{
}

FxAudioFile::FxAudioFile( const FxString& filename )
	: _filename(filename)
	, _isValid(FxFalse)
	, _bitsPerSample(0)
	, _sampleRate(0)
	, _numChannels(0) 
	, _numSamples(0)
	, _pSampleBuffer(NULL)
{
}

FxAudioFile::FxAudioFile( FxAudio* pAudio )
{
	if( pAudio )
	{
		_filename      = "";
		_isValid       = FxTrue;
		_bitsPerSample = pAudio->GetBitsPerSample();
		_sampleRate    = pAudio->GetSampleRate();
		_numChannels   = pAudio->GetNumChannels();
		_numSamples    = pAudio->GetNumSamples();
		FxRawSampleBuffer rawSampleBuffer = pAudio->ExportRawSampleBuffer();
		FxSize bufferSize = FxGetBufferSize(rawSampleBuffer);
		_pSampleBuffer = new FxByte[bufferSize];
		FxMemcpy(_pSampleBuffer, rawSampleBuffer.GetSamples(), bufferSize);
	}
}

FxAudioFile::~FxAudioFile()
{
	if( _pSampleBuffer )
	{
		delete [] _pSampleBuffer;
		_pSampleBuffer = NULL;
	}
}

void FxAudioFile::Startup( void )
{
	// Nothing to do here anymore, but it is reserved for future use.
}

void FxAudioFile::Shutdown( void )
{
	// Nothing to do here anymore, but it is reserved for future use.
}

FxString FxAudioFile::GetVersion( void )
{
#ifndef __UNREAL__
	FxChar version[128] = {0};
	sf_command(NULL, SFC_GET_LIB_VERSION, &version, 128);
	return FxString(version);
#else
	return FxString();
#endif // __UNREAL__
}

FxArray<FxString> FxAudioFile::GetSupportedExtensions( void )
{
	FxArray<FxString> exts;
#ifndef __UNREAL__
	SF_FORMAT_INFO info;
	int numFormats = 0;
	sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &numFormats, sizeof(int));
	// +1 for the extra aif.
	exts.Reserve(numFormats+1);
	for( int i = 0; i < numFormats; ++i )
	{
		info.format = i;
		sf_command(NULL, SFC_GET_FORMAT_MAJOR, &info, sizeof(info));
		exts.PushBack(info.extension);
		// libsndfile returns the aiff extention as aiff and not also as aif
		// which is also common.  So it is added here manually just after aiff.
		if( !FxStrcmp("aiff", info.extension) )
		{
			exts.PushBack("aif");
		}
	}
#endif // __UNREAL__
	return exts;
}

FxArray<FxString> FxAudioFile::GetDescriptions( void )
{
	FxArray<FxString> descs;
#ifndef __UNREAL__
	SF_FORMAT_INFO info;
	int numFormats = 0;
	sf_command(NULL, SFC_GET_FORMAT_MAJOR_COUNT, &numFormats, sizeof(int));
	// +1 for the extra aif.
	descs.Reserve(numFormats+1);
	for( int i = 0; i < numFormats; ++i )
	{
		info.format = i;
		sf_command(NULL, SFC_GET_FORMAT_MAJOR, &info, sizeof(info));
		descs.PushBack(info.name);
		// libsndfile returns the aiff extention as aiff and not also as aif
		// which is also common.  So it is added here manually just after aiff.
		if( FxStrstr(info.name, "AIFF") )
		{
			descs.PushBack(info.name);
		}
	}
#endif // __UNREAL__
	return descs;
}

FxBool FxAudioFile::Load( void )
{
#ifndef __UNREAL__
	SF_INFO  sfinfo;
	FxMemset(&sfinfo, 0, sizeof(sfinfo));

	// All .raw files are assumed to be 16-bit, 16-kHz, mono.
	if( "raw" == _filename.AfterLast('.').ToLower() )
	{
		sfinfo.channels   = 1;
		sfinfo.format     = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
		sfinfo.samplerate = 16000;
	}

	SNDFILE* pFile = sf_open(_filename.GetCstr(), SFM_READ, &sfinfo);
	if( pFile )
	{
		switch( sfinfo.format & SF_FORMAT_SUBMASK )
		{
/*
			case SF_FORMAT_PCM_S8:
			case SF_FORMAT_PCM_U8:
			case SF_FORMAT_ULAW:
			case SF_FORMAT_ALAW:
			case SF_FORMAT_DPCM_8:
				_bitsPerSample = 8;
				break;
*/
			case SF_FORMAT_PCM_16:
			case SF_FORMAT_DPCM_16:
				_bitsPerSample = 16;
				break;
			default:
				FxError("FxAudioFile::Load(): Unrecognized or unsupported SF_FORMAT_SUBMASK!");
				return FxFalse;
				break;
		}

		_sampleRate  = sfinfo.samplerate;

		if( _sampleRate < FxDefaultSampleRate )
		{
			FxError("FxAudioFile::Load(): Unsupported sample rate!");
			return FxFalse;
		}

		_numChannels = sfinfo.channels;
		_numSamples  = sfinfo.frames;
	
		FxSize numBytes = (_numSamples * _numChannels * _bitsPerSample) / FX_CHAR_BIT;

		_pSampleBuffer = new FxByte[numBytes];
		FxMemset(_pSampleBuffer, 0, numBytes);
		
		FxInt32 numFramesToRead = (8 == _bitsPerSample) ? (_numSamples / 2) : _numSamples;
		FxInt32 numReadFrames   = 0;
		numReadFrames = sf_readf_short(pFile, reinterpret_cast<FxInt16*>(_pSampleBuffer), numFramesToRead);
		FxAssert(numReadFrames == numFramesToRead);
		__FxAudioFile_print_libsndfile_log(pFile);
		sf_close(pFile);
		_isValid = FxTrue;
		return FxTrue;
	}
#endif // __UNREAL__
	return FxFalse;
}

FxBool FxAudioFile::Save( const FxString& filename )
{
#ifndef __UNREAL__
	if( _pSampleBuffer && 16 == _bitsPerSample )
	{
		SF_INFO  sfinfo;
		FxMemset(&sfinfo, 0, sizeof(sfinfo));

		sfinfo.channels   = _numChannels;
		sfinfo.samplerate = _sampleRate;

		// .wav, .aif (.aiff), and .raw are the only audio file types that 
		// FxAudioFile is designed to save.  In addition, only 16-bit file 
		// types are supported for saving.
		FxString ext = filename.AfterLast('.').ToLower();

		if( "raw" == ext )
		{
			// All .raw files must be saved as 16-bit, 16-kHz, mono. 
			if( 16000 != _sampleRate && 1 != _numChannels )
			{
				return FxFalse;
			}
			sfinfo.format = SF_FORMAT_RAW | SF_FORMAT_PCM_16;
		}
		else if( "wav" == ext )
		{
			sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		}
		else if( "aif" == ext || "aiff" == ext )
		{
			sfinfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
		}
		else
		{
			return FxFalse;
		}

		SNDFILE* pFile = sf_open(filename.GetCstr(), SFM_WRITE, &sfinfo);
		if( pFile )
		{
			sf_writef_short(pFile, reinterpret_cast<FxInt16*>(_pSampleBuffer), _numSamples);
			__FxAudioFile_print_libsndfile_log(pFile);
			sf_close(pFile);
			return FxTrue;
		}
	}
#endif // __UNREAL__
	return FxFalse;
}

const FxString& FxAudioFile::GetFilename( void ) const
{
	return _filename;
}

FxBool FxAudioFile::IsValid( void ) const
{
	return _isValid;
}

FxSize FxAudioFile::GetBitsPerSample( void ) const
{
	return _bitsPerSample;
}

FxSize FxAudioFile::GetSampleRate( void ) const
{
	return _sampleRate;
}

FxSize FxAudioFile::GetNumChannels( void ) const
{
	return _numChannels;
}

FxSize FxAudioFile::GetNumSamples( void ) const
{
	return _numSamples;
}

FxByte* FxAudioFile::GetSampleBuffer( void ) const
{
	return _pSampleBuffer;
}

void FxAudioFile::SetBitsPerSample( FxInt32 bitsPerSample )
{
	_bitsPerSample = bitsPerSample;
}

void FxAudioFile::SetSampleRate( FxInt32 sampleRate )
{
	_sampleRate = sampleRate;
}

void FxAudioFile::SetNumChannels( FxInt32 channelCount )
{
	_numChannels = channelCount;
}

void FxAudioFile::SetNumSamples( FxInt32 sampleCount )
{
	_numSamples = sampleCount;
}

void FxAudioFile::SetSampleBuffer( FxByte* pSampleBuffer )
{
	_pSampleBuffer = pSampleBuffer;
}

} // namespace Face

} // namespace OC3Ent
