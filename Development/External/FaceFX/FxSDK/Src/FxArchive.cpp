//------------------------------------------------------------------------------
// This class implements a FaceFx archive.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArchive.h"
#include "FxArray.h"
#include "FxStringHash.h"
#include "FxObject.h"
#include "FxLazyLoader.h"
#include "FxString.h"
#include "FxVersionInfo.h"
#include "FxSDK.h"
#include "FxUtil.h"
#include "FxArchiveStoreMemory.h"

namespace OC3Ent
{

namespace Face
{

// Macro to issue progress updates.
#define UPDATE_PROGRESS(size) \
if( _progressCallback ) \
{ \
	_numBytesLoaded += size; \
	_progress = static_cast<FxReal>(_numBytesLoaded) / _numBytesTotal; \
	if( _progress - _lastProgress >= _updateFrequency ) \
	{ \
		_progressCallback(_progress); \
		_lastProgress = _progress; \
	} \
}

// Currently loaded archives.
static FxArray<FxArchive*>* pMountedArchives = NULL;

// A name entry in the archive's internal "name table."
#define kCurrentFxArchiveNameEntryVersion 0
class FxArchiveNameEntry
{
public:
	// Constructors.
	FxArchiveNameEntry() {}
	FxArchiveNameEntry( const FxString& name ) : NameStr(name) {}
	// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, FxArchiveNameEntry& name );
	// The name (as an FxString).
	FxString NameStr;
	// The actual name.
	FxName Name;
};
FxArchive& operator<<( FxArchive& arc, FxArchiveNameEntry& name )
{
	// Eat the version on pre-1.7 archives.
	if( arc.GetSDKVersion() < 1700 && arc.IsLoading() )
	{
		FxUInt16 version = 0;
		arc << version;
	}

	arc << name.NameStr;

	return arc;
}

// The "directory" information for an archive.  This is a header structure that
// is at the very beginning of all valid FaceFX archives beginning with version
// 1.6.  It is called a "directory" because in the future it
// may actually contain information related to objects and their locations
// contained in the archive.  For now it simply maintains a list of the names
// (as strings) stored in the archive as well as the total number of FxObject
// pointers stored in the archive.
#define kCurrentFxArchiveDirectoryVersion 1
class FxArchiveDirectory
{
public:
	// Constructor.
	FxArchiveDirectory() {}
	// Destructor.
	~FxArchiveDirectory() {}
	// The names (as strings) stored in the archive.
	FxArray<FxArchiveNameEntry> Names;
	// The total number of FxObject pointers stored in the archive.
	FxSize NumObjectPointers;
	// The versions for each class in the class table at the time the archive
	// was created.  It contains 2^7 (128) hash bins.
	FxStringHash<FxUInt16, 7> ClassVersions;

	// Finds the name in the Names array returning its index or FxInvalidIndex
	// if it was not found.
	FxSize FindName( const FxString& name ) const
	{
		FxSize numNames = Names.Length();
		for( FxSize i = 0; i < numNames; ++i )
		{
			if( Names[i].NameStr == name )
			{
				return i;
			}
		}
		return FxInvalidIndex;
	}

	// Initializes the names stored in the directory.  This should only be 
	// called after the directory has been fully loaded from the archive but
	// before the rest of the serialization happens.
	void InitializeNames( void )
	{
		FxSize numNames = Names.Length();
		for( FxSize i = 0; i < numNames; ++i )
		{
			Names[i].Name = FxName(Names[i].NameStr.GetData());
		}
	}

	void InsertClassVersion( const FxChar* classname )
	{
		const FxClass* pClass = FxClass::FindClassDesc(classname);
		FxAssert(pClass);
		FxUInt16 classversion = pClass->GetCurrentVersion();
		ClassVersions.Insert(classname, classversion);
	}

	void InsertNonFxObjectDerivedClassVersion( const FxChar* classname, FxUInt16 classversion )
	{
		ClassVersions.Insert(classname, classversion);
	}

