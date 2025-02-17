//------------------------------------------------------------------------------
// This class implements a FaceFx archive.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxArchive_H__
#define FxArchive_H__

#include "FxPlatform.h"
#include "FxUtil.h"
#include "FxMemory.h"
#include "FxArchiveStore.h"

// The FaceFX object serialization subsystem is based in part on NASA's PObject 
// serialization system.  NASA has detailed documentation on the PObject 
// serialization system located at 
// http://www.grc.nasa.gov/WWW/price000/pfc/htm/pobject_serial.html.

namespace OC3Ent
{

namespace Face
{

// Forward declare FxObject.
class FxObject;
// Forward declare FxLazyLoader.
class FxLazyLoader;
// Forward declare FxArchiveInternalData.
class FxArchiveInternalData;

/// A FaceFX archive.
/// Archives can contain lazy-loaded data, so you should either create them on 
/// the heap with operator \p new and add them to the managed archives (by calling MountArchive()),
/// or make sure that the archive is available for the lifetime of any object
/// that might be lazy-loaded from it.  If the archive is opened with AM_LinearLoad or 
/// the lazy-loaders are manually flushed, it is safe for the archive to be created on the stack.
/// Note that AM_Load is now the same as AM_LinearLoad so all loading operations are now
/// linear and you must explicitly specify AM_LazyLoad.  We recommend that you not use AM_LazyLoad
/// in-game and instead create the archive on the stack and simply let it go out of scope.
/// \sa \ref loadActor "Loading an Actor"
/// \ingroup object
class FxArchive : public FxUseAllocator
{
public:
	/// The archive mode.	
	enum FxArchiveMode
	{
		AM_LinearLoad,				///< Linearly loading from an archive.
		AM_LazyLoad,				///< Lazy loading from an archive.
		AM_CreateDirectory,			///< Create the directory information for an archive.
		AM_ApproximateMemoryUsage,	///< Approximates the number of bytes used by the archived data.
		AM_Save,					///< Saving to an archive.
        AM_Load = AM_LinearLoad		///< Loading from an archive (default is linear loading).
	};
	/// The archive byte order.
	enum FxArchiveByteOrder
	{
		ABO_LittleEndian,	///< Little-Endian byte order.
		ABO_BigEndian		///< Big-Endian byte order.
	};

	/// Constructor.
	/// \param iStore A pointer to the FxArchiveStore. Note that the \p iStore 
	/// pointer \b must be the return value of a call to the appropriate 
	/// FxArchiveStore's Create() function.  Do not attempt to cache a pointer
	/// to the archive store or delete it: the archive assumes control of the
	/// pointer.
	/// \param mode Whether to save or load.
	/// \param byteOrder The byte order for saving this archive.  This parameter
	/// is only used when mode is AM_Save and by default it is ABO_LittleEndian.
	FxArchive( FxArchiveStore* iStore, FxArchiveMode mode, 
		       FxArchiveByteOrder byteOrder = ABO_LittleEndian );
	/// Destructor.
	~FxArchive();

	/// Opens the archive.
	void Open( void );
	/// Closes the archive.
	void Close( void );

	/// Starts up the archive management system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Startup( void );
	/// Shuts down the archive management system.
	/// \note For internal use only.  Never call this directly.
	static void FX_CALL Shutdown( void );

	/// Returns \p FxTrue if the archive and its store are in valid states.
	FX_INLINE FxBool IsValid( void ) const { return _isValid; }
	/// Returns \p FxTrue if the archive mode is \p AM_Save.
	FX_INLINE FxBool IsSaving( void ) const
	{
		return (AM_Save == _mode || AM_CreateDirectory == _mode || AM_ApproximateMemoryUsage == _mode);
	}
	/// Returns \p FxTrue if the archive mode is \p AM_Load or \p AM_LinearLoad.
	FX_INLINE FxBool IsLoading( void ) const
	{
		return (AM_Load == _mode || AM_LinearLoad == _mode || AM_LazyLoad == _mode);
	}

