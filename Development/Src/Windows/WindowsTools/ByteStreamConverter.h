/*=============================================================================
	WindowsSupport/ByteStreamConverter.h: Conversion from / to data stream for basic types.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/ 

#pragma once

/**
 *	Simple byte stream converter for basic types: int, string, etc.
 */
class CByteStreamConverter
{
public:
	/// Retrieves int from buffer assuming big endian is used
	static unsigned int LoadIntBE(char*& Data, int& DataSize)
	{
		if(DataSize >= 4)
		{
			const unsigned int Value =
				((unsigned char) Data[0]) << 24 |
				((unsigned char) Data[1]) << 16 |
				((unsigned char) Data[2]) << 8 |
				((unsigned char) Data[3]);

			Data += 4;
			DataSize -= 4;

			return Value;
		}

		return 0;
	}

	/// Retrieves int from buffer assuming little endian is used
	static unsigned int LoadIntLE(char*& Data, int& DataSize)
	{
		if(DataSize >= 4)
		{
			const unsigned int Value =
				((unsigned char) Data[3]) << 24 |
				((unsigned char) Data[2]) << 16 |
				((unsigned char) Data[1]) << 8 |
				((unsigned char) Data[0]);

			Data += 4;
			DataSize -= 4;

			return Value;
		}

		return 0;
	}

	/// Retrieves string from buffer assuming big endian is used
	static string LoadStringBE(char*& Data, int& DataSize)
	{
		int StringLen = LoadIntBE(Data, DataSize);

		if(StringLen > DataSize)
		{
			StringLen = DataSize;
		}

		if(StringLen > 0)
		{
			char* Buffer = new char[StringLen + 1];
			memcpy(Buffer, Data, StringLen);
			Buffer[StringLen] = '\0';

			Data += StringLen;
			DataSize -= StringLen;

			string Result = Buffer;
			delete[] Buffer;

			return Result;
		}

		return "";
	}

	/// Retrieves string from buffer assuming big endian is used
	static string LoadStringLE(char*& Data, int& DataSize)
	{
		int StringLen = LoadIntLE(Data, DataSize);

		if(StringLen > DataSize)
		{
			StringLen = DataSize;
		}

		if(StringLen > 0)
		{
			char* Buffer = new char[StringLen + 1];
			memcpy(Buffer, Data, StringLen);
			Buffer[StringLen] = '\0';

			Data += StringLen;
			DataSize -= StringLen;

			string Result = Buffer;
			delete[] Buffer;
			return Result;
		}

		return "";
	}

	static void StoreIntBE(char* Data, unsigned int Value)
	{
		Data[0] = (char) (Value >> 24) & 255;
		Data[1] = (char) (Value >> 16) & 255;
		Data[2] = (char) (Value >> 8) & 255;
		Data[3] = (char) (Value & 255);
	}
};
