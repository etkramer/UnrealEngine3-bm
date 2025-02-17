/*=============================================================================
    FFileManagerXenon.h: Unreal Xenon file manager.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"
#include "FFileManagerXenon.h"
#include "XeFileCaching.h"

#include <sys/types.h>
#include <sys/stat.h>

UBOOL GEnableHDDCaching = TRUE;

/*-----------------------------------------------------------------------------
    File Manager.
-----------------------------------------------------------------------------*/

// File manager.
FArchiveFileReaderXenon::FArchiveFileReaderXenon( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InSize, INT InDebugTrackingNumber )
:   Handle          ( InHandle )
,	StatsHandle		( InStatsHandle )
#if !FINAL_RELEASE
,	Filename		( InFilename )
#endif
,   Error           ( InError )
,   Size            ( InSize )
,   Pos             ( 0 )
,   BufferBase      ( 0 )
,   BufferCount     ( 0 )
,	DebugTrackingNumber( InDebugTrackingNumber )
,	Cache			( NULL )
{
    ArIsLoading = ArIsPersistent = 1;

	// create the HDD cache helper
	if ( GEnableHDDCaching )
	{
		Cache = GHDDCacheManager->GetCache(InFilename);
	}
}

FArchiveFileReaderXenon::~FArchiveFileReaderXenon()
{
	if( Handle )
	{
		Close();
	}

	if ( Cache )
	{
		delete Cache;
	}
}

UBOOL FArchiveFileReaderXenon::InternalPrecache( INT PrecacheOffset, INT PrecacheSize )
{
	UBOOL bSuccess = TRUE;

	// Only precache at current position and avoid work if precaching same offset twice.
	if( Pos == PrecacheOffset && (!BufferBase || !BufferCount || BufferBase != Pos) )
	{
		BufferBase = Pos;
		BufferCount = Min( Min( PrecacheSize, (INT)(ARRAY_COUNT(Buffer) - (Pos&(ARRAY_COUNT(Buffer)-1))) ), Size-Pos );
		if ( GEnableHDDCaching )
		{
			SCOPED_FILE_IO_READ_STATS( StatsHandle, BufferCount, Pos );
			bSuccess = Cache->Serialize( Buffer, BufferCount );
		}
		else
		{
			// Read data from device via Win32 ReadFile API.
			SCOPED_FILE_IO_READ_STATS( StatsHandle, BufferCount, Pos );
			INT Count = 0;
			ReadFile( Handle, Buffer, BufferCount, (DWORD*)&Count, NULL );
			bSuccess = Count == BufferCount;
		}
		if( !bSuccess )
		{
			ArIsError = 1;
			Error->Logf( TEXT("ReadFile failed: BufferCount=%i Error=%s"), BufferCount, appGetSystemErrorMessage() );
			appHandleIOFailure( NULL );
		}
	}
	return bSuccess;
}

void FArchiveFileReaderXenon::Seek( INT InPos )
{
    check(InPos>=0);
    check(InPos<=Size);
	UBOOL bSuccess = TRUE;
	if ( GEnableHDDCaching )
	{
		bSuccess = Cache->Seek( InPos );
	}
	else
	{
		bSuccess = SetFilePointer( Handle, InPos, 0, FILE_BEGIN ) == INVALID_SET_FILE_POINTER;
	}
    if ( !bSuccess )
    {
        ArIsError = 1;
        Error->Logf( TEXT("SetFilePointer Failed %i/%i: %i %s"), InPos, Size, Pos, appGetSystemErrorMessage() );
		appHandleIOFailure( NULL );
    }
    Pos         = InPos;
    BufferBase  = Pos;
    BufferCount = 0;
}

INT FArchiveFileReaderXenon::Tell()
{
    return Pos;
}

INT FArchiveFileReaderXenon::TotalSize()
{
    return Size;
}

UBOOL FArchiveFileReaderXenon::Close()
{
    if( Handle )
	{
        CloseHandle( Handle );
	}
    Handle = NULL;
    return !ArIsError;
}

