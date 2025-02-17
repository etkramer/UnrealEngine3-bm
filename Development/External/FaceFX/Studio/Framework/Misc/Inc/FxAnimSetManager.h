//------------------------------------------------------------------------------
// External animation set manager.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimSetManager_H__
#define FxAnimSetManager_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxArray.h"
#include "FxAnimSet.h"
#include "FxAnimUserData.h"

#ifdef __UNREAL__
	class UFaceFXAnimSet;
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxAnimSetMiniSession.
//------------------------------------------------------------------------------

// A mini session for an animation set.
class FxAnimSetMiniSession : public FxObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnimSetMiniSession, FxObject);
public:
	// Constructor.
	FxAnimSetMiniSession();
	// Copy constructor.
	FxAnimSetMiniSession( const FxAnimSetMiniSession& other );
	// Assignment operator.
	FxAnimSetMiniSession& operator=( const FxAnimSetMiniSession& other );
	// Destructor.
	virtual ~FxAnimSetMiniSession();

	// Clears the animation user data out of the mini session.
	void Clear( void );

	// Searches for the animation user data named animUserDataName and returns
	// a pointer to it or NULL if it does not exist.
	FxAnimUserData* FindAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Adds the animation user data.
	void AddAnimUserData( FxAnimUserData* pAnimUserData );
	// Searches for and removes the animation user data named animUserDataName.
	void RemoveAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Searches for and removes orphaned animation user data objects.
	void FindAndRemoveOrphanedAnimUserData( void );

	// Returns the number of user data objects contained in the mini session.
	FxSize GetNumUserDataObjects( void ) const;
	// Clones the user data object at index and returns a pointer to the clone.
	// The caller assumes control of the memory for the cloned object.  It is
	// theoretically possible that this could return NULL.
	FxAnimUserData* GetUserDataObjectClone( FxSize index ) const;

	// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	// The animation user data list.
	FxArray<FxAnimUserData*> _animUserDataList;
};

// Utility function to load a mini session from a file on disc.
FxBool FxLoadAnimSetMiniSessionFromFile( FxAnimSetMiniSession& miniSession, const FxChar* filename, const FxBool bUseFastMethod );
// Utility function to save a mini session to a file on disc.
FxBool FxSaveAnimSetMiniSessionToFile( FxAnimSetMiniSession& miniSession, const FxChar* filename );
// Utility function to load a mini session from a block of memory.
FxBool FxLoadAnimSetMiniSessionFromMemory( FxAnimSetMiniSession& miniSession, const FxByte* pMemory, const FxSize numBytes );
// Utility function to save a mini session to a block of memory.
FxBool FxSaveAnimSetMiniSessionToMemory( FxAnimSetMiniSession& miniSession, FxByte*& pMemory, FxSize& numBytes );

#ifdef IS_FACEFX_STUDIO

//------------------------------------------------------------------------------
// FxAnimSetManagerEntry.
//------------------------------------------------------------------------------

// An entry in the FxAnimSetManager.
class FxAnimSetManagerEntry
{
public:
	// Constructor.
	FxAnimSetManagerEntry();
	// Destructor.
	~FxAnimSetManagerEntry();

	// Clears the entry.
	void Clear( void );

	// Sets the full path to the animation set archive.
	void SetAnimSetArchivePath( const FxString& animSetArchivePath );
	// Returns the full path to the animation set archive.
	const FxString& GetAnimSetArchivePath( void ) const;

	// Sets the full path to the animation set mini session archive.
	void SetAnimSetSessionArchivePath( const FxString& animSetSessionArchivePath );
	// Returns the full path to the animation set mini session archive.
	const FxString& GetAnimSetSessionArchivePath( void ) const;

	// Returns the name of the animation group contained in the animation set in
	// the entry.
	const FxName& GetContainedAnimGroupName( void ) const;

	// Sets the animation set contained in the entry.
	void SetAnimSet( const FxAnimSet& animSet );
	// Returns the animation set contained in the entry.
	const FxAnimSet& GetAnimSet( void ) const;
	// Sets the animation set mini session contained in the entry.
	void SetAnimSetSession( const FxAnimSetMiniSession& animSetMiniSession );
	// Returns the animation set mini session contained in the entry.
	const FxAnimSetMiniSession& GetAnimSetSession( void ) const;

	// Searches for the animation user data named animUserDataName and returns
	// a pointer to it or NULL if it does not exist.
	FxAnimUserData* FindAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Adds the animation user data.
	void AddAnimUserData( FxAnimUserData* pAnimUserData );
	// Searches for and removes the animation user data named animUserDataName.
	void RemoveAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Searches for and removes orphaned animation user data objects.
	void FindAndRemoveOrphanedAnimUserData( void );

