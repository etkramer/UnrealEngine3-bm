//------------------------------------------------------------------------------
// The session proxy gives internal and external commands a clean and 
// consistent interface to the current FaceFX Studio session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSessionProxy_H__
#define FxSessionProxy_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxActor.h"
#include "FxActorInstance.h"
#include "FxWidget.h"
#include "FxAudio.h"
#include "FxPhonWordList.h" //@todo Temporary!

#ifdef __UNREAL__
	class UFaceFXAsset;
	class UFaceFXAnimSet;
#endif

namespace OC3Ent
{

namespace Face
{

// The session proxy.
class FxSessionProxy
{
public:
//------------------------------------------------------------------------------
// Application functions.
//------------------------------------------------------------------------------
	static void ExitApplication( void );
	static void ToggleWidgetMessageDebugging( void );
	static void QueryLanguageInfo( FxArray<FxLanguageInfo>& languages );
	static void QueryRenderWidgetCaps( FxRenderWidgetCaps& renderWidgetCaps );

//------------------------------------------------------------------------------
// Actor functions.
//------------------------------------------------------------------------------
	// Create a new actor.
	static FxBool CreateActor( void );
	// Load an actor file.
	static FxBool LoadActor( const FxString& filename );
	// Close the actor.
	static FxBool CloseActor( void );
	// Save an actor file.
	static FxBool SaveActor( const FxString& filename );

	// Gets the current actor's name.
	static FxBool GetActorName( FxString& actorName );

	// Return a pointer to the session's current actor or NULL.
	static FxBool GetActor( FxActor** ppActor );
	// Return a pointer to the compiled face graph in the actor or NULL.
	static FxBool GetCompiledFaceGraph( FxCompiledFaceGraph** ppCG );
	// Return a pointer to the session's current actor instance or NULL.
	static FxBool GetActorInstance( FxActorInstance** ppActorInstance );
	// Sets the register value.
	static FxBool SetRegister( const FxName& regName, FxReal value, FxRegisterOp regOp );
	// Gets the register value.
	static FxBool GetRegister( const FxName& regName, FxReal& regValue );
	// Gets the original register value.
	static FxBool GetOriginalRegisterValue( const FxName& regName, FxReal& regValue );
	// Disables all registers.
	static FxBool DisableAllRegisters( void );
	// Updates the instance.
	static FxBool UpdateActorInstance( FxBool useDummyRegisters = FxFalse );
#ifdef __UNREAL__
	// Return a pointer to the session's current FaceFXAsset or NULL.
	static FxBool GetFaceFXAsset( UFaceFXAsset** ppFaceFXAsset );
#endif
	// Set whether or not actor changed messages are allowed to be sent.
	static FxBool SetNoActorChangedMessages( FxBool noActorChangedMessages );
	// Tell the session that the actor has changed in some way.
	static FxBool ActorDataChanged( FxActorDataChangedFlag flags = ADCF_All );

