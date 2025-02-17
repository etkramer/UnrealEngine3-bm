/*=============================================================================
	UnAsyncWork.h: Definition of queued work classes
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _UNASYNCWORK_H
#define _UNASYNCWORK_H


/**
 * Base class for simple implementatin of async queued work.
 */
class FAsyncWorkBase : public FQueuedWork
{
private:
#if CONSOLE
	/**
	 * Use a spin lock because we know that the thread is not competing with
	 * the main thread for cycles
	 */
	volatile INT bIsDone;
#else
	/**
	 * Event used to indicate the copy is done
	 */
	FEvent* DoneEvent;
#endif

	/**
	 * Thread safe counter that is optionally used to signal completion of the
	 * async work in the case of auto- destroy.
	 */
	FThreadSafeCounter* WorkCompletionCounter;

	/**
	 * Explicitly hidden.
	 */
	FAsyncWorkBase()
	{}

public:
	/**
	 * Caches the thread safe counter and uses self deletion to free itself if it's non- NULL.
	 * A NULL pointer indicates that IsDone is going to be called instead and the caller is 
	 * responsible for deletion.
	 *
	 * @param InWorkCompletionCounter	Counter to decrement on completion of task if non- NULL
	 */
	FAsyncWorkBase( FThreadSafeCounter* InWorkCompletionCounter );

	/**
	 * Cleans up the event on non-Console platforms.
	 */
	virtual ~FAsyncWorkBase(void);

	/**
	 * Returns whether the work is done.
	 *
	 * @return TRUE if work has been completed, FALSE otherwise
	 */
	UBOOL IsDone(void);

	/**
	 * Called if entry is removed from queue before it has started which might happen at exit.
	 */
	virtual void Abandon(void)
	{}

	/**
	 * Called after work has finished. Marks object as being done.	 
	 */ 
	virtual void Dispose(void);
};

/**
 * Asynchronous memory copy, used for copying large amounts of data in the
 * background
 */
class FAsyncCopy : public FAsyncWorkBase
{
protected:
	/** The source memory for the copy						*/
	void* Src;
	/** The destination memory being written to				*/
	void* Dest;
	/** The amount of data to copy in bytes					*/
	DWORD Size;

public:
	/**
	 * Initializes the data and creates the event on non Xbox platforms
	 */
	FAsyncCopy(void* Dest,void* Src,DWORD Size,FThreadSafeCounter* Counter = NULL);

	/**
	 * Performs the async copy
	 */
	virtual void DoWork(void);
};

/**
 * Asynchronous memory copy, used for copying large amounts of data in the
 * background. This version has platform specific alignment and caching 
 * restrictions so use with care!
 */
class FAsyncCopyAligned : public FAsyncCopy
{
public:
	/**
	 * Passes the data to FAsyncCopy constructor.
	 */
	FAsyncCopyAligned(void* InDest,void* InSrc,DWORD InSize,FThreadSafeCounter* InCounter = NULL)
	: FAsyncCopy( InDest, InSrc, InSize, InCounter )
	{}

	/**
	 * Performs the async copy
	 */
	virtual void DoWork(void);
};

/**
 * Asynchronous decompression, used for decompressing chunks of memory in the background
 */
class FAsyncUncompress : public FAsyncWorkBase
{
protected:
	/** Buffer containing uncompressed data					*/
	void*	UncompressedBuffer;
	/** Size of uncompressed data in bytes					*/
	INT		UncompressedSize;
	/** Buffer compressed data is going to be written to	*/
	void*	CompressedBuffer;
	/** Size of CompressedBuffer data in bytes				*/
	INT		CompressedSize;
	/** Flags to control decompression						*/
	ECompressionFlags Flags;
	/** Whether the source memory is padded with a full cache line at the end */
	UBOOL	bIsSourceMemoryPadded;

public:
	/**
	 * Initializes the data and creates the event.
	 *
	 * @param	Flags				Flags to control what method to use for decompression
	 * @param	UncompressedBuffer	Buffer containing uncompressed data
	 * @param	UncompressedSize	Size of uncompressed data in bytes
	 * @param	CompressedBuffer	Buffer compressed data is going to be written to
	 * @param	CompressedSize		Size of CompressedBuffer data in bytes
	 * @param	Counter				Counter that is being decremented on completion of work
	 * @param	bIsSourcePadded		Whether the source memory is padded with a full cache line at the end
	 */
	FAsyncUncompress( ECompressionFlags Flags, void* UncompressedBuffer, INT UncompressedSize, void* CompressedBuffer, INT CompressedSize, FThreadSafeCounter* Counter = NULL, UBOOL bIsSourcePadded = FALSE );

