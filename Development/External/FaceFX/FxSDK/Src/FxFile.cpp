//------------------------------------------------------------------------------
// This class is responsible for abstracting away the platform-dependant aspects
// of file manipulation.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

// Note that on PS3 FxFile does not perform any file I/O as most of its
// implementation is not compiled on PS3.

#include "FxFile.h"

#include "FxMemory.h"
#include <cstdio>

namespace OC3Ent
{

namespace Face
{

FxFile::FxFile()
	: _file(NULL)
	, _length(0)
{
}

FxFile::FxFile( const FxChar* filename, FxInt32 mode )
	: _file(NULL)
{
	Open(filename, mode);
}

FxFile::~FxFile()
{
	Close();
}

FxBool FX_CALL FxFile::Exists( const FxChar* filename )
{
	FxFile file(filename, FxFile::FM_Read);
	return file.IsValid();
}

FxByte* FX_CALL FxFile::ReadBinaryFile( const FxChar* filename, FxSize& numBytes )
{
	FxFile file(filename);
	FxInt32 fileLength = 0;
	FxByte* retval = NULL;

	if( file.IsValid() )
	{
		fileLength = file.Length();
		retval = static_cast<FxByte*>(FxAlloc(fileLength, "ReadBinaryFileChunk"));
		file.Read(retval, fileLength);
	}
	numBytes = static_cast<FxSize>(fileLength);
	return retval;
}

FxBool FxFile::IsValid( void ) const
{
	return _file ? FxTrue : FxFalse;
}

FxBool FxFile::Open( const FxChar* filename, FxInt32 mode )
{
#if !PS3
	FxChar filemode[3] = {0};
	filemode[0] = (mode & FM_Write)  ? 'w' : 'r';
	filemode[1] = (mode & FM_Binary) ? 'b' : 't';
	
#if _MSC_VER >= 1400
	fopen_s(reinterpret_cast<FILE**>(&_file), filename, filemode);
#else
	_file = static_cast<void*>(fx_std(fopen)(filename, filemode));
#endif
	
	// If we're reading, cache the length and reset the file pointer.
	if( IsValid() && mode & FM_Read )
	{
		fx_std(fseek)(static_cast<FILE*>(_file), 0, SEEK_END);
		_length = fx_std(ftell)(static_cast<FILE*>(_file));
		fx_std(fseek)(static_cast<FILE*>(_file), 0, SEEK_SET);
	}
#endif
	return IsValid();
}

void FxFile::Close( void )
{
#if !PS3
	if( _file )
	{
		fx_std(fclose)(static_cast<FILE*>(_file));
	}
#endif
	_file = NULL;
}

FxSize FxFile::Read( FxByte* data, FxSize numBytes )
{
#if !PS3
	if( _file )
	{
		return static_cast<FxSize>(
		   fx_std(fread)(static_cast<void*>(data), sizeof(FxByte), numBytes, 
		                 static_cast<FILE*>(_file)));
	}
#endif
	return 0;
}

FxSize FxFile::Write( const FxByte* data, FxSize numBytes )
{
#if !PS3
	if( _file )
	{
		return static_cast<FxSize>(
		   fx_std(fwrite)(static_cast<const void*>(data), sizeof(FxByte), numBytes,
				          static_cast<FILE*>(_file)));
	}
#endif
	return 0;
}

FxInt32 FxFile::Tell( void ) const
{
#if !PS3
	if( _file )
	{
		return fx_std(ftell)(static_cast<FILE*>(_file));
	}
#endif
	return 0;
}

void FxFile::Seek( const FxInt32 pos )
{
#if !PS3
	if( _file )
	{
		fx_std(fseek)(static_cast<FILE*>(_file), pos, SEEK_SET);
	}
#endif
}

FxInt32 FxFile::Length( void ) const
{
	return _length;
}

} // namespace Face

} // namespace OC3Ent
