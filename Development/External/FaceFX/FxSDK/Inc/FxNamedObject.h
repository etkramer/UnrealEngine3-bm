//------------------------------------------------------------------------------
// This class implements a named FaceFx object.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxNamedObject_H__
#define FxNamedObject_H__

#include "FxPlatform.h"
#include "FxObject.h"
#include "FxArchive.h"
#include "FxName.h"

namespace OC3Ent
{

namespace Face
{

/// A named FxObject
/// \ingroup object
class FxNamedObject : public FxObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxNamedObject, FxObject)
public:
	/// Constructor.
	FxNamedObject();
	/// Copy constructor.
	FxNamedObject( const FxNamedObject& other );
	/// Construct from name.
	FxNamedObject( const FxName& name );
	/// Destructor.
	virtual ~FxNamedObject();

	/// Assignment.
	FxNamedObject& operator=( const FxNamedObject& other );

	/// Sets the name of the object.
	void SetName( const FxName& name );
	/// Renames the named object and any dependencies.
	void Rename( const FxChar* name );
	/// Gets the name of the object.
	FX_INLINE const FxName& GetName( void ) const { return _name; }
	/// Gets the name of the object as an FxString.
	FX_INLINE const FxString& GetNameAsString( void ) const { return _name.GetAsString(); } 
	/// Gets the name of the object as a C-style string
	FX_INLINE const FxChar* GetNameAsCstr( void ) const { return *_name.GetAsString(); }

	/// Serializes the object to an archive.
	virtual void Serialize( FxArchive& arc );

protected:
	/// The name of the object.
	FxName _name;
};

} // namespace Face

} // namespace OC3Ent

#endif
