//------------------------------------------------------------------------------
// Abstract base class for widgets.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWidget_H__
#define FxWidget_H__

#include "stdwx.h"

#include "FxPlatform.h"
#include "FxName.h"
#include "FxArray.h"
#include "FxActor.h"
#include "FxAnim.h"
#include "FxPhonWordList.h"
#include "FxPhonemeMap.h"

// Forward declare the configuration class.
class WXDLLIMPEXP_BASE wxFileConfig;

namespace OC3Ent
{

namespace Face
{

enum FxActorDataChangedFlag
{
	ADCF_FaceGraphChanged       = 1,
	ADCF_CurvesChanged          = 2,
	ADCF_AnimationsChanged      = 4,
	ADCF_AnimationGroupsChanged = 8,
	ADCF_CamerasChanged			= 16,
	ADCF_WorkspacesChanged		= 32,
	ADCF_NodeGroupsChanged		= 64,
	ADCF_All = ADCF_FaceGraphChanged|ADCF_CurvesChanged|ADCF_AnimationsChanged|ADCF_AnimationGroupsChanged|ADCF_CamerasChanged|ADCF_WorkspacesChanged|ADCF_NodeGroupsChanged
};

// Forward declare FxWidgetMediator.
class FxWidgetMediator;

// Forward declare FxCoarticulationConfig.
class FxCoarticulationConfig;
// Forward declare FxGestureConfig.
class FxGestureConfig;

// A simple structure for containing the position of nodes in the face graph
// used during session serialization.
struct FxNodePositionInfo
{
	FxString nodeName;
	FxInt32  x;
	FxInt32  y;
};

// A simple structure for containing the languages supported by the current
// analysis DLL.
struct FxLanguageInfo
{
	FxInt32   languageCode;
	FxString  languageName;
	FxWString nativeLanguageName;
	FxBool    isDefault;
};

// A simple structure for containing the capabilities of the current render
// widget.
struct FxRenderWidgetCaps
{
	FxRenderWidgetCaps()
		: renderWidgetName("Undefined")
		, renderWidgetVersion(0)
		, supportsOffscreenRenderTargets(FxFalse)
		, supportsMultipleCameras(FxFalse)
		, supportsFixedAspectRatioCameras(FxFalse)
		, supportsBoneLockedCameras(FxFalse)
	{
	}

	// The name of the render widget.
	FxString renderWidgetName;
	// The version number of the render widget multiplied by 1000.
	FxSize renderWidgetVersion;
	// FxTrue if the render widget supports creating offscreen render targets
	// and viewports.
	FxBool supportsOffscreenRenderTargets;
	// FxTrue if the render widget supports multiple cameras.
	FxBool supportsMultipleCameras;
	// FxTrue if the render widget supports fixed aspect ratio cameras.
	FxBool supportsFixedAspectRatioCameras;
	// FxTrue if the render widget supports locking cameras to bones.
	FxBool supportsBoneLockedCameras;
};

// A widget.
class FxWidget
{
public:
	// Constructor.
	FxWidget( FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxWidget() = 0;

	// Called when the application is starting up.
	virtual void OnAppStartup( FxWidget* sender );
	// Called when the application is shutting down.
	virtual void OnAppShutdown( FxWidget* sender );
	// Called when an actor is creating.
	virtual void OnCreateActor( FxWidget* sender );
	// Called when an actor is loading.
	virtual void OnLoadActor( FxWidget* sender, const FxString& actorPath );
	// Called when an actor is closing.
	virtual void OnCloseActor( FxWidget* sender );
	// Called when an actor is saving.
	virtual void OnSaveActor( FxWidget* sender, const FxString& actorPath );
	// Called when the current actor has changed.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the current actor's internal data has changed.  Usually the
	// result of a Face Graph node being added or deleted or some data in
	// a Face Graph node being updated.
    virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags = ADCF_All );
	// Called when the actor name changes
	virtual void OnActorRenamed( FxWidget* sender );
	// Called to query the currently supported analysis languages.
	virtual void OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages );
	// Called in response to a request to create a new animation.
	virtual void OnCreateAnim( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	// Called in response to the audio minimum or maximum changing.
	virtual void OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax );
	// Called in response to a request to reanalyze a range of an animation.
	virtual void OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart, 
		FxReal rangeEnd, const FxWString& analysisText );
	// Called in response to a request to delete an animation.
	virtual void OnDeleteAnim( FxWidget* sender, const FxName& animGroupName,
		const FxName& animName );
	// Called in response to a request to delete an animation group.  Note that
	// the group must be empty in order for it to be deleted.
	virtual void OnDeleteAnimGroup( FxWidget* sender, const FxName& animGroupName );
	// Called when the current animation has changed.
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	// Called when the current animation's phoneme / word list has changed.
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, 
		FxPhonWordList* phonemeList );
	// Called when the phoneme mapping has changed.
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );
	// Called when the current animation curve selection has changed.
	virtual void OnAnimCurveSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selAnimCurveNames );
	// Called to layout the Face Graph.
	virtual void OnLayoutFaceGraph( FxWidget* sender );
	// Called when a node has been added to the Face Graph.  The node has already
	// been added when this message is sent.
	virtual void OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when a node is about to be removed from the Face Graph.  The node has
	// not been removed yet when this message is sent.
	virtual void OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when the current Face Graph node selection has changed.
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	// Called when the current Face Graph node group selection has changed.
	virtual void OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup );
	// Called when the current link selection has changed
	virtual void OnLinkSelChanged( FxWidget* sender, 
		const FxName& inputNode, const FxName& outputNode );
    // Called when the widget needs to refresh itself for some reason.
	virtual void OnRefresh( FxWidget* sender );
	// Called when the current time changes.
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	// Called to play an animation.
	virtual void OnPlayAnimation( FxWidget* sender, FxReal startTime = FxInvalidValue,
		FxReal endTime = FxInvalidValue, FxBool loop = FxFalse );
	// Called to stop an animation.
	virtual void OnStopAnimation( FxWidget* sender );
	// Called when an animation is stopped.
	virtual void OnAnimationStopped( FxWidget* sender );
	// Called when a widget should load/save its options.
	virtual void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );
	// Called to get/set options in the face graph.
	virtual void OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
		FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
		FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel );
	// Called when the widgets should recalculate their griding, if the have any.
	virtual void OnRecalculateGrid( FxWidget* sender );
	// Called to query the current render widget's capabilities.
	virtual void OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps );
	// Called to interact with the workspace edit widget.
	virtual void OnInteractEditWidget( FxWidget* sender, FxBool isQuery, FxBool& shouldSave );
	
	// Called when the generic coarticulation configuration has changed.
	virtual void OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config );
	// Called when the gestures configuration has changed.
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );

protected:
	// The mediator.
	FxWidgetMediator* _mediator;
};

// A widget mediator.
class FxWidgetMediator : public FxWidget
{
public:
	// Constructor.
	FxWidgetMediator();
	// Destructor.
	virtual ~FxWidgetMediator();

	// Adds a widget to the mediator.
	void AddWidget( FxWidget* widget );
	// Removes a widget from the mediator.
	void RemoveWidget( FxWidget* widget );

	// Called when the application is starting up.
	virtual void OnAppStartup( FxWidget* sender );
	// Called when the application is shutting down.
	virtual void OnAppShutdown( FxWidget* sender );
	// Called when an actor is creating.
	virtual void OnCreateActor( FxWidget* sender );
	// Called when an actor is loading.
	virtual void OnLoadActor( FxWidget* sender, const FxString& actorPath );
	// Called when an actor is closing.
	virtual void OnCloseActor( FxWidget* sender );
	// Called when an actor is saving.
	virtual void OnSaveActor( FxWidget* sender, const FxString& actorPath );
	// Called when the current actor has changed.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	// Called when the current actor's internal data has changed.  Usually the
	// result of a Face Graph node being added or deleted or some data in
	// a Face Graph node being updated.
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	// Called when the actor name changes
	virtual void OnActorRenamed( FxWidget* sender );
	// Called to query the currently supported analysis languages.
	virtual void OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages );
	// Called in response to a request to create a new animation.
	virtual void OnCreateAnim( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	// Called in response to the audio minimum or maximum changing.
	virtual void OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax );
	// Called in response to a request to reanalyze a range of an animation.
	virtual void OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart, 
		FxReal rangeEnd, const FxWString& analysisText );
	// Called in response to a request to delete an animation.
	virtual void OnDeleteAnim( FxWidget* sender, const FxName& animGroupName,
		const FxName& animName );
	// Called in response to a request to delete an animation group.  Note that
	// the group must be empty in order for it to be deleted.
	virtual void OnDeleteAnimGroup( FxWidget* sender, const FxName& animGroupName );
	// Called when the current animation has changed.
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	// Called when the current animation's phoneme / word list has changed.
	virtual void OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, 
		FxPhonWordList* phonemeList );
	// Called when the phoneme mapping has changed.
	virtual void OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap );
	// Called when the current animation curve selection has changed.
	virtual void OnAnimCurveSelChanged( FxWidget* sender, 
		const FxArray<FxName>& selAnimCurveNames );
	// Called to layout the Face Graph.
	virtual void OnLayoutFaceGraph( FxWidget* sender );
	// Called when a node has been added to the Face Graph.  The node has already
	// been added when this message is sent.
	virtual void OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when a node is about to be removed from the Face Graph.  The node has
	// not been removed yet when this message is sent.
	virtual void OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node );
	// Called when the current Face Graph node selection has changed.
	virtual void OnFaceGraphNodeSelChanged( FxWidget* sender,
		const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection = FxTrue );
	// Called when the current Face Graph node group selection has changed.
	virtual void OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup );
	// Called when the current link selection has changed
	virtual void OnLinkSelChanged( FxWidget* sender, 
		const FxName& inputNode, const FxName& outputNode );
	// Called when the widget needs to refresh itself for some reason.
	virtual void OnRefresh( FxWidget* sender );
	// Called when the current time changes.
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	// Called to play an animation.
	virtual void OnPlayAnimation( FxWidget* sender, FxReal startTime = FxInvalidValue,
		FxReal endTime = FxInvalidValue, FxBool loop = FxFalse );
	// Called to stop an animation.
	virtual void OnStopAnimation( FxWidget* sender );
	// Called when an animation is stopped.
	virtual void OnAnimationStopped( FxWidget* sender );
	// Called when a widget should load/save its options.
	virtual void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );
	// Called to get/set options in the face graph
	virtual void OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
		FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
		FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel );
	// Called when the widgets should recalculate their griding, if the have any.
	virtual void OnRecalculateGrid( FxWidget* sender );
	// Called to query the current render widget's capabilities.
	virtual void OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps );
	// Called to interact with the workspace edit widget.
	virtual void OnInteractEditWidget( FxWidget* sender, FxBool isQuery, FxBool& shouldSave );

	// Called when the generic coarticulation configuration has changed.
	virtual void OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config );
	// Called when the gestures configuration has changed.
	virtual void OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config );

protected:
	// The widgets in the mediator.
	FxArray<FxWidget*> _widgets;
};

} // namespace Face

} // namespace OC3Ent

#endif

