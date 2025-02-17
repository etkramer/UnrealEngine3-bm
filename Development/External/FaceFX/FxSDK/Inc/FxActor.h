//------------------------------------------------------------------------------
// This class implements a FaceFx actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxActor_H__
#define FxActor_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxList.h"
#include "FxArray.h"
#include "FxFaceGraph.h"
#include "FxCompiledFaceGraph.h"
#include "FxMasterBoneList.h"
#include "FxAnim.h"
#include "FxAnimSet.h"
#include "FxPhonemeMap.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxActorInstance.
class FxActorInstance;

/// Extended information that can optionally be returned from 
/// FxActor::MountAnimSet() and FxActor::UnmountAnimSet().
enum FxMountUnmountExtendedInfo
{
	MUE_Duplicate = 0,			   ///< The animation group already exists.
	MUE_InvalidMountedAnimSet,	   ///< The group is not a mounted animation set.
	MUE_CurrentlyPlayingTryLater,  ///< An instance of the actor is currently 
								   ///< playing an animation.  Try the operation 
								   ///< again later.
	MUE_None					   ///< No error occurred.
};

/// A FaceFx actor.
/// The actor contains the \ref OC3Ent::Face::FxMasterBoneList "master bone list", 
/// the \ref OC3Ent::Face::FxFaceGraph "Face Graph", and the 
/// \ref OC3Ent::Face::FxAnimGroup "animation groups" for a particular character.
/// It also contains the register definitions for the actor.  When instanced, each
/// instance will contain registers as defined by the actor.
/// \sa \ref loadActor "Loading an Actor"
class FxActor : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxActor, FxNamedObject)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxActor)

