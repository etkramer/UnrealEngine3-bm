//------------------------------------------------------------------------------
// User data structures for FxAnims.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxAnimUserData.h"
#include "FxConsole.h"

#ifdef IS_FACEFX_STUDIO
	#include "FxSessionProxy.h"
#endif // IS_FACEFX_STUDIO

#ifndef NO_FXCG
#include "FxCGConfigManager.h"
#endif // ifndef NO_FXCG

namespace OC3Ent
{

namespace Face
{

FxCurveOwnerData::FxCurveOwnerData( FxName curve, FxBool ownedByAnalysis )
	: curveName(curve)
	, isOwnedByAnalysis(ownedByAnalysis)
{
}

FxArchive& operator<<( FxArchive& arc, FxCurveOwnerData& obj )
{
	arc << obj.curveName << obj.isOwnedByAnalysis;
	return arc;
}

#define kCurrentFxAnimUserDataVersion 7

FX_IMPLEMENT_CLASS(FxAnimUserData,kCurrentFxAnimUserDataVersion,FxNamedObject);

FxAnimUserData::FxAnimUserData()
	: _isSafeToUnload(FxTrue)
	, _pAudio(NULL)
	, _pPhonemeWordList(NULL)
	, _shouldContainAnalysisResults(FxTrue)
	, _shouldContainSpeechGestures(FxTrue)
	, _randomSeed(0)
{
#ifdef IS_FACEFX_STUDIO
	FxArray<FxLanguageInfo> languages;
	FxSessionProxy::QueryLanguageInfo(languages);
	for( FxSize i = 0; i < languages.Length(); ++i )
	{
		if( languages[i].isDefault )
		{
			_language = languages[i].languageName;
			break;
		}
	}
	_coarticulationConfig = FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString();
	_gestureConfig = FxCGConfigManager::GetDefaultGestureConfigName().GetAsString();
#endif // IS_FACEFX_STUDIO
}

FxAnimUserData::FxAnimUserData( const FxAnimUserData& other )
	: Super(other)
	, FxLazyLoader(other)
	, _isSafeToUnload(FxTrue)
	, _pAudio(NULL)
	, _pPhonemeWordList(NULL)
	, _shouldContainAnalysisResults(FxTrue)
	, _shouldContainSpeechGestures(FxTrue)
	, _randomSeed(0)
{
	other.Load();
	_safeFreeLazyLoadedData();
	_animGroupName = other._animGroupName;
	_animName = other._animName;
	_isSafeToUnload = other._isSafeToUnload;
	_language = other._language;
	_coarticulationConfig = other._coarticulationConfig;
	_gestureConfig = other._gestureConfig;
	_audioPath = other._audioPath;
	// Deep copy the _pAudio.
	if( other._pAudio )
	{
		_pAudio = new FxAudio();
		*_pAudio = *other._pAudio;
		_isLoaded = FxTrue;
	}
	_analysisText   = other._analysisText;
	_shouldContainAnalysisResults = other._shouldContainAnalysisResults;
	_shouldContainSpeechGestures  = other._shouldContainSpeechGestures;
	_randomSeed     = other._randomSeed;
	_curveOwnerData = other._curveOwnerData;
	// Deep copy the _pPhonemeWordList.
	if( other._pPhonemeWordList )
	{
		_pPhonemeWordList = new FxPhonWordList();
		*_pPhonemeWordList = *other._pPhonemeWordList;
		_isLoaded = FxTrue;
	}
	if( _archive )
	{
		_archive->AddLazyLoader(this);
	}
}

FxAnimUserData& FxAnimUserData::operator=( const FxAnimUserData& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	FxLazyLoader::operator=(other);
	other.Load();
	_safeFreeLazyLoadedData();
	_animGroupName = other._animGroupName;
	_animName = other._animName;
	_isSafeToUnload = other._isSafeToUnload;
	_language = other._language;
	_coarticulationConfig = other._coarticulationConfig;
	_gestureConfig = other._gestureConfig;
	_audioPath = other._audioPath;
	// Deep copy the _pAudio.
	if( other._pAudio )
	{
		_pAudio = new FxAudio();
		*_pAudio = *other._pAudio;
		_isLoaded = FxTrue;
	}
	_analysisText   = other._analysisText;
	_shouldContainAnalysisResults = other._shouldContainAnalysisResults;
	_shouldContainSpeechGestures  = other._shouldContainSpeechGestures;
	_randomSeed     = other._randomSeed;
	_curveOwnerData = other._curveOwnerData;
	// Deep copy the _pPhonemeWordList.
	if( other._pPhonemeWordList )
	{
		_pPhonemeWordList = new FxPhonWordList();
		*_pPhonemeWordList = *other._pPhonemeWordList;
		_isLoaded = FxTrue;
	}
	if( _archive )
	{
		_archive->AddLazyLoader(this);
	}
	return *this;
}

FxAnimUserData::~FxAnimUserData()
{
	_safeFreeLazyLoadedData();
	if( _archive )
	{
		_archive->RemoveLazyLoader(this);
	}
}

void FxAnimUserData::
SetNames( const FxString& animGroupName, const FxString& animName )
{
	_animGroupName = FxName(animGroupName.GetData());
	_animName = FxName(animName.GetData());
}

void FxAnimUserData::
GetNames( FxString& animGroupName, FxString& animName )
{
	animGroupName = _animGroupName.GetAsString();
	animName = _animName.GetAsString();
}

FxAudio* FxAnimUserData::GetAudioPointer( void )
{
	Load();
	return _pAudio;
}

void FxAnimUserData::SetAudio( FxAudio* pAudio )
{
	if( _pAudio )
	{
		if( _archive )
		{
			_archive->ReplaceObject(NULL, _archive->FindObject(_pAudio));
		}
		delete _pAudio;
		_pAudio = NULL;
	}
	if( pAudio )
	{
		_pAudio = new FxAudio();
		*_pAudio = *pAudio;
		_isLoaded = FxTrue;
	}
}

FxWString FxAnimUserData::GetAnalysisText( void )
{
	Load();
	return _analysisText;
}

void FxAnimUserData::SetAnalysisText( const FxWString& analysisText )
{
	_analysisText = analysisText;
	_isLoaded = FxTrue;
}

FxPhonWordList* FxAnimUserData::GetPhonemeWordListPointer( void )
{
	Load();
	return _pPhonemeWordList;
}

void FxAnimUserData::SetPhonemeWordList( FxPhonWordList* pPhonemeWordList )
{
	if( _pPhonemeWordList )
	{
		if( _archive )
		{
			_archive->ReplaceObject(NULL, _archive->FindObject(_pPhonemeWordList));
		}
		delete _pPhonemeWordList;
		_pPhonemeWordList = NULL;
	}
	if( pPhonemeWordList )
	{
		_pPhonemeWordList = new FxPhonWordList();
		*_pPhonemeWordList = *pPhonemeWordList;
		_isLoaded = FxTrue;
	}
}

FxBool FxAnimUserData::ShouldContainAnalysisResults( void )
{
	Load();
	return _shouldContainAnalysisResults;
}

void FxAnimUserData::
SetShouldContainAnalysisResults( FxBool shouldContainAnalysisResults )
{
	_shouldContainAnalysisResults = shouldContainAnalysisResults;
	_isLoaded = FxTrue;
}

FxBool FxAnimUserData::ShouldContainSpeechGestures( void )
{
	Load();
	return _shouldContainSpeechGestures;
}

void  FxAnimUserData
::SetShouldContainSpeechGestures( FxBool shouldContainSpeechGestures )
{
	_shouldContainSpeechGestures = shouldContainSpeechGestures;
	_isLoaded = FxTrue;
}

FxInt32 FxAnimUserData::GetRandomSeed( void )
{
	Load();
	return _randomSeed;
}

void FxAnimUserData::SetRandomSeed( FxInt32 seed )
{
	_randomSeed = seed;
	_isLoaded = FxTrue;
}

FxBool FxAnimUserData::IsCurveOwnedByAnalysis( const FxName& name )
{
	for( FxSize i = 0; i < _curveOwnerData.Length(); ++i )
	{
		if( _curveOwnerData[i].curveName == name )
		{
			return _curveOwnerData[i].isOwnedByAnalysis;
		}
	}
	return FxFalse;
}

void FxAnimUserData::SetCurveOwnedByAnalysis( const FxName& name, FxBool ownedByAnalysis )
{
	for( FxSize i = 0; i < _curveOwnerData.Length(); ++i )
	{
		if( _curveOwnerData[i].curveName == name )
		{
			_curveOwnerData[i].isOwnedByAnalysis = ownedByAnalysis;
			return;
		}
	}
	_curveOwnerData.PushBack(FxCurveOwnerData(name, ownedByAnalysis));
}

void FxAnimUserData::ClearCurveOwnerInfo( void )
{
	_curveOwnerData.Clear();
}

const FxString& FxAnimUserData::GetLanguage( void ) const
{
	return _language;
}

void FxAnimUserData::SetLanguage( const FxString& language )
{
	_language = language;
}

const FxString& FxAnimUserData::GetCoarticulationConfig( void ) const
{
	return _coarticulationConfig;
}

void FxAnimUserData::SetCoarticulationConfig( const FxString& coarticulationConfig )
{
	_coarticulationConfig = coarticulationConfig;
}

const FxString& FxAnimUserData::GetGestureConfig( void ) const
{
	return _gestureConfig;
}

void FxAnimUserData::SetGestureConfig( const FxString& gestureConfig )
{
	_gestureConfig = gestureConfig;
}

const FxString& FxAnimUserData::GetAudioPath( void ) const
{
	return _audioPath;
}

void FxAnimUserData::SetAudioPath( const FxString& audioPath )
{
	_audioPath = audioPath;
}

void FxAnimUserData::SetIsSafeToUnload( FxBool isSafeToUnload )
{
	_isSafeToUnload = isSafeToUnload;
}

void FxAnimUserData::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	_version = arc.SerializeClassVersion("FxAnimUserData");
	