	// Sets isForcedDirty to FxTrue if the session has been forced to be dirty.
	static FxBool IsForcedDirty( FxBool& isForcedDirty );
	// Sets whether or not the session is forced to be dirty.
	static FxBool SetIsForcedDirty( FxBool isForcedDirty );

//------------------------------------------------------------------------------
// Time functions.
//------------------------------------------------------------------------------
	// Return the current session time.
	static FxBool GetSessionTime( FxReal& sessionTime );
	// Set the current session time.
	static FxBool SetSessionTime( FxReal newTime );
	// Set the current session frame.
	static FxBool SetSessionFrame( FxInt32 newFrame );

//------------------------------------------------------------------------------
// Selection functions.
//------------------------------------------------------------------------------
	// Return the currently selected animation group.
	static FxBool GetSelectedAnimGroup( FxString& animGroup );
	// Return the currently selected animation.
	static FxBool GetSelectedAnim( FxString& anim );
	// Return the list of currently selected animation curves.
	static FxBool GetSelectedAnimCurves( FxArray<FxString>& animCurves );
	// Return FxTrue if the specified curve is selected.
	static FxBool IsAnimCurveSelected( const FxString& curve );
	// Return the currently selected face graph node group.
	static FxBool GetSelectedFaceGraphNodeGroup( FxString& nodeGroup );
	// Return the list of currently selected face graph nodes.
	static FxBool GetSelectedFaceGraphNodes( FxArray<FxString>& nodes );
	// Return FxTrue if the specified node is selected.
	static FxBool IsFaceGraphNodeSelected( const FxString& node );
	// Return the currently selected face graph node link.
	static FxBool GetSelectedFaceGraphNodeLink( FxString& node1, FxString& node2 );
	// Set the currently selected animation group.
	static FxBool SetSelectedAnimGroup( const FxString& animGroup );
	// Set the currently selected animation.
	static FxBool SetSelectedAnim( const FxString& anim );
	// Set the list of currently selected animation curves.
	static FxBool SetSelectedAnimCurves( const FxArray<FxString>& animCurves );
	// Add the curve to the current curve selection.
	static FxBool AddAnimCurveToSelection( const FxString& curve );
	// Remove the curve from the current curve selection.
	static FxBool RemoveAnimCurveFromSelection( const FxString& curve );
	// Set the currently selected face graph node group.
	static FxBool SetSelectedFaceGraphNodeGroup( const FxString& nodeGroup );
	// Set the list of currently selected face graph nodes.
	static FxBool SetSelectedFaceGraphNodes( const FxArray<FxString>& nodes, FxBool zoomToSelection = FxTrue );
	// Add the node to the current node selection.
	static FxBool AddFaceGraphNodeToSelection( const FxString& node );
	// Remove the node from the current selection.
	static FxBool RemoveFaceGraphNodeFromSelection( const FxString& node );
	// Set the currently selected face graph node link.
	static FxBool SetSelectedFaceGraphNodeLink( const FxString& node1, const FxString& node2 );

//------------------------------------------------------------------------------
// Face graph functions.
//------------------------------------------------------------------------------
	// Layout the face graph.
	static FxBool LayoutFaceGraph( void );
	// Returns the position of the node named 'node'.
	static FxBool GetFaceGraphNodePos( const FxString& node, FxInt32& x, FxInt32& y );
	// Sets the position of the node named 'node'.
	static FxBool SetFaceGraphNodePos( const FxString& node, FxInt32 x, FxInt32 y );
	// Add a node to the face graph.
	static FxBool AddFaceGraphNode( FxFaceGraphNode* pNode );
	// Remove a node from the face graph.
	static FxBool RemoveFaceGraphNode( FxFaceGraphNode* pNode );
	// Return a pointer to the face graph node named 'node' or NULL.
	static FxBool GetFaceGraphNode( const FxString& node, FxFaceGraphNode** ppNode );
	// Set up a link from pFromNode to pToNode using the specified link function
	// and parameters.
	static FxBool AddFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
		FxFaceGraphNode* pToNode, const FxLinkFn* pLinkFn, 
		const FxLinkFnParameters& linkFnParams );
	// Returns the link between pFromNode and pToNode.
	static FxBool GetFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
		FxFaceGraphNode* pToNode, FxString& linkFn, 
		FxLinkFnParameters& linkFnParams );
	// Removes the link between pFromNode and pToNode.
	static FxBool RemoveFaceGraphNodeLink( FxFaceGraphNode* pFromNode, FxFaceGraphNode* pToNode );
	// Edits the link between pFromNode and pToNode.
	static FxBool EditFaceGraphNodeLink( FxFaceGraphNode* pFromNode, 
		FxFaceGraphNode* pToNode, const FxLinkFn* pLinkFn, 
		const FxLinkFnParameters& linkFnParams );
	// Sets a face graph node's name.
	static FxBool SetFaceGraphNodeName( FxName oldName, FxName newName );
	// Tell the session that the face graph has changed in some way.
	static FxBool FaceGraphChanged( void );
	// Compile the face graph.
	static FxBool CompileFaceGraph( void );