	FxUInt16 FindClassVersion( const FxChar* classname ) const
	{
		FxUInt16 classversion = 0;
		FxBool foundClass = FxFalse;
		foundClass = ClassVersions.Find(classname, classversion);
		FxAssert(foundClass);
		return classversion;
	}

	// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, FxArchiveDirectory& directory );
};
FxArchive& operator<<( FxArchive& arc, FxArchiveDirectory& directory )
{
	FxUInt16 version = kCurrentFxArchiveDirectoryVersion;
	arc << version;

	if( version >= 1 )
	{
		arc << directory.ClassVersions;
	}
	arc << directory.Names << directory.NumObjectPointers;

	if( arc.IsLoading() )
	{
		// Remap FxDigitalAudio to FxAudio.
		if( directory.ClassVersions.Find("FxDigitalAudio") )
		{
			FxAssert(!directory.ClassVersions.Find("FxAudio"));
			FxUInt16 FxDigitalAudioVersion = 0;
			directory.ClassVersions.Remove("FxDigitalAudio", FxDigitalAudioVersion);
			directory.ClassVersions.Insert("FxAudio", FxDigitalAudioVersion);
		}
		directory.InitializeNames();
	}

	return arc;
}

// Internal data that can't be exposed through the public FxArchive.h header.
class FxArchiveInternalData
{
public:
	// Constructor.
	FxArchiveInternalData() {}
	// Destructor.
	~FxArchiveInternalData() {}
	// The object table.
	FxArray<FxObject*> ObjectTable;
	// The lazy loaders.
	FxArray<FxLazyLoader*> LazyLoaders;
	// The archive directory.
	FxArchiveDirectory Directory;
};

FxArchive::FxArchive( FxArchiveStore* iStore, FxArchiveMode mode, 
					  FxArchiveByteOrder byteOrder )
	: _store(iStore)
	, _mode(mode)
	, _byteOrder(byteOrder)
	, _isValid(FxFalse)
	, _fileFormatVersion(0)
	, _sdkVersion(0)
	, _sdkVersionString(NULL)
	, _sdkVersionStringSize(0)
	, _licenseeName(NULL)
	, _licenseeNameSize(0)
	, _licenseeProjectName(NULL)
	, _licenseeProjectNameSize(0)
	, _licenseeVersion(0)
	, _progressCallback(NULL)
	, _updateFrequency(0.0f)
	, _numBytesLoaded(0)
	, _progress(0.0f)
	, _lastProgress(0.0f)
{
}

FxArchive::~FxArchive()
{
	Close();
}

void FxArchive::Open( void )
{
	_pInternalData = static_cast<FxArchiveInternalData*>(FxAlloc(sizeof(FxArchiveInternalData), "FxArchiveInternalData"));
	FxDefaultConstruct(_pInternalData);

	if( _store )
	{
		if( IsSaving() )
		{
#ifdef NO_SAVE_VERSION
			_isValid = FxFalse;
#else
			// Set the version information.
			_fileFormatVersion       = FxFileFormatVersion;
			_sdkVersion              = FxSDKGetVersion();
			FxTempCharBuffer<FxChar> licenseeNameTempBuffer(FxSDKGetLicenseeName().GetData());
			_licenseeName            = licenseeNameTempBuffer.Release();
			_licenseeNameSize        = licenseeNameTempBuffer.GetSize();
			FxTempCharBuffer<FxChar> licenseeProjectNameTempBuffer(FxSDKGetLicenseeProjectName().GetData());
			_licenseeProjectName     = licenseeProjectNameTempBuffer.Release();
			_licenseeProjectNameSize = licenseeProjectNameTempBuffer.GetSize();
			_licenseeVersion         = FxSDKGetLicenseeVersion();
			// Open the archive store for writing.
			_store->OpenWrite();
			if( _store->IsValid() )
			{
				// Temporarily set that the archive is valid so the serialization 
				// can proceed.
				_isValid = FxTrue;
			}
			else
			{
				// Don't try to write to the invalid store.
				return;
			}
			// Write the archive id.
			FxChar archiveId[4];
			archiveId[0] = 'F';
			archiveId[1] = 'A';
			archiveId[2] = 'C';
			if( ABO_LittleEndian == _byteOrder )
			{
				archiveId[3] = 'E';
				_byteOrder = ABO_LittleEndian;
			}
			else // ABO_BigEndian == _byteOrder.
			{
				archiveId[3] = 'B';
				_byteOrder = ABO_BigEndian;
			}
			*this << archiveId[0];
			*this << archiveId[1];
			*this << archiveId[2];
			*this << archiveId[3];
			// Write the version information to the archive.
			*this << _sdkVersion;
			*this << _fileFormatVersion;
			*this << _licenseeNameSize;
			SerializePODArray(_licenseeName, _licenseeNameSize);
			*this << _licenseeProjectNameSize;
			SerializePODArray(_licenseeProjectName, _licenseeProjectNameSize);
			*this << _licenseeVersion;
			_numBytesTotal = 0.0f;
			_isValid = _store->IsValid();
#endif // NO_SAVE_VERSION
		}
		else
		{
			// Open the archive store for reading.
			_store->OpenRead();
			if( _store->IsValid() )
			{
				// Temporarily set that the archive is valid so the serialization 
				// can proceed.
				_isValid = FxTrue;
			}
			_numBytesTotal = static_cast<FxReal>(_store->Length());
			// We require at least 4 bytes to be a valid archive.
			if( _numBytesTotal > 4 )
			{
				// Read the archive id.
				FxChar archiveId[4];
				*this << archiveId[0];
				*this << archiveId[1];
				*this << archiveId[2];
				*this << archiveId[3];
				if( archiveId[0]  == 'F' && archiveId[1] == 'A' && 
					archiveId[2]  == 'C' && 
					(archiveId[3] == 'E' || archiveId[3] == 'B') )
				{
					if( archiveId[3] == 'E' )
					{
						_byteOrder = ABO_LittleEndian;
					}
					else // archiveId[3] == 'B'.
					{
						_byteOrder = ABO_BigEndian;
					}

					// Read the version information from the archive.
					*this << _sdkVersion;

					FxBool canLoadData = FxFalse;
					// Version 1.7.2 introduced the file format version.
					if( _sdkVersion >= 1720 )
					{
						*this << _fileFormatVersion;
						if( _fileFormatVersion <= FxFileFormatVersion )
						{
							// It's safe to load data from this archive.
							canLoadData = FxTrue;
						}
						else
						{
							// If the file format version of the archive is 
							// greater than the current file format version we 
							// cannot possibly serialize correctly, so bail 
							// gracefully.
							_store->Close();
							_numBytesTotal = 0.0f;
							FxAssert(!"Attempt to load archive saved with newer file format version!");
						}
					}
					else
					{
						if( _sdkVersion <= FxSDKGetVersion() )
						{
							// It's safe to load data from this archive.
							canLoadData = FxTrue;
						}
						else
						{
							// If the version of the archive is greater than the current
							// SDK version we cannot possibly serialize correctly, so
							// bail gracefully.
							_store->Close();
							_numBytesTotal = 0.0f;
							FxAssert(!"Attempt to load archive saved with newer version of FaceFX SDK!");
						}
					}

					// Load the data from the archive if it is safe to do so.
					if( canLoadData )
					{
						if( _sdkVersion < 1700 )
						{
							// Eat the old FxString version.
							FxUInt16 version = 0;
							*this << version;
						}
						*this << _licenseeNameSize;
						// +1 for NULL.
						_licenseeName = static_cast<FxAChar*>(FxAlloc(sizeof(FxAChar)*(_licenseeNameSize+1), "FxArchiveLicenseeName"));
						SerializePODArray(_licenseeName, _licenseeNameSize);
						_licenseeName[_licenseeNameSize] = 0;
						_licenseeNameSize++;
						if( _sdkVersion < 1700 )
						{
							// Eat the old FxString version.
							FxUInt16 version = 0;
							*this << version;
						}
						*this << _licenseeProjectNameSize;
						// +1 for NULL.
						_licenseeProjectName = static_cast<FxAChar*>(FxAlloc(sizeof(FxAChar)*(_licenseeProjectNameSize+1), "FxArchiveLicenseeProjectName"));
						SerializePODArray(_licenseeProjectName, _licenseeProjectNameSize);
						_licenseeProjectName[_licenseeProjectNameSize] = 0;
						_licenseeProjectNameSize++;
						*this << _licenseeVersion;

						// Check for the presence of the archive directory.
						// Archive directories were added with version 1.6.
						if( _sdkVersion >= 1600 )
						{
							*this << _pInternalData->Directory;
							_pInternalData->ObjectTable.Reserve(_pInternalData->Directory.NumObjectPointers);
						}
					}
				}
				else
				{
					_store->Close();
					_numBytesTotal = 0.0f;
				}
			}
			else
			{
				_store->Close();
				_numBytesTotal = 0.0f;
			}
			_isValid = _store->IsValid();
		}
	}
}

void FxArchive::Close( void )
{
	if( _sdkVersionString )
	{
		FxFree(_sdkVersionString, _sdkVersionStringSize);
		_sdkVersionString = NULL;
		_sdkVersionStringSize = 0;
	}
	if( _licenseeName )
	{
		FxFree(_licenseeName, _licenseeNameSize);
		_licenseeName = NULL;
		_licenseeNameSize = 0;
	}
	if( _licenseeProjectName )
	{
		FxFree(_licenseeProjectName, _licenseeProjectNameSize);
		_licenseeProjectName = NULL;
		_licenseeProjectNameSize = 0;
	}
	FlushLazyLoaders();
	FxDelete(_pInternalData);
	if( _store )
	{
		FxSize size = _store->GetClassSize();
		_store->Close();
		_store->Destroy();
		FxFree(_store, size);
		_store = NULL;
	}

	_isValid = FxFalse;
}

void FX_CALL FxArchive::Startup( void )
{
	if( !pMountedArchives )
	{
		pMountedArchives = static_cast<FxArray<FxArchive*>*>(FxAlloc(sizeof(FxArray<FxArchive*>), "MountedArchiveList"));
		FxDefaultConstruct(pMountedArchives);
	}
}

void FX_CALL FxArchive::Shutdown( void )
{
	UnmountAllArchives();
	FxDelete(pMountedArchives);
	pMountedArchives = NULL;
}

const FxChar* FxArchive::GetSDKVersionString( void )
{
	if( _sdkVersionString )
	{
		FxFree(_sdkVersionString, _sdkVersionStringSize);
		_sdkVersionString = NULL;
		_sdkVersionStringSize = 0;
	}
	FxTempCharBuffer<FxChar> sdkVersionStringTempBuffer(FxSDKConvertVersionToString(_sdkVersion).GetData());
	_sdkVersionString        = sdkVersionStringTempBuffer.Release();
	_sdkVersionStringSize    = sdkVersionStringTempBuffer.GetSize();
	return _sdkVersionString;
}

void FxArchive::RegisterProgressCallback( void(FX_CALL *callbackFunction)(FxReal), 
										  FxReal updateFrequency )
{
	if( IsLoading() )
	{
		_progressCallback = callbackFunction;
		_updateFrequency  = updateFrequency;
	}
}

FxObject* FxArchive::FindObject( FxSize index ) const
{
	if( index != FxInvalidIndex && 
		index < _pInternalData->ObjectTable.Length() )
	{
		return _pInternalData->ObjectTable[index];
	}
	return NULL;
}

FxSize FxArchive::FindObject( FxObject* object ) const
{
	return _pInternalData->ObjectTable.Find(object);
}

FxSize FxArchive::AddObject( FxObject* object, FxSize index )
{
	if( object )
	{
		// In the default case of an invalid index, simply add the
		// object to the end of the table if it isn't already
		// in the table.
		if( index == FxInvalidIndex )
		{
			FxSize foundObjectIndex = FindObject(object);
			if( foundObjectIndex != FxInvalidIndex )
			{
				return foundObjectIndex;
			}
			else
			{
				FxSize objectLocation = _pInternalData->ObjectTable.Length();
				_pInternalData->ObjectTable.PushBack(object);
				return objectLocation;
			}
		}
		else
		{
			// Run up to where this object should be, filling in the gap with
			// NULL pointers.
			if( index >= _pInternalData->ObjectTable.Length() )
			{
				FxSize numToAdd = index == 0 ? 
					1 : index - _pInternalData->ObjectTable.Length() + 1;
				for( FxSize i = 0; i < numToAdd; ++i )
				{
					_pInternalData->ObjectTable.PushBack(NULL);
				}
			}
			// Add the object to this slot in the table.
			_pInternalData->ObjectTable[index] = object;
			return index;
		}
	}
	return FxInvalidIndex;
}

FxObject* FxArchive::ReplaceObject( FxObject* object, FxSize index )
{
	FxObject* cachedObject = NULL;
	if( FxInvalidIndex != index )
	{
		// Run up to where this object should be, filling in the gap with
		// NULL pointers.
		if( index >= _pInternalData->ObjectTable.Length() )
		{
			FxSize numToAdd = index == 0 ? 
				1 : index - _pInternalData->ObjectTable.Length() + 1;
			for( FxSize i = 0; i < numToAdd; ++i )
			{
				_pInternalData->ObjectTable.PushBack(NULL);
			}
		}
		// Cache the previous object in this slot.
		cachedObject = _pInternalData->ObjectTable[index];
		// Add the object to this slot in the table.
		_pInternalData->ObjectTable[index] = object;
	}
	// Return the cached object.
	return cachedObject;
}

FxSize FxArchive::AddName( void* name )
{
	FxAssert(name != NULL);
	FxString* pString = reinterpret_cast<FxString*>(name);
	FxSize index = FxInvalidIndex;
	if( pString )
	{
		index = _pInternalData->Directory.FindName(*pString);
		if( FxInvalidIndex == index )
		{
			_pInternalData->Directory.Names.PushBack(FxArchiveNameEntry(*pString));
			index = _pInternalData->Directory.Names.Length() - 1;
		}
	}
	FxAssert(FxInvalidIndex != index);
	return index;
}

void** FxArchive::GetName( FxSize index )
{
	return reinterpret_cast<void**>(_pInternalData->Directory.Names[index].Name.GetRefString());
}

FxUInt16 FxArchive::SerializeClassVersion( const FxChar* classname, 
										   FxBool bIsNonFxObjectDerived, 
										   FxUInt16 classversion  )
{
	// When both loading and saving in version 1.7+ the class versions come from
	// the class version table in the archive directory.
	FxUInt16 version = 0;
	if( _sdkVersion >= 1700 )
	{
		if( IsSaving() )
		{
			// If saving insert the current version of the class.  This ensures
			// that only the class version information for object types stored
			// in the archive are in the archive's class version table and not
			// all classes currently registered.  This will not add duplicate
			// entries.
			if( bIsNonFxObjectDerived )
			{
				_pInternalData->Directory.InsertNonFxObjectDerivedClassVersion(classname, classversion);
			}
			else
			{
				_pInternalData->Directory.InsertClassVersion(classname);
			}
		}
		version = _pInternalData->Directory.FindClassVersion(classname);
	}
	else
	{
		// We *must* be loading here.
		FxAssert(IsLoading());
		// Prior to version 1.7, each object had its class version serialized in 
		// place inside the archive.  So when loading archives created prior to
		// version 1.7 the version needs to be read directly from the archive in 
		// place.
		*this << version;
	}
	return version;
}

FxInt32 FxArchive::Tell( void ) const
{
	if( _store )
	{
		return _store->Tell();
	}
	return 0;
}

void FxArchive::Seek( const FxInt32 pos )
{
	if( _store )
	{
		_store->Seek(pos);
	}
}

void FX_CALL FxArchive::MountArchive( FxArchive* arc )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to mount archive before FxArchive::Startup() called!");
	}
	else
	{
		pMountedArchives->PushBack(arc);
	}
}

