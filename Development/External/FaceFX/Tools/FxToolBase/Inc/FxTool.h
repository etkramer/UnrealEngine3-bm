//------------------------------------------------------------------------------
// The interface for all FaceFx plug-ins.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxTool_H__
#define FxTool_H__

#include "FxPlatform.h"
#include "FxVersionInfo.h"
#include "FxActor.h"

namespace OC3Ent
{

namespace Face
{

// The interface for any FaceFx plug-ins.
class FxTool : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxTool, FxObject);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxTool);
public:
	// Constructor.
	FxTool();
	// Destructor.
	virtual ~FxTool();

//------------------------------------------------------------------------------
// Initialization and shutdown.
//------------------------------------------------------------------------------
	
	// Starts up the tool.  If this is overloaded, make sure to call the
	// base class version first.
	virtual FxBool Startup( void );
	// Shuts down the tool.  If this is overloaded, make sure to call the
	// base class version first.
	virtual FxBool Shutdown( void );

//------------------------------------------------------------------------------
// Actor level operations.
//------------------------------------------------------------------------------
	
	// Returns the name of the actor or an empty string if there is no current
	// actor.
	FxString GetActorName( void );
	// Creates a new actor with the specified name.
	virtual FxBool CreateActor( const FxString& name );
	// Loads an actor.
	virtual FxBool LoadActor( const FxString& filename );
	// Saves an actor.
	virtual FxBool SaveActor( const FxString& filename );

//------------------------------------------------------------------------------
// Bone level operations.
//------------------------------------------------------------------------------

	// Imports the reference pose to the specified frame.
	virtual FxBool ImportRefPose( FxInt32 frame ) = 0;
	// Exports the reference pose from the specified frame containing the 
	// specified bones.
	virtual FxBool ExportRefPose( FxInt32 frame, const FxArray<FxString>& bones ) = 0;

	// Imports specified bone pose to the specified frame.
	virtual FxBool ImportBonePose( const FxString& name, FxInt32 frame ) = 0;
	// Exports a bone pose from a specified frame with the specified name.
	virtual FxBool ExportBonePose( FxInt32 frame, const FxString& name ) = 0;
	// Batch imports bone poses using the specified configuration file.
	virtual FxBool BatchImportBonePoses( const FxString& filename, 
		FxInt32& startFrame, FxInt32& endFrame, FxBool onlyFindFrameRange = FxFalse );
	// Batch exports bone poses using the specified configuration file.
	virtual FxBool BatchExportBonePoses( const FxString& filename );

//------------------------------------------------------------------------------
// Animation level operations.
//------------------------------------------------------------------------------

	// Import the specified animation at the specified frame rate.
	virtual FxBool ImportAnim( const FxString& group, const FxString& name, 
		FxReal fps ) = 0;

//------------------------------------------------------------------------------
// Get and Set operations
//------------------------------------------------------------------------------
	// Set the _shouldDisplayWarningDialogs variable.
	void SetShouldDisplayWarningDialogs( FxBool shouldDisplayWarningDialogs );
	// Get the _shouldDisplayWarningDialogs variable.
	FxBool GetShouldDisplayWarningDialogs( void ) const;