	// Returns a pointer to the actor's face graph.
	static FxBool GetFaceGraph( FxFaceGraph*& pFaceGraph );
	// Sets the actor's face graph.
	static FxBool SetFaceGraph( const FxFaceGraph& faceGraph );

//------------------------------------------------------------------------------
// Node group functions.
//------------------------------------------------------------------------------
	// Create a new node group, adding nodes if desired.
	static FxBool CreateNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes );
	// Remove a node group.
	static FxBool RemoveNodeGroup( const FxString& nodeGroup );
	
	// Return the list of nodes in a given group.
	static FxBool GetNodeGroup( const FxString& nodeGroup, FxArray<FxString>& nodes );
	// Set the list of nodes in a given group.
	static FxBool SetNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes );

	// Adds a list of nodes to a given group.
	static FxBool AddToNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes );
	// Removes a list of nodes from a given group.
	static FxBool RemoveFromNodeGroup( const FxString& nodeGroup, const FxArray<FxString>& nodes );

//------------------------------------------------------------------------------
// Phoneme list functions.
//------------------------------------------------------------------------------
	//@todo Temporary!
	static FxBool GetPhonWordList( FxPhonWordList* phonWordList );
	static FxBool SetPhonWordList( FxPhonWordList* newPhonWordList );

//------------------------------------------------------------------------------
// Mapping functions.
//------------------------------------------------------------------------------
	// Adds a track to the mapping.
	static FxBool AddTrackToMapping( const FxString& track );
	// Removes a track from the mapping.
	static FxBool RemoveTrackFromMapping( const FxString& track );
	// Checks if a track is in the mapping.
	static FxBool IsTrackInMapping( const FxString& track );
	// Gets the tracks contained in the mapping as an array of names.
	static FxBool GetTrackNamesInMapping( FxArray<FxName>& trackNames );

	// Gets the value of the mapping from phoneme to track.
	static FxBool GetMappingEntry( const FxString& phoneme, const FxString& track, FxReal& value );
	// Sets the value of the mapping from phoneme to track.
	static FxBool SetMappingEntry( const FxString& phoneme, const FxString& track, const FxReal& value );

	// Clears the mapping.
	static FxBool ClearMapping( void );
	// Clears the mapping for a phoneme.
	static FxBool ClearPhonemeMapping( const FxString& phoneme );
	// Clears the mapping for a track.
	static FxBool ClearTrackMapping( const FxString& track );

	// Returns the phoneme map.
	static FxBool GetPhonemeMap( FxPhonemeMap& phonemeMap );
	// Sets the phoneme map.
	static FxBool SetPhonemeMap( const FxPhonemeMap& phonemeMap );

	// Gets the mapping information for a phoneme.
	static FxBool GetPhonemeMapping( const FxString& phoneme, FxArray<FxString>& tracks, FxArray<FxReal>& values );
	// Gets the mapping information for a track.
	static FxBool GetTrackMapping( const FxString& track, FxArray<FxString>& phonemes, FxArray<FxReal>& values );

	// Starts a batch update for the mapping.
	static void BeginMappingBatch( void );
	// Ends a batch update for the mapping, triggering a refresh.
	static void EndMappingBatch( void );
	// Ends a batch update, without triggering a refresh.
	static void EndMappingBatchNoRefresh( void );

//------------------------------------------------------------------------------
// Animation group functions.
//------------------------------------------------------------------------------
	// Create a new animation group.
	static FxBool CreateAnimGroup( const FxString& animGroup );
	// Remove an animation group.
	static FxBool RemoveAnimGroup( const FxString& animGroup );

	// Return a pointer to the specified animation group or NULL.
	static FxBool GetAnimGroup( const FxString& animGroup, FxAnimGroup** ppAnimGroup );

//------------------------------------------------------------------------------
// Animation set functions.
//------------------------------------------------------------------------------
	// Mounts an external animation set.
	static FxBool MountAnimSet( const FxString& animSetPath );
	// Unmounts an external animation set.
	static FxBool UnmountAnimSet( const FxName& animSetName );
	// Imports an external animation set.
	static FxBool ImportAnimSet( const FxString& animSetPath );
	// Exports an external animation set.
	static FxBool ExportAnimSet( const FxName& animSetName, const FxString& animSetPath );

