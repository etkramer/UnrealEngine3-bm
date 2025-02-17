//------------------------------------------------------------------------------
// Global colour information for the application.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxColourMapping_H__
#define FxColourMapping_H__

#include "FxPlatform.h"
#include "FxName.h"
#include "FxArray.h"
#include "FxArchive.h"

namespace OC3Ent
{

namespace Face
{

// An entry in the FxColourMap
struct FxColourMapEntry
{
	FxColourMapEntry( const FxName& n = FxName::NullName, const wxColour& c = *wxBLACK )
		: name(n)
		, colour(c)
	{
	}

	FxName   name;
	wxColour colour;
};

// Colour map entry serialization.
FxArchive& operator<<( FxArchive& arc, FxColourMapEntry& obj );

// A map from FxNames to wxColours
class FxColourMap
{
public:
	// Clears the mapping
	static void Clear();
	// Sets an entry in the mapping
	static void Set( const FxName& name, const wxColour& colour );
	// Gets an entry from the mapping
	static wxColour Get( const FxName& name );
	// Removes an entry from the mapping
	static void Remove( const FxName& name );

	// Exports the colour map (for saving in templates).
	static FxArray<FxColourMapEntry> ExportColourMap();

	// Returns the custom colours
	static wxColour* GetCustomColours();
	// Returns the number of custom colours
	static FxSize GetNumCustomColours();

	// Serializes to an archive
	static void Serialize( FxArchive& arc );

private:
	static FxSize Find( const FxName& name );
	static FxArray<FxColourMapEntry> _colourMap;
	static wxColour _customColours[16];
};

} // namespace Face

} // namespace OC3Ent

#endif