void FArchiveFileReaderXenon::Serialize( void* V, INT Length )
{
    while( Length>0 )
    {
        INT Copy = Min( Length, BufferBase+BufferCount-Pos );
        if( Copy==0 )
        {
            if( Length >= ARRAY_COUNT(Buffer) )
            {
				UBOOL bSuccess = TRUE;
				if ( GEnableHDDCaching )
				{
					SCOPED_FILE_IO_READ_STATS( StatsHandle, Length, Pos );
					bSuccess = Cache->Serialize( V, Length );
				}
				else
				{
					// Read data from device via Win32 ReadFile API.
					SCOPED_FILE_IO_READ_STATS( StatsHandle, Length, Pos );
	                INT Count = 0;
	                ReadFile( Handle, V, Length, (DWORD*)&Count, NULL );
					bSuccess = Count == Length;
				}
                if ( !bSuccess )
                {
                    ArIsError = 1;
                    Error->Logf( TEXT("ReadFile failed: Length=%i Error=%s"), Length, appGetSystemErrorMessage() );
					appHandleIOFailure( NULL );
                }

                Pos += Length;
                BufferBase += Length;
                return;
            }
			InternalPrecache( Pos, MAXINT );
            Copy = Min( Length, BufferBase+BufferCount-Pos );
            if( Copy<=0 )
            {
                ArIsError = 1;
                Error->Logf( TEXT("ReadFile beyond EOF %i+%i/%i for file %s"), 
					Pos, Length, Size, *Filename );
            }
            if( ArIsError )
                return;
        }
        appMemcpy( V, Buffer+Pos-BufferBase, Copy );
        Pos       += Copy;
        Length    -= Copy;
        V          = (BYTE*)V + Copy;
    }
}

FArchiveFileWriterXenon::FArchiveFileWriterXenon( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InPos )
:   Handle      ( InHandle )
,	StatsHandle ( InStatsHandle )
#if !FINAL_RELEASE
,	Filename	( InFilename )
#endif
,   Error       ( InError )
,   Pos         ( InPos )
,   BufferCount ( 0 )
{
    ArIsSaving = ArIsPersistent = 1;
}

FArchiveFileWriterXenon::~FArchiveFileWriterXenon()
{
	FILE_IO_STATS_CLOSE_HANDLE( StatsHandle );
    if( Handle )
	{
        Close();
	}
    Handle = NULL;
}

void FArchiveFileWriterXenon::Seek( INT InPos )
{
    Flush();
    if( SetFilePointer( Handle, InPos, 0, FILE_BEGIN )==0xFFFFFFFF )
    {
        ArIsError = 1;
        Error->Logf( *LocalizeError("SeekFailed",TEXT("Core")) );
    }
    Pos = InPos;
}

INT FArchiveFileWriterXenon::Tell()
{
    return Pos;
}

UBOOL FArchiveFileWriterXenon::Close()
{
    Flush();
    if( Handle && !CloseHandle(Handle) )
    {
        ArIsError = 1;
        Error->Logf( *LocalizeError("WriteFailed",TEXT("Core")) );
    }
	Handle = NULL;
    return !ArIsError;
}

void FArchiveFileWriterXenon::Serialize( void* V, INT Length )
{
    Pos += Length;
    INT Copy;
    while( Length > (Copy=ARRAY_COUNT(Buffer)-BufferCount) )
    {
        appMemcpy( Buffer+BufferCount, V, Copy );
        BufferCount += Copy;
        Length      -= Copy;
        V            = (BYTE*)V + Copy;
        Flush();
    }
    if( Length )
    {
        appMemcpy( Buffer+BufferCount, V, Length );
        BufferCount += Length;
    }
}

void FArchiveFileWriterXenon::Flush()
{
    if( BufferCount )
    {
		SCOPED_FILE_IO_WRITE_STATS( StatsHandle, BufferCount, 0 );
        INT Result=0;
        if( !WriteFile( Handle, Buffer, BufferCount, (DWORD*)&Result, NULL ) )
        {
            ArIsError = 1;
            Error->Logf( *LocalizeError("WriteFailed",TEXT("Core")) );
        }
    }
    BufferCount = 0;
}

/** 
 * Read the contents of a TOC file
 */
void FFileManagerXenon::ReadTOC( const TCHAR* ToCName, UBOOL bRequired )
{
	// read in the Table Of Contents file (without using CreateFileReader which will use the TOC for file size info)
	HANDLE TOCHandle = CreateFileA(
		TCHAR_TO_ANSI( *GetPlatformFilepath( *( appGameDir() + ToCName + TEXT( ".txt" ) ) ) ), 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, 
		NULL );

	// Dirty disk handling.
	if( TOCHandle == INVALID_HANDLE_VALUE )
	{
		if( bRequired )
		{
			// TOC is required to exist now
			checkf( FALSE, TEXT( "Missing %s.txt. Make sure to use CookerFrontend or CookerSync to copy files to the xbox"), ToCName );
			
			appHandleIOFailure( NULL );
		}
	}
	else
	{
		debugf( NAME_Init, TEXT( " ... caching ToC: %s" ), ToCName );

		// read in the TOC file into a string buffer
		INT TOCSize = GetFileSize( TOCHandle, NULL );
		ANSICHAR* Buffer = ( ANSICHAR* )appMalloc( TOCSize + 1 );
		// terminate the buffer for below
		Buffer[TOCSize] = 0;
		DWORD BytesRead;
		if( ReadFile( TOCHandle, Buffer, TOCSize, &BytesRead, NULL ) == 0 )
		{
			appHandleIOFailure( NULL );
		}

		// make sure it succeeded
		check( BytesRead == TOCSize );

		// close the file
		CloseHandle( TOCHandle );

		// parse the TOC file
		TOC.ParseFromBuffer( Buffer );

		// free the memory for the temp buffer
		appFree( Buffer );
	}
}


