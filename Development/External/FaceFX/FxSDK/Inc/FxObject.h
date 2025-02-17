//------------------------------------------------------------------------------
// This is the base FaceFx object class.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxObject_H__
#define FxObject_H__

#include "FxPlatform.h"
#include "FxClass.h"
#include "FxArchive.h"
#include "FxMemory.h"

namespace OC3Ent
{

namespace Face
{

/// The base FaceFX object class. 
/// Any class that needs to have a pointer
/// to its type serialized must be derived from FxObject (or FxNamedObject, if
/// the class also needs a name associated with it.) 
/// \ingroup object
class FxObject : public FxUseAllocator
{
	/// Declare the base class.
	FX_DECLARE_BASE_CLASS(FxObject)
public:
	/// Constructor.
	FxObject();
	/// Destructor.
	virtual ~FxObject();

	/// Serialize the object to an archive.
	virtual void Serialize( FxArchive& arc );
};

/// Defines a type-safe dynamic cast for FxObjects.
template<class T> T* FxCast( FxObject* pObj )
{
	return pObj && 
		   pObj->IsKindOf(T::StaticClass()) ? static_cast<T*>(pObj) : NULL;
}

/// A data container allows user data to be associated with an object, handy for implementing tools.
/// \ingroup object
class FxDataContainer
{
public:
	/// Constructor.
	FxDataContainer();
	/// Copy constructor.
	FxDataContainer( const FxDataContainer& other );
	/// Assignment operator.
	FxDataContainer& operator=( const FxDataContainer& other );
	/// Returns the user data pointer.
	void* GetUserData( void );
	/// Sets the user data pointer.
	void  SetUserData( void* userData );
protected:
	/// The user data.
	void* _userData;
};

} // namespace Face

} // namespace OC3Ent

#endif