void FX_CALL FxArchive::UnmountArchive( FxSize index, FxBool load )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to unmount archive before FxArchive::Startup() called!");
	}
	else
	{
		FxArchive* pArchive = (*pMountedArchives)[index];
		if( pArchive )
		{
			pArchive->FlushLazyLoaders(load);
			FxDelete(pArchive);
			(*pMountedArchives)[index] = NULL;
		}
	}
}

FxSize FX_CALL FxArchive::GetNumMountedArchives( void )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to use mounted archives before FxArchive::Startup() called!");
		return 0;
	}
	else
	{
		return pMountedArchives->Length();
	}
}

void FX_CALL FxArchive::UnmountAllArchives( FxBool load )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to use mounted archives before FxArchive::Startup() called!");
	}
	else
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			UnmountArchive(i, load);
		}
		pMountedArchives->Clear();
	}
}

void FX_CALL FxArchive::ForceLoadAllMountedArchives( void )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to use mounted archives before FxArchive::Startup() called!");
	}
	else
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			FxArchive* pArchive = (*pMountedArchives)[i];
			if( pArchive )
			{
				FxSize numLazyLoaders = pArchive->_pInternalData->LazyLoaders.Length();
				for( FxSize j = 0; j < numLazyLoaders; ++j )
				{
					FxLazyLoader* lazyLoader = pArchive->_pInternalData->LazyLoaders[j];
					if( lazyLoader )
					{
						lazyLoader->Load();
					}
				}
			}
		}
	}
}

