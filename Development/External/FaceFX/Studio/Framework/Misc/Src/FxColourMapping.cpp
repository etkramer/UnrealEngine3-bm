//------------------------------------------------------------------------------
// Global colour information for the application.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxColourMapping.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxColourMapVersion 0

FxArray<FxColourMapEntry> FxColourMap::_colourMap;
wxColour FxColourMap::_customColours[16];

FxArchive& operator<<( FxArchive& arc, FxColourMapEntry& obj )
{
	FxUChar r = obj.colour.Red();
	FxUChar g = obj.colour.Green();
	FxUChar b = obj.colour.Blue();
	arc << obj.name << r << g << b;
	obj.colour = wxColour(r, g, b);
	return arc;
}

// Clears the mapping
void FxColourMap::Clear()
{
	_colourMap.Clear();
}

// Sets an entry in the mapping
void FxColourMap::Set( const FxName& name, const wxColour& colour )
{
	// Attempt to find the name in the list.
	FxSize location = Find( name );
	if( location == FxInvalidIndex )
	{
		// If we're a new entry, add it to the list.
		FxColourMapEntry newEntry(name, colour);
		_colourMap.PushBack( newEntry );
	}
	else
	{
		// Otherwise, update the colour already in the list.
		_colourMap[location].colour = colour;
	}
}

// Gets an entry from the mapping
wxColour FxColourMap::Get( const FxName& name )
{
	FxSize location = Find( name );
	if( location != FxInvalidIndex )
	{
		return _colourMap[location].colour;
	}
	return *wxBLACK;
}

// Removes an entry from the mapping
void FxColourMap::Remove( const FxName& name )
{
	FxSize location = Find( name );
	if( location != FxInvalidIndex )
	{
		_colourMap.Remove(location);
	}
}

// Exports the colour map (for saving in templates).
FxArray<FxColourMapEntry> FxColourMap::ExportColourMap()
{
	return _colourMap;
}

FxSize FxColourMap::Find( const FxName& name )
{
	for( FxSize i = 0; i < _colourMap.Length(); ++i )
	{
		if( _colourMap[i].name == name )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

// Returns the custom colours
wxColour* FxColourMap::GetCustomColours()
{
	return _customColours;
}

// Returns the number of custom colours
FxSize FxColourMap::GetNumCustomColours()
{
	return 16;
}

// Serializes to an archive
void FxColourMap::Serialize( FxArchive& arc )
{
	arc.SerializeClassVersion("FxColourMap", FxTrue, kCurrentFxColourMapVersion);
	arc << _colourMap;
}

} // namespace Face

} // namespace OC3Ent
