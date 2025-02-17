//------------------------------------------------------------------------------
// A class representing a Fast Fourier Transform.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxFFT.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

static const FxDReal kPi = 3.14159265358979323846;

FxFFT::FxFFT( FxInt32 windowLength, FxInt32 sampleRate )
	: _windowLength(windowLength)
	, _logWindowLength(0)
	, _windowFnType(WT_Hamming)
	, _pWindowFn(NULL)
	, _pBitReverseTable(NULL)
	, _pFFT(NULL)
	, _ppExp(NULL)
	, _pSampleBuffer(NULL)
	, _sampleRate(sampleRate)
{
	_initialize();
}

FxFFT::~FxFFT()
{
	_cleanup();
}

FxInt32 FxFFT::GetWindowLength( void ) const
{
	return _windowLength;
}

void FxFFT::SetWindowLength( FxInt32 windowLength )
{
	_windowLength = windowLength;
	_initialize();
}

FxFFT::FxFFTWindowType FxFFT::GetWindowFunction( void ) const
{
	return _windowFnType;
}

void FxFFT::SetWindowFunction( FxFFT::FxFFTWindowType windowType )
{
	_windowFnType = windowType;
	_createWindowFn();
}

void FxFFT::CopySamples( const FxDReal* pSampleBuffer, FxInt32 numSamples )
{
	if( pSampleBuffer && _pWindowFn && numSamples == _windowLength )
	{
		// Copy the samples from pSampleBuffer.
		FxMemcpy(_pSampleBuffer, pSampleBuffer, numSamples * sizeof(FxDReal));
		// Apply the windowing function and initialize the FFT.
		for( FxInt32 i = 0; i < _windowLength; ++i )
		{
			_putInFFT(i, _pWindowFn[i] * _pSampleBuffer[i]);
		}
	}
}

void FxFFT::ForwardTransform( void )
{
	// step = 2 ^ (level-1).
	// increm = 2 ^ level;
	FxInt32 step = 1;
	for( FxInt32 level = 1; level <= _logWindowLength; ++level )
	{
		FxInt32 increm = step * 2;
		for( FxInt32 j = 0; j < step; ++j )
		{
			// U = exp ( - 2 PI j / 2 ^ level ).
			FxComplexNumber U = _ppExp[level][j];
			for( FxInt32 i = j; i < _windowLength; i += increm )
			{
				// Butterfly.
				FxComplexNumber T = U;
				T *= _pFFT[i + step];
				_pFFT[i + step] = _pFFT[i];
				_pFFT[i + step] -= T;
				_pFFT[i] += T;
			}
		}
		step *= 2;
	}
}

void FxFFT::InverseTransform( void )
{
	FxFFT ifft(_windowLength, _sampleRate);
	for( FxInt32 i = 0; i < _windowLength; ++i )
	{
		ifft._putInFFT(i, _pFFT[i].GetConjugate());
	}
	ifft.ForwardTransform();
	FxComplexNumber oneOverWindowLength(1.0 / _windowLength);
	for( FxInt32 i = 0; i < _windowLength; ++i )
	{
		FxComplexNumber result = ifft._pFFT[i].GetConjugate();
		result *= oneOverWindowLength;
		_putInFFT(i, result);
	}
}

FxInt32 FxFFT::GetNumResults( void ) const
{
	return _windowLength / 2;
}

FxComplexNumber FxFFT::GetResult( FxInt32 index ) const
{
	return _pFFT[index];
}

FxDReal FxFFT::GetIntensity( FxInt32 index ) const
{
	if( index == 0 )
	{
		return _pFFT[index].GetRealPart() * _pFFT[index].GetRealPart();
	}
	return _pFFT[index].GetRealPart()      * _pFFT[index].GetRealPart() +
		   _pFFT[index].GetImaginaryPart() * _pFFT[index].GetImaginaryPart();
}

FxDReal FxFFT::GetFrequency( FxInt32 index ) const
{
	return FxFFT::GetFrequency(index, _sampleRate, _windowLength);
}

FxDReal FxFFT::GetFrequency( FxInt32 index, FxInt32 sampleRate, FxInt32 windowLength )
{
	return index * sampleRate / static_cast<FxDReal>(windowLength);
}