	if( arc.IsSaving() )
	{
		Load();
		// Normally-loaded objects.
		arc << _animGroupName;
		arc << _animName;
		arc << _curveOwnerData;
		arc << _shouldContainAnalysisResults;
		arc << _shouldContainSpeechGestures;
		arc << _randomSeed;
		arc << _language;
#ifndef NO_FXCG
		_coarticulationConfig = FxCGConfigManager::GetGenericCoarticulationConfigName(_coarticulationConfig.GetData()).GetAsString();
		_gestureConfig = FxCGConfigManager::GetGestureConfigName(_gestureConfig.GetData()).GetAsString();
#endif // ifndef NO_FXCG
		arc << _coarticulationConfig;
		arc << _gestureConfig;
		arc << _audioPath;
		// Placeholder.
		FxInt32 startPos = arc.Tell();
		arc << startPos;
		// Lazy-loaded objects.
#ifdef __UNREAL__
		// Never save digital audio into sessions when in Unreal.
		FxAudio* pTempAudio = _pAudio;
		_pAudio = NULL;
#endif
		arc << _pAudio << _analysisText << _pPhonemeWordList;
#ifdef __UNREAL__
		_pAudio = pTempAudio;
#endif
		FxInt32 endPos = arc.Tell();
		arc.Seek(startPos);
		// Starting with version 5 we write out an offset from the current 
		// position in the archive rather than an absolute offset from the 
		// beginning of the archive.  The sizeof(FxInt32) term here is to 
		// compensate for the fact that when loading we actually use the offset
		// after having read it from the archive (i.e. it is the offset from the
		// position in the archive after having written out the offset itself).
		// That means we need to subtract its size in bytes from the offset 
		// before actually writing it out.
		FxInt32 offset = endPos - startPos - sizeof(FxInt32);
		arc << offset;
		arc.Seek(endPos);
	}
	else
	{
		if( _version >= 6 )
		{
			arc << _animGroupName;
			arc << _animName;
		}
		else
		{
			FxString animGroupName;
			FxString animName;
			const FxString& fullName = _name.GetAsString();
			FxBool foundDot = FxFalse;
			for( FxSize i = 0; i < fullName.Length(); ++i )
			{
				if( fullName[i] == '.' )
				{
					foundDot = FxTrue;
				}
				else
				{
					if( !foundDot )
					{
						animGroupName = FxString::Concat(animGroupName, fullName[i]);
					}
					else
					{
						animName = FxString::Concat(animName, fullName[i]);
					}
				}
			}
			_animGroupName.SetName(animGroupName.GetData());
			_animName.SetName(animName.GetData());
		}
		// Load the normally-loaded objects.
		arc << _curveOwnerData;
		if( _version >= 1 )
		{
			if( _version >= 4 )
			{
				arc << _shouldContainAnalysisResults;
			}
			else
			{
				_shouldContainAnalysisResults = FxTrue;
			}
			if( _version >= 7 )
			{
				arc << _shouldContainSpeechGestures;
			}
			else
			{
				_shouldContainSpeechGestures = FxTrue;
			}
			arc << _randomSeed;
		}
		else
		{
			_shouldContainAnalysisResults = FxTrue;
			_shouldContainSpeechGestures  = FxTrue;
			_randomSeed = static_cast<FxInt32>(time(0));
		}
		if( _version >= 2 )
		{
			arc << _language;
#ifndef NO_FXCG
			// Update the language if required.  "USEnglish" and "UKEnglish" 
			// were updated to "American English" and "British English"
			// respectively, so if we are loading an old-style language name we
			// must update it to the new-style language name or the reanalyze
			// selection feature will not work properly.  "USEnglish" is
			// the only possible language that could need updating, therefore
			// it is the only one checked for.
			if( _language == "USEnglish" )
			{
				_language = "American English";
			}
#ifdef IS_FACEFX_STUDIO
			// This is to trap cases of loading sessions created in the LIPSinc
			// version with the Fonix version.  In that case, the language was
			// set to an empty string (in the LIPSinc version) so we query the
			// installed languages and update it to the default language.  This
			// is mostly for convenience of loading up customer content such 
			// that the reanalyze selection feature works properly.
			if( _language.Length() == 0 )
			{
				FxArray<FxLanguageInfo> languages;
				FxSessionProxy::QueryLanguageInfo(languages);
				for( FxSize i = 0; i < languages.Length(); ++i )
				{
					if( languages[i].isDefault )
					{
						_language = languages[i].languageName;
						break;
					}
				}
			}
#endif // IS_FACEFX_STUDIO
#endif // ifndef NO_FXCG
			arc << _coarticulationConfig;
			arc << _gestureConfig;
#ifndef NO_FXCG
			_coarticulationConfig = FxCGConfigManager::GetGenericCoarticulationConfigName(_coarticulationConfig.GetData()).GetAsString();
			_gestureConfig = FxCGConfigManager::GetGestureConfigName(_gestureConfig.GetData()).GetAsString();
#endif // ifndef NO_FXCG
		}
		else
		{
#ifdef IS_FACEFX_STUDIO
			FxArray<FxLanguageInfo> languages;
			FxSessionProxy::QueryLanguageInfo(languages);
			for( FxSize i = 0; i < languages.Length(); ++i )
			{
				if( languages[i].isDefault )
				{
					_language = languages[i].languageName;
					break;
				}
			}
			_coarticulationConfig = FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString();
			_gestureConfig = FxCGConfigManager::GetDefaultGestureConfigName().GetAsString();
#endif // IS_FACEFX_STUDIO
		}
		if( _version >= 4 )
		{
			arc << _audioPath;
		}
		// Load the placeholder.
		FxInt32 seekPos;
		arc << seekPos;

		if( arc.IsLinearLoading() )
		{
			// If we are linearly loading an archive with animation user data
			// it is *never* safe to unload the user data as it can never be
			// found and loaded again (i.e. it becomes perma-lost).
			_isSafeToUnload = FxFalse;
			arc << _pAudio;
			if( _version >= 3 )
			{
				arc << _analysisText;
			}
			else
			{
				FxString tempText;
				arc << tempText;
				_analysisText = FxWString(tempText.GetData());
			}
			arc << _pPhonemeWordList;
			_isLoaded = FxTrue;
		}
		else
		{
			arc.AddLazyLoader(this);
			if( _version < 5 )
			{
				// In versions prior to 5 the seekPos is an absolute offset in
				// bytes from the beginning of the archive.
				arc.Seek(seekPos);
			}
			else
			{
				// In version 5 and higher seekPos is an offset in bytes from
				// the current position (after reading seekPos) in the archive.
				arc.Seek(arc.Tell() + seekPos);
			}
		}
	}
}

void FxAnimUserData::Load( void ) const
{
	if( _isLoaded == FxFalse )
	{
		if( _archive )
		{
			//FxMsg( "Lazy-loading animation user data for %s...", GetNameAsCstr() );
			FxInt32 savedPos = _archive->Tell();
			_archive->Seek(_pos);
			*_archive << _pAudio;
			if( _version >= 3 )
			{
				*_archive << _analysisText;
			}
			else
			{
				FxString tempText;
				*_archive << tempText;
				_analysisText = FxWString(tempText.GetData());
			}
			*_archive << _pPhonemeWordList;
			_isLoaded = FxTrue;
			_archive->Seek(savedPos);
			//FxMsg( "done." );
		}
	}
}

void FxAnimUserData::Unload( void )
{
	if( _isLoaded == FxTrue )
	{
#ifdef __UNREAL__
		if( _pAudio )
		{
			delete _pAudio;
			_pAudio = NULL;
		}
#endif // __UNREAL__
		if( _isSafeToUnload && _archive )
		{
			//FxMsg( "Unloading animation user data for %s...", GetNameAsCstr() );
			_safeFreeLazyLoadedData();
			//FxMsg( "done." );
		}
	}
}

void FxAnimUserData::_safeFreeLazyLoadedData( void )
{
	if( _pAudio )
	{
		if( _archive )
		{
			_archive->ReplaceObject(NULL, _archive->FindObject(_pAudio));
		}
		delete _pAudio;
		_pAudio = NULL;
	}
	_analysisText.Clear();
	if( _pPhonemeWordList )
	{
		if( _archive )
		{
			_archive->ReplaceObject(NULL, _archive->FindObject(_pPhonemeWordList));
		}
		delete _pPhonemeWordList;
		_pPhonemeWordList = NULL;
	}
	_isLoaded = FxFalse;
}

} // namespace Face

} // namespace OC3Ent
