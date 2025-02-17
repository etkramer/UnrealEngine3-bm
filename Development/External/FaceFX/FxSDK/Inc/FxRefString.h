//------------------------------------------------------------------------------
// This class implements a reference counted string.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRefString_H__
#define FxRefString_H__

#include "FxPlatform.h"
#include "FxString.h"
#include "FxRefObject.h"

namespace OC3Ent
{

namespace Face
{

/// A reference-counted string.
/// \ingroup support
class FxRefString : public FxRefObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxRefString, FxRefObject)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxRefString)
public:
	/// Constructor.
	FxRefString();
	/// Construct from const FxChar*.
	FxRefString( const FxChar* string );
	/// Destructor.
	virtual ~FxRefString();

	/// Returns the string.
	FX_INLINE const FxString& GetString( void ) { return _string; }
	/// Sets the string.
	void SetString( const FxChar* string );

	/// Serializing an FxRefString is an invalid operation will assert.
	virtual void Serialize( FxArchive& arc );

private:
	/// The string.
	FxString _string;
};

} // namespace Face

} // namespace OC3Ent

#endif