void FxFFT::_initialize( void )
{
	// Allocate the sample buffer and fill it with silence.
	_pSampleBuffer = new FxDReal[_windowLength];
	for( FxInt32 i = 0; i < _windowLength; ++i )
	{
		_pSampleBuffer[i] = 0.0;
	}

	// Create the windowing function.
	_pWindowFn = new FxDReal[_windowLength];
	_createWindowFn();

	// Calculate the binary log.
	FxInt32 windowLength = _windowLength;
	_logWindowLength = 0;
	windowLength--;
	while( windowLength != 0 )
	{
		windowLength >>= 1;
		_logWindowLength++;
	}

	_pBitReverseTable = new FxInt32[_windowLength];
	_pFFT  = new FxComplexNumber[_windowLength];
	_ppExp = new FxComplexNumber*[_logWindowLength + 1];
	
	// Pre-compute the complex exponentials.
	FxInt32 l2 = 2;
	for( FxInt32 l = 1; l <= _logWindowLength; ++l )
	{
		_ppExp[l] = new FxComplexNumber[_windowLength];

		for( FxInt32 i = 0; i < _windowLength; ++i )
		{
			FxDReal real      = FxCos(2.0 * kPi * i / l2);
			FxDReal imaginary = -FxSin(2.0 * kPi * i / l2);
			_ppExp[l][i] = FxComplexNumber(real, imaginary);
		}
		l2 *= 2;
	}
	
	// Set up the bit reverse table.
	FxInt32 reverse = 0;
	FxInt32 halfPoints = _windowLength / 2;
	for( FxInt32 i = 0; i < _windowLength - 1; ++i )
	{
		_pBitReverseTable[i] = reverse;
		FxInt32 mask = halfPoints;
		// Add one backwards.
		while( reverse >= mask )
		{
			// Turn this bit off.
			reverse -= mask;
			mask >>= 1;
		}
		reverse += mask;
	}
	_pBitReverseTable[_windowLength - 1] = _windowLength - 1;
}

void FxFFT::_cleanup( void )
{
	if( _pSampleBuffer )
	{
		delete [] _pSampleBuffer;
		_pSampleBuffer = NULL;
	}
	if( _pWindowFn )
	{
		delete [] _pWindowFn;
		_pWindowFn = NULL;
	}
	if( _pBitReverseTable )
	{
		delete [] _pBitReverseTable;
		_pBitReverseTable = NULL;
	}
	for( FxInt32 l = 1; l <= _logWindowLength; ++l )
	{
		if( _ppExp[l] )
		{
			delete [] _ppExp[l];
			_ppExp[l] = NULL;
		}
	}
	if( _ppExp )
	{
		delete [] _ppExp;
		_ppExp = NULL;
	}
	if( _pFFT )
	{
		delete [] _pFFT;
		_pFFT = NULL;
	}
}

void FxFFT::_putInFFT( FxInt32 index, FxDReal value )
{
	_pFFT[_pBitReverseTable[index]] = FxComplexNumber(value);
}

void FxFFT::_putInFFT( FxInt32 index, const FxComplexNumber& value )
{
	_pFFT[_pBitReverseTable[index]] = value;
}

void FxFFT::_createWindowFn( void ) 
{
	// _pWindowFn will have already been allocated to the proper window length
	// from _initialize() so there is no need to allocate any memory in this
	// function.
	if( _pWindowFn )
	{
		switch( _windowFnType )
		{
		case WT_Bartlett:
			{
				FxDReal k = 2.0 / (_windowLength - 1);
				FxInt32 i = 0;
				for( i = 0; i <= (_windowLength - 1) / 2; ++i )
				{
					_pWindowFn[i] = i * k;
				}
				for( ; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 2.0 - (i * k);
				}
			}
			break;
		case WT_Blackman:
			{
				FxDReal k = 2.0 * kPi / (_windowLength - 1);
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 0.42 - 0.5 * FxCos(k * i) + 
						0.08 * FxCos(2.0 * k * i);
				}
			}
			break;
		case WT_BlackmanHarris:
			{
				FxDReal k = 2.0 * kPi / (_windowLength - 1);
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 0.35875 - 0.48829 * FxCos(k * i) + 
									0.14128 * FxCos(2.0 * k * i) - 0.01168 * 
									FxCos(3.0 * k * i);
				}
			}
			break;
		case WT_Hann:
			{
				FxDReal k = 2.0 * kPi / (_windowLength - 1);
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 0.50 - 0.50 * FxCos(k * i);
				}
			}
			break;
		case WT_Rectangle:
			{
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 1.0;
				}
			}
			break;
		case WT_Welch:
			{
				FxDReal k = 2.0 / (_windowLength - 1);
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 1.0 - (k * i);
				}
			}
			break;
		case WT_Hamming:
			// Hamming falls through on purpose as it will be the default if no
			// valid window function was supplied (should never happen).
		default:
			{
				FxDReal k = 2.0 * kPi / (_windowLength - 1);
				for( FxInt32 i = 0; i < _windowLength; ++i )
				{
					_pWindowFn[i] = 0.54 - 0.46 * FxCos(k * i);
				}	
			}
			break;
		}
	}
}

} // namespace Face

} // namespace OC3Ent
