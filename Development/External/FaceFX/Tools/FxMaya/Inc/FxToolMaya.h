//------------------------------------------------------------------------------
// The FaceFx Maya plug-in interface.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxToolMaya_H__
#define FxToolMaya_H__

#include "FxTool.h"
#include "FxMayaAnimController.h"
#include <maya/MPlug.h>

namespace OC3Ent
{

namespace Face
{

// The interface for the FaceFx Maya plug-in.
class FxToolMaya : public FxTool
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxToolMaya, FxTool);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxToolMaya);
public:
	// Constructor.
	FxToolMaya();
	// Destructor.
	~FxToolMaya();

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
	virtual FxBool ImportRefPose( FxInt32 frame );
	// Exports the reference pose from the specified frame containing the 
	// specified bones.
	virtual FxBool ExportRefPose( FxInt32 frame, const FxArray<FxString>& bones );

	// Imports specified bone pose to the specified frame.
	virtual FxBool ImportBonePose( const FxString& name, FxInt32 frame );
	// Exports a bone pose from a specified frame with the specified name.
	virtual FxBool ExportBonePose( FxInt32 frame, const FxString& name );
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
		FxReal fps );

//------------------------------------------------------------------------------
// Maya level operations.
//------------------------------------------------------------------------------

	// Cleans up all animation curves in Maya associated with the current
	// actor.
	void Clean( void );

	// Contains all information to associate a Maya BlendShape plug structure
	// with the data needed to query FaceFX for the animation data.
	class FxMayaBlendShapePlugInfo
	{
	public:
		// Constructor.
		FxMayaBlendShapePlugInfo( const FxName& iOwnerNodeName,
								  const FxName& iTargetName,
								  const FxString& iTargetFullName,
								  const MPlug& iTargetPlug )
			: ownerNodeName(iOwnerNodeName)
			, targetName(iTargetName)
			, targetFullName(iTargetFullName)
			, targetPlug(iTargetPlug) {}
		// Destructor.
		~FxMayaBlendShapePlugInfo() {}
		// Copy constructor.
		FxMayaBlendShapePlugInfo( const FxMayaBlendShapePlugInfo& other )
			: ownerNodeName(other.ownerNodeName)
			, targetName(other.targetName)
			, targetFullName(other.targetFullName)
			, targetPlug(other.targetPlug) {}
		// Assignment operator.
		FxMayaBlendShapePlugInfo& operator=( const FxMayaBlendShapePlugInfo& other )
		{
			if( this == &other ) return *this;
			ownerNodeName = other.ownerNodeName;
			targetName    = other.targetName;
			targetFullName = other.targetFullName;
			targetPlug    = other.targetPlug;
			return *this;
		}
		
		// The name of the Face Graph Node that "owns" this target.
		FxName ownerNodeName;
		// The name of the actual target (Maya BlendShape).
		FxName targetName;
		// The full name of the actual target (i.e. "blendshape1.Eat")
		FxString targetFullName;
		// The MPlug associated with the actual target (Maya BlendShape).
		MPlug  targetPlug;
	};

protected:
	// Finds the specified bone in Maya's DAG and returns it's transformation.
	// This only returns the bone's transformation in the current frame.  
	// Returns FxTrue if the bone was found and the transform is valid or
	// FxFalse otherwise.
	FxBool _getBoneTransform( const FxString& boneName, FxBone& bone );
	// Sets the specified bone's transform for the current frame.
	// Returns FxTrue if the transform was set or FxFalse otherwise.
	FxBool _setBoneTransform( const FxBone& bone );
	// Initializes the animation controller.
	FxBool _initializeAnimController( void );
	// Stops any currently playing animation in Maya.
	void _stopAnimationPlayback( void );

	// Called when the actor has changed so that a MEL script function can be
	// called to alert any MEL scripts that define the "callback" function
	// __FaceFxOnActorChanged().
	void _actorChanged( void );
	// Called when the list of reference bones has changed so that a MEL script
	// function can be called to alert any MEL scripts that define the 
	// "callback" function __FaceFxOnRefBonesChanged().
	void _refBonesChanged( void );
	// Called when the list of bone poses has changed so that a MEL script
	// function can be called to alert any MEL scripts that define the
	// "callback" function __FaceFxOnBonePosesChanged().
	void _bonePosesChanged( void );
	// Get an array of FxMayaBlendShapePlugInfo objects for morph curves that 
	// match morph target names contained in morph target Face Graph nodes.
	FxArray<FxMayaBlendShapePlugInfo> _getMayaBlendShapePlugInfos( void );
	// Deletes animation curves for morph targets that match nodes in the Face Graph.
	void _deleteMorphCurves( void );
	
	// The bone animation controller.
	FxMayaAnimController _animController;
};

} // namespace Face

} // namespace OC3Ent

#endif