/** 
 * Initialize the file manager/file system. 
 * @param Startup TRUE  if this the first call to Init at engine startup
 */
void FFileManagerXenon::Init( UBOOL Startup )
{
	if ( GHDDCacheManager == NULL )
	{
		GHDDCacheManager = new FXeHDDCacheManager;
		GHDDCacheManager->Initialize();
	}

	// Read in the base TOC file with all the language agnostic files
	ReadTOC( TEXT( "Xbox360TOC" ), TRUE );

	// Read in the language specific ToCs
	for( INT LangIndex = 0; GKnownLanguageExtensions[LangIndex]; LangIndex++ )
	{
		FString ToCName = FString::Printf( TEXT( "Xbox360TOC_%s" ), GKnownLanguageExtensions[LangIndex] );
		ReadTOC( *ToCName, FALSE );
	}
}

FArchive* FFileManagerXenon::CreateFileReader( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error )
{
	INT StatsHandle = FILE_IO_STATS_GET_HANDLE( Filename );
	SCOPED_FILE_IO_READ_OPEN_STATS( StatsHandle );

	HANDLE Handle = NULL;
	INT ExistingFileSize = 0;
	if ( !GEnableHDDCaching )
	{
		Handle = CreateFileA( 
					TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), 
					GENERIC_READ, 
					FILE_SHARE_READ, 
					NULL, 
					OPEN_EXISTING, 
					FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN, 
					NULL );
		if( Handle==INVALID_HANDLE_VALUE )
		{
		    DWORD Error = GetLastError();
		    // Missing files do
		    if( Error != ERROR_PATH_NOT_FOUND && Error != ERROR_FILE_NOT_FOUND )
		    {
			    appHandleIOFailure( Filename );
		    }
			return NULL;
		}
		ExistingFileSize = GetFileSize( Handle, NULL );
	}
	else
	{
		ExistingFileSize = FileSize( Filename );
	}

	// Did the file not exist?
	if ( ExistingFileSize < 0 )
	{
		// if we were told NoFail, then abort
		if (Flags & FILEREAD_NoFail)
		{
			appErrorf(TEXT("Failed to open %s, which was marked as NoFail"), Filename);
		}
		return NULL;
	}

	FArchive* Archive = new FArchiveFileReaderXenon(Handle, StatsHandle, Filename, Error, ExistingFileSize, 0);
	if( Archive && (Flags & FILEREAD_SaveGame) )
	{
		Archive->SetIsSaveGame(TRUE);
	}

	return Archive;
}

FArchive* FFileManagerXenon::CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error, INT MaxFileSize )
{
	INT StatsHandle = FILE_IO_STATS_GET_HANDLE( Filename );
	SCOPED_FILE_IO_WRITE_OPEN_STATS( StatsHandle );

	// ensure the directory exists
	MakeDirectory(*FFilename(Filename).GetPath(), TRUE);

	if( (GFileManager->FileSize (Filename) >= 0) && (Flags & FILEWRITE_EvenIfReadOnly) )
	{
        SetFileAttributesA(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), 0);
	}
    HANDLE Handle    = CreateFileA( 
							TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), 
							GENERIC_WRITE, 
							(Flags & FILEWRITE_AllowRead) ? FILE_SHARE_READ : 0,
							NULL, 
							(Flags & FILEWRITE_Append) ? OPEN_ALWAYS : (Flags & FILEWRITE_NoReplaceExisting) ? CREATE_NEW : CREATE_ALWAYS, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL );
    INT Pos = 0;
    if( Handle==INVALID_HANDLE_VALUE )
    {
		if( Flags & FILEWRITE_NoFail )
		{
			appErrorf( TEXT("Failed to create file: %s Error: %d"), Filename, GetLastError() );
		}
        return NULL;
    }
    if( Flags & FILEWRITE_Append )
	{
        Pos = SetFilePointer( Handle, 0, 0, FILE_END );
	}
	FArchive* retArch = new FArchiveFileWriterXenon(Handle,StatsHandle,Filename,Error,Pos);
	if( retArch && (Flags & FILEREAD_SaveGame) )
	{
		retArch->SetIsSaveGame(TRUE);
	}
	return retArch;
}