//------------------------------------------------------------------------------
// Animation functions.
//------------------------------------------------------------------------------
	// Add a new animation to the specified group.
	static FxBool AddAnim( const FxString& animGroup, const FxString& anim );
	// Add an existing animation to the specified group.
	static FxBool AddAnim( const FxString& animGroup, FxAnim& anim );
	// Removed the specified animation from the specified group.
	static FxBool RemoveAnim( const FxString& animGroup, const FxString& anim );

	// Return a pointer to the specified animation or NULL.
	static FxBool GetAnim( const FxString& animGroup, const FxString& anim, FxAnim** ppAnim );
	// Move the specified animation between groups.
	static FxBool MoveAnim( const FxString& fromGroup, const FxString& toGroup, const FxString& anim );
	// Copy the specified animation between groups.
	static FxBool CopyAnim( const FxString& fromGroup, const FxString& toGroup, const FxString& anim );

//------------------------------------------------------------------------------
// Playback functions.
//------------------------------------------------------------------------------
	static FxBool PlayAnimation( FxReal startTime = FxInvalidValue, 
		FxReal endTime = FxInvalidValue, FxBool loop = FxFalse );
	static FxBool StopAnimation( void );

//------------------------------------------------------------------------------
// Analysis functions.
//------------------------------------------------------------------------------
	// Analyze audio from an entire directory structure and create animations.
	static FxBool AnalyzeDirectory( const FxString& directory, const FxString& animGroupName,
		const FxString& language, const FxString& coarticulationConfig,
		const FxString& gestureConfig, FxBool generateSpeechGestures, 
		FxBool overwrite, FxBool recurse, FxBool notext );
	// Analyze a single file and create an animation.  If optionalText is empty,
	// a .txt file corresponding to audioFile is used (if present).  Otherwise,
	// optionalText overrides any .txt file.
	static FxBool AnalyzeFile( const FxString& audioFile, const FxWString& optionalText,
		const FxString& animGroupName, const FxString& animName, const FxString& language, 
		const FxString& coarticulationConfig, const FxString& gestureConfig, 
		FxBool generateSpeechGestures, FxBool overwrite );
	// Analyze raw in-memory audio data and create an animation.
	static FxBool AnalyzeRawAudio( FxAudio* pAudio, const FxWString& optionalText,
		const FxString& animGroupName, const FxString& animName, const FxString& language,
		const FxString& coarticulationConfig, const FxString& gestureConfig, 
		FxBool generateSpeechGestures, FxBool overwrite );
	// Change the audio minimum or maximum.
	static FxBool ChangeAudioMinMax( FxReal audioMin, FxReal audioMax );

//------------------------------------------------------------------------------
// Curve functions.
//------------------------------------------------------------------------------
	// Add a new curve to the specified animation.
	static FxBool AddCurve( const FxString& group, const FxString& anim, const FxString& curve, FxSize index = FxInvalidIndex );
	// Add an existing curve to the specified animation.
	static FxBool AddCurve( const FxString& group, const FxString& anim, FxAnimCurve& curve, FxSize index = FxInvalidIndex );
	// Removes a curve from the specified animation.
	static FxBool RemoveCurve( const FxString& group, const FxString& anim, const FxString& curve, FxSize& oldIndex );

	// Returns whether the curve is owned by the analysis or the user.
	static FxBool GetCurveOwner( const FxString& group, const FxString& anim, const FxString& curve, FxBool& isOwnedByAnalysis);
	// Sets whether the curve is owned by the analysis or the user.
	static FxBool SetCurveOwner( const FxString& group, const FxString& anim, const FxString& curve, FxBool isOwnedByAnalysis);
	// Returns the curve's interpolation algorithm.
	static FxBool GetCurveInterp( const FxString& group, const FxString& anim, const FxString& curve, FxInterpolationType& interpolator );
	// Sets the curve's interpolation algorithm.
	static FxBool SetCurveInterp( const FxString& group, const FxString& anim, const FxString& curve, FxInterpolationType interpolator );

	// Returns a pointer to the curve, or NULL.
	static FxBool GetCurve( const FxString& group, const FxString& anim, const FxString& curve, FxAnimCurve** ppCurve );