	/// Returns \p FxTrue if the archive mode is \p AM_LinearLoad.
	FX_INLINE FxBool IsLinearLoading( void ) const { return AM_LinearLoad == _mode;	}
	/// Returns \p FxTrue if the archive mode is \p AM_LazyLoad.
	FX_INLINE FxBool IsLazyLoading( void ) const { return AM_LazyLoad == _mode;	}


	/// Returns the mode of the archive.
	FX_INLINE FxArchiveMode GetMode( void ) const { return _mode; }
	/// Returns the byte order of the archive.
	FX_INLINE FxArchiveByteOrder GetByteOrder( void ) const { return _byteOrder; }

	/// Returns the version of the FaceFX file format that the current archive
	/// was saved in.
	FX_INLINE FxSize GetFileFormatVersion( void ) const { return _fileFormatVersion; }
	/// Returns the version of the FaceFX SDK that the current archive was 
	/// saved with.  This version number is a series of four single-digit 
	/// version (major, minor, bugfix, revision) concatenated together. Thus 
	/// version 1.0 is 1000, version 1.7.1 is 1710, etc.
	FX_INLINE FxSize GetSDKVersion( void ) const { return _sdkVersion; }
	/// Returns the version of the FaceFX SDK that the current archive was saved
	/// with as a string. This version number is in the format: 
	/// major.minor.bugfix.revision.
	const FxChar* GetSDKVersionString( void );
	/// Returns the name of the FaceFX SDK licensee that the current archive was
	/// saved with.
	FX_INLINE const FxChar* GetLicenseeName( void ) const { return _licenseeName; }
	/// Returns the name of the FaceFX SDK licensee project that the current
	/// archive was saved with.
	FX_INLINE const FxChar* GetLicenseeProjectName( void ) const { return _licenseeProjectName; }
	/// Returns the FaceFX SDK licensee version number that the current archive
	/// was saved with.  This version number has been multiplied by 1000, so 
	/// for example version 1.0 returns 1000, version 1.1 returns 1100, and 
	/// so on.
	FX_INLINE FxSize GetLicenseeVersion( void ) const { return _licenseeVersion; }

	/// Registers a callback to receive archive loading progress updates.
	/// \param callbackFunction The callback function.  Its signature must be 
	/// \code void callbackFunction(FxReal percentComplete) \endcode
	/// \param updateFrequency How often you wish to receive update.
	/// E.g., 0.01f would be a callback for each single percent loaded.
	/// \note An archive in saving mode will not generate progress callbacks.
	void RegisterProgressCallback( void(FX_CALL *callbackFunction)(FxReal), 
								   FxReal updateFrequency = 0.01f );

	/// Finds the object at index in the object table and returns a pointer to 
	/// it or \p NULL if there is no object at that index.
	/// \note Internal use only.
	FxObject* FindObject( FxSize index ) const;
	/// Finds the object in the object table and returns its index.
	/// \param object The object to find.
	/// \return The index of the object, or \p FxInvalidIndex if the object is not in the table.
	/// \note Internal use only.
	FxSize FindObject( FxObject* object ) const;
	/// Adds an object to the object table.
	/// Adds \a object to the object table at \a index or to the end of the 
	/// table if \a index is \p FxInvalidIndex (provided that \a object does not
	/// already exist in the table, in which case \a object isn't added but the
	/// index of the pre-existing object is returned).
	/// \param object The object to add.
	/// \param index The index at which to add, or \p FxInvalidIndex to add at the end.
	/// \return The index of the object in the table.
	/// \note Internal use only.
	FxSize AddObject( FxObject* object, FxSize index = FxInvalidIndex );
	/// Replaces the object at index in the object table with the specified
	/// object.
	/// \param object The object to place in the object table.
	/// \param index The index to place \p object in the table.
	/// \return A pointer to the original object in the table or \p NULL if there was no object at that index.
	/// \note If \a index is \p FxInvalidIndex, nothing happens and \p NULL is returned.
	/// \note Internal use only.
	FxObject* ReplaceObject( FxObject* object, FxSize index );

