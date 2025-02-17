#ifndef _FXTOOLXSI_H__
#define _FXTOOLXSI_H__

#include "FxTool.h"
#include "FxXSIAnimController.h"
#include <xsi_customproperty.h>

namespace OC3Ent
{

	namespace Face
	{

		// The interface for the FaceFx XSI plug-in.
		class FxToolXSI : public FxTool
		{
			// Declare the class.
			FX_DECLARE_CLASS_NO_DYNCREATE_NO_SERIALIZE(FxToolXSI, FxTool);
			// Disable copy construction and assignment.
			FX_NO_COPY_OR_ASSIGNMENT(FxToolXSI);
		public:
			// Constructor.
			FxToolXSI();
			// Destructor.
			~FxToolXSI();

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

			FxActor* GetActor(){return _actor;}

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
			/* D.P. 02/27/2008 - Removed Delete Bone Pose functions.
			FxBool RemoveBonePose(const FxString& name );
			*/
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
				FxReal fps);

			//------------------------------------------------------------------------------
			// XSI level operations.
			//------------------------------------------------------------------------------

			// Cleans up all animation curves in Maya associated with the current
			// actor.
			void Clean( void );

		protected:
			// Finds the specified bone in XSI's scene and returns it's transformation.
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

			FxXSIAnimController m_animController;
			XSI::CustomProperty m_UI;
		};

	} // namespace Face

} // namespace OC3Ent

#endif //_FXTOOLXSI_H__
