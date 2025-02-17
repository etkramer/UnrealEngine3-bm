/*=============================================================================
    FFileManagerXenon.h: Unreal Xenon file manager.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __FFILEMANAGERXENON_H__
#define __FFILEMANAGERXENON_H__

#include "FFileManagerGeneric.h"
#include "FTableOfContents.h"

/*-----------------------------------------------------------------------------
    File Manager.
-----------------------------------------------------------------------------*/

// File manager.
class FArchiveFileReaderXenon : public FArchive
{
public:
    FArchiveFileReaderXenon( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InSize, INT InDebugTrackingNumber );
	~FArchiveFileReaderXenon();

	virtual void Seek( INT InPos );
	virtual INT Tell();
	virtual INT TotalSize();
	virtual UBOOL Close();
	virtual void Serialize( void* V, INT Length );

protected:
	UBOOL InternalPrecache( INT PrecacheOffset, INT PrecacheSize );

	HANDLE          Handle;
	/** Handle for stats tracking. */
	INT				StatsHandle;
	/** Filename for debugging purposes. */
	FString			Filename;
    FOutputDevice*  Error;
    INT             Size;
    INT             Pos;
	INT				RequestedPos;
    INT             BufferBase;
    INT             BufferCount;
    BYTE            Buffer[1024];

	/** Unique number for tracking file operations */
	INT				DebugTrackingNumber;

	/** HDD cache helper */
	class FSideBySideCache* Cache;
};

class FArchiveFileWriterXenon : public FArchive
{
public:
    FArchiveFileWriterXenon( HANDLE InHandle, INT InStatsHandle, const TCHAR* InFilename, FOutputDevice* InError, INT InPos );
    ~FArchiveFileWriterXenon();
    void Seek( INT InPos );
    INT Tell();
    UBOOL Close();
    void Serialize( void* V, INT Length );
    void Flush();

protected:
    HANDLE          Handle;
	/** Handle for stats tracking. */
	INT				StatsHandle;
	/** Filename for debugging purposes. */
	FString			Filename;
	FOutputDevice*  Error;
    INT             Pos;
    INT             BufferCount;
    BYTE            Buffer[4096];
};

class FFileManagerXenon : public FFileManagerGeneric
{
public:
	/** 
	 * Initialize the file manager/file system. 
	 * @param Startup TRUE  if this the first call to Init at engine startup
	 */
	virtual void Init(UBOOL Startup);

    virtual FArchive* CreateFileReader( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error );
    virtual FArchive* CreateFileWriter( const TCHAR* Filename, DWORD Flags, FOutputDevice* Error, INT MaxFileSize );
	/**
	 * If the given file is compressed, this will return the size of the uncompressed file,
	 * if the platform supports it.
	 * @param Filename Name of the file to get information about
	 * @return Uncompressed size if known, otherwise -1
	 */
    virtual INT UncompressedFileSize( const TCHAR* Filename );
	/**
	 * Returns the starting "sector" of a file on its storage device (ie DVD)
	 * @param Filename Name of the file to get information about
	 * @return Starting sector if known, otherwise -1
	 */
	virtual INT GetFileStartSector( const TCHAR* Filename );
	virtual DWORD Copy( const TCHAR* DestFile, const TCHAR* SrcFile, UBOOL ReplaceExisting, UBOOL EvenIfReadOnly, UBOOL Attributes, FCopyProgress* Progress );
    virtual UBOOL Delete( const TCHAR* Filename, UBOOL RequireExists=0, UBOOL EvenReadOnly=0 );
    virtual UBOOL IsReadOnly( const TCHAR* Filename );
    virtual UBOOL Move( const TCHAR* Dest, const TCHAR* Src, UBOOL Replace=1, UBOOL EvenIfReadOnly=0, UBOOL Attributes=0 );
    virtual SQWORD GetGlobalTime( const TCHAR* Filename );
    virtual UBOOL SetGlobalTime( const TCHAR* Filename );
    virtual UBOOL MakeDirectory( const TCHAR* Path, UBOOL Tree=0 );
    virtual UBOOL DeleteDirectory( const TCHAR* Path, UBOOL RequireExists=0, UBOOL Tree=0 );
    virtual void FindFiles( TArray<FString>& Result, const TCHAR* Wildcard, UBOOL Files, UBOOL Directories );
	virtual DOUBLE GetFileAgeSeconds( const TCHAR* Filename );
	virtual DOUBLE GetFileTimestamp( const TCHAR* Filename );
	virtual UBOOL SetDefaultDirectory();
	virtual FString GetCurrentDirectory();

	/**
	 * Updates the modification time of the file on disk to right now, just like the unix touch command
	 * @param Filename Path to the file to touch
	 * @return TRUE if successful
	 */
	virtual UBOOL TouchFile(const TCHAR* Filename);

	/** 
	 * Get the timestamp for a file
	 *
	 * @param Path		Path for file
	 * @param Timestamp Output timestamp
	 * @return success code
	 */  
	virtual UBOOL GetTimestamp( const TCHAR* /*Filename*/, timestamp& /*Timestamp*/ );

	/**
	 *	Threadsafely converts the platform-independent Unreal filename into platform-specific full path.
	 *
	 *	@param Filename		Platform-independent Unreal filename
	 *	@return				Platform-dependent full filepath
	 **/
	virtual FString GetPlatformFilepath( const TCHAR* Filename );

	/** 
	 * Read the contents of a TOC file
	 */
	void ReadTOC( const TCHAR* ToCName, UBOOL bRequired );

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

	/** Helper object to store the TOC */
	FTableOfContents TOC;

	/**
	 *	Looks up the size of a file by opening a handle to the file.
	 *
	 *	@param	Filename	The path to the file.
	 *	@return	The size of the file or -1 if it doesn't exist.
	 */
	virtual INT InternalFileSize( const TCHAR* Filename );

	virtual UBOOL IsDrive( const TCHAR* Path );
};


#endif
