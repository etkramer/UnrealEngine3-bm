//------------------------------------------------------------------------------
// A FaceFx Studio session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStudioSession_H__
#define FxStudioSession_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxActor.h"
#include "FxActorInstance.h"
#include "FxArray.h"
#include "FxWidget.h"
#include "FxAnimUserData.h"
#include "FxTimeManager.h"

#ifdef __UNREAL__
	class UFaceFXAsset;
	class UFaceFXAnimSet;
#endif

namespace OC3Ent
{

namespace Face
{

// Forward declare the animation controller.
class FxAnimController;
// Forward declare the console variable class.
class FxConsoleVariable;

// A session.
class FxStudioSession : public FxObject, public FxWidgetMediator
{
	// Declare the class.
	FX_DECLARE_CLASS(FxStudioSession, FxObject);
public:
	// Constructor.
	FxStudioSession();
	// Destructor.
	virtual ~FxStudioSession();

	// Exit the application.
	void Exit( void );
	// Refresh the application undo / redo menu.
	void RefreshUndoRedoMenu( void );

	// Checks to see if the actor or session need to be saved and if so prompts
	// the user to save.  If FxFalse is returned, the user pressed cancel on the
	// prompt dialog.  Otherwise the user pressed either yes or no.
	FxBool PromptToSaveChangedActor( void );
	// Sets the prompt to save actor flag.
	void SetPromptToSaveActor( FxBool promptToSaveActor );

	// Clears all data from the session.
	void Clear( void );

	// Sets the audio timer pointer from the main window.
	void SetAudioTimerPointer( void* pAudioTimer );

	// Creates a new actor.
	FxBool CreateActor( void );
	// Loads the actor specified by filename.
	FxBool LoadActor( const FxString& filename );
	// Closes the actor.
	FxBool CloseActor( void );
	// Saves the actor to filename.
	FxBool SaveActor( const FxString& filename );

	// Mounts an external animation set.
	FxBool MountAnimSet( const FxString& animSetPath );
	// Unmounts an external animation set.
	FxBool UnmountAnimSet( const FxName& animSetName );
	// Imports an external animation set.
	FxBool ImportAnimSet( const FxString& animSetPath );
	// Exports an external animation set.
	FxBool ExportAnimSet( const FxName& animSetName, const FxString& animSetPath );
	
	// Analyze audio from an entire directory structure and create animations.
	FxBool AnalyzeDirectory( const FxString& directory, const FxString& animGroup,
		const FxString& language, const FxString& coarticulationConfig,
		const FxString& gestureConfig, FxBool generateSpeechGestures, 
		FxBool overwrite, FxBool recurse, FxBool notext );
	// Analyze a single file and create an animation.  If optionalText is empty,
	// a .txt file corresponding to audioFile is used (if present).  Otherwise,
	// optionalText overrides any .txt file.
	FxBool AnalyzeFile( const FxString& audioFile, const FxWString& optionalText, 
		const FxString& animGroup, const FxString& animName, const FxString& language, 
		const FxString& coarticulationConfig, const FxString& gestureConfig,
		FxBool generateSpeechGestures, FxBool overwrite );
	// Analyze raw in-memory audio data and create an animation.
	FxBool AnalyzeRawAudio( FxAudio* pAudio, const FxWString& optionalText,
		const FxString& animGroupName, const FxString& animName, const FxString& language,
		const FxString& coarticulationConfig, const FxString& gestureConfig, 
		FxBool generateSpeechGestures, FxBool overwrite );

	// Returns the current actor for the session.
	FxActor* GetActor( void );
	// Returns the current actor instance for the session.
	FxActorInstance* GetActorInstance( void );
#ifdef __UNREAL__
	// Returns the current FaceFXAsset for the session.
	UFaceFXAsset* GetFaceFXAsset( void );
#endif
    // Returns the full file path for the actor.
	const FxString& GetActorFilePath( void ) const;

	// Returns the current animation controller for the session.
	FxAnimController* GetAnimController( void );

	// Returns the current phoneme map for the session.
	FxPhonemeMap* GetPhonemeMap( void );
	// Returns the current phonetic alphabet.
	FxPhoneticAlphabet GetPhoneticAlphabet( void );

	// Returns the forced dirty flag.
	FxBool IsForcedDirty( void ) const;
	// Sets the forced dirty flag.
	void SetIsForcedDirty( FxBool isForcedDirty );

	// Serialization.
	virtual void Serialize( FxArchive& arc );