//------------------------------------------------------------------------------
// Query operations.
//------------------------------------------------------------------------------

	// Returns the number of animation groups in the current actor or zero if
	// there is no current actor.
	FxSize GetNumAnimGroups( void );
	// Returns the name of the group at index or an empty string if there is
	// no current actor.
	FxString GetAnimGroupName( FxSize index );
	// Returns the number of animations in the specified group or zero if there
	// is no current actor or the group does not exist.
	FxSize GetNumAnims( const FxString& group );
	// Returns the name of the animation at index in the specified group or an
	// empty string if there is no current actor or the group does not exist.
	FxString GetAnimName( const FxString& group, FxSize index );
	// Returns the number of animation curves in the specified animation in the
	// specified group or zero if there is no current actor or the group (or
	// animation) does not exist.
	FxSize GetNumAnimCurves( const FxString& group, const FxString& anim );
	// Returns the name of the animation curve at index in the specified 
	// animation in the specified group or an empty string if there is no
	// current actor or the group (or animation) does not exist.
	FxString GetAnimCurveName( const FxString& group, const FxString& anim,
		FxSize index );
	// Returns an array of strings containing the names of each face graph node
	// of the specified type in the face graph or an empty array if there is no
	// current actor or no nodes of the specified type exist in the face graph.
	// An empty string array is also returned if type is a non-recognized FaceFx
	// type.
	FxArray<FxString> GetNodesOfType( const FxString& type );
	// Returns the number of reference bones in the current actor or zero if
	// there is no current actor.
	FxSize GetNumRefBones( void );
	// Returns the name of the reference bone at index or an empty string if 
	// there is no current actor.
	FxString GetRefBoneName( FxSize index );
	// Returns the number of bones in the specified pose in the current actor
	// or zero if there is no current actor or the specified pose does not
	// exist.
	FxSize GetNumBones( const FxString& pose );
	// Returns the name of the bone at index in the specified pose or an empty
	// string if there is no current actor or the specified pose does not exist.
	FxString GetBoneName( const FxString& pose, FxSize index );
	// Returns an array of key values for the specified curve in the specified
	// group and animation.  Most users will want to get "baked" curves instead 
	// to constuct a curve that accurately reflects a node's value over time.  
	// The function returns an empty array if the curve can't be found.
	FxArray<FxReal> GetRawCurveKeyValues( const FxString& group, const FxString& anim, const FxString& curve );
	// Returns an array of key times for the specified curve in the specified
	// group and animation.  Most users will want to get "baked" curves instead 
	// to constuct a curve that accurately reflects a node's value over time.  
	// The function returns an empty array if the curve can't be found.
	FxArray<FxReal> GetRawCurveKeyTimes( const FxString& group, const FxString& anim, const FxString& curve );
	// Returns an array of key input slopes for the specified curve in the 
	// specified group and animation.  Most users will want to get "baked" curves
	// instead to constuct a curve that accurately reflects a node's value over 
	// time.  Baked curves always have input and output slopes of 0.  The 
	// function returns an empty array if the curve can't be found.
	FxArray<FxReal> GetRawCurveKeySlopeIn( const FxString& group, const FxString& anim, const FxString& curve );
	// Returns an array of key output slopes for the specified curve in the 
	// specified group and animation.  Most users will want to get "baked" curves
	// instead to constuct a curve that accurately reflects a node's value over 
	// time.  Baked curves always have input and output slopes of 0.  The 
	// function returns an empty array if the curve can't be found.
	FxArray<FxReal> GetRawCurveKeySlopeOut( const FxString& group, const FxString& anim, const FxString& curve );
	// Returns an array of key values for the specified node in the specified 
	// group and animation.  The funtion evaluates the face graph at 60fps and
	// returns values for a baked curve that is key reduced and accurately 
	// reflects a node's value over time.  The function should be used in
	// conjunction with GetBakedCurveKeyTimes. The function returns an empty 
	// array if the node or animation can't be found.
	FxArray<FxReal> GetBakedCurveKeyValues( const FxString& group, const FxString& anim, const FxString& nodeName );
	// Returns an array of key times for the specified node in the specified 
	// group and animation.  The funtion evaluates the face graph at 60fps and
	// returns time values for a baked curve that is key reduced and accurately 
	// reflects a node's value over time.  The function should be used in
	// conjunction with GetBakedCurveKeyValues. The function returns an empty 
	// array if the node or animation can't be found.
	FxArray<FxReal> GetBakedCurveKeyTimes( const FxString& group, const FxString& anim, const FxString& nodeName );
	// Returns an array of key input slopes for the specified node in the specified 
	// group and animation.  The function evaluates the face graph at 60fps and
	// returns values for a baked curve that is key reduced and accurately 
	// reflects a node's value over time.  The function returns an empty 
	// array if the node or animation can't be found.
	FxArray<FxReal> GetBakedCurveKeySlopeIn( const FxString& group, const FxString& anim, const FxString& nodeName );
	// Returns an array of key ouput slopes for the specified node in the specified 
	// group and animation.  The function evaluates the face graph at 60fps and
	// returns values for a baked curve that is key reduced and accurately 
	// reflects a node's value over time.  The function returns an empty 
	// array if the node or animation can't be found.
	FxArray<FxReal> GetBakedCurveKeySlopeOut( const FxString& group, const FxString& anim, const FxString& nodeName );
	// Returns the specified animation length in seconds
	FxReal GetAnimDuration( const FxString& group, const FxString& anim );

	// Inserts a key into the specified curve.  Will create the curve if it doesn't
	// exist, but the group and animation must be present. Returns index of inserted key.
	FxSize InsertKey( const FxString& group, const FxString& anim, const FxString& curve, 
		FxReal keyTime, FxReal keyValue, FxReal keySlopeIn, FxReal keySlopeOut );

	// Deletes all keys in the specified curve.
	FxBool DeleteAllKeys( const FxString& group, const FxString& anim, const FxString& curve);

	// Returns if this version of the plugin is a mod developer version.
	FxBool IsModDeveloper( );

	// Returns if this version of the plugin is a no save version.
	FxBool IsNoSave( );
protected:
	// The actor.
	FxActor* _actor;
	// FxTrue if the tool is performing a batch operation.
	FxBool _isBatchOp;
	// FxTrue if Face Graph nodes were overwritten.
	FxBool _faceGraphNodesOverwritten;
	// FxTrue if bone poses were exported with all bones pruned.
	FxBool _allBonesPruned;
	// FxTrue if you want the plugin to set/remove morph keys when a morph
	// target has the same name as a node in the Face Graph.
	// @todo expose functions for setting/getting this variable so you can 
	// do itfrom the GUI/Maxscrip/MEL.
	FxBool _doMorph;
	// Toggles on/off all Message box warnings.
	FxBool _shouldDisplayWarningDialogs;
	// A class to hold an entry from a batch import export poses file.
	class FxImportExportPosesFileEntry
	{
	public:
		FxString poseName;
		FxInt32  poseFrame;
	};

	// Overwrites the pCurrentNode node in the Face Graph with the 
	// pNodeToAdd node.
	FxBool _overwriteFaceGraphNode( FxFaceGraphNode* pCurrentNode, 
		FxFaceGraphNode* pNodeToAdd );
	// Parses batch import and batch export operation files.  The results are
	// stored in the poses array.
	FxBool _parseBatchImportExportFile( const FxString& filename, 
		FxArray<FxImportExportPosesFileEntry>& poses );
};

} // namespace Face

} // namespace OC3Ent

#endif