/**
 *	Returns the size of a file. (Thread-safe)
 *
 *	@param Filename		Platform-independent Unreal filename.
 *	@return				File size in bytes or -1 if the file didn't exist.
 **/
INT FFileManagerXenon::FileSize( const TCHAR* Filename )
{
	// GetFileSize() automatically takes care of TOC synchronization for us
	INT FileSize = TOC.GetFileSize(Filename);
	if (FileSize == -1)
	{
		FileSize = InternalFileSize(Filename);
	}

	return FileSize;
}

/**
 *	Looks up the size of a file by opening a handle to the file.
 *
 *	@param	Filename	The path to the file.
 *	@return	The size of the file or -1 if it doesn't exist.
 */
INT FFileManagerXenon::InternalFileSize(const TCHAR* Filename)
{
	HANDLE Handle = CreateFileA(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), 
							GENERIC_READ, 
							FILE_SHARE_READ,
							NULL, 
							OPEN_EXISTING, 
							FILE_ATTRIBUTE_NORMAL, 
							NULL );
								
	if(Handle == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	DWORD Result = GetFileSize(Handle, NULL);
	CloseHandle(Handle);

	return Result;
}

/**
 * If the given file is compressed, this will return the size of the uncompressed file,
 * if the platform supports it.
 * @param Filename Name of the file to get information about
 * @return Uncompressed size if known, otherwise -1
 */
INT FFileManagerXenon::UncompressedFileSize( const TCHAR* Filename )
{
	return TOC.GetUncompressedFileSize(Filename);
}

/**
 * Returns the starting "sector" of a file on its storage device (ie DVD)
 * @param Filename Name of the file to get information about
 * @return Starting sector if known, otherwise -1
 */
INT FFileManagerXenon::GetFileStartSector( const TCHAR* Filename )
{
	return TOC.GetFileStartSector(Filename);
}

DWORD FFileManagerXenon::Copy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, FCopyProgress* Progress )
{
    if( EvenIfReadOnly )
	{
        SetFileAttributesA(TCHAR_TO_ANSI(*GetPlatformFilepath(DestFile)), 0);
	}
    DWORD Result;
    if( Progress )
	{
		Result = FFileManagerGeneric::Copy( DestFile, SrcFile, ReplaceExisting, EvenIfReadOnly, Attributes, Progress );
	}
	else
	{
		if( CopyFileA(TCHAR_TO_ANSI(*GetPlatformFilepath(SrcFile)), TCHAR_TO_ANSI(*GetPlatformFilepath(DestFile)), !ReplaceExisting) != 0)
		{
			Result = COPY_OK;
		}
		else
		{
			Result = COPY_MiscFail;
		}
	}
    if( Result==COPY_OK && !Attributes )
	{
        SetFileAttributesA(TCHAR_TO_ANSI(*GetPlatformFilepath(DestFile)), 0);
	}
    return Result;
}

UBOOL FFileManagerXenon::Delete( const TCHAR* Filename, UBOOL RequireExists, UBOOL EvenReadOnly )
{
    if( EvenReadOnly )
	{
        SetFileAttributesA(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)),FILE_ATTRIBUTE_NORMAL);
	}
    INT Result = DeleteFileA(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename))) != 0;
    DWORD Error = GetLastError();
	Result = Result || (!RequireExists && (Error==ERROR_FILE_NOT_FOUND || Error==ERROR_PATH_NOT_FOUND));
    if( !Result )
    {
		debugf( NAME_Warning, TEXT("Error deleting file '%s' (%d)"), Filename, Error );
    }
    return Result!=0;
}

UBOOL FFileManagerXenon::IsReadOnly( const TCHAR* Filename )
{
    DWORD rc;
    if( FileSize( Filename ) < 0 )
	{
        return( 0 );
	}
    rc = GetFileAttributesA(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)));
    if (rc != 0xFFFFFFFF)
	{
        return ((rc & FILE_ATTRIBUTE_READONLY) != 0);
	}
    else
    {
        debugf( NAME_Warning, TEXT("Error reading attributes for '%s'"), Filename );
        return (0);
    }
}

