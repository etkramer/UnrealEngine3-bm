/*=============================================================================
	FFileManagerWindows.h: Unreal Windows OS based file manager.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FILEMANAGERWINDOWS_H__
#define __FILEMANAGERWINDOWS_H__

#include "FFileManagerGeneric.h"

/*-----------------------------------------------------------------------------
	FArchiveFileReaderWindows
-----------------------------------------------------------------------------*/

// File manager.
class FArchiveFileReaderWindows : public FArchive
{
public:
	FArchiveFileReaderWindows( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InSize );
	~FArchiveFileReaderWindows();

	virtual void Seek( INT InPos );
	virtual INT Tell();
	virtual INT TotalSize();
	virtual UBOOL Close();
	virtual void Serialize( void* V, INT Length );

protected:
	UBOOL InternalPrecache( INT PrecacheOffset, INT PrecacheSize );

	HANDLE          Handle;
	/** Handle for stats tracking */
	INT				StatsHandle;
	/** Filename for debugging purposes. */
	FString			Filename;
	FOutputDevice*  Error;
	INT             Size;
	INT             Pos;
	INT             BufferBase;
	INT             BufferCount;
	BYTE            Buffer[1024];
};


/*-----------------------------------------------------------------------------
	FArchiveFileWriterWindows
-----------------------------------------------------------------------------*/

class FArchiveFileWriterWindows : public FArchive
{
public:
	FArchiveFileWriterWindows( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InPos );
	~FArchiveFileWriterWindows();

	virtual void Seek( INT InPos );
	virtual INT Tell();
	virtual INT TotalSize();
	virtual UBOOL Close();
	virtual void Serialize( void* V, INT Length );
	virtual void Flush();

protected:
	HANDLE          Handle;
	/** Handle for stats tracking */
	INT				StatsHandle;
	/** Filename for debugging purposes */
	FString			Filename;
	FOutputDevice*  Error;
	INT             Pos;
	INT             BufferCount;
	BYTE            Buffer[4096];
};


/*-----------------------------------------------------------------------------
	FFileManagerWindows
-----------------------------------------------------------------------------*/

class FFileManagerWindows : public FFileManagerGeneric
{
public:
	void Init(UBOOL Startup);

	UBOOL SetDefaultDirectory();
	FString GetCurrentDirectory();

	FArchive* CreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );
	FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error, INT MaxFileSize );
	INT	UncompressedFileSize( const TCHAR* Filename );
	DWORD Copy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, FCopyProgress* Progress );
	UBOOL Delete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 );
	UBOOL IsReadOnly( const TCHAR* Filename );
	UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 );
	UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 );
	UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 );
	void FindFiles( TArray<FString>& Result, const TCHAR* Filename, UBOOL Files, UBOOL Directories );
	DOUBLE GetFileAgeSeconds( const TCHAR* Filename );
	DOUBLE GetFileTimestamp( const TCHAR* Filename );
	UBOOL GetTimestamp( const TCHAR* Filename, timestamp& Timestamp );

	/**
	 * Updates the modification time of the file on disk to right now, just like the unix touch command
	 * @param Filename Path to the file to touch
	 * @return TRUE if successful
	 */
	UBOOL TouchFile(const TCHAR* Filename);

	/**
	 * Converts a path pointing into the installed directory (C:\Program Files\MyGame\ExampleGame\Config\ExampleEngine.ini)
	 * to a path that a least-privileged user can write to (C:\<UserDir>\MyGame\ExampleGame\Config\ExampleEngine.ini)
	 *
	 * @param AbsolutePath Source path to convert
	 *
	 * @return Path to the user directory
	 */
	FString ConvertAbsolutePathToUserPath(const TCHAR* AbsolutePath);

	/**
	 * Converts passed in filename to use an absolute path.
	 *
	 * @param	Filename	filename to convert to use an absolute path, safe to pass in already using absolute path
	 * 
	 * @return	filename using absolute path
	 */
	FString ConvertToAbsolutePath( const TCHAR* Filename );

	/**
	 *	Opens a file. (Thread-safe)
	 *	If opened for write, it will create any necessary directory structure.
	 *
	 *	@param Filename		Platform-independent Unreal filename
	 *	@param Flags		A combination of EFileOpenFlags, specifying read/write access.
	 *	@return				File handle
	 **/
	virtual FFileHandle FileOpen( const TCHAR* Filename, DWORD Flags );

	/**
	 *	Closes a file. (Thread-safe)
	 *
	 *	@param Handle		File handle
	 **/
	virtual void FileClose( FFileHandle FileHandle );

	/**
	 *	Sets the current position in a file. (Thread-safe)
	 *
	 *	@param FileHandle		File handle
	 *	@param Offset			New file offset, counted in bytes from the specified 'Base'.
	 *	@param Base				A EFileSeekFlags flag, specifying the base of the seek operation.
	 *	@return					New file offset, counted from the beginning of the file, or INDEX_NONE on error
	 **/
	virtual INT FileSeek( FFileHandle FileHandle, INT Offset, EFileSeekFlags Base=IO_SEEK_BEGIN );

	/**
	 *	Sets the current position in a file. (Thread-safe)
	 *
	 *	@param FileHandle		File handle
	 *	@return					Current file position, counted from the beginning of the file, or INDEX_NONE on error.
	 **/
	virtual INT GetFilePosition( FFileHandle FileHandle );

	/**
	 *	Reads a number of bytes from file. (Thread-safe)
	 *
	 *	@param FileHandle		File handle
	 *	@param Buffer			Buffer to store the read data
	 *	@param Size				Number of bytes to read
	 *	@return					Actual number of bytes read into 'Buffer', or INDEX_NONE upon error
	 **/
	virtual INT FileRead( FFileHandle FileHandle, void* Buffer, INT Size );

	/**
	 *	Writes a number of bytes to file. (Thread-safe)
	 *
	 *	@param FileHandle		File handle
	 *	@param Buffer			Buffer that contains the data to write
	 *	@param Size				Number of bytes to write
	 *	@return					Actual number of bytes written to file, or INDEX_NONE upon error
	 **/
	virtual INT FileWrite( FFileHandle FileHandle, const void* Buffer, INT Size );

	/**
	 *	Truncates an existing file, discarding data at the end to make it smaller. (Thread-safe)
	 *
	 *	@param Filename		Platform-independent Unreal filename.
	 *	@param FileSize		New file size to truncate to. If this is larger than current file size, the function won't do anything.
	 *	@return				Resulting file size or INDEX_NONE if the file didn't exist.
	 **/
	virtual INT FileTruncate( const TCHAR* Filename, INT FileSize );

	/**
	 *	Returns the size of a file. (Thread-safe)
	 *
	 *	@param Filename		Platform-independent Unreal filename.
	 *	@return				File size in bytes or INDEX_NONE if the file didn't exist.
	 **/
	virtual INT FileSize( const TCHAR* Filename );

	/**
	 *	Sets the timestamp of a file. (Thread-safe)
	 *
	 *	@param Filename		Platform-independent Unreal filename.
	 *	@param Timestamp	Timestamp to set
	 *	@return				TRUE if the operation succeeded.
	 **/
	virtual UBOOL SetFileTimestamp( const TCHAR* Filename, DOUBLE TimeStamp );

	/**
	 *	Flushes the file to make sure any buffered writes have been fully written to disk. (Thread-safe)
	 *
	 *	@param FileHandle		File handle
	 **/
	virtual void FileFlush( FFileHandle FileHandle );

