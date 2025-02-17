//------------------------------------------------------------------------------
// This class implements a reference counted object.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRefObject_H__
#define FxRefObject_H__

#include "FxPlatform.h"
#include "FxObject.h"

namespace OC3Ent
{

namespace Face
{

/// A reference-counted FxObject.
/// \ingroup object
class FxRefObject : public FxObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxRefObject, FxObject)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxRefObject)
public:
	/// Constructor.
	/// FxRefObjects should only ever be created via a call to operator \p new.
	FxRefObject();
	/// Destructor.
	virtual ~FxRefObject();
	
	/// Adds a reference to this object and returns the number of references.
	FxUInt32 AddReference( void );
	/// Removes a reference to this object and returns the number of references.
	/// If the number of references is zero, the object is destroyed.
	FxUInt32 RemoveReference( void );

	/// Returns the number of references.
	FxUInt32 GetReferenceCount( void );

	/// Serializing an FxRefObject is an invalid operation will assert.
	virtual void Serialize( FxArchive& arc );

private:
	/// The object's reference count.
	FxUInt32 _referenceCount;
};

FX_INLINE FxUInt32 FxRefObject::AddReference( void )
{
	_referenceCount++;
	return _referenceCount;
}

FX_INLINE FxUInt32 FxRefObject::RemoveReference( void )
{
	_referenceCount--;
	if( _referenceCount <= 0 )
	{
		const FxSize size = this->GetClassDesc()->GetSize();
		this->~FxRefObject();
		FxFree(this, size);
		return 0;
	}
	return _referenceCount;
}

FX_INLINE FxUInt32 FxRefObject::GetReferenceCount( void )
{
	return _referenceCount;
}

} // namespace Face

} // namespace OC3Ent

#endif
