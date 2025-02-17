//------------------------------------------------------------------------------
// A pseudorandom number generator.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxRandomGenerator.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

FxRange::FxRange( FxReal iRangeMin, FxReal iRangeMax )
	: rangeMin(iRangeMin)
	, rangeMax(iRangeMax)
{
}

FxArchive& operator<<(FxArchive& arc, FxRange& range)
{
	arc << range.rangeMin << range.rangeMax;
	return arc;
}

FxCentredRange::FxCentredRange( FxReal iCentre, FxReal iRange )
	: centre(iCentre)
	, range(iRange)
{
}

FxArchive& operator<<(FxArchive& arc, FxCentredRange& centredRange)
{
	arc << centredRange.centre << centredRange.range;
	return arc;
}

FxRandomGenerator::FxRandomGenerator( FxInt32 seed )
	: _idum(seed)
{
}

// A random bool with a given probability of being true.
FxBool FxRandomGenerator::Bool( FxReal trueProbability )
{
	return ran0() < trueProbability;
}

// A random index in a weighted array.
FxSize FxRandomGenerator::Index( const FxArray<FxReal>& weights )
{
	FxReal sum = 0.f;
	for( FxSize i = 0; i < weights.Length(); ++i )
	{
		sum += weights[i];
	}

	FxReal randomWeight = RealInRange(0.f, sum);
	sum = 0.f;
	for( FxSize i = 0; i < weights.Length(); ++i )
	{
		if( randomWeight <= weights[i] + sum ) return i;
		sum += weights[i];
	}

	return FxInvalidIndex;
}

// A random integer in [centre-halfRange, centre+halfRange]
FxInt32 FxRandomGenerator::Int32InCentredRange( FxInt32 centre, FxInt32 halfRange )
{
	return Int32InRange(centre - halfRange, centre + halfRange);
}

// A random integer in [rangeMin, rangeMax]
FxInt32 FxRandomGenerator::Int32InRange( FxInt32 rangeMin, FxInt32 rangeMax )
{
	// Pad the range min and max to ensure each integer has equal probabilty.
	return FxNearestInteger(FxLinearInterpolate(rangeMin - 0.5f, rangeMax + 0.5f, ran0()));
}

// A random real in [centre-halfRange, centre+halfRange]
FxReal FxRandomGenerator::RealInCentredRange( FxReal centre, FxReal halfRange )
{
	return RealInRange(centre - halfRange, centre + halfRange);
}

// A random real in the centred range.
FxReal FxRandomGenerator::RealInCentredRange( const FxCentredRange& centredRange )
{
	return RealInRange(centredRange.centre - centredRange.range, centredRange.centre + centredRange.range);
}

// A random real in [rangeMin, rangeMax]
FxReal FxRandomGenerator::RealInRange( FxReal rangeMin, FxReal rangeMax )
{
	return FxLinearInterpolate(rangeMin, rangeMax, ran0());
}

// A random real in the range.
FxReal FxRandomGenerator::RealInRange( const FxRange& range )
{
	return RealInRange(range.rangeMin, range.rangeMax);
}

void FxRandomGenerator::ResetSeed( FxInt32 seed )
{
	_idum = seed;
}

// Implementation of the "minimal" random number generator of Park and Miller.
// Adapted from "Numerical Recipes in C".
#define IA 16807
#define IM 2147483647
#define AM (1.0f/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876
FxReal FxRandomGenerator::ran0( void )
{
	_idum ^= MASK;
	FxInt32 k = _idum / IQ;
	_idum = IA * (_idum - k * IQ) - IR * k;
	if( _idum < 0 )
	{
		_idum += IM;
	}
	FxReal ans = AM * _idum;
	_idum ^= MASK;

	return ans;
}

FxRandomGenerator FxRandom::_random;

FxBool FxRandom::Bool( FxReal trueProbability )
{
	return _random.Bool(trueProbability);
}

FxSize FxRandom::Index( const FxArray<FxReal>& weights )
{
	return _random.Index(weights);
}

FxInt32 FxRandom::Int32InCentredRange( FxInt32 centre, FxInt32 halfRange )
{
	return _random.Int32InCentredRange(centre, halfRange);
}

FxInt32 FxRandom::Int32InRange( FxInt32 rangeMin, FxInt32 rangeMax )
{
	return _random.Int32InRange(rangeMin, rangeMax);
}

FxReal FxRandom::RealInCentredRange( FxReal centre, FxReal halfRange )
{
	return _random.RealInCentredRange(FxCentredRange(centre, halfRange));
}

FxReal FxRandom::RealInCentredRange( const FxCentredRange& centredRange )
{
	return _random.RealInCentredRange(centredRange);
}

FxReal FxRandom::RealInRange( FxReal rangeMin, FxReal rangeMax )
{
	return _random.RealInRange(FxRange(rangeMin, rangeMax));
}

FxReal FxRandom::RealInRange( const FxRange& range )
{
	return _random.RealInRange(range);
}

void FxRandom::ResetSeed( FxInt32 seed )
{
	_random.ResetSeed(seed);
}

} // namespace Face

} // namespace OC3Ent
