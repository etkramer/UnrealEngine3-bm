/*=============================================================================
    XeFileCaching.h: Unreal Xenon harddisk file caching.
    Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __XEFILECACHING_H__
#define __XEFILECACHING_H__

#include "FileCaching.h"

/**
 *	Xenon manager for the HDD caching system.
 */
class FXeHDDCacheManager : public FHDDCacheManager
{
public:
	/** Default constructor */
	FXeHDDCacheManager();

	/** Virtual destructor */
	virtual ~FXeHDDCacheManager();

	/**
	 * Reserves HDD cache space for the specified file.
	 *
	 * @param FileHandle			File to reserve space for
	 * @param NumBytesRequired		Amount of cache space required for the file, in total
	 * @return						TRUE if the HDD cache has enough space
	 */
	virtual UBOOL ReserveCacheSpace( FFileHandle FileHandle, INT NumBytesRequired );

	/**
	 * Returns the full path to the cached version of the file
	 *
	 * @param Filename	Source file
	 * @return			Full path of the cached file
	 */
	virtual FString GetCachedFilepath(const TCHAR* Filename);

protected:

	/**
	 *	Determines whether the file path refers to a partition or folder that is allowed to be cached.
	 *	@param FilePath	- Path to the file that we're considering for caching.
	 *	@return			- TRUE if the file is contained in a path that is allowed to be cached
	 */
	virtual UBOOL ShouldCachePath( const TCHAR* FilePath );

	/**
	 *	Returns the platform-specific maximum length of a filename with full path.
	 **/
	virtual INT GetMaxFilePathLength();

	/**
	 *	Make sure that all cache files has matching files and delete invalid files.
	 *	@param bClearCache	Whether to clear the whole HDD cache
	 */
	virtual void ValidateCache( UBOOL bClearCache );

	/** Whether or not the cache: partition was successfully mounted. */
	UBOOL	bIsUtilityMounted;
};


#endif