void FX_CALL FxArchive::ForceUnloadAllMountedArchives( void )
{
	if( !pMountedArchives )
	{
		FxAssert(!"Attempt to use mounted archives before FxArchive::Startup() called!");
	}
	else
	{
		FxSize numMountedArchives = pMountedArchives->Length();
		for( FxSize i = 0; i < numMountedArchives; ++i )
		{
			FxArchive* pArchive = (*pMountedArchives)[i];
			if( pArchive )
			{
				FxSize numLazyLoaders = pArchive->_pInternalData->LazyLoaders.Length();
				for( FxSize j = 0; j < numLazyLoaders; ++j )
				{
					FxLazyLoader* lazyLoader = pArchive->_pInternalData->LazyLoaders[j];
					if( lazyLoader )
					{
						lazyLoader->Unload();
					}
				}
			}
		}
	}
}

void FxArchive::AddLazyLoader( FxLazyLoader* lazyLoader )
{
	if( lazyLoader )
	{
		if( lazyLoader->_archive == NULL )
		{
			lazyLoader->_archive = this;
			lazyLoader->_pos = _store->Tell();
		}
		_pInternalData->LazyLoaders.PushBack(lazyLoader);
	}
}

void FxArchive::RemoveLazyLoader( FxLazyLoader* lazyLoader )
{
	FxSize index = _pInternalData->LazyLoaders.Find(lazyLoader);
	if( index != FxInvalidIndex )
	{
		_pInternalData->LazyLoaders.Remove(index);
		lazyLoader->_archive = NULL;
		lazyLoader->_pos = 0;
	}
}