	/**
	 * Performs the async decompression
	 */
	virtual void DoWork();
};


/**
 * Asynchronous compression, used for compressing chunks of memory in the background
 */
class FAsyncCompress : public FAsyncWorkBase
{
protected:
	/** Buffer compressed data is going to be written to	*/
	void*	CompressedBuffer;
	/** Reference to size of CompressedBuffer data in bytes	*/
	INT&	CompressedSize;
	/** Buffer containing uncompressed data					*/
	void*	UncompressedBuffer;
	/** Size of uncompressed data in bytes					*/
	INT		UncompressedSize;
	/** Flags to control compression						*/
	ECompressionFlags Flags;

public:
	/**
	 * Initializes the data and creates the event.
	 *
	 * @param	Flags						Flags to control what method to use and optionally control memory vs speed
	 * @param	CompressedBuffer			Buffer compressed data is going to be written to
	 * @param	CompressedSize	[in/out]	Size of CompressedBuffer, at exit will be size of compressed data
	 * @param	UncompressedBuffer			Buffer containing uncompressed data
	 * @param	UncompressedSize			Size of uncompressed data in bytes
	 */
	FAsyncCompress( ECompressionFlags Flags, void* CompressedBuffer, INT& CompressedSize, void* UncompressedBuffer, INT UncompressedSize, FThreadSafeCounter* Counter = NULL );
	
	/**
	 * Performs the async decompression
	 */
	virtual void DoWork();
};


/**
 * Asynchronous SHA verification
 */
class FAsyncSHAVerify : public FAsyncWorkBase
{
protected:
	/** Buffer to run the has on. This class can take ownership of the buffer is bShouldDeleteBuffer is TRUE */
	void* Buffer;

	/** Size of Buffer */
	INT BufferSize;

	/** Hash to compare against */
	BYTE Hash[20];

	/** Filename to lookup hash value (can be empty if hash was passed to constructor) */
	FString Pathname;

	/** If this is TRUE, and looking up the hash by filename fails, this will abort execution */
	UBOOL bIsUnfoundHashAnError;

	/** Should this class delete the buffer memory when verification is complete? */
	UBOOL bShouldDeleteBuffer;

public:
	/**
	 * Constructor. 
	 * 
	 * @param InBuffer Buffer of data to calculate has on. MUST be valid until this task completes (use Counter or pass ownership via bInShouldDeleteBuffer)
	 * @param InBufferSize Size of InBuffer
	 * @param bInShouldDeleteBuffer TRUE if this task should appFree InBuffer on completion of the verification. Useful for a fire & forget verification
	 *        NOTE: If you pass ownership to the task MAKE SURE you are done using the buffer as it could go away at ANY TIME
	 * @param InHash Precalculated hash to test the InBuffer's hash against
	 * @param Counter Counter to decrement on completion of task if non- NULL
	 */
	FAsyncSHAVerify(void* InBuffer, INT InBufferSize, UBOOL bInShouldDeleteBuffer, BYTE* InHash, FThreadSafeCounter* Counter = NULL);

	/**
	 * Constructor. 
	 * 
	 * @param InBuffer Buffer of data to calculate has on. MUST be valid until this task completes (use Counter or pass ownership via bInShouldDeleteBuffer)
	 * @param InBufferSize Size of InBuffer
	 * @param bInShouldDeleteBuffer TRUE if this task should appFree InBuffer on completion of the verification. Useful for a fire & forget verification
	 *        NOTE: If you pass ownership to the task MAKE SURE you are done using the buffer as it could go away at ANY TIME
	 * @param InPathname Pathname to use to have the platform lookup the hash value
	 * @param bInIsUnfoundHashAnError TRUE if failing to lookup the hash should trigger an error
	 * @param Counter Counter to decrement on completion of task if non- NULL
	 */
	FAsyncSHAVerify(void* InBuffer, INT InBufferSize, UBOOL bInShouldDeleteBuffer, const TCHAR* InPathname, UBOOL bIsUnfoundHashAnError, FThreadSafeCounter* Counter = NULL);

	/**
	 * Performs the async hash verification
	 */
	virtual void DoWork();
};

/**
 * Asynchronous vorbis decompression
 */
class FAsyncVorbisDecompress : public FAsyncWorkBase
{
protected:
	class USoundNodeWave*		Wave;

public:
	FAsyncVorbisDecompress( USoundNodeWave* InWave, FThreadSafeCounter* Counter = NULL );

	/**
	 * Performs the async vorbis decompression
	 */
	virtual void DoWork();
};

#endif