public:
	/// Constructor.
	FxActor();
	/// Destructor.
	virtual ~FxActor();

	/// The "NewActor" actor name.
	static FxName NewActor;

	/// Starts up the actor management system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Startup( void );
	/// Shuts down the actor management system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Shutdown( void );

	/// Returns FxTrue if the client application should perform its own FaceFX
	/// actor relinking steps.  This is provided for convenience and will only 
	/// return FxTrue if client application code has specifically called 
	/// FxActor::SetShouldClientRelink(FxTrue).
	FX_INLINE FxBool ShouldClientRelink( void ) const { return _shouldClientRelink; }
	/// This is provided for convenience so that the client application can
	/// force its own FaceFX actor relinking steps to be executed.
	void SetShouldClientRelink( FxBool shouldClientRelink );

	/// Returns FxTrue if the actor is currently open in FaceFX Studio.
	FX_INLINE FxBool IsOpenInStudio( void ) const { return _isOpenInStudio; }
	/// \note For internal use only.  This is called with FxTrue as the argument
	/// when the actor is opened in FaceFX Studio.
	void SetIsOpenInStudio( FxBool isOpenInStudio );

	/// Returns a non-const reference to the actor's master bone list.
	FX_INLINE FxMasterBoneList& GetMasterBoneList( void ) { return _masterBoneList; }

	/// Returns a non-const reference to the actor's decompiled %Face Graph.
	/// \note GetDecompiledFaceGraph() returns the decompiled representation of 
	/// the %Face Graph.  You should not be using this representation in-game as
	/// it is not in an optimized format and is only useful in the FaceFX tools.
	/// If you do need to use it and the face graph returned is empty, it could 
	/// mean that the face graph is in fact empty or you have not yet called 
	/// FxActor::DecompileFaceGraph().
	FX_INLINE FxFaceGraph& GetDecompiledFaceGraph( void ) { return _decompiledFaceGraph; }

	//@todo finish compiled graph docs
	/// Returns a non-const reference to the actor's compiled %Face Graph.
	FX_INLINE FxCompiledFaceGraph& GetCompiledFaceGraph( void ) { return _compiledFaceGraph; }
	/// Compiles the actor's face graph from the decompiled face graph.
	/// \note This is generally only useful in the FaceFX tools.
	void CompileFaceGraph( void );
	/// Decompiles the actor's face graph from the compiled face graph.
	/// \note This is generally only useful in the FaceFX tools.
	void DecompileFaceGraph( void );
	
	/// Adds an animation to the specified group.  
	/// \param group The name of the animation group.  If the group isn't found, it is created.  
	/// \param anim The animation to be added.
	/// \return \p FxFalse if the animation couldn't be added, otherwise \p FxTrue.
	FxBool AddAnim( const FxName& group, const FxAnim& anim );
	/// Removes an animation from the specified group. 
	/// \param group The name of the animation group.
	/// \param anim The name of the animation.
	/// \return \p FxFalse if the animation couldn't be removed, otherwise \p FxTrue.
	FxBool RemoveAnim( const FxName& group, const FxName& anim );
	/// Adds an animation group.
	FxBool AddAnimGroup( const FxName& name );
	/// Removes an animation group.
	FxBool RemoveAnimGroup( const FxName& name );
	/// Returns the total number of animation groups (including currently
	/// mounted groups).
	FxSize GetNumAnimGroups( void ) const;
	/// Returns the index of the animation group with the specified name.
	/// \param name The name of the animation group to find.
	/// \return The index of the group with the specified name, or 
	/// \p FxInvalidIndex if the group was not found.
	FxSize FindAnimGroup( const FxName& name ) const;
	/// Returns a reference to the animation group at index.
	FxAnimGroup& GetAnimGroup( FxSize index );
	/// Returns FxTrue if the specified group is a currently mounted animation
	/// group or FxFalse otherwise.
	FxBool IsAnimGroupMounted( const FxName& group ) const;
	/// Utility function that returns a pointer to an FxAnim object given the
	/// animation group name and animation name.
	/// \param group The name of the animation group.
	/// \param anim The name of the animation.
	/// \return \p NULL if the animation does not exist in the specified group,
	/// otherwise a valid pointer to the animation.
	FxAnim* GetAnimPtr( const FxName& group, const FxName& anim );

	/// Mounts an animation set into the actor.
	/// \param animSet The animation set to mount.
	/// \param pExtendedInfo If non-NULL, will be filled out with one of the
	/// FxMountUnmountExtendedInfo values indicating what actually happened.
	/// \return \p FxTrue if the animation set was mounted correctly or \p FxFalse
	/// if it was not.  \p FxFalse can be returned if there is already an animation
	/// group with the same name as the group contained in the animation set
	/// for instance.
	/// \sa OC3Ent::Face::FxAnimSet
	FxBool MountAnimSet( const FxAnimSet& animSet, 
		                 FxMountUnmountExtendedInfo* pExtendedInfo = NULL );
	/// Unmounts an animation set from the actor.
	/// \param animSetName The name of the animation set to unmount.  (Note that the
	/// parameter is named animSetName but that a valid animation group name is actually
	/// required here.  Generally animation sets share the same name as the animation
	/// group which they contain as that is the default behavior of the ExportAnimSet()
	/// function).
	/// \param bStopPlayback If FxTrue, will stop playback on any instances of the
	/// actor that are currently playing an animation from the animation group attempting
	/// to be unmounted.  If FxFalse and there is an instance of the actor currently 
	/// playing an animation from the group attempting to be unmounted nothing happens 
	/// and it is the client's responsibility to try the operation again at a later time.
	/// \param pExtendedInfo If non-NULL, will be filled out with one of the
	/// FxMountUnmountExtendedInfo values indicating what actually happened.
	/// \return \p FxTrue if the animation set was unmounted correctly or \p FxFalse
	/// if it was not.  \p FxFalse can be returned if no animation set by that
	/// name exists in the actor or when attempting to unmount the Default animation group 
	/// for instance or if there is an instance of the actor currently playing
	/// an animation from the group attempting to be unmounted but bStopPlayback is FxFalse.
	/// \sa OC3Ent::Face::FxAnimSet
	FxBool UnmountAnimSet( const FxName& animSetName, FxBool bStopPlayback = FxTrue, 
		                   FxMountUnmountExtendedInfo* pExtendedInfo = NULL );
	/// Imports an animation set into the actor as an animation group.
	/// \note This is only for use in FaceFX Studio.  Game code should never need to call this.
	/// \param animSet The animation set to import.
	/// \return FxTrue if the animations set was imported correctly or \p FxFalse
	/// if it was not.  \p FxFalse can be returned if there is already an animation
	/// group with the same name as the group contained in the animation set 
	/// for instance.
	FxBool ImportAnimSet( const FxAnimSet& animSet );
	/// Exports an animation group from the actor into an animation set.
	/// \note This is only for use in FaceFX Studio.  Game code should never need to call this.
	/// \param animGroupName The name of the animation group to export to an animation set.
	/// \param exportedAnimSet The animation set as exported from the actor.  This is only valid
	/// if \p FxTrue is returned.
	/// \return \p FxTrue if the animation group was exported to an animation set
	/// correctly or \p FxFalse if it was not.  \p FxFalse can be returned if no animation
	/// group by that name exists or when attempting to export the Default animation group
	/// for instance.
	FxBool ExportAnimSet( const FxName& animGroupName, FxAnimSet& exportedAnimSet );

	/// Finds and returns a pointer to the specified actor the global actor list.
	/// \param name The name of the actor to find.
	/// \return \p NULL if the actor is not found, otherwise a valid pointer to the actor.
	static FxActor* FX_CALL FindActor( const FxName& name );
	/// Returns FxTrue if the FxActor pointer \a actor is contained in the 
	/// global actor list.
	/// \param actor A pointer to the actor.
	/// \return FxTrue if \a actor is in the global actor list, FxFalse 
	/// otherwise.
	static FxBool FX_CALL FindActorPtr( FxActor* actor );
	/// Sets the flag that determines if the built-in FaceFX actor management system should
	/// assume control of the memory used by actors added to the actor management system.
	/// The default value is FxTrue.
	static void FX_CALL SetShouldFreeActorMemory( FxBool shouldFreeActorMemory );
	/// Returns the status of the flag that determines if the built-in FaceFX actor management
	/// system should assume control of the memory used by actors added to the actor management
	/// system.  The default value is FxTrue.
	static FxBool FX_CALL GetShouldFreeActorMemory( void );
	/// Adds the actor to the global actor list.  
	/// If there is already an actor in the list that shares the same name as 
	/// the \a actor,\a actor is not added.
	/// \param actor A pointer to the actor to add.  This should be allocated with
	/// operator \p new, and this class will take control of the memory.
	/// \return \p FxTrue if \a actor was added to the list, otherwise \p FxFalse.
	static FxBool FX_CALL AddActor( FxActor* actor );
	/// Removes an actor from the global actor list.
	/// Frees the memory associated with \a actor.
	/// \param actor A pointer to the actor to remove.
	/// \return \p FxTrue if the actor existed in the list and was freed, otherwise \p FxFalse.
	static FxBool FX_CALL RemoveActor( FxActor* actor );
	/// Clears the global actor list.
	/// Frees the memory associated with all the actors in the global actor list.
	static void FX_CALL FlushActors( void );

	/// Returns the phoneme map for the actor.
	FX_INLINE FxPhonemeMap& GetPhonemeMap( void ) { return _phonemeMap; }
	/// Sets the phoneme map for the actor.
	FX_INLINE void SetPhonemeMap( const FxPhonemeMap& phonemeMap ) { _phonemeMap = phonemeMap; }

	/// Adds an instance of this actor to the tracked instance list.
	/// \note Internal use only.
	void AddInstance( FxActorInstance* pActorInstance );
	/// Removes an instance of this actor from the tracked instance list.
	/// \note Internal use only.
	void RemoveInstance( FxActorInstance* pActorInstance );
	
	/// Serializes an FxActor to an archive.
	virtual void Serialize( FxArchive& arc );