UBOOL FFileManagerXenon::Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace, UBOOL EvenIfReadOnly, UBOOL Attributes )
{
    //warning: MoveFileEx is broken on Windows 95 (Microsoft bug).
    Delete( Dest, 0, EvenIfReadOnly );
    INT Result = MoveFileA(TCHAR_TO_ANSI(*GetPlatformFilepath(Src)),TCHAR_TO_ANSI(*GetPlatformFilepath(Dest)));
    if( !Result )
    {
        DWORD error = GetLastError();
        debugf( NAME_Warning, TEXT("Error moving file '%s' to '%s' (%d)"), Src, Dest, error );
    }
    return Result!=0;
}

SQWORD FFileManagerXenon::GetGlobalTime( const TCHAR* Filename )
{
    //return grenwich mean time as expressed in nanoseconds since the creation of the universe.
    //time is expressed in meters, so divide by the speed of light to obtain seconds.
    //assumes the speed of light in a vacuum is constant.
    //the file specified by Filename is assumed to be in your reference frame, otherwise you
    //must transform the result by the path integral of the minkowski metric tensor in order to
    //obtain the correct result.

	WIN32_FILE_ATTRIBUTE_DATA	FileAttributes;

	if(GetFileAttributesEx(TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)),GetFileExInfoStandard,&FileAttributes))
		return *(SQWORD*)&FileAttributes.ftLastWriteTime;
	else
	    return 0;
}

UBOOL FFileManagerXenon::SetGlobalTime( const TCHAR* Filename )
{
    return 0;
}

UBOOL FFileManagerXenon::MakeDirectory( const TCHAR* Path, UBOOL Tree )
{
    if( Tree )
		return FFileManagerGeneric::MakeDirectory( Path, Tree );
	return CreateDirectoryA(TCHAR_TO_ANSI(*GetPlatformFilepath(Path)),NULL) !=0 || GetLastError()==ERROR_ALREADY_EXISTS;
}

UBOOL FFileManagerXenon::DeleteDirectory( const TCHAR* Path, UBOOL RequireExists, UBOOL Tree )
{
    if( Tree )
        return FFileManagerGeneric::DeleteDirectory( Path, RequireExists, Tree );
    return RemoveDirectoryA(TCHAR_TO_ANSI(*GetPlatformFilepath(Path))) !=0 || (!RequireExists && GetLastError()==ERROR_FILE_NOT_FOUND);
}

void FFileManagerXenon::FindFiles( TArray<FString>& Result, const TCHAR* Wildcard, UBOOL Files, UBOOL Directories )
{
	// cache a ffilename version
	FFilename			FullWildcard(Wildcard);

	// HDD Cache and downloadable content uses normal FindFiles method (touching HDD isn't so bad)
	if (appStrnicmp(Wildcard, TEXT("cache:"), 6) == 0 ||
		appStrnicmp(Wildcard, TEXT("DLC"), 3) == 0)
	{
		// get the 
		FFilename			BasePath	= FullWildcard.GetPath() + PATH_SEPARATOR;
		WIN32_FIND_DATAA	Data;
		HANDLE				Handle		= FindFirstFileA(TCHAR_TO_ANSI(*GetPlatformFilepath(Wildcard)), &Data);

		if (Handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				// skip ., .., and accept files if we want them, and directories if we want them
				if (stricmp(Data.cFileName, ".") != 0 && 
                    stricmp(Data.cFileName, "..") != 0 &&
                    ((Data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? Directories : Files))
				{
					// get the filename
					FString Filename = FString(ANSI_TO_TCHAR(Data.cFileName));

					// add this to the results
					Result.AddItem(*Filename);

					// store this file in the TOC
					TOC.AddEntry(*(BasePath + Filename), Data.nFileSizeLow);
				}
			}
			// keep going until done
			while( FindNextFileA(Handle,&Data) );

			// close the find handle
			FindClose( Handle );
		}
	}
	else
	{
		TOC.FindFiles(Result, Wildcard, Files, Directories);
	}
}