	/// Adds a name to the archive's internal name table.  If the name is already
	/// in the archive's internal name table the index of the existing name is
	/// returned and no duplicate is added.
	/// \param name A pointer to the name (as an FxString) to be added to the 
	/// archive's internal name table.  It is passed as a void* to avoid header
	/// conflicts and is internally casted to an FxString*.
	/// \return The index of the name in the table.
	/// \note Internal use only.
	FxSize AddName( void* name );
	/// Returns the name at index in the archive's internal name table.
	/// \param index Index of the name to return.
	/// \return An FxRefString** as a void**.
	/// \note Internal use only.
	void** GetName( FxSize index );

	//@todo Document this!
	FxUInt16 SerializeClassVersion( const FxChar* classname, 
		                            FxBool bIsNonFxObjectDerived = FxFalse, 
									FxUInt16 classversion = 0 );

	/// Returns the current position in the archive relative to the beginning.
	FxInt32 Tell( void ) const;
	/// Sets the current position in the archive relative to the beginning.
	void Seek( const FxInt32 pos );

	/// Mounts an archive.
	static void FX_CALL MountArchive( FxArchive* arc );
	/// Unmounts the specified archive fully loading all data in the archive
	/// if \a load is \p FxTrue.
	static void FX_CALL UnmountArchive( FxSize index, FxBool load = FxFalse );
	/// Returns the number of mounted archives in the system.
	static FxSize FX_CALL GetNumMountedArchives( void );
	/// Unmounts all mounted archives from the system causing all data in the
	/// archives to be fully loaded if \a load is \p FxTrue.
	static void FX_CALL UnmountAllArchives( FxBool load = FxFalse );
	/// Loads all lazy loaded objects in all mounted archives.
	static void FX_CALL ForceLoadAllMountedArchives( void );
	/// Unloads all lazy loaded objects in all mounted archives.
	static void FX_CALL ForceUnloadAllMountedArchives( void );

	/// Adds a lazy loader to the archive.
	void AddLazyLoader( FxLazyLoader* lazyLoader );
	/// Removes the lazy loader from the archive.
	void RemoveLazyLoader( FxLazyLoader* lazyLoader );
	/// Flushes all lazy loaders causing all data in the archive to be fully 
	/// loaded if load is FxTrue.
	/// \note If you have loaded an actor from an archive and do not plan on 
	/// mounting the archive, and would like all the actor's data to be 
	/// fully loaded and ready for use, follow the call to 
	/// FlushLazyLoaders() with a call to 
	/// \ref OC3Ent::Face::FxActor::Link() "FxActor::Link()".
	/// \param load \p FxTrue if all the data in the lazy loaders should be loaded,
	/// \p FxFalse if it should be discarded.
	void FlushLazyLoaders( FxBool load = FxFalse );

	/// Archives an FxInt8.
	FxArchive& operator<<( FxInt8& elem );
	/// Archives an FxUInt8.
	FxArchive& operator<<( FxUInt8& elem );
	/// Archives an FxInt16.
	FxArchive& operator<<( FxInt16& elem );
	/// Archives an FxUInt16.
	FxArchive& operator<<( FxUInt16& elem );
	/// Archives an FxInt32.
	FxArchive& operator<<( FxInt32& elem );
	/// Archives an FxUInt32.
	FxArchive& operator<<( FxUInt32& elem );
	/// Archives an FxReal.
	FxArchive& operator<<( FxReal& elem );
	/// Archives an FxDReal.
	FxArchive& operator<<( FxDReal& elem );

#ifdef FX_USE_BUILT_IN_BOOL_TYPE
	/// Archives an FxBool, if the platform provides a built-in boolean type.
	FxArchive& operator<<( FxBool& elem );
#endif

