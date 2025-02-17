/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#define _DEBUG_VOICE_PACKET_ENCODING 0

#ifndef MAX_VOICE_DATA_SIZE
	#define MAX_VOICE_DATA_SIZE 100
#endif

#ifndef MAX_SPLITSCREEN_TALKERS
	#define MAX_SPLITSCREEN_TALKERS 4
#endif

/** Defines the data involved in a voice packet */
struct FVoicePacket
{
	/** The unique net id of the talker sending the data */
	FUniqueNetId Sender;
	/** The data that is to be sent/processed */
	BYTE Buffer[MAX_VOICE_DATA_SIZE];
	/** The current amount of space used in the buffer for this packet */
	WORD Length;
	/** Number of references outstanding to this object */
	BYTE RefCount;
	/** Determines whether this packet is ref counted or not (not UBOOL for packing reasons) */
	BYTE bShouldUseRefCount;

private:
	/** Hidden so that only the DecRef() and FVoiceData can delete this object */
	~FVoicePacket()
	{
	}

	friend struct FVoiceData;

public:
	/** Zeros members and validates the assumptions */
	FVoicePacket(void) :
		Sender(0),
		Length(0),
		RefCount(0),
		bShouldUseRefCount(0)
	{
	}

	/**
	 * Inits the packet
	 *
	 * @param InRefCount the starting ref count to use
	 */
	FVoicePacket(BYTE InRefCount) :
		Sender(0),
		Length(0),
		RefCount(InRefCount),
		bShouldUseRefCount(TRUE)
	{
		check(RefCount < 255 && RefCount > 0);
	}

	/**
	 * Copies another packet and inits the ref count
	 *
	 * @param InRefCount the starting ref count to use
	 */
	FVoicePacket(const FVoicePacket& Other,BYTE InRefCount) :
		Sender(Other.Sender),
		Length(Other.Length),
		RefCount(InRefCount),
		bShouldUseRefCount(TRUE)
	{
		check(RefCount < 255 && RefCount > 0);
		// Copy the contents of the voice packet
		appMemcpy(Buffer,Other.Buffer,Other.Length);
	}

	/**
	 * Increments the ref count
	 */
	FORCEINLINE void AddRef(void)
	{
		check(RefCount < 255);
		if (bShouldUseRefCount)
		{
			RefCount++;
		}
	}

	/** 
	 * Decrements the ref count and deletes the object if needed
	 */
	FORCEINLINE void DecRef(void)
	{
		check(RefCount > 0 && bShouldUseRefCount);
		// Delete self if unreferenced
		if (bShouldUseRefCount && --RefCount == 0)
		{
			delete this;
		}
	}

	/**
	 * Reads the data for this object from the buffer and returns
	 * the number of bytes read from the byte stream
	 *
	 * @param ReadBuffer the source data to parse
	 *
	 * @return the amount of data read from the buffer
	 */
	inline WORD ReadFromBuffer(BYTE* ReadBuffer)
	{
		checkSlow(ReadBuffer);
		// Copy the unique net id and packet size info
		Sender = *(FUniqueNetId*)ReadBuffer;
		ReadBuffer += sizeof(FUniqueNetId);
		Length = *(WORD*)ReadBuffer;
		ReadBuffer += sizeof(WORD);
		// If the size is valid, copy the voice buffer
		check(Length <= MAX_VOICE_DATA_SIZE);
		appMemcpy(Buffer,ReadBuffer,Length);
#if _DEBUG_VOICE_PACKET_ENCODING
		// Read and verify the CRC
		ReadBuffer += Length;
		DWORD CRC = *(DWORD*)ReadBuffer;
		check(CRC == appMemCrc(Buffer,Length));
		return sizeof(FUniqueNetId) + sizeof(WORD) + sizeof(DWORD) + Length;
#else
		return sizeof(FUniqueNetId) + sizeof(WORD) + Length;
#endif
	}

	/**
	 * Writes the data for this object to the buffer and returns
	 * the number of bytes written. Assumes there is enough space
	 */
	inline WORD WriteToBuffer(BYTE* WriteAt)
	{
		// Copy the xuid & length manually
		*(FUniqueNetId*)WriteAt = Sender;
		WriteAt += sizeof(FUniqueNetId);
		*(WORD*)WriteAt = Length;
		WriteAt += sizeof(WORD);
		// Block copy the raw voice data
		appMemcpy(WriteAt,Buffer,Length);
#if _DEBUG_VOICE_PACKET_ENCODING
		// Send a CRC of the packet
		WriteAt += Length;
		*(DWORD*)WriteAt = appMemCrc(Buffer,Length);
		return sizeof(FUniqueNetId) + sizeof(WORD) + sizeof(DWORD) + Length;
#else
		return sizeof(FUniqueNetId) + sizeof(WORD) + Length;
#endif
	}

	/** Returns the amount of space this packet will consume in a buffer */
	FORCEINLINE WORD GetTotalPacketSize(void)
	{
#if _DEBUG_VOICE_PACKET_ENCODING
		return sizeof(FUniqueNetId) + sizeof(WORD) + sizeof(DWORD) + Length;
#else
		return sizeof(FUniqueNetId) + sizeof(WORD) + Length;
#endif
	}


	/**
	 * Serializes the voice packet data into/from an archive
	 *
	 * @param Ar the archive to serialize with
	 * @param VoicePacket the voice data to serialize
	 */
	friend FArchive& operator<<(FArchive& Ar,FVoicePacket& VoicePacket);
};

/** Make the tarray of voice packets a bit more readable */
typedef TArray<FVoicePacket*> FVoicePacketList;

/** Holds the global voice packet data state */
struct FVoiceData
{
	/** Data used by the local talkers before sent */
	FVoicePacket LocalPackets[MAX_SPLITSCREEN_TALKERS];
	/** Holds the set of received packets that need to be processed by XHV */
	FVoicePacketList RemotePackets;
	/** Holds the next packet index for transmitting local packets */
	DWORD NextVoicePacketIndex;

	/** Just zeros the packet data */
	FVoiceData(void)
	{
		appMemzero(LocalPackets,sizeof(FVoicePacket) * MAX_SPLITSCREEN_TALKERS);
	}
};

/** Global packet data to be shared between the net layer and the subsystem layer */
extern FVoiceData GVoiceData;
