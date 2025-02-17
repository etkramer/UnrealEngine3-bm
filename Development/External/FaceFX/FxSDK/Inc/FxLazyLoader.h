//------------------------------------------------------------------------------
// This is the virtual base class for a "load-on-demand" or "lazily-loaded"
// object that implements the Lazy Load design pattern.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxLazyLoader_H__
#define FxLazyLoader_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

// Forward declare FxArchive.
class FxArchive;

/// Provides lazy-loading for objects contained in archives.
/// Using a lazy loader is slightly involved.  For one, note the const modifier
/// on the Load() function.  Any class you wish to be lazy-loaded 
/// should be derived from FxLazyLoader and should implement Load() and
/// Unload() (it is up to you if you want Unload() to actually do anything).
/// Don't forget to implement FxLazyLoader portions in your copy constructors
/// and assignment operators (adding the lazy loader to the archive as well).
///
/// You should call Load() first in any function that actually uses
/// the data contained in the lazy-loaded object.  FxLazyLoaders act as "ghost"
/// objects in this sense.  See FxAnimUserData.cpp in the FaceFX Studio source
/// code for an example and details of the serialization.
///
/// Another point to be aware of is that currently you should not create a
/// lazy-loaded object that contains a pointer to some other serialized object.
/// Due to the way pointer serialization works, this could lead to the real
/// pointed-to object being skipped over during serialization and some other
/// object actually loading that references the same pointed-to object, causing
/// the other object to try to load the pointed-to object "in-place" in the 
/// archive, which will hose everything.
/// \ingroup object
class FxLazyLoader
{
public:
	/// Constructor.
	FxLazyLoader()
		: _archive(NULL)
		, _pos(0) 
		, _isLoaded(FxFalse) {}
	/// Destructor.
	virtual ~FxLazyLoader() {}

	/// Loads the object.  Load() is const because Load() should sometimes
	/// be called inside of const member functions.  All data loaded through
	/// the lazy loader should be declared mutable for this reason.
	virtual void Load( void ) const = 0;
	/// Unloads the object.
	virtual void Unload( void ) = 0;

	/// Precache is an alias for Load().
	void Precache( void ) const
	{
		Load();
	}

	/// Returns FxTrue if the object is loaded, FxFalse otherwise.
	FxBool IsLoaded( void ) const
	{
		return _isLoaded;
	}
	/// Directly sets the loaded flag.
	void SetLoaded( FxBool isLoaded )
	{
		_isLoaded = isLoaded;
	}

	/// Archives need access to protected data.
	friend class FxArchive;

protected:
	/// The managed archive this lazy loader belongs to.
	FxArchive* _archive;
	/// The position of the lazily-loaded data in the managed archive.
	FxInt32 _pos;
	/// Whether or not this lazy loader has been fully loaded or not.
	mutable FxBool _isLoaded;
};

} // namespace Face

} // namespace OC3Ent

#endif
