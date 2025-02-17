//------------------------------------------------------------------------------
// Class defining color operations in RGB and HLS color space.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxColor.h"
#include "FxUtil.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

const FxReal FxColor::kUndefinedHue = 361.0f;

FxColor::FxColor()
	: _red(0.0f)
	, _green(0.0f)
	, _blue(0.0f)
	, _hue(kUndefinedHue)
	, _luminance(0.0f)
	, _saturation(0.0f)
{
}

FxColor::FxColor( FxByte red, FxByte green, FxByte blue )
	: _red(red / 255.0f)
	, _green(green / 255.0f)
	, _blue(blue / 255.0f)
	, _hue(0.0f)
	, _luminance(0.0f)
	, _saturation(0.0f)
{
	_rgbToHls();
}

FxColor::FxColor( FxReal hue, FxReal luminance, FxReal saturation )
	: _red(0.0f)
	, _green(0.0f)
	, _blue(0.0f)
	, _hue(hue)
	, _luminance(luminance)
	, _saturation(saturation)
{
	if( _saturation == 0.0f )
	{
		_hue = kUndefinedHue;
	}
	_hlsToRgb();
}

FxColor::~FxColor()
{
}

FxBool FxColor::operator== ( const FxColor& other ) const
{
	return ( FxRealEquality( _red, other._red )              &&
		     FxRealEquality( _green, other._green )          &&
			 FxRealEquality( _blue, other._blue )            &&
			 FxRealEquality( _hue, other._hue )              &&
			 FxRealEquality( _luminance, other._luminance )  &&
			 FxRealEquality( _saturation, other._saturation ) );
}

FxBool FxColor::operator!= ( const FxColor& other ) const
{
	return !(operator==( other ));
}

FxReal FxColor::GetRed( void ) const
{
	return _red;
}

FxReal FxColor::GetGreen( void ) const
{
	return _green;
}

FxReal FxColor::GetBlue( void ) const
{
	return _blue;
}

FxByte FxColor::GetRedByte( void ) const
{
	return static_cast<FxByte>(_red * 255.0f);
}

FxByte FxColor::GetGreenByte( void ) const
{
	return static_cast<FxByte>(_green * 255.0f);
}

FxByte FxColor::GetBlueByte( void ) const
{
	return static_cast<FxByte>(_blue * 255.0f);
}

FxReal FxColor::GetHue( void ) const
{
	return _hue;
}

FxReal FxColor::GetLuminance( void ) const
{
	return _luminance;
}

FxReal FxColor::GetSaturation( void ) const
{
	return _saturation;
}

// _hlsToRgb(), _value(), and  _rgbToHls() are straight from Foley and van Dam.
void FxColor::_hlsToRgb( void )
{
	if( _saturation == 0.0 )
	{
		FxAssert( _hue == kUndefinedHue );
		_red = _green = _blue = _luminance;
	}
	else
	{
		// The (_luminance + _luminance * _saturation) term here is slightly
		// different than the term presented in Foley and van Dam.
		FxReal m2 = (_luminance <= 0.5f) ? (_luminance + _luminance * _saturation)
			: (_luminance + _saturation - _luminance * _saturation);
		FxReal m1 = 2.0f * _luminance - m2;
		_red   = _value( m1, m2, _hue + 120.0f );
		_green = _value( m1, m2, _hue );
		_blue  = _value( m1, m2, _hue - 120.0f );
	}
}

FxReal FxColor::_value( FxReal n1, FxReal n2, FxReal hue )
{
	if( hue > 360.0f )
	{
		hue -= 360.0f;
	}
	else if( hue < 0.0f )
	{
		hue += 360.0f;
	}
	if( hue < 60.0f )
	{
		return n1 + (n2 - n1) * hue / 60.0f;
	}
	else if( hue < 180.0f )
	{
		return n2;
	}
	else if( hue < 240.0f )
	{
		return n1 + (n2 - n1) * (240.0f - hue) / 60.0f;
	}
	else
	{
		return n1;
	}
}

void FxColor::_rgbToHls( void )
{
	FxReal maxRGB = FxMax<FxReal>(_red, FxMax<FxReal>(_green, _blue));
	FxReal minRGB = FxMin<FxReal>(_red, FxMin<FxReal>(_green, _blue));

	// Luminance.
	_luminance = (maxRGB + minRGB) / 2.0f;

	// Saturation.
	if( maxRGB == minRGB )
	{
		_saturation = 0.0f;
		_hue = kUndefinedHue;
	}
	else
	{
		FxReal delta = maxRGB - minRGB;
		_saturation = (_luminance <= 0.5f) ? (delta / (maxRGB + minRGB)) 
			: (delta / (2.0f - (maxRGB + minRGB)));
		// Hue.
		if( _red == maxRGB )
		{
			// Between yellow and magenta.
			_hue = (_green - _blue) / delta;
		}
		else if( _green == maxRGB )
		{
			// Between cyan and yellow.
			_hue = 2.0f + (_blue - _red) / delta;
		}
		else if( _blue == maxRGB )
		{
			// Between magenta and cyan.
			_hue = 4.0f + (_red - _green) / delta;
		}
		// Convert to degrees.
		_hue *= 60.0f;
		// Make degrees be non-negative.
		if( _hue < 0.0f )
		{
			_hue += 360.0f;
		}
	}
}

} // namespace Face

} // namespace OC3Ent
