//------------------------------------------------------------------------------
// A built-in FaceFx class.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxClass.h"

namespace OC3Ent
{

namespace Face
{

FxArray<FxClass*>* FxClass::_classTable = NULL;

FxClass::FxClass( const FxChar* className, FxUInt16 currentVersion, 
		          const FxClass* pBaseClassDesc, FxSize classSize, 
		          FxConstructObjectFunc constructObjectFn )
	: _name(className)
	, _currentVersion(currentVersion)
	, _pBaseClassDesc(pBaseClassDesc)
	, _numChildren(0)
	, _size(classSize)
	, _constructObjectFn(constructObjectFn)
{
	if( !_classTable )
	{
		FxAssert(!"Attempt to create FxClass before FxClass::Startup() called!");
	}
	const FxClass* duplicateClass = FindClassDesc(className);
	if( duplicateClass != NULL )
	{
		FxAssert(!"Attempting to create duplicate FxClass!");
	}
	if( _pBaseClassDesc )
	{
		// Temporarily cast away const to increment the child count of 
		// the base class.
		const_cast<FxClass*>(_pBaseClassDesc)->_numChildren++;
	}
	_classTable->PushBack(this);
}

FxClass::~FxClass()
{
}

// This is the number of entries to reserve in the class table.  Currently there
// are only 36 base FaceFX SDK classes, but 64 should give us a good cushion
// and doesn't waste too much space.  This prevents memory fragmentation 
// resulting from the _classTable array being reallocated during SDK startup 
// when all of the classes are registered.  Note that if the base FaceFX SDK 
// ever exceeds 64 classes it will be a good idea to bump this number up to keep
// preventing memory fragmentation.
#define FX_NUM_CLASSES_TO_RESERVE 64
void FX_CALL FxClass::Startup( void )
{
	if( !_classTable )
	{
		_classTable = static_cast<FxArray<FxClass*>*>(FxAlloc(sizeof(FxArray<FxClass*>), "ClassTable"));
		FxDefaultConstruct(_classTable);
		_classTable->Reserve(FX_NUM_CLASSES_TO_RESERVE);
	}
}

void FX_CALL FxClass::Shutdown( void )
{
	if( _classTable )
	{
		// Delete all classes in the class table.
		FxSize numClasses = _classTable->Length();
		for( FxSize i = 0; i < numClasses; ++i )
		{
			FxDelete((*_classTable)[i]);
		}
		_classTable->Clear();
		FxDelete(_classTable);
	}
}

const FxName& FxClass::GetName( void ) const
{ 
	return _name; 
}

const FxUInt16 FxClass::GetCurrentVersion( void ) const
{ 
	return _currentVersion; 
}

const FxClass* FxClass::GetBaseClassDesc( void ) const
{ 
	return _pBaseClassDesc; 
}

const FxSize FxClass::GetSize( void ) const
{ 
	return _size; 
}

const FxSize FxClass::GetNumChildren( void ) const
{
	return _numChildren;
}

FxObject* FxClass::ConstructObject( void ) const
{ 
	if( _constructObjectFn ) 
	{
		return _constructObjectFn(); 
	}
	return NULL;
}

const FxClass* FX_CALL FxClass::FindClassDesc( const FxName& className )
{
	if( _classTable )
	{
		FxSize numClasses = _classTable->Length();
		for( FxSize i = 0; i < numClasses; ++i )
		{
			if( (*_classTable)[i]->GetName() == className )
			{
				return (*_classTable)[i];
			}
		}
	}
	return NULL;
}

FxSize FX_CALL FxClass::GetNumClasses( void )
{
	if( _classTable )
	{
		return _classTable->Length();
	}
	return 0;
}

const FxClass* FX_CALL FxClass::GetClass( FxSize index )
{
	if( _classTable )
	{
		return (*_classTable)[index];
	}
	return NULL;
}

FxBool FxClass::IsExactKindOf( const FxClass* classDesc ) const
{
	return IsA(classDesc);
}

FxBool FxClass::IsKindOf( const FxClass* classDesc ) const
{
	const FxClass* testClass = this;
	while( testClass )
	{
		if( testClass == classDesc ) return FxTrue;
		testClass = testClass->GetBaseClassDesc();
	}
	return FxFalse;
}

FxBool FxClass::IsA( const FxClass* classDesc ) const
{
	return (this == classDesc);
}

} // namespace Face

} // namespace OC3Ent
