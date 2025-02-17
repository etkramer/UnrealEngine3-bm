//------------------------------------------------------------------------------
// A class representing a complex number.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxComplexNumber_H__
#define FxComplexNumber_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

// A complex number.
class FxComplexNumber
{
public:
	// Constructor.
	FxComplexNumber()
		: _real(0.0)
		, _imaginary(0.0)
	{
	}
	// Construct from real part.
	FxComplexNumber( FxDReal realPart )
		: _real(realPart)
		, _imaginary(0.0)
	{
	}
	// Construct from real and imaginary parts.
	FxComplexNumber( FxDReal realPart, FxDReal imaginaryPart )
		: _real(realPart)
		, _imaginary(imaginaryPart)
	{
	}

	// Returns the real part.
	FxDReal GetRealPart( void ) const
	{
		return _real;
	}
	// Returns the imaginary part.
	FxDReal GetImaginaryPart( void ) const
	{
		return _imaginary;
	}

	// Computes the conjugate.
	FxComplexNumber GetConjugate( void )
	{
		return FxComplexNumber(_real, -_imaginary);
	}

	// Addition.
	void operator+=( const FxComplexNumber& rhs )
	{
		_real      += rhs._real;
		_imaginary += rhs._imaginary;
	}
	// Subtraction.
	void operator-=( const FxComplexNumber& rhs )
	{
		_real      -= rhs._real;
		_imaginary -= rhs._imaginary;
	}
	// Multiplication.
	void operator*=( const FxComplexNumber& rhs )
	{
		FxDReal tmpReal = rhs._real * _real - rhs._imaginary * _imaginary;
		_imaginary = rhs._real * _imaginary + rhs._imaginary * _real;
		_real = tmpReal;
	}
	// Negate.
	FxComplexNumber operator-( void )
	{
		return FxComplexNumber(-_real, -_imaginary);
	}

private:
	// Real part.
	FxDReal _real;
	// Imaginary part.
	FxDReal _imaginary;	
};

} // namespace Face

} // namespace OC3Ent

#endif