	// FxWidget message handlers.
	virtual void OnAppStartup( FxWidget* sender );
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnActorRenamed( FxWidget* sender );
	virtual void OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages );
	virtual void OnCreateAnim( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax );
	virtual void OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart, 
		FxReal rangeEnd, const FxWString& analysisText );
	virtual void OnDeleteAnim( FxWidget* sender, const FxName& animGroupName,
		const FxName& animName );
	virtual void OnDeleteAnimGroup( FxWidget* sender, const FxName& animGroupName );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, 
		FxPhonWordList* phonemeList );
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );
	virtual void OnAnimCurveSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selAnimCurveNames );
	virtual void OnLayoutFaceGraph( FxWidget* sender );
	virtual void OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	virtual void OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	virtual void OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup );
	virtual void OnLinkSelChanged( FxWidget* sender,
		const FxName& inputNode, const FxName& outputNode );
	virtual void OnRefresh( FxWidget* sender );
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	virtual void OnPlayAnimation( FxWidget* sender, FxReal startTime = FxInvalidValue,
		FxReal endTime = FxInvalidValue, FxBool loop = FxFalse );
	virtual void OnStopAnimation( FxWidget* sender );
	virtual void OnAnimationStopped( FxWidget* sender );
	virtual void OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
		FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
		FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel );
	virtual void OnRecalculateGrid( FxWidget* sender );
	virtual void OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps );
	virtual void OnInteractEditWidget( FxWidget* sender, FxBool isQuery, FxBool& shouldSave );
	virtual void OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config );
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );

#ifdef __UNREAL__
	// Unreal-specific.
	// Sets the current actor for the session.  This also sets _noDeleteActor
    // to FxTrue, so the session will never delete the actor passed in.
	FxBool SetActor( FxActor*& pActor, UFaceFXAsset* pFaceFXAsset, 
		             UFaceFXAnimSet* pFaceFXAnimSet = NULL );
	FxBool SetActorInstance( FxActorInstance* pActorInstance );
#endif

	// The session proxy has access to all session data.
	friend class FxSessionProxy;
	// The animation set manager needs access to internal animation user data
	// functionality.
	friend class FxAnimSetManager;

protected:
	// FxTrue if message debugging is enabled.
	FxBool _debugWidgetMessages;
	// FxTrue if the prompt to save actor feature is enabled.
	FxBool _promptToSaveActor;
	// FxTrue if the session's actor pointer should never be deleted.  The 
	// session's actor pointer will be NULLed, however.
	FxBool _noDeleteActor;
	// FxTrue if the session's actor instance pointer should never be deleted.
	// The session's actor instance pointer will be NULLed, however.
	FxBool _noDeleteActorInstance;
	// The current actor.
	FxActor* _pActor;
	// The current actor instance.
	FxActorInstance* _pActorInstance;
	// Dummy registers (used specifically for the drawing in the curve widget so
	// as not to hose the real registers in the actor instance).
	FxArray<FxRegister> _dummyRegisters;
	// The current animation.
	FxAnim* _pAnim;
#ifdef __UNREAL__
	// Unreal-specific.
	UFaceFXAsset* _pFaceFXAsset;
	UFaceFXAnimSet* _pFaceFXAnimSet;
#endif
	// The actor's full file path.
	FxString _actorFilePath;
	// The animation controller.
	FxAnimController* _pAnimController;
	// The phoneme map for the specified actor.
	FxPhonemeMap* _pPhonemeMap;
	// The g_loopallfromhere console variable.
	FxConsoleVariable* g_loopallfromhere;

	// The animation user data list.
	FxArray<FxAnimUserData*> _animUserDataList;

	// Searches for the animation user data with the specified names and returns
	// a pointer to it or NULL if it does not exist.
	FxAnimUserData* _findAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Adds the animation user data.
	void _addAnimUserData( FxAnimUserData* pAnimUserData );
	// Searches for and removes the animation user data with the specified names.
	void _removeAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Searches for and removes orphaned animation user data objects.
	void _findAndRemoveOrphanedAnimUserData( void );

	// Checks the content for any anomalies.
	void _checkContent( void );
    
	// Issues widget messages to deselect everything in the application.
	void _deselectAll( void );

	// The currently selected time.
	FxReal _currentTime;
	// The current animation group name.
	FxName _selAnimGroupName;
	// The current animation name.
	FxName _selAnimName;
	// The list of currently selected animation curves.
	FxArray<FxName> _selAnimCurveNames;
	// The list of currently selected face graph nodes.
	FxArray<FxName> _selFaceGraphNodeNames;
	// The currently selected face graph node group.
	FxName _selNodeGroup;
	// The currently selected link.
	FxName _inputNode;
	FxName _outputNode;
	// The current times.
	FxReal _minTime;
	FxReal _maxTime;
	// The view rect and zoom level (for making sure the face graph stuff is
	// properly serialized even when in command line mode).
	FxInt32 _viewLeft;
	FxInt32 _viewRight;
	FxInt32 _viewTop;
	FxInt32 _viewBottom;
	FxReal  _zoomLevel;
	// FxTrue if the session was forced to be dirty.
	FxBool _isForcedDirty;
};

} // namespace Face

} // namespace OC3Ent

#endif