	/// Returns a pointer to the archive store.
	/// \note Internal use only.
	FX_INLINE FxArchiveStore* GetStore( void ) { return _store; }

	//@todo
	const FxArchiveInternalData* GetInternalData( void ) const;
	void SetInternalDataState( const FxArchiveInternalData* pData );

	/// Serializes an array of built-in arithmetic types.
	/// \param array A pointer to the first element of the array.
	/// \param length The number of elements in the array.
	/// \note This function is only safe to call when the template type \p T is
	/// an arithmetic type (i.e. FxChar, FxInt32, FxReal, etc.).
	template<typename T> void SerializePODArray( T* array, FxSize length )
	{
		if( IsValid() )
		{
			if( IsSaving() )
			{
				// Byte swap if needed.
#ifdef FX_BIG_ENDIAN
				if( ABO_LittleEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#else
				if( ABO_BigEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#endif
				// Save the array.
				_store->Write(reinterpret_cast<FxByte*>(array), length * sizeof(T));
#ifdef FX_BIG_ENDIAN
				if( ABO_LittleEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#else
				if( ABO_BigEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#endif
			}
			else
			{
				// Read the array.
				_store->Read(reinterpret_cast<FxByte*>(array), length * sizeof(T));

				// Byte swap if needed.
#ifdef FX_BIG_ENDIAN
				if( ABO_LittleEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#else
				if( ABO_BigEndian == _byteOrder )
				{
					_byteSwapPODArray(array, length);
				}
#endif
			}
		}
	}

private:
	/// The store this archive is using.
	FxArchiveStore* _store;
	/// The mode of the archive.
	FxArchiveMode _mode;
	/// The byte order of the archive.
	FxArchiveByteOrder _byteOrder;
	/// Whether the archive valid.
	FxBool _isValid;

	/// The FaceFX file format this archive was saved in.
	FxSize _fileFormatVersion;
	/// The version of the FaceFX SDK this archive was saved with.
	FxSize _sdkVersion;
	/// The version of the FaceFX SDK this archive was saved with as a string.
	FxChar* _sdkVersionString;
	/// The size of _sdkVersionString.
	FxSize _sdkVersionStringSize;
	/// The FaceFX SDK licensee name this archive was saved with.
	FxChar* _licenseeName;
	/// The size of _licenseeName.
	FxSize _licenseeNameSize;
	/// The FaceFX SDK licensee project name this archive was saved with.
	FxChar* _licenseeProjectName;
	/// The size of _licenseeProjectName.
	FxSize _licenseeProjectNameSize;
	/// The FaceFX SDK licensee version number this archive was saved with.
	FxSize _licenseeVersion;

	/// Various internal data that cannot be exposed through the public
	/// FxArchive header.  The object table and lazy loader arrays, for 
	/// instance.
	FxArchiveInternalData* _pInternalData;

	/// The progress callback.
	void (FX_CALL *_progressCallback)(FxReal);
	/// The update frequency.
	FxReal _updateFrequency;
	/// The number of bytes loaded.
	FxInt32 _numBytesLoaded;
	/// The total number of bytes in the archive.
	FxReal _numBytesTotal;
	/// The current progress.
	FxReal _progress;
	/// The last time we sent an update.
	FxReal _lastProgress;

	/// Disable copy construction.
	FxArchive( const FxArchive& );
	/// Disable assignment.
	FxArchive& operator=( const FxArchive& );

	/// Byte swaps an array of built-in arithmetic types.
	template<typename T> void _byteSwapPODArray( T* array, FxSize length )
	{
		if( sizeof(T) > 1 )
		{
			for( FxSize i = 0; i < length; ++i )
			{
				FxByteSwap(reinterpret_cast<FxByte*>(&array[i]), sizeof(T));
			}
		}
	}
};

/// Serializes raw data to the archive.
/// \note Internal use only.
template<typename T> FxArchive& FX_CALL SerializeRawData( FxArchive& arc, T& data );

} // namespace Face

} // namespace OC3Ent

#endif