DOUBLE FFileManagerXenon::GetFileAgeSeconds( const TCHAR* Filename )
{
	FILETIME UTCFileTime;
	UTCFileTime.dwHighDateTime = 0;
	UTCFileTime.dwLowDateTime = 0;
	INT NewFileSize = INDEX_NONE;
	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	UBOOL bSuccess = (WinHandle != INVALID_HANDLE_VALUE);
	if ( bSuccess )
	{
		bSuccess = GetFileTime( WinHandle, NULL, NULL, &UTCFileTime );
		CloseHandle( WinHandle );
	}
	if ( bSuccess )
	{
		// A 64-bit value the number of 100-nanosecond intervals since January 1, 1601.
		QWORD IntegerFileTime = (QWORD(UTCFileTime.dwHighDateTime) << 32) | QWORD(UTCFileTime.dwLowDateTime);

		SYSTEMTIME SystemUTCTime;
		FILETIME SystemFileTime;
		GetSystemTime( &SystemUTCTime );
		SystemTimeToFileTime( &SystemUTCTime, &SystemFileTime );
		QWORD IntegerSystemTime = (QWORD(SystemFileTime.dwHighDateTime) << 32) | QWORD(SystemFileTime.dwLowDateTime);

		QWORD Age = IntegerSystemTime - IntegerFileTime;
		return DOUBLE(Age) / 10000.0;
	}
	return -1.0;
}

DOUBLE FFileManagerXenon::GetFileTimestamp( const TCHAR* Filename )
{
	FILETIME UTCFileTime;
	UTCFileTime.dwHighDateTime = 0;
	UTCFileTime.dwLowDateTime = 0;
	INT NewFileSize = INDEX_NONE;
	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	UBOOL bSuccess = (WinHandle != INVALID_HANDLE_VALUE);
	if ( bSuccess )
	{
		bSuccess = GetFileTime( WinHandle, NULL, NULL, &UTCFileTime );
		CloseHandle( WinHandle );
	}
	if ( bSuccess )
	{
		// A 64-bit value the number of 100-nanosecond intervals since January 1, 1601.
		QWORD IntegerTime = (QWORD(UTCFileTime.dwHighDateTime) << 32) | QWORD(UTCFileTime.dwLowDateTime);
		return DOUBLE(IntegerTime);
	}
	return -1.0;
}

UBOOL FFileManagerXenon::SetDefaultDirectory()
{
	return true;
}

FString FFileManagerXenon::GetCurrentDirectory()
{
	return TEXT("D:\\");
}

/** 
 * Get the timestamp for a file
 *
 * @param Path		Path for file
 * @param Timestamp Output timestamp
 * @return success code
 */  
UBOOL FFileManagerXenon::GetTimestamp( const TCHAR* Filename, timestamp& Timestamp )
{
	SYSTEMTIME UTCTime;
	appMemzero( &UTCTime, sizeof(UTCTime) );

	INT NewFileSize = INDEX_NONE;
	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	UBOOL bSuccess = (WinHandle != INVALID_HANDLE_VALUE);
	if ( bSuccess )
	{
		FILETIME UTCFileTime;
		bSuccess = GetFileTime( WinHandle, NULL, NULL, &UTCFileTime );
		bSuccess = bSuccess && FileTimeToSystemTime( &UTCFileTime, &UTCTime );
		CloseHandle( WinHandle );
	}
	Timestamp.Day       = UTCTime.wDay;
	Timestamp.Month     = UTCTime.wMonth;
	Timestamp.DayOfWeek = UTCTime.wDayOfWeek;
	Timestamp.Hour      = UTCTime.wHour;
	Timestamp.Minute    = UTCTime.wMinute;
	Timestamp.Second    = UTCTime.wSecond;
	Timestamp.Year      = UTCTime.wYear;
	return bSuccess;
}

/**
 * Updates the modification time of the file on disk to right now, just like the unix touch command
 * @param Filename Path to the file to touch
 * @return TRUE if successful
 */
UBOOL FFileManagerXenon::TouchFile(const TCHAR* Filename)
{
	appErrorf(TEXT("Implement me!"));

	return FALSE;
}

/**
 *	Threadsafely converts the platform-independent Unreal filename into platform-specific full path.
 *
 *	@param Filename		Platform-independent Unreal filename
 *	@return				Platform-dependent full filepath
 **/
