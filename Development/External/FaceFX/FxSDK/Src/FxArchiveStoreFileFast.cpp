//------------------------------------------------------------------------------
// A very fast file-based archive store.  It loads the entire file into a 
// memory store with one file read and serialization happens from the memory 
// store from that point forward.  Note that the entire file is in memory after
// this operation, so the effect of lazy-loading is zero (that is, all objects
// are stored in memory after this operation even if they haven't been
// "serialized" yet).  This archive store is only used for loading and prevents
// saving.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArchiveStoreFileFast.h"
#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

FxArchiveStoreFileFast* 
FxArchiveStoreFileFast::Create( const FxChar* filename )
{
	return new FxArchiveStoreFileFast(filename);
}

FxBool FxArchiveStoreFileFast::IsValid( void ) const
{
	if( _pMemoryStore )
	{
		return (_pMemoryStore->IsValid() && FxArchiveStoreFile::IsValid());
	}
	return FxFalse;
}

FxBool FxArchiveStoreFileFast::OpenRead( void )
{
	FxBool returnValue = FxFalse;
	if( FxArchiveStoreFile::OpenRead() )
	{
		_rawFileSize = static_cast<FxSize>(FxArchiveStoreFile::Length());
		_pRawFileMemory = static_cast<FxByte*>(FxAlloc(_rawFileSize * sizeof(FxByte), "FxArchiveStoreFileFast::OpenRead()"));
		if( FxArchiveStoreFile::Read(_pRawFileMemory, _rawFileSize) )
		{
			_pMemoryStore = FxArchiveStoreMemoryNoCopy::Create(_pRawFileMemory, _rawFileSize);
			returnValue = FxTrue;
		}
	}
	return returnValue;
}

FxBool FxArchiveStoreFileFast::OpenWrite( void )
{
	return FxFalse;
}

FxBool FxArchiveStoreFileFast::Close( void )
{
	if( _pMemoryStore )
	{
		_pMemoryStore->Destroy();
		FxFree(_pMemoryStore, sizeof(FxArchiveStoreMemoryNoCopy));
		_pMemoryStore = NULL;
	}
	if( _pRawFileMemory )
	{
		FxFree(_pRawFileMemory, _rawFileSize);
		_pRawFileMemory = NULL;
		_rawFileSize = 0;
	}
	return FxArchiveStoreFile::Close();
}

FxBool FxArchiveStoreFileFast::Read( FxByte* data, FxSize size )
{
	if( _pMemoryStore )
	{
		return _pMemoryStore->Read(data, size);
	}
	return FxFalse;
}

FxBool 
FxArchiveStoreFileFast::Write( const FxByte* FxUnused(data), FxSize FxUnused(size) )
{
	return FxFalse;
}

FxInt32 FxArchiveStoreFileFast::Tell( void ) const
{
	if( _pMemoryStore )
	{
		return _pMemoryStore->Tell();
	}
	return 0;
}

void FxArchiveStoreFileFast::Seek( const FxInt32 pos )
{
	if( _pMemoryStore )
	{
		_pMemoryStore->Seek(pos);
	}
}

FxInt32 FxArchiveStoreFileFast::Length( void ) const
{
	if( _pMemoryStore )
	{
		return _pMemoryStore->Length();
	}
	return 0;
}

FxArchiveStoreFileFast::FxArchiveStoreFileFast( const FxChar* filename )
	: FxArchiveStoreFile(filename)
	, _pRawFileMemory(NULL)
	, _rawFileSize(0)
	, _pMemoryStore(NULL)
{
}

FxArchiveStoreFileFast::~FxArchiveStoreFileFast()
{
	Close();
}

void FxArchiveStoreFileFast::Destroy( void )
{
	this->~FxArchiveStoreFileFast();
}

FxSize FxArchiveStoreFileFast::GetClassSize( void )
{
	return static_cast<FxSize>(sizeof(FxArchiveStoreFileFast));
}

} // namespace Face

} // namespace OC3Ent
