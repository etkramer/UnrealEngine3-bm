//------------------------------------------------------------------------------
// Class defining color operations in RGB and HLS color space.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxColor_H__
#define FxColor_H__

#include "FxPlatform.h"

namespace OC3Ent
{

namespace Face
{

#define FX_RGB( r, g, b ) static_cast<FxByte>(r), static_cast<FxByte>(g), \
	                       static_cast<FxByte>(b)

#define FX_BLACK   FX_RGB(0,0,0)
#define FX_GREY    FX_RGB(128,128,128)
#define FX_WHITE   FX_RGB(255,255,255)
#define FX_RED     FX_RGB(255,0,0)
#define FX_GREEN   FX_RGB(0,255,0)
#define FX_BLUE    FX_RGB(0,0,255)
#define FX_CYAN    FX_RGB(0,255,255)
#define FX_MAGENTA FX_RGB(255,0,255)
#define FX_YELLOW  FX_RGB(255,255,0)

// A color.
class FxColor
{
public:
	// The undefined hue value (361.0 degrees).
	static const FxReal kUndefinedHue;

	// Constructors.
	// Default case is no color (black).
	FxColor();
	// Note if red == green == blue, hue will be undefined.
	FxColor( FxByte red, FxByte green, FxByte blue );
	// Note if saturation is zero hue will be undefined.
	FxColor( FxReal hue, FxReal luminance, FxReal saturation );
	// Destructor.
	~FxColor();

	// Equality operator.
	FxBool operator== ( const FxColor& other ) const;
	// Inequality operator.
	FxBool operator!= ( const FxColor& other ) const;

	// Returns the red value [0,1].
	FxReal GetRed( void ) const;
	// Returns the green value [0,1].
	FxReal GetGreen( void ) const;
	// Returns the blue value [0,1].
	FxReal GetBlue( void ) const;

	// Returns the red value as byte [0,255]).
	FxByte GetRedByte( void ) const;
	// Returns the green value as byte [0,255]).
	FxByte GetGreenByte( void ) const;
	// Returns the blue value as byte [0,255]).
	FxByte GetBlueByte( void ) const;

	// Returns the hue value [0,360].
	FxReal GetHue( void ) const;
	// Returns the luminance value [0,1].
	FxReal GetLuminance( void ) const;
	// Returns the saturation value [0,1].
	FxReal GetSaturation( void ) const;

private:
	// Red.
	FxReal _red;
	// Green.
	FxReal _green;
	// Blue.
	FxReal _blue;

	// Hue.
	FxReal _hue;
	// Luminance.
	FxReal _luminance;
	// Saturation.
	FxReal _saturation;

	// Converts the current HLS components to RGB.
	void _hlsToRgb( void );
	// Helper for _hlsToRgb.
	FxReal _value( FxReal n1, FxReal n2, FxReal hue );
	// Converts the current RGB components to HLS.
	void _rgbToHls( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