protected:
	/// Manages all actors in FaceFx.
	static FxList<FxActor*>* _pActorList;
	/// Flag to tell the actor management system that it should control the 
	/// memory associated with the actors.  By default this is FxTrue.
	static FxBool _shouldFreeActorMemory;

	/// Tells the client application whether or not it should perform its own
	/// FaceFX actor relinking steps.
	FxBool _shouldClientRelink;

	/// This is FxTrue if the actor is currently open in FaceFX Studio.
	FxBool _isOpenInStudio;

	/// The master bone list for this actor.
	FxMasterBoneList _masterBoneList;

	/// The \ref OC3Ent::Face::FxFaceGraph "Face Graph" for this actor.
	FxFaceGraph _decompiledFaceGraph;

	/// The \ref OC3Ent::Face::FxCompiledFaceGraph "Compiled Face Graph" for 
	/// this actor.
	FxCompiledFaceGraph _compiledFaceGraph;

	/// The default animation group for this actor.
	FxAnimGroup _defaultAnimGroup;
	/// The animation groups for this actor.
	FxArray<FxAnimGroup> _animGroups;
	/// The currently mounted animation groups for this actor.
	FxList<FxAnimGroup> _mountedAnimGroups;

	/// A list of all actor instances referring to this actor.
	FxList<FxActorInstance*> _instanceList;

	/// The phoneme map for the actor.
	FxPhonemeMap _phonemeMap;

	/// Finds an instance in the instance list.
	FxBool _findInstance( FxActorInstance* pActorInstance, 
		                  FxList<FxActorInstance*>::Iterator& instanceIter );
};

