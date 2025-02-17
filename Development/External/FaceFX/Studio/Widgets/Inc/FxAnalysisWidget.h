//------------------------------------------------------------------------------
// A widget that uses the FaceFx Analysis Library to produce animation
// results.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnalysisWidget_H__
#define FxAnalysisWidget_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxWidget.h"
#include "FxConsoleVariable.h"
#include "FxAnimController.h"

#ifdef __UNREAL__
#pragma pack(push, 8)
#endif // __UNREAL__
	#include "FxAnalysis.h"
#ifdef __UNREAL__
#pragma pack(pop)
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

// Forward-declare the anim user data.
class FxAnimUserData;

class FxAnalysisWidget : public FxWidget
{
public:
	// Constructor.
	FxAnalysisWidget( FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxAnalysisWidget();

	// Sets the animation controller pointer.
	void SetAnimControllerPointer( FxAnimController* pAnimController );
	// Returns FxTrue if the analysis core is expired.  If there is no valid
	// analysis core, it is assumed to be expired and FxTrue is returned.
	FxBool IsExpired( void ) const;
	// Returns the licensee information from the analysis library.
	FxAnalysisLicenseeInfo GetLicenseeInfo( void ) const;

	// FxWidget message handlers.
	virtual void OnAppStartup( FxWidget* sender );
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages );
	virtual void OnCreateAnim( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax );
	virtual void OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart,
		FxReal rangeEnd, const FxWString& analysisText );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, 
		FxPhonWordList* phonemeList );
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );

	virtual void OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config );
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );

	// Callbacks for when the audio minimum and audio maximum console variables
	// are changed.
	static void OnAudioMinimumUpdated( FxConsoleVariable& cvar );
	static void OnAudioMaximumUpdated( FxConsoleVariable& cvar );

protected:
	// The animation controller.
	FxAnimController* _pAnimController;
	// The current actor.
	FxActor* _pActor;
	// The current animation.
	FxAnim* _pAnim;
	// The current phoneme / word list.
	FxPhonWordList* _pPhonemeList;
	// The current phoneme mapping.
	FxPhonemeMap* _pPhonemeMap;
	// The supported languages.
	FxArray<FxLanguageInfo> _supportedLanguages;

	// The minimum audio duration that can be analyzed.
	static FxReal _audioMinimum;
	// The maximum audio duration that can be analyzed.
	static FxReal _audioMaximum;
	
	// Parses the speech detection console variable and sets the appropriate state.
	void _setSpeechDetectionState();
	// Sets up the analysis parameters
	void _SetupAnalysis( FxAnimUserData* pUserData, 
						 FxAnalysisAudio& analysisAudio,
						 FxWString& analysisText,
						 FxAnalysisConfig& analysisConfig );
	// Post-process a phoneme list during reanalyze range.
	void _PostProcessPhonemeList( FxPhonWordList* pPhonList, 
								  FxReal audioEndTime );
};

} // namespace Face

} // namespace OC3Ent

#endif
