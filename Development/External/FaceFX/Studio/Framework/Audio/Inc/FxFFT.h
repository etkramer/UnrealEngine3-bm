//------------------------------------------------------------------------------
// A class representing a Fast Fourier Transform.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFFT_H__
#define FxFFT_H__

#include "FxPlatform.h"
#include "FxComplexNumber.h"

namespace OC3Ent
{

namespace Face
{

// A Fast Fourier Transform.
class FxFFT
{
public:
	// Constructor.  windowLength must be a power of 2.
	FxFFT( FxInt32 windowLength, FxInt32 sampleRate );
	// Destructor.
	~FxFFT();

	// Returns the window length of the FFT.
	FxInt32 GetWindowLength( void ) const;
	// Sets the window length of the FFT.  windowLength must be a power of 2.
	void SetWindowLength( FxInt32 windowLength );

	// The type of windowing function applied to the incoming audio.
	enum FxFFTWindowType
	{
		WT_Bartlett = 0,
		WT_Blackman,
		WT_BlackmanHarris,
		WT_Hamming,
		WT_Hann,
		WT_Rectangle,
		WT_Welch
	};

	// Returns the type of windowing function applied to the incoming audio.
	FxFFTWindowType GetWindowFunction( void ) const;
	// Sets the type of windowing function applied to the incoming audio.
	void SetWindowFunction( FxFFTWindowType windowType );

	// Copies numSamples from pSampleBuffer into the FFT.
	// numSamples must always be equal to the windowLength of the FFT.
	void CopySamples( const FxDReal* pSampleBuffer, FxInt32 numSamples );

	// Perform the FFT.
	void ForwardTransform( void );
	// Perform the inverse FFT.
	void InverseTransform( void );

	// Returns the number of results Transform computes (window length / 2
	// is the frame step and number of results).
	FxInt32 GetNumResults( void ) const;

	// Returns the FFT result at index.
	FxComplexNumber GetResult( FxInt32 index ) const;

	// Returns intensity at index.
	FxDReal GetIntensity( FxInt32 index ) const;

	// Returns the frequency that the FFT result at index corresponds to.
	FxDReal GetFrequency( FxInt32 index ) const;

	// Returns the frequency that an FFT result at index would correspond to.
	static FxDReal GetFrequency( FxInt32 index, FxInt32 sampleRate, FxInt32 windowLength );
	
private:

	// Initialize the FFT.
	void _initialize( void );
	// Clean up the FFT.
	void _cleanup( void ) ;

	// Puts the value in the FFT at index.
	void _putInFFT( FxInt32 index, FxDReal value );
	void _putInFFT( FxInt32 index, const FxComplexNumber& value );

	// Creates the windowing function.
	void _createWindowFn( void );

	// The window length of the FFT.  Must be a power of 2.
	FxInt32 _windowLength;
	// The log of the window length.
	FxInt32 _logWindowLength;
	// Type of windowing function to use.
	FxFFTWindowType _windowFnType;
	// Array of values representing the windowing function.
	FxDReal* _pWindowFn;
	// Bit reverse table.
	FxInt32* _pBitReverseTable;
	// The FFT (calculated in-place).
	FxComplexNumber* _pFFT;
	// Complex exponentials lookup table.
	FxComplexNumber** _ppExp;
	// The sample buffer to compute the FFT from.
	FxDReal* _pSampleBuffer;
	// The sample rate of the sample buffer.
	FxInt32 _sampleRate;
};

} // namespace Face

} // namespace OC3Ent

#endif