/// Utility function to load an actor from a file on disc.
/// \param actor A reference to the actor to load into.
/// \param filename The path to the file on disc to load from.
/// \param bUseFastMethod If \p FxTrue FxArchiveStoreFileFast is used internally.
/// If \p FxFalse FxArchiveStoreFile is used.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadActorFromFile( FxActor& actor, const FxChar* filename, const FxBool bUseFastMethod,
						            void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to save an actor to a file on disc.
/// \param actor A reference to the actor to save from.
/// \param filename The path to the file on disc to save to.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveActorToFile( FxActor& actor, const FxChar* filename, 
						          FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
						          void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to load an actor from a block of memory.
/// \param actor A reference to the actor to load into.
/// \param pMemory The array of bytes containing the data to load from.
/// \param numBytes The size, in bytes, of pMemory.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadActorFromMemory( FxActor& actor, const FxByte* pMemory, const FxSize numBytes,
							          void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to save an actor to a block of memory.
/// \param actor A reference to the actor to save from.
/// \param pMemory A reference to a pointer that will hold the array of bytes
/// containing the actor data.  This must be NULL when passed in and will be
/// allocated internally via FxAlloc().  The client is responsible for freeing 
/// this memory and it must be freed via a call to FxFree().
/// \param numBytes Upon success, this is the size, in bytes, of pMemory.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveActorToMemory( FxActor& actor, FxByte*& pMemory, FxSize& numBytes, 
						            FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
						            void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );

FX_INLINE FxSize FxActor::GetNumAnimGroups( void ) const 
{ 
	return 1 + _animGroups.Length() + _mountedAnimGroups.Length();
}

FX_INLINE FxSize FxActor::FindAnimGroup( const FxName& name ) const
{	
	FxSize numAnimGroups = _animGroups.Length();
	FxList<FxAnimGroup>::ConstIterator magIter    = _mountedAnimGroups.Begin();
	FxList<FxAnimGroup>::ConstIterator magEndIter = _mountedAnimGroups.End();
	for( FxSize magIndex = 0; magIter != magEndIter; ++magIter, ++magIndex )
	{
		if( magIter->GetName() == name )
		{
			return magIndex + numAnimGroups + 1;
		}
	}
	for( FxSize i = 0; i < numAnimGroups; ++i )
	{
		if( _animGroups[i].GetName() == name )
		{
			return i + 1;
		}
	}
	if( _defaultAnimGroup.GetName() == name )
	{
		return 0;
	}
	return FxInvalidIndex;
}

FX_INLINE FxAnimGroup& FxActor::GetAnimGroup( FxSize index ) 
{ 
	FxSize numAnimGroups = _animGroups.Length();
	if( 0 == index )
	{
		return _defaultAnimGroup;
	}
	else if( index <= numAnimGroups )
	{
		return _animGroups[index - 1];
	}
	else
	{
		FxSize magIndex = index - numAnimGroups - 1;
		FxList<FxAnimGroup>::Iterator magIter = _mountedAnimGroups.Begin();
		magIter.Advance(magIndex);
		return *magIter;
	}
}

FX_INLINE FxBool FxActor::IsAnimGroupMounted( const FxName& group ) const
{
	FxList<FxAnimGroup>::ConstIterator magIter    = _mountedAnimGroups.Begin();
	FxList<FxAnimGroup>::ConstIterator magEndIter = _mountedAnimGroups.End();
	for( ; magIter != magEndIter; ++magIter )
	{
		if( magIter->GetName() == group )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

FX_INLINE FxAnim* FxActor::GetAnimPtr( const FxName& group, const FxName& anim )
{
	FxSize groupIndex = FindAnimGroup(group);
	if( FxInvalidIndex != groupIndex )
	{
		FxAnimGroup& animGroup = GetAnimGroup(groupIndex);
		FxSize animIndex = animGroup.FindAnim(anim);
		if( FxInvalidIndex != animIndex )
		{
			return animGroup.GetAnimPtr(animIndex);
		}
	}
	return NULL;
}

} // namespace Face

} // namespace OC3Ent

#endif