	// Loads the separate animation set and mini session archives into the
	// entry.
	void Load( const FxString& animSetArchivePath );
	// Saves the separate animation set and mini session archives contained
	// in the entry.
	void Save( const FxString& animSetArchivePath );

	// Unreal-specific entry handling.
#ifdef __UNREAL__
	// Sets the UFaceFXAnimSet object contained in the entry.
	void SetUE3AnimSet( UFaceFXAnimSet* pAnimSet );
	// Returns the UFaceFXAnimSet object contained in the entry.
	UFaceFXAnimSet* GetUE3AnimSet( void );
	// Sets whether or not the UFaceFXAnimSet should be remounted in the UFaceFXAsset.
	// For example, if Matinee had already mounted it then it should be remounted.
	void SetShouldRemount( FxBool bShouldRemount );
	// Returns whether or not the UFaceFXAnimSet should be remounted in the UFaceFXAsset.
	FxBool ShouldRemount( void );
	// Loads the separate animation set and mini session archives contained in
	// pAnimSet into the entry.
	void Load( void );
	// Saves the separate animation set and mini session archives contained in
	// the entry into pAnimSet.
	void Save( void );
#endif // __UNREAL__

protected:
	// The full path to the animation set archive.
	FxString _animSetArchivePath;
	// The full path to the animation set mini session archive.
	FxString _animSetSessionArchivePath;
	// A copy of the original animation set.
	FxAnimSet _animSet;
	// A copy of the mini session for the animation set.
	FxAnimSetMiniSession _animSetMiniSession;

	// Unreal-specific.
#ifdef __UNREAL__
	UFaceFXAnimSet* _pAnimSet;
	FxBool _bShouldRemount;
#endif // __UNREAL__
};

//------------------------------------------------------------------------------
// FxAnimSetManager.
//------------------------------------------------------------------------------

// Manages the external animation sets.
class FxAnimSetManager
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxAnimSetManager);
public:

	// Removes all animation sets from the manager.
	static void FlushAllMountedAnimSets( void );

	// Returns the number of currently mounted animation sets.
	static FxSize GetNumMountedAnimSets( void );
	// Returns the name of the animation group contained in the mounted 
	// animation set at index.
	static FxString GetMountedAnimSetName( FxSize index );

	// Checks all files to see if any of them are read-only.  Returns FxTrue if 
	// any file is read-only.  Only returns FxFalse if absolutely no files are 
	// read-only.  If __UNREAL__ is defined this does nothing and always returns
	// FxFalse.
	static FxBool AreAnyFilesReadOnly( void );

	// Mounts an external animation set.
	static FxBool MountAnimSet( const FxString& animSetPath );
	// Unmounts an external animation set.
	static FxBool UnmountAnimSet( const FxName& animSetName );
	// Imports an external animation set.
	static FxBool ImportAnimSet( const FxString& animSetPath );
	// Exports an external animation set.
	static FxBool ExportAnimSet( const FxName& animSetName, const FxString& animSetPath );

	// Unreal-specific animation set handling.
#ifdef __UNREAL__
	// Mounts a UFaceFXAnimSet object.
	static FxBool MountUE3AnimSet( UFaceFXAnimSet* pAnimSet );
	// Unmounts a UFaceFXAnimSet object.
	static FxBool UnmountUE3AnimSet( UFaceFXAnimSet* pAnimSet );
	// Unmounts all UFaceFXAnimSet objects.
	static void UnmountAllUE3AnimSets( void );
#endif // __UNREAL__

	// Searches for the mounted animation set that contains the animation group
	// named animGroupName.  Returns FxInvalidIndex if it does not exist, or a
	// valid index if it does.
	static FxSize FindMountedAnimSet( const FxName& animGroupName );

    // Searches for the animation user data named animUserDataName and returns
	// a pointer to it or NULL if it does not exist.
	static FxAnimUserData* FindAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Adds the animation user data.
	static void AddAnimUserData( FxAnimUserData* pAnimUserData );
	// Searches for and removes the animation user data named animUserDataName.
	static void RemoveAnimUserData( const FxName& animGroupName, const FxName& animName );
	// Searches for and removes orphaned animation user data objects.
	static void FindAndRemoveOrphanedAnimUserData( void );
    
	// Serialization.  This is used for saving only!
	static void Serialize( void );

protected:
	// The external animation set entires.
	static FxArray<FxAnimSetManagerEntry> _externalAnimSetEntries;
	// The previously selected animation group used to keep the session state
	// synchronized when saving.
	static FxString _cachedSelectedAnimGroupName;
	// The previously selected animation name used to keep the session state
	// synchronized when saving.
	static FxString _cachedSelectedAnimName;
};

#endif // IS_FACEFX_STUDIO

} // namespace Face

} // namespace OC3Ent

#endif
