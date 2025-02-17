/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/** Thread-safe appSRand based RNG. */
class FRandomStream
{
public:

	/** Initialization constructor. */
	FRandomStream(INT InSeed):
		Seed(InSeed)
	{}

	/** @return A random number between 0 and 1. */
	FLOAT GetFraction()
	{
		MutateSeed();

		//@todo fix type aliasing
		static const FLOAT SRandTemp = 1.0f;
		FLOAT Result;
		*(INT*)&Result = (*(INT*)&SRandTemp & 0xff800000) | (Seed & 0x007fffff);
		return appFractional(Result); 
	}

	/** @return A random number between 0 and MAX_INT. */
	INT GetInt()
	{
		MutateSeed();

		return Seed;
	}

private:

	/** Mutates the current seed into the next seed. */
	void MutateSeed()
	{
		Seed = (Seed * 196314165) + 907633515; 
	}

	INT Seed;
};
