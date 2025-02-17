/*
	Copyright (C) 2005-2006 Feeling Software Inc.
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FUtils/FUFile.h"

//
// FUFile
//

FUFile::FUFile(const fstring& filename, Mode mode)
:	filePtr(NULL)
{
	Open(filename, mode);
}

FUFile::FUFile(const fchar* filename, Mode mode)
:	filePtr(NULL)
{
	Open(filename, mode);
}

FUFile::FUFile()
{
	filePtr = NULL;
}

FUFile::~FUFile()
{
	if (filePtr != NULL)
	{
		Close();
	}
}

bool FUFile::Open(const fchar* filename, Mode mode)
{
	if (filePtr != NULL) return false;
		
	const fchar* openMode;
	switch (mode)
	{
	case READ: openMode = FC("rb"); break;
	case WRITE: openMode = FC("wb"); break;
	default: openMode = FC("rb"); break;
	}

#ifdef UNICODE
	filePtr = _wfopen(filename, openMode);
#else
	filePtr = fopen(filename, openMode);
#endif // UNICODE

	return filePtr != NULL;
}

// Retrieve the file length
size_t FUFile::GetLength()
{
	FUAssert(IsOpen(), return 0);

	size_t currentPosition = (size_t) ftell(filePtr);
	if (fseek(filePtr, 0, SEEK_END) != 0) return 0;

	size_t length = (size_t) ftell(filePtr);
	if (fseek(filePtr, (long) currentPosition, SEEK_SET) != 0) return 0;

	return length;
}

// Reads in a piece of the file into the given buffer
bool FUFile::Read(void* buffer, size_t length)
{
	FUAssert(IsOpen(), return false);
	return fread(buffer, length, 1, filePtr) == 1;
}

// Write out some data to a file
bool FUFile::Write(const void* buffer, size_t length)
{
	FUAssert(IsOpen(), return false);
	return fwrite(buffer, length, 1, filePtr) == 1;
}

// Flush/close the file stream
void FUFile::Flush()
{
	FUAssert(IsOpen(), );
	fflush(filePtr);
}

void FUFile::Close()
{
	FUAssert(IsOpen(), );
	fclose(filePtr);
	filePtr = NULL;
}
