//------------------------------------------------------------------------------
// User data structures for FxAnims.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimUserData_H__
#define FxAnimUserData_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"
#include "FxLazyLoader.h"
#include "FxAudio.h"
#include "FxPhonWordList.h"

namespace OC3Ent
{

namespace Face
{

// A struct to pair a curve name with the state of the lock.
struct FxCurveOwnerData
{
	FxCurveOwnerData( FxName curve = FxName::NullName, FxBool ownedByAnalysis = FxFalse);

	FxName curveName;
	FxBool isOwnedByAnalysis;
};

// User data for an FxAnim is named (so it can be linked up with the
// correct FxAnim) and lazy loaded because it contains a memory-heavy
// FxAudio member.
class FxAnimUserData : public FxNamedObject, public FxLazyLoader
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnimUserData, FxNamedObject);
public:
	// Constructor.
	FxAnimUserData();
	// Copy constructor.
	FxAnimUserData( const FxAnimUserData& other );
	// Assignment operator.
	FxAnimUserData& operator=( const FxAnimUserData& other );
	// Destructor.
	~FxAnimUserData();

	// Sets the names of the animation group and animation the user data belongs
	// to.
	void SetNames( const FxString& animGroupName, const FxString& animName );
	// Gets the names of the animation group and animation the user data belongs
	// to as individual strings.
	void GetNames( FxString& animGroupName, FxString& animName );

	// Returns the digital audio pointer.
	FxAudio* GetAudioPointer( void );
	// Sets the digital audio pointer.  This makes a deep copy of the data, 
	// so the caller is responsible for deleting the passed in digital
	// audio pointer.
	void SetAudio( FxAudio* pAudio );

	// Returns a copy of the text used for analysis.  If no text was used,
	// an empty FxString is returned.
	FxWString GetAnalysisText( void );
	// Sets the text used for analysis.  If an empty FxString is passed in,
	// no text will be used during analysis.
	void SetAnalysisText( const FxWString& analysisText );

	// Returns the phoneme / word list pointer.
	FxPhonWordList* GetPhonemeWordListPointer( void );
	// Sets the phoneme / word list pointer.  This makes a deep copy of the 
	// data, so the caller is responsible for deleting the passed in
	// phoneme / word list pointer.
	void SetPhonemeWordList( FxPhonWordList* pPhonemeWordList );

	// Returns FxTrue if the animation should contain audio analysis results.
	FxBool ShouldContainAnalysisResults( void );
	// Sets whether the animation should contain audio analysis results.
	void SetShouldContainAnalysisResults( FxBool shouldContainAnalysisResults );

	// Returns FxTrue if the animation should contain speech gestures.
	FxBool ShouldContainSpeechGestures( void );
	// Sets whether the animation should contain speech gestures.
	void SetShouldContainSpeechGestures( FxBool shouldContainSpeechGestures );

	// Returns the random seed for the animation.
	FxInt32 GetRandomSeed( void );
	// Sets the random seed for the animation.
	void SetRandomSeed( FxInt32 seed );

	// Returns FxTrue if a curve is owned by the analysis, FxFalse if owned by the user.
	FxBool IsCurveOwnedByAnalysis( const FxName& name );
	// Sets whether a curve is owned by the analysis.
	void SetCurveOwnedByAnalysis( const FxName& name, FxBool ownedByAnalysis );
	// Clears the curve owner lookup table.
	void ClearCurveOwnerInfo( void );

	// Returns the language that the animation was analyzed with.
	const FxString& GetLanguage( void ) const;
	// Sets the language that the animation should be analyzed with.
	void SetLanguage( const FxString& language );
	
	// Returns the coarticulation configuration that the animation was created
	// with.
	const FxString& GetCoarticulationConfig( void ) const;
	// Sets the coarticulation configuration that the animation should be
	// created with.
	void SetCoarticulationConfig( const FxString& coarticulationConfig );
	// Returns the gesture configuration that the animation was created with.
	const FxString& GetGestureConfig( void ) const;
	// Sets the gesture configuration that the animation should be created with.
	void SetGestureConfig( const FxString& gestureConfig );

	// Returns the relative path to the audio file used to create the animation.
	const FxString& GetAudioPath( void ) const;
	// Sets the relative path to the audio file used to create the animation.
	void SetAudioPath( const FxString& audioPath );

	// Returns FxTrue if the animation user data is safe to unload.  Otherwise
	// FxFalse is returned and the animation user data should not be unloaded.
	//FxBool IsSafeToUnload( void ) const;
	// Sets whether or not it is safe to unload the animation user data.
	void SetIsSafeToUnload( FxBool isSafeToUnload );

	// Serialization.
	virtual void Serialize( FxArchive& arc );

	// Lazy loader interface.
	virtual void Load( void ) const;
	virtual void Unload( void );

protected:
	// The name of the animation group the user data belongs to.
	FxName _animGroupName;
	// The name of the animation the user data belongs to.
	FxName _animName;
	// FxTrue if the animation should contain audio analysis results.
	FxBool _shouldContainAnalysisResults;
	// FxTrue if the animation should contain speech gestures.
	FxBool _shouldContainSpeechGestures;
	// The random seed.
	FxInt32 _randomSeed;
	// FxFalse if the animation user data has become unlinked from the session
	// (via an animation delete command followed by an undo) and should not
	// be unloaded.
	FxBool _isSafeToUnload;
	// The language that the animation was analyzed with.
	FxString _language;
	// The coarticulation configuration that the animation was created with.
	FxString _coarticulationConfig;
	// The gesture configuration that the animation was created with.
	FxString _gestureConfig;
	// The relative path to the audio file used to create the animation.
	FxString _audioPath;
	// The raw audio.
	mutable FxAudio* _pAudio;
	// The text used to analyze the audio (if any).
	mutable FxWString _analysisText;
	// The phoneme / word list.
	mutable FxPhonWordList* _pPhonemeWordList;
	// The lookup table to see if a curve is owned by the analysis or the user.
	FxArray<FxCurveOwnerData> _curveOwnerData;

	// The version of the object (needed for lazy-loading)
	FxUInt16 _version;

	// Safely removes the lazy-loaded data from the animation user data.
	void _safeFreeLazyLoadedData( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