//------------------------------------------------------------------------------
// Key functions.
//------------------------------------------------------------------------------
	// Returns the selected keys as parallel arrays of curve/key indices.
	static FxBool GetSelectedKeys( FxArray<FxSize>& curveIndices, FxArray<FxSize>& keyIndices );
	// Returns the information for the selected keys as parallel arrays.
	static FxBool GetSelectedKeyInfos( FxArray<FxReal>& times, FxArray<FxReal>& values, FxArray<FxReal>& incDerivs, FxArray<FxReal>& outDerivs, FxArray<FxInt32>& flags);
	// Sets the selected keys from parallel arrays of curve/key indices.
	static FxBool SetSelectedKeys( const FxArray<FxSize>& curveIndices, const FxArray<FxSize>& keyIndices );
	// Clears the current key selection.
	static FxBool ClearSelectedKeys( void );
	// Applies a selection to the keys based on various criteria.
	static FxBool ApplyKeySelection( FxReal minTime, FxReal maxTime, FxReal minValue, FxReal maxValue, FxBool toggle );
	// Transforms the selected keys by a time or value delta.
	static FxBool TransformSelectedKeys( FxReal timeDelta, FxReal valueDelta );
	// Sets all selected keys's slopes to a specific value
	static FxBool SetSelectedKeysSlopes( FxReal slope );
	// Gets the pivot curve index and key index.
	static FxBool GetPivotKey( FxSize& curveIndex, FxSize& keyIndex );
	// Sets the pivot curve index and key index.
	static FxBool SetPivotKey( FxSize curveIndex, FxSize keyIndex );
	// Deletes the key selection.
	static FxBool RemoveKeySelection( void );
	// Inserts keys into curves.
	static FxBool InsertKeys( const FxArray<FxSize>& curveIndicies, const FxArray<FxReal>& times, 
		const FxArray<FxReal>& values, const FxArray<FxReal>& incDerivs, const FxArray<FxReal>& outDerivs, const FxArray<FxInt32>& flags );
	// Gets the information for a single key.
	static FxBool GetKeyInfo( FxSize curveIndex, FxSize keyIndex, FxReal& time, FxReal& value, FxReal& slopeIn, FxReal& slopeOut );
	// Edits the information for a single key.
	static FxBool EditKeyInfo( FxSize curveIndex, FxSize& keyIndex, FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut );
	// Inserts a single key and fills out the index where it was placed.
	static FxBool InsertKey( FxSize curveIndex, FxReal time, FxReal value, FxReal slopeIn, FxReal slopeOut, FxSize& keyIndex, FxBool clearSelection, FxBool autoSlope = FxFalse );
	// Removes a single key.
	static FxBool RemoveKey( FxSize curveIndex, FxSize keyIndex );
	// Inserts a key into all selected curves, or the single visible curve
	static FxBool DefaultInsertKey( FxReal time, FxReal value );

	// Disables sending refresh messages.
	static void SetNoRefreshMessages( FxBool state );
	// Tries to send a refresh message.
	static void RefreshControls( void );

//------------------------------------------------------------------------------
// Internal use only.
//------------------------------------------------------------------------------
	static void SetSession( void* pSession );
	static void GetSession( void** ppSession );
	static void SetNumDummyRegisters( FxSize numDummyRegisters );
	static void GetDummyRegister( const FxName& regName, FxReal& regValue );
	static void SetPhonemeList( void* pPhonemeList );
	static void RefreshUndoRedoMenu( void );
	static void PreRemoveAnim( const FxString& animGroup, const FxString& anim );
};

} // namespace Face

} // namespace OC3Ent

#endif
