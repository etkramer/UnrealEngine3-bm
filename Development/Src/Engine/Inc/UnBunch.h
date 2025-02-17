/*=============================================================================
	UnBunch.h: Unreal bunch class.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if WITH_UE3_NETWORKING

//
// A bunch of data to send.
//
class FOutBunch : public FBitWriter
{
public:
	// Variables.
	FOutBunch*		Next;
	UChannel*		Channel;
	DOUBLE			Time;
	UBOOL			ReceivedAck;
	INT				ChIndex;
	INT				ChType;
	INT				ChSequence;
	INT				PacketId;
	BYTE			bOpen;
	BYTE			bClose;
	BYTE			bReliable;

	// Functions.
	FOutBunch();
	FOutBunch( UChannel* InChannel, UBOOL bClose );
	FArchive& operator<<( FName& Name );
	FArchive& operator<<( UObject*& Object );

	/**
	 * Assignment operator that is faster than the compiler generated one.
	 * This is due to the Buffer tarray fast copy
	 *
	 * @param In the bunch that is being copied
	 */
	FORCEINLINE FOutBunch& operator=(const FOutBunch& In)
	{
		// Skip if we are the same
		if (&In != this)
		{
			FBitWriter::operator=(In);
			// Now memcpy our fields for speed
			appMemcpy(&Next,&In.Next,sizeof(FOutBunch) - sizeof(FBitWriter));
		}
		return *this;
	}
};

//
// A bunch of data received from a channel.
//
class FInBunch : public FBitReader
{
public:
	// Variables.
	INT				PacketId;
	FInBunch*		Next;
	UNetConnection*	Connection;
	INT				ChIndex;
	INT				ChType;
	INT				ChSequence;
	BYTE			bOpen;
	BYTE			bClose;
	BYTE			bReliable;

	// Functions.
	FInBunch( UNetConnection* InConnection );

	FArchive& operator<<( FName& Name );
	FArchive& operator<<( UObject*& Object );
};

#endif	//#if WITH_UE3_NETWORKING



