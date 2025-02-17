//------------------------------------------------------------------------------
// A file-based archive store.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxArchiveStoreFile.h"

namespace OC3Ent
{

namespace Face
{

FxArchiveStoreFile* FxArchiveStoreFile::Create( const FxChar* filename )
{
	return new FxArchiveStoreFile(filename);
}

FxBool FxArchiveStoreFile::IsValid( void ) const
{
	return _file.IsValid();
}

FxBool FxArchiveStoreFile::OpenRead( void )
{
	_file.Open(_filename.GetData(), FxFile::FM_Read | FxFile::FM_Binary);
	return _file.IsValid();
}

FxBool FxArchiveStoreFile::OpenWrite( void )
{
	_file.Open(_filename.GetData(), FxFile::FM_Write | FxFile::FM_Binary);
	return _file.IsValid();
}

FxBool FxArchiveStoreFile::Close( void )
{
	_file.Close();
	return FxTrue;
}

FxBool FxArchiveStoreFile::Read( FxByte* data, FxSize size )
{
	_file.Read(data, size);
	return FxTrue;
}

FxBool FxArchiveStoreFile::Write( const FxByte* data, FxSize size )
{
	_file.Write(data, size);
	return FxTrue;
}

FxInt32 FxArchiveStoreFile::Tell( void ) const
{
	return _file.Tell();
}

void FxArchiveStoreFile::Seek( const FxInt32 pos )
{
	_file.Seek(pos);
}

FxInt32 FxArchiveStoreFile::Length( void ) const
{
	return _file.Length();
}

FxArchiveStoreFile::FxArchiveStoreFile( const FxChar* filename )
	: _filename(filename)
{
}

FxArchiveStoreFile::~FxArchiveStoreFile()
{
	Close();
}

void FxArchiveStoreFile::Destroy( void )
{
	this->~FxArchiveStoreFile();
}

FxSize FxArchiveStoreFile::GetClassSize( void )
{
	return static_cast<FxSize>(sizeof(FxArchiveStoreFile));
}

} // namespace Face

} // namespace OC3Ent
