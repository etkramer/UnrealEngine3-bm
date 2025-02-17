//------------------------------------------------------------------------------
// This class implements a named FaceFx object.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxNamedObject.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxNamedObjectVersion 0

FX_IMPLEMENT_CLASS(FxNamedObject, kCurrentFxNamedObjectVersion, FxObject)

FxNamedObject::FxNamedObject()
	: _name(FxName::NullName)
{
}

FxNamedObject::FxNamedObject( const FxNamedObject& other )
{
	_name = other.GetName();
}

FxNamedObject::FxNamedObject( const FxName& name )
	: _name(name)
{
}

FxNamedObject::~FxNamedObject()
{
}

FxNamedObject& FxNamedObject::operator=( const FxNamedObject& other )
{
	if( this == &other ) return *this;
	_name = other.GetName();
	return *this;
}

void FxNamedObject::SetName( const FxName& name )
{
	_name = name;
}

void FxNamedObject::Rename( const FxChar* name )
{
	_name.Rename(name);
}

void FxNamedObject::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxNamedObject");

	arc << _name;
}

} // namespace Face

} // namespace OC3Ent
