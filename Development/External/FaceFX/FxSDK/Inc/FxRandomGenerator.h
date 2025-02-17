//------------------------------------------------------------------------------
// A pseudorandom number generator.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRandomGenerator_H__
#define FxRandomGenerator_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

/// A range used in the random number generator.
struct FxRange
{
	FxRange( FxReal iRangeMin, FxReal iRangeMax );

	FX_INLINE FxReal Length( void ) const { return rangeMax - rangeMin; }

	FxReal rangeMin;
	FxReal rangeMax;
};

/// Serialization support for FxRange.
FxArchive& operator<<(FxArchive& arc, FxRange& range);

/// A centred range used in the random number generator.
struct FxCentredRange
{
	FxCentredRange( FxReal iCentre, FxReal iRange );

	FxReal centre;
	FxReal range;
};

/// Serialization support for FxCentredRange.
FxArchive& operator<<(FxArchive& arc, FxCentredRange& centredRange);

/// A pseudorandom number generator.
class FxRandomGenerator
{
public:
	/// Constructor
	/// \param seed The seed for the PRNG.
	FxRandomGenerator( FxInt32 seed = 0);

	/// A random bool with a given probability of being true.
	FxBool Bool( FxReal trueProbability = 0.5f );

	/// A random index in a weighted array.
	FxSize Index( const FxArray<FxReal>& weights );

	/// A random integer in [centre-halfRange, centre+halfRange]
	FxInt32 Int32InCentredRange( FxInt32 centre, FxInt32 halfRange );
	/// A random integer in [rangeMin, rangeMax]
	FxInt32 Int32InRange( FxInt32 rangeMin, FxInt32 rangeMax );

	/// A random real in [centre-halfRange, centre+halfRange]
	FxReal RealInCentredRange( FxReal centre, FxReal halfRange );
	/// A random real in the centred range.
	FxReal RealInCentredRange( const FxCentredRange& centredRange );
	/// A random real in [rangeMin, rangeMax]
	FxReal RealInRange( FxReal rangeMin, FxReal rangeMax );
	/// A random real in the range.
	FxReal RealInRange( const FxRange& range );

	/// Resets the seed for the PRNG.
	void ResetSeed( FxInt32 seed );

protected:
	// Returns a random float between 0 and 1.
	FxReal ran0( void );

	FxInt32 _idum;
};

/// A pseudorandom number generator usable via singleton model.
class FxRandom
{
public:
	/// A random bool with a given probability of being true.
	static FxBool Bool( FxReal trueProbability = 0.5f );

	/// A random index in a weighted array.
	static FxSize Index( const FxArray<FxReal>& weights );

	/// A random integer in [centre-halfRange, centre+halfRange]
	static FxInt32 Int32InCentredRange( FxInt32 centre, FxInt32 halfRange );
	/// A random integer in [rangeMin, rangeMax]
	static FxInt32 Int32InRange( FxInt32 rangeMin, FxInt32 rangeMax );

	/// A random real in [centre-halfRange, centre+halfRange]
	static FxReal RealInCentredRange( FxReal centre, FxReal halfRange );
	/// A random real in the centred range.
	static FxReal RealInCentredRange( const FxCentredRange& centredRange );
	/// A random real in [rangeMin, rangeMax]
	static FxReal RealInRange( FxReal rangeMin, FxReal rangeMax );
	/// A random real in the range.
	static FxReal RealInRange( const FxRange& range );

	/// Resets the seed for the PRNG.
	static void ResetSeed( FxInt32 seed );
private:
	static FxRandomGenerator _random;
};

} // namespace Face

} // namespace OC3Ent

#endif
