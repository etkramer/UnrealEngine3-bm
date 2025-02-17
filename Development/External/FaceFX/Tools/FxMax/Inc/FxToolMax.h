//------------------------------------------------------------------------------
// The FaceFx Max plug-in interface.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxToolMax_H__
#define FxToolMax_H__

#include "FxTool.h"
#include "FxMaxMain.h"
#include "FxMaxAnimController.h"

namespace OC3Ent
{

namespace Face
{

// The interface for the FaceFx Max plug-in.
class FxToolMax : public FxTool
{
	// Declare the class.
	FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxToolMax, FxTool);
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxToolMax);
public:
	// Constructor.
	FxToolMax();
	// Destructor.
	~FxToolMax();

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
// Max level operations.
//------------------------------------------------------------------------------

	// Set the Max interface pointer.
	void SetMaxInterfacePointer( Interface* pMaxInterface );
	// Set the normalize scale option (the default is FxTrue).
	void SetNormalizeScale( FxBool normalizeScale );
	// Gets the normalize scale option (the default is FxTrue).
	FxBool GetNormalizeScale( void ) const;
	// Cleans up all animation curves in Max associated with the current
	// actor.
	void CleanBonesMorph( void );

protected:
	// Finds the specified bone in the Max scene and returns it's transformation.
	// This only returns the bone's transformation in the current frame.  
	// Returns FxTrue if the bone was found and the transform is valid or
	// FxFalse otherwise.
	FxBool _getBoneTransform( const FxString& boneName, FxBone& bone );
	// Sets the specified bone's transform for the current frame.
	// Returns FxTrue if the transform was set or FxFalse otherwise.
	FxBool _setBoneTransform( const FxBone& bone );
	// Initializes the animation controller.
	FxBool _initializeAnimController( void );
	// Stops any currently playing animation in Max.
	void _stopAnimationPlayback( void );
	// Returns an array of Morpher Modifiers found in the scene.
	bool _recurseForMorpher( INode* node, OC3Ent::Face::FxArray<MorphR3*> &MorpherArray );
	// Helper function for finding the Morpher Modifier.
	MorphR3* _findMorpherModifier( INode* pNode );
	// The Max interface pointer.
	Interface* _pMaxInterface;
	// FxTrue if the normalize scale option is enabled (the default).
	FxBool _normalizeScale;
	// The bone animation controller.
	FxMaxAnimController _animController;
};

} // namespace Face

} // namespace OC3Ent

#endif