void FxArchive::FlushLazyLoaders( FxBool load )
{
	FxSize numLazyLoaders = _pInternalData->LazyLoaders.Length();
	for( FxSize i = 0; i < numLazyLoaders; ++i )
	{
		FxLazyLoader* lazyLoader = _pInternalData->LazyLoaders[i];
		if( load )
		{
			lazyLoader->Load();
		}
		lazyLoader->_archive = NULL;
		lazyLoader->_pos = 0;
	}
	_pInternalData->LazyLoaders.Clear();
}

FxArchive& FxArchive::operator<<( FxInt8& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxUInt8& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxInt16& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxUInt16& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxInt32& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxUInt32& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxReal& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

FxArchive& FxArchive::operator<<( FxDReal& elem )
{
	SerializeRawData(*this, elem);
	UPDATE_PROGRESS(sizeof(elem));
	return *this;
}

#ifdef FX_USE_BUILT_IN_BOOL_TYPE
	FxArchive& FxArchive::operator<<( FxBool& elem )
	{
		FxUInt32 temp = elem == FxTrue ? 1 : 0;
		SerializeRawData(*this, temp);
		elem = temp == 1 ? FxTrue : FxFalse;
		UPDATE_PROGRESS(sizeof(elem));
		return *this;
	}
#endif

const FxArchiveInternalData* FxArchive::GetInternalData( void ) const
{
	FxAssert(AM_CreateDirectory == _mode);
	return _pInternalData;
}

void FxArchive::SetInternalDataState( const FxArchiveInternalData* pData )
{
	FxAssert(pData);
	FxAssert(_pInternalData);
	FxAssert(AM_Save == _mode);
	_pInternalData->Directory.ClassVersions = pData->Directory.ClassVersions;
	_pInternalData->Directory.Names = pData->Directory.Names;
	_pInternalData->Directory.NumObjectPointers = pData->ObjectTable.Length();
	*this << _pInternalData->Directory;
}

#ifdef _MSC_VER
	#pragma warning( push )
	// Conditional expression is constant (sizeof(T) > 1).
	#pragma warning( disable : 4127 ) 
#endif

template<typename T> FxArchive& FX_CALL SerializeRawData( FxArchive& arc, T& data )
{
	if( arc.IsValid() )
	{
		if( arc.IsSaving() )
		{
#ifndef NO_SAVE_VERSION
	#ifdef FX_BIG_ENDIAN
			if( FxArchive::ABO_LittleEndian == arc.GetByteOrder() )
			{
				if( sizeof(T) > 1 )
				{
					T temp = data;
					FxByteSwap(reinterpret_cast<FxByte*>(&temp), sizeof(T));
					arc.GetStore()->Write(reinterpret_cast<FxByte*>(&temp), sizeof(T));
				}
				else
				{
					arc.GetStore()->Write(reinterpret_cast<FxByte*>(&data), sizeof(T));
				}
			}
			else
			{
				arc.GetStore()->Write(reinterpret_cast<FxByte*>(&data), sizeof(T));
			}
	#else
			if( FxArchive::ABO_BigEndian == arc.GetByteOrder() )
			{
				if( sizeof(T) > 1 )
				{
					T temp = data;
					FxByteSwap(reinterpret_cast<FxByte*>(&temp), sizeof(T));
					arc.GetStore()->Write(reinterpret_cast<FxByte*>(&temp), sizeof(T));
				}
				else
				{
					arc.GetStore()->Write(reinterpret_cast<FxByte*>(&data), sizeof(T));
				}
			}
			else
			{
				arc.GetStore()->Write(reinterpret_cast<FxByte*>(&data), sizeof(T));
			}
	#endif
#endif
		}
		else
		{
			arc.GetStore()->Read(reinterpret_cast<FxByte*>(&data), sizeof(T));
#ifdef FX_BIG_ENDIAN
			if( FxArchive::ABO_LittleEndian == arc.GetByteOrder() )
			{
				if( sizeof(T) > 1 )
				{
					FxByteSwap(reinterpret_cast<FxByte*>(&data), sizeof(T));
				}
			}
#else
			if( FxArchive::ABO_BigEndian == arc.GetByteOrder() )
			{
				if( sizeof(T) > 1 )
				{
					FxByteSwap(reinterpret_cast<FxByte*>(&data), sizeof(T));
				}
			}
#endif
		}
	}
	return arc;
}

#ifdef _MSC_VER
	#pragma warning( pop )
#endif

} // namespace Face

} // namespace OC3Ent