FString FFileManagerXenon::GetPlatformFilepath( const TCHAR* Filename )
{
	// store the Xbox version of the name
	FString XenonFilename;

	// any files on an "DLCnn:" drive are already in xbox format
	if (appStrnicmp(Filename, TEXT("DLC"), 3) == 0)
	{
		// Skip digits.
		const TCHAR* FilePathMatching = Filename + 3;
		while ( *FilePathMatching && appIsDigit(*FilePathMatching) )
		{
			FilePathMatching++;
		}

		// Check for ':' character.
		if ( *FilePathMatching == TEXT(':') )
		{
			XenonFilename = Filename;
		}
	}

	// XenonFilename not filled in yet?
	if ( XenonFilename.Len() == 0 )
	{
		// any files on an "cache:" drive are already in xbox format
		if (XenonFilename.Len() == 0 && appStrnicmp(Filename, TEXT("cache:"), 6) == 0)
		{
			XenonFilename = Filename;
		}
		// any files on an "update:" drive are already in xbox format
		else if (appStrnicmp(Filename, TEXT("update:"), 7) == 0)
		{
			XenonFilename = Filename;
		}
		else
		{
			// Skip leading "..\".
			if( Filename[0] == L'.' )
			{
				Filename += 3;
			}
			if( Filename[0] == L'.' )
			{
				appErrorf(TEXT("No support for relative path names apart from leading \"..\\\" [%s]."), Filename);
			}
			if( Filename[0] == L'\\')
			{
				Filename += 1;
			}

			// tack on the filename
			XenonFilename = TEXT("D:\\");
			XenonFilename += Filename;
		}
	}

	// Convert PATH_SEPARATOR
	XenonFilename = XenonFilename.Replace(TEXT("/"), TEXT("\\"));

	return XenonFilename;
}

/**
 *	Opens a file. (Thread-safe)
 *	If opened for write, it will create any necessary directory structure.
 *
 *	@param Filename		Platform-independent Unreal filename
 *	@param Flags		A combination of EFileOpenFlags, specifying read/write access.
 *	@return				File handle
 **/
FFileHandle FFileManagerXenon::FileOpen( const TCHAR* Filename, DWORD Flags )
{
	FFilename FullPath( GetPlatformFilepath(Filename) );

	DWORD AccessFlags = (Flags & IO_READ) ? GENERIC_READ : 0;
	DWORD CreateFlags = 0;
	if ( Flags & IO_WRITE )
	{
		MakeDirectory(*FullPath.GetPath(), TRUE);
		AccessFlags |= GENERIC_WRITE;
		CreateFlags = (Flags & IO_APPEND) ? OPEN_ALWAYS : CREATE_ALWAYS;
	}
	else
	{
		CreateFlags = OPEN_EXISTING;
	}

	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*FullPath), AccessFlags, FILE_SHARE_READ, NULL, CreateFlags, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( WinHandle == INVALID_HANDLE_VALUE )
    {
        WinHandle = (HANDLE) INDEX_NONE;
    }
	else if ( Flags & IO_APPEND )
	{
		SetFilePointer( WinHandle, 0, NULL, FILE_END );
	}

	return FFileHandle( PTRINT(WinHandle), 0 );
}

/**
 *	Closes a file. (Thread-safe)
 *
 *	@param Handle		File handle
 **/
void FFileManagerXenon::FileClose( FFileHandle FileHandle )
{
	if ( FileHandle.IsValid() )
	{
		CloseHandle( (HANDLE) FileHandle.Handle );
	}
}

/**
 *	Sets the current position in a file. (Thread-safe)
 *
 *	@param FileHandle		File handle
 *	@param Offset			New file offset, counted in bytes from the specified 'Base'.
 *	@param Base				A EFileSeekFlags flag, specifying the base of the seek operation.
 *	@return					New file offset, counted from the beginning of the file, or INDEX_NONE on error
 **/
INT FFileManagerXenon::FileSeek( FFileHandle FileHandle, INT Offset, EFileSeekFlags Base/*=IO_SEEK_BEGIN*/ )
{
	if ( FileHandle.IsValid() )
	{
		DWORD SeekFlags;
		switch ( Base )
		{
			case IO_SEEK_END:
				SeekFlags = FILE_END;
				break;
			case IO_SEEK_CURRENT:
				SeekFlags = FILE_CURRENT;
				break;
			case IO_SEEK_BEGIN:
			default:
				SeekFlags = FILE_BEGIN;
		}
		DWORD NewPosition = SetFilePointer( (HANDLE) FileHandle.Handle, Offset, NULL, SeekFlags );
		if ( NewPosition != INVALID_SET_FILE_POINTER )
		{
			return NewPosition;
		}
	}
	return INDEX_NONE;
}

/**
 *	Sets the current position in a file. (Thread-safe)
 *
 *	@param FileHandle		File handle
 *	@return					Current file position, counted from the beginning of the file, or INDEX_NONE on error.
 **/
INT FFileManagerXenon::GetFilePosition( FFileHandle FileHandle )
{
	if ( FileHandle.IsValid() )
	{
		DWORD NewPosition = SetFilePointer( (HANDLE) FileHandle.Handle, 0, NULL, FILE_CURRENT );
		if ( NewPosition != INVALID_SET_FILE_POINTER )
		{
			return NewPosition;
		}
	}
	return INDEX_NONE;
}