protected:
	FArchive* InternalCreateFileReader( const TCHAR* InFilename, DWORD Flags, FOutputDevice* Error );
	FArchive* InternalCreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error );
	/**
	 *	Looks up the size of a file by opening a handle to the file.
	 *
	 *	@param	Filename	The path to the file.
	 *	@return	The size of the file or -1 if it doesn't exist.
	 */
	virtual INT InternalFileSize( const TCHAR* Filename );
	DWORD InternalCopy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, FCopyProgress* Progress );
	UBOOL InternalDelete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 );
	UBOOL InternalIsReadOnly( const TCHAR* Filename );
	UBOOL InternalMove( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 );
	UBOOL InternalMakeDirectory( const TCHAR* Path, UBOOL Tree=0 );
	UBOOL InternalDeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 );
	void InternalFindFiles( TArray<FString>& Result, const TCHAR* Filename, UBOOL Files, UBOOL Directories );
	DOUBLE InternalGetFileAgeSeconds( const TCHAR* Filename );
	DOUBLE InternalGetFileTimestamp( const TCHAR* Filename );
	UBOOL InternalGetTimestamp( const TCHAR* Filename, timestamp& Timestamp );

	/**
	 * Updates the modification time of the file on disk to right now, just like the unix touch command
	 * @param Filename Path to the file to touch
	 * @return TRUE if successful
	 */
	UBOOL InternalTouchFile(const TCHAR* Filename);

	/**
	 *	Sets the timestamp of a file.
	 *
	 *	@param Filename		Full path to the file.
	 *	@param Timestamp	Timestamp to set
	 *	@return				File size in bytes or -1 if the file didn't exist.
	 **/
	UBOOL InternalSetFileTimestamp( const TCHAR* Filename, DOUBLE TimeStamp );

	/**
	 *	Truncates an existing file, discarding data at the end to make it smaller. (Thread-safe)
	 *
	 *	@param Filename		Full path to the file.
	 *	@param FileSize		New file size to truncate to. If this is larger than current file size, the function won't do anything.
	 *	@return				Resulting file size or INDEX_NONE if the file didn't exist.
	 **/
	INT InternalFileTruncate( const TCHAR* Filename, INT FileSize );

	/**
	 *	Opens a file. (Thread-safe)
	 *	If opened for write, it will create any necessary directory structure.
	 *
	 *	@param Filename		Full path to the file
	 *	@param Flags		A combination of EFileOpenFlags, specifying read/write access.
	 *	@return				File handle
	 **/
	FFileHandle InternalFileOpen( const TCHAR* Filename, DWORD Flags );

	/** Directory where a Standard User can write to (to save settings, etc) */
	FString WindowsUserDir;

	/** Directory where the game in installed to */
	FString WindowsRootDir;

	/** Is the game running as if installed, ie, out of a directory a Standard User can't write to? */
	UBOOL bIsRunningInstalled;

};

#endif
