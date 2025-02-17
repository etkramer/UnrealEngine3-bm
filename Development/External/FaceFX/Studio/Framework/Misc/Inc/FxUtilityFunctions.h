//------------------------------------------------------------------------------
// A few utility functions available throughout Studio.
// 
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxUtilityFunctions_H__
#define FxUtilityFunctions_H__

#include "FxPlatform.h"
#include "FxColor.h"
#include "FxMath.h"
#include "FxString.h"
#include "FxArray.h"

#ifdef __UNREAL__
	class USoundNode;
	class USoundNodeWave;
	class USoundCue;
#endif

namespace OC3Ent
{

namespace Face
{

#ifdef __UNREAL__
// Finds a USoundNodeWave by name in a USoundCue and returns a pointer to it or
// NULL if the specified USoundNodeWave was not contained in the USoundCue.
USoundNodeWave* FindSoundNodeWave( const USoundCue* SoundCue, const FxString& SoundNodeWaveName );
// Finds all USoundNodeWave objects contained in a USoundCue and returns their 
// names via an array of wxStrings.
FxArray<wxString> FindAllSoundNodeWaves( const USoundCue* SoundCue );
#endif

// Pops up a message box, making sure to respect always on top for any tear-off
// windows.
int FxMessageBox( const wxString& message, 
				  const wxString& caption = wxMessageBoxCaptionStr, 
				  int style = wxOK|wxCENTRE, wxWindow* parent = NULL, 
				  int x = wxDefaultCoord, int y = wxDefaultCoord );

// Returns a string containing the name of the interpolation algorithm.
FxString GetInterpName( FxInterpolationType interp );
// Returns an interpolation algorithm given a string name.
FxInterpolationType GetInterpolator( const FxString& name );

// Converts a string representation of a colour (e.g., "255 0 0") to a colour.
wxColour FxStringToColour( const wxString& string );
// Converts a colour into a string representation.
wxString FxColourToString( const wxColour& colour );
// Converts a string representation of a color (e.g., "255 0 0") to a color.
FxColor FxStringToColor( const wxString& string );
// Converts a colour to a string representation.
wxString FxColorToString( const FxColor& color );

// Compares two FxNames in a wxListCtrl.
int wxCALLBACK CompareNames( long item1, long item2, long sortData );
// Compares two FxStrings in a wxListCtrl.
int wxCALLBACK CompareStrings( long item1, long item2, long sortData );

// The types of script trigger events.
enum FxScriptTriggerEventType
{
	STET_Invalid = -1,
	STET_PostLoadApp,
	STET_PreLoadActor,
	STET_PostLoadActor,
	STET_PreSaveActor,
	STET_PostSaveActor,

	STET_First = STET_PostLoadApp,
	STET_Last  = STET_PostSaveActor, 
	STET_NumScriptTriggerEventTypes = STET_Last - STET_First + 1 
};

// Triggers a script event and calls the appropriate script with an exec 
// command.
void FxTriggerScriptEvent( FxScriptTriggerEventType trigger, void* param = NULL );

// Returns true if a file exists, and false if it doesn't.
FxBool FxFileExists( const FxString& filename );
FxBool FxFileExists( const wxString& filename );

// Returns the text in the text file or an empty string.  This loads the file
// in a Unicode friendly way.
wxString FxLoadUnicodeTextFile( const FxString& filename );

//@todo These currently only work on Win32 and always return FxFalse when
//      __UNREAL__ is defined or WIN32 is not defined.
FxBool FxFileIsReadOnly( const FxString& filename );
FxBool FxFileIsReadOnly( const wxString& filename );

} // namespace Face

} // namespace OC3Ent

#endif
