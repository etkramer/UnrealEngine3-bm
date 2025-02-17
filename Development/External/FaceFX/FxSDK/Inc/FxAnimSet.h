//------------------------------------------------------------------------------
// A FaceFx animation set that can be dynamically mounted and unmounted from
// any FaceFx Actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimSet_H__
#define FxAnimSet_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxNamedObject.h"
#include "FxAnim.h"

namespace OC3Ent
{

namespace Face
{

/// An animation set.
/// Animation sets provide a way of dynamically mounting and unmounting 
/// animation groups from actors.  The animation sets can then be stored in
/// separate archives on disc, streamed in, and mounted in actors on on-the-fly.
/// \ingroup animation
/// Animation sets created by FaceFX Studio generally use the extension ".fxe".
///
/// \b Sample: Loading and mounting an FxAnimSet.
/// \dontinclude SampleCode.h
/// \skip Mounting an anim set
/// \until }
///
/// \b Sample: Unmounting an FxAnimSet.
/// \skip Assumes the group name was cached
/// \until UnmountAnimSet
///
/// Animation sets need not be used only with the actor from which they were
/// initially exported.  They can be mounted to any actor in the system.
/// They are only effective, though, when the actor's %Face Graph contains the
/// appropriately named nodes, so the animation will actually affect the 
/// character.
class FxAnimSet : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxAnimSet, FxNamedObject)
public:
	/// Constructor.
	FxAnimSet();
	/// Copy constructor.
	FxAnimSet( const FxAnimSet& other );
	/// Assignment.
	FxAnimSet& operator=( const FxAnimSet& other );
	/// Destructor.
	virtual ~FxAnimSet();

	/// Sets the name of the FaceFX Actor that originally created the animation
	/// set.
	void SetOwningActorName( const FxName& owningActorName );
	/// Gets the name of the FaceFX Actor that originally created the animation
	/// set.
	const FxName& GetOwningActorName( void ) const;

	/// Sets the animation group contained in the animation set.
	void SetAnimGroup( const FxAnimGroup& animGroup );
	/// Gets the animation group contained in the animation set.
	const FxAnimGroup& GetAnimGroup( void ) const;
	/// Returns a pointer to the animation at index.
	FxAnim* GetAnimPtr( FxSize index );

	// Unreal-specific stuff.
#ifdef __UNREAL__
	// Sets all user data pointers in each animation contained in the FxAnimSet
	// to NULL.
	void NullAllAnimUserDataPtrs( void );
#endif // __UNREAL__

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	/// The name of the FaceFX Actor that originally created the animation set.
	FxName _owningActorName;
	/// The animation group contained in the animation set.
	FxAnimGroup _animGroup;
};

/// Utility function to load an animation set from a file on disc.
/// \param animSet A reference to the animation set to load into.
/// \param filename The path to the file on disc to load from.
/// \param bUseFastMethod If \p FxTrue FxArchiveStoreFileFast is used internally.
/// If \p FxFalse FxArchiveStoreFile is used.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadAnimSetFromFile( FxAnimSet& animSet, const FxChar* filename, const FxBool bUseFastMethod,
							          void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to save an animation set to a file on disc.
/// \param animSet A reference to the animation set to load into.
/// \param filename The path to the file on disc to save to.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveAnimSetToFile( FxAnimSet& animSet, const FxChar* filename,
						            FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
						            void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to load an animation set from a block of memory.
/// \param animSet A reference to the animation set to load into.
/// \param pMemory The array of bytes containing the data to load from.
/// \param numBytes The size, in bytes, of pMemory.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxLoadAnimSetFromMemory( FxAnimSet& animSet, const FxByte* pMemory, const FxSize numBytes,
							            void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );
/// Utility function to save an animation set to a block of memory.
/// \param animSet A reference to the animation set to load into.
/// \param pMemory A reference to a pointer that will hold the array of bytes
/// containing the animation set data.  This must be NULL when passed in and will be
/// allocated internally via FxAlloc().  The client is responsible for freeing 
/// this memory and it must be freed via a call to FxFree().
/// \param numBytes Upon success, this is the size, in bytes, of pMemory.
/// \param byteOrder The byte order to save in.
/// \return \p FxTrue if successful, \p FxFalse otherwise.
/// \ingroup object
FxBool FX_CALL FxSaveAnimSetToMemory( FxAnimSet& animSet, FxByte*& pMemory, FxSize& numBytes,
							          FxArchive::FxArchiveByteOrder byteOrder = FxArchive::ABO_LittleEndian,
							          void(FX_CALL *callbackFunction)(FxReal) = NULL, FxReal updateFrequency = 0.01f );

} // namespace Face

} // namespace OC3Ent

#endif