/**
 *	Reads a number of bytes from file. (Thread-safe)
 *
 *	@param FileHandle		File handle
 *	@param Buffer			Buffer to store the read data
 *	@param Size				Number of bytes to read
 *	@return					Actual number of bytes read into 'Buffer', or INDEX_NONE upon error
 **/
INT FFileManagerXenon::FileRead( FFileHandle FileHandle, void* Buffer, INT Size )
{
	if ( FileHandle.IsValid() )
	{
		DWORD BytesRead = 0;
		BOOL bSuccess = ReadFile( (HANDLE) FileHandle.Handle, Buffer, Size, &BytesRead, NULL );
		if ( bSuccess )
		{
			return BytesRead;
		}
	}
	return INDEX_NONE;
}

/**
 *	Writes a number of bytes to file. (Thread-safe)
 *
 *	@param FileHandle		File handle
 *	@param Buffer			Buffer that contains the data to write
 *	@param Size				Number of bytes to write
 *	@return					Actual number of bytes written to file, or INDEX_NONE upon error
 **/
INT FFileManagerXenon::FileWrite( FFileHandle FileHandle, const void* Buffer, INT Size )
{
	if ( FileHandle.IsValid() )
	{
		DWORD BytesWritten = 0;
		BOOL bSuccess = WriteFile( (HANDLE) FileHandle.Handle, Buffer, Size, &BytesWritten, NULL );
		if ( bSuccess )
		{
			return BytesWritten;
		}
	}
	return INDEX_NONE;
}

/**
 *	Truncates an existing file, discarding data at the end to make it smaller. (Thread-safe)
 *
 *	@param Filename		Platform-independent Unreal filename.
 *	@param FileSize		New file size to truncate to. If this is larger than current file size, the function won't do anything.
 *	@return				Resulting file size or INDEX_NONE if the file didn't exist.
 **/
INT FFileManagerXenon::FileTruncate( const TCHAR* Filename, INT FileSize )
{
	INT NewFileSize = INDEX_NONE;
	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	UBOOL bSuccess = (WinHandle != INVALID_HANDLE_VALUE);
	if ( bSuccess )
	{
		DWORD CurrentFileSize = GetFileSize( WinHandle, NULL );
		NewFileSize = INT(CurrentFileSize);
		UBOOL bSuccess = (CurrentFileSize != INVALID_FILE_SIZE);
		if ( bSuccess && CurrentFileSize > DWORD(FileSize) )
		{
			DWORD NewPosition = SetFilePointer( WinHandle, FileSize, NULL, FILE_BEGIN );
			bSuccess = bSuccess && NewPosition == DWORD(FileSize) && SetEndOfFile( WinHandle );
			NewFileSize = FileSize;
		}
		CloseHandle( WinHandle );
	}
	return bSuccess ? NewFileSize : INDEX_NONE;
}

/**
 *	Sets the timestamp of a file. (Thread-safe)
 *
 *	@param Filename		Platform-independent Unreal filename.
 *	@param Timestamp	Timestamp to set
 *	@return				TRUE if the operation succeeded.
 **/
UBOOL FFileManagerXenon::SetFileTimestamp( const TCHAR* Filename, DOUBLE TimeStamp )
{
	INT NewFileSize = INDEX_NONE;
	HANDLE WinHandle = CreateFileA( TCHAR_TO_ANSI(*GetPlatformFilepath(Filename)), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	UBOOL bSuccess = (WinHandle != INVALID_HANDLE_VALUE);
	if ( bSuccess )
	{
		FILETIME UTCFileTime;
		QWORD IntegerFileTime = QWORD(TimeStamp);
		UTCFileTime.dwHighDateTime = IntegerFileTime >> 32;
		UTCFileTime.dwLowDateTime = IntegerFileTime & 0xffffffff;
		bSuccess = SetFileTime( WinHandle, NULL, NULL, &UTCFileTime );
		CloseHandle( WinHandle );
	}
	return bSuccess;
}

/**
 *	Flushes the file to make sure any buffered writes have been fully written to disk. (Thread-safe)
 *
 *	@param FileHandle		File handle
 **/
void FFileManagerXenon::FileFlush( FFileHandle FileHandle )
{
	if ( FileHandle.IsValid() )
	{
		FlushFileBuffers( (HANDLE) FileHandle.Handle );
	}
}

UBOOL FFileManagerXenon::IsDrive( const TCHAR* Path )
{
	if ( appStricmp(Path,TEXT("cache:")) == 0 )
		return TRUE;
	return FFileManagerGeneric::IsDrive( Path );
}
