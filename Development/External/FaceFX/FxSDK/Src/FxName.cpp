//------------------------------------------------------------------------------
// This class is responsible for managing the names of all the named objects
// in the system.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxName.h"
#include "FxUtil.h"
#include "FxList.h"
#include "FxRefString.h"
#include "FxAnim.h"
#include "FxActor.h"

#ifdef FX_DISALLOW_SPACES_IN_NAMES
	#pragma message( "FX_DISALLOW_SPACES_IN_NAMES is defined." )
#endif

namespace OC3Ent
{

namespace Face
{

// The maximum number of characters a name may contain.  If a name contains
// more characters than this then the name is truncated before being added
// into the name system.
#define FX_MAX_NAME_LENGTH 512

// The name table type.
typedef FxList<FxRefString*> FxNameTable;

// The global name table.
static FxNameTable* NameTable;

// Returns the "safe" version of a name, stripping '|' characters.  After this
// function the "safe" version of name will be stored in ProofNameScratch and
// the length of the "safe" version of name will be stored in 
// ProofNameScratchLength.  Note that if name contains more characters than
// FX_MAX_NAME_LENGTH it is simply truncated in ProofNameScratch and no assert
// is triggered.
FX_INLINE void ProofName( const FxChar* name, FxChar* ProofNameScratch, 
						  FxSize ProofNameScratchLength )
{
	for( FxSize i = 0; i < FX_MAX_NAME_LENGTH; ++i )
	{
		if( name[i] == '\0' )
		{
			ProofNameScratchLength = i;
			break;
		}
		else if( name[i] == '|' )
		{
			ProofNameScratch[i] = '_';
		}
		else if( name[i] == '"' )
		{
			ProofNameScratch[i] = '\'';
		}
#ifdef FX_DISALLOW_SPACES_IN_NAMES
		else if( name[i] == ' ' )
		{
			ProofNameScratch[i] = '_';
		}
#endif
		else
		{
			ProofNameScratch[i] = name[i];
		}
	}
	ProofNameScratch[ProofNameScratchLength] = '\0';
}

#define kCurrentFxNameVersion 1

// FxName static initialization.
FxName FxName::NullName;
FxStringHash<FxRefString**> FxName::_hashTable;

FxName::FxName()
	: _ppRefString(NULL)
{
	// Every named object should have a valid name, even if it is the "NULL" 
	// name.
	_ppRefString = NullName._ppRefString;
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
}

FxName::FxName( const FxChar* name )
	: _ppRefString(NULL)
{
	FxAssert(NameTable != NULL);
	_addNameToTable(name);
}

FxName::FxName( const FxName& other )
	: _ppRefString(other._ppRefString)
{
	FxAssert(NameTable != NULL);
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
}

FxName& FxName::operator=( const FxChar* other )
{
	FxAssert(NameTable != NULL);
	_safeRemoveRef();
	_addNameToTable(other);
	return *this;
}

FxName& FxName::operator=( const FxName& other )
{
	if( this == &other ) return *this;
	_safeRemoveRef();
	_ppRefString = other._ppRefString;
	if( _ppRefString )
	{
		(*_ppRefString)->AddReference();
	}
	return *this;
}

FxName::~FxName()
{
	// Only call _safeRemoveRef() if NameTable is valid.  The only way NameTable
	// can be invalid is if FxName::Shutdown() has already been called and it
	// has been destroyed.  In that case, all names are cleaned up already
	// anyway.
	if( NameTable )
	{
		_safeRemoveRef();
	}
}

void FX_CALL FxName::Startup( void )
{
	FxAssert(NameTable == NULL);
	if( !NameTable )
	{
		NameTable = static_cast<FxNameTable*>(FxAlloc(sizeof(FxNameTable), "NameTable"));
		FxDefaultConstruct(NameTable);
		NullName = "";
		FxAnimGroup::Default = "Default";
		FxActor::NewActor = "NewActor";
	}
}

void FX_CALL FxName::Shutdown( void )
{
	// Clear out the name hash.
	_hashTable.Clear();
	// Clear out the name table.
	FxAssert(NameTable != NULL);
	FxNameTable::Iterator curr = NameTable->Begin();
	FxNameTable::Iterator end  = NameTable->End();
	for( ; curr != end; ++curr )
	{
		if( *curr )
		{
			FxRefString* badPointer = *curr;
			FxDelete(*curr);
			FxNameTable::Iterator pos;
			while( (pos = NameTable->Find(badPointer)) != end )
			{
				(*pos) = NULL;
			}
		}
	}
	FxDelete(NameTable);
	NullName._ppRefString = NULL;
	FxAnimGroup::Default._ppRefString = NULL;
	FxActor::NewActor._ppRefString = NULL;
}

FxBool FxName::operator==( const FxChar* other ) const
{
	if( _ppRefString )
	{
		FxAssert(_ppRefString != NULL);
		return (*_ppRefString)->GetString() == other;
	}
	else
	{
		_ppRefString = NullName._ppRefString;
		FxAssert(_ppRefString != NULL);
		return (*_ppRefString)->GetString() == other;
	}
}

FxBool FxName::operator!=( const FxChar* other ) const
{
	if( _ppRefString )
	{
		FxAssert(_ppRefString != NULL);
		return (*_ppRefString)->GetString() != other;
	}
	else
	{
		_ppRefString = NullName._ppRefString;
		FxAssert(_ppRefString != NULL);
		return (*_ppRefString)->GetString() != other;
	}
}

void FxName::SetName( const FxChar* name )
{
	// If the current name is valid remove a reference from it.
	_safeRemoveRef();
	// Add the new name to the name table and name hash.
	_addNameToTable(name);
}

const FxString& FxName::GetAsString( void ) const
{
	if( !_ppRefString )
	{
		_ppRefString = NullName._ppRefString;
	}
	FxAssert(_ppRefString != NULL);
	return (*_ppRefString)->GetString();
}

const FxChar* FxName::GetAsCstr( void ) const
{
	if( !_ppRefString )
	{
		_ppRefString = NullName._ppRefString;
	}
	FxAssert(_ppRefString != NULL);
	return *((*_ppRefString)->GetString());
}

// Implementation note: FxName::Rename() does not need to be particularly fast 
// as it should never be called in-game.  It is strictly a FaceFX Studio-only 
// operation.
void FxName::Rename( const FxChar* name )
{
	FxAssert(_ppRefString != NULL);

	// Make sure that we aren't doing something stupid like renaming a
	// class name, link function name, link function parameter, or the null 
	// name.
	if( (*_ppRefString)->GetString() == NullName.GetAsString() )
	{
		return;
	}
	else
	{
		// Check for class name.
		FxSize numClasses = FxClass::GetNumClasses();
		for( FxSize i = 0; i < numClasses; ++i )
		{
			const FxClass* pClass = FxClass::GetClass(i);
			if( pClass )
			{
				if( (*_ppRefString)->GetString() == pClass->GetName().GetAsString() )
				{
					return;
				}
			}
		}
		// Check for link function.
		FxSize numLinkFunctions = FxLinkFn::GetNumLinkFunctions();
		for( FxSize i = 0; i < numLinkFunctions; ++i )
		{
			const FxLinkFn* pLinkFn = FxLinkFn::GetLinkFunction(i);
			if( pLinkFn )
			{
				if( (*_ppRefString)->GetString() == pLinkFn->GetNameAsString() )
				{
					return;
				}
				// Check for parameter name.
				FxSize numParams = pLinkFn->GetNumParams();
				for( FxSize j = 0; j < numParams; ++j )
				{
					if( (*_ppRefString)->GetString() == pLinkFn->GetParamName(j) )
					{
						return;
					}
				}
			}
		}
	}

	// If execution gets here we can actually rename the name.
	FxChar ProofNameScratch[FX_MAX_NAME_LENGTH];
	FxSize ProofNameScratchLength = 0;
	ProofName(name, ProofNameScratch, ProofNameScratchLength);
	const FxString& theString = (*_ppRefString)->GetString();
	if( theString != ProofNameScratch )
	{
		// Remove the current name from the name hash.
		FxBool removeResult = FxFalse;
		removeResult = _hashTable.Remove(theString.GetData());
		FxAssert(FxFalse != removeResult);

		// Attempt to find the new name in the name table.
		FxNameTable::Iterator curr = NameTable->Begin();
		FxNameTable::Iterator end  = NameTable->End();
		for( ; curr != end; ++curr )
		{
			if( (*curr)->GetString() == ProofNameScratch )
			{
				// The new name is in the name table so cache the number of 
				// references to the old name.
				FxSize numRefs = (*_ppRefString)->GetReferenceCount();
				// And remove that many references from it.  This will cause
				// the reference count to drop to zero at which point the
				// string will be cleaned up.
				for( FxSize i = 0; i < numRefs; ++i )
				{
					(*_ppRefString)->RemoveReference();
				}
				// Set the pointer in the linked list to point to the correct 
				// refstring that was already in the name table.
				(*_ppRefString) = *curr;
				// Add the correct number of references to the refstring.
				for( FxSize i = 0; i < numRefs; ++i )
				{
					(*_ppRefString)->AddReference();
				}
				// The new name should already be in the hash so assert if it
				// isn't.
				FxAssert(FxFalse != _hashTable.Find(ProofNameScratch));
				return;
			}
		}
		// The new name was not in the name table.  Add the new name to the 
		// name hash.
		FxBool insertResult = FxFalse;
		insertResult = _hashTable.Insert(ProofNameScratch, _ppRefString);
		FxAssert(FxFalse != insertResult);
        // Set the refstring directly to the new name.
		(*_ppRefString)->SetString(ProofNameScratch);
	}
}

FxSize FX_CALL FxName::GetNumNames( void )
{
	FxAssert(NameTable != NULL);
	return NameTable->Length();
}

FxSize FX_CALL FxName::Find( const FxChar* nameToFind )
{
	FxAssert(NameTable != NULL);
	FxSize numNames = NameTable->Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		const FxString* pString = GetNameAsString(i);
		if( pString )
		{
			if( *pString == nameToFind )
			{
				return i;
			}
		}
	}
	return FxInvalidIndex;
}

FxSize FX_CALL FxName::GetNameTableSize( void )
{
	FxAssert(NameTable != NULL);
	FxSize result = 0;
	FxSize numNames = NameTable->Length();
	for( FxSize i = 0; i < numNames; ++i )
	{
		const FxString* pString = GetNameAsString(i);
		if( pString )
		{
			result += pString->Length() * sizeof(FxChar);
		}
	}
	return result;
}

const FxString* FX_CALL FxName::GetNameAsString( FxSize index )
{
	FxAssert(NameTable != NULL);
	FxNameTable::Iterator pos = NameTable->Begin();
	pos.Advance(index);
	return &(*pos)->GetString();
}

FxSize FX_CALL FxName::GetNumReferences( FxSize index )
{
	FxAssert(NameTable != NULL);
	FxNameTable::Iterator pos = NameTable->Begin();
	pos.Advance(index);
	return (*pos)->GetReferenceCount();
}

void FX_CALL FxName::GetNameStats( FxSize& numCollisions, FxSize& numNamesAdded,
						           FxSize& numProbes, FxSize& numHashBins, 
						           FxSize& numUsedHashBins, FxSize& memUsed )
{
	numCollisions   = _hashTable.GetNumCollisions();
	numNamesAdded   = _hashTable.GetNumInsertions();
	numProbes       = _hashTable.GetNumProbes();
	numHashBins     = _hashTable.GetNumBins();
	numUsedHashBins = _hashTable.GetNumBinsInUse();
	memUsed         = _hashTable.GetMemUsedEstimate();
}

void FxName::_addNameToTable( const FxChar* name )
{
	FxAssert(NameTable != NULL);
	// Proof the string, make sure it has no '|' characters.
	FxChar ProofNameScratch[FX_MAX_NAME_LENGTH];
	FxSize ProofNameScratchLength = 0;
	ProofName(name, ProofNameScratch, ProofNameScratchLength);
	// Proofed name and its length are now stored in ProofNameScratch and
	// ProofNameScratchLength.

	// Find ProofNameScratch in the name hash.
	FxRefString** ppRefString = NULL;
	FxBool findResult = _hashTable.Find(ProofNameScratch, ppRefString);
	if( findResult && ppRefString )
	{
		// ProofNameScratch is already in the table and name hash.
		_ppRefString = ppRefString;
	}
	else
	{
		// ProofNameScratch is not already in the table and should be added 
		// to both the name hash and the actual name table.
		_ppRefString = &(*NameTable->Insert(new FxRefString(ProofNameScratch), NameTable->End()));
		FxBool insertResult = FxFalse;
        insertResult = _hashTable.Insert(ProofNameScratch, _ppRefString);
		FxAssert(FxFalse != insertResult);
	}
	FxAssert(_ppRefString != NULL);
	// Add a reference to the string.
	(*_ppRefString)->AddReference();
}

FX_INLINE void FxName::_safeRemoveRef( void )
{
	FxAssert(NameTable != NULL);
	if( _ppRefString )
	{
		// Cache a pointer to the underlying refstring.
		FxRefString* pRefString = *_ppRefString;
		// If we're removing the last reference to the name remove it from the
		// name hash and the name table.
		if( pRefString && pRefString->GetReferenceCount() == 1 )
		{
			const FxString& theString = pRefString->GetString();
			FxBool removeResult = FxFalse;
			removeResult = _hashTable.Remove(theString.GetData());
			FxAssert(FxFalse != removeResult);
			NameTable->RemoveIfEqual(pRefString);
			_ppRefString = NULL;
		}
		pRefString->RemoveReference();
	}
}

FxArchive& operator<<( FxArchive& arc, FxName& name )
{
	FxUInt16 version = arc.SerializeClassVersion("FxName", FxTrue, kCurrentFxNameVersion);

	FxAssert(NameTable != NULL);
	if( arc.IsSaving() )
	{
		FxString tempString;
		if( name._ppRefString )
		{
			tempString = (*name._ppRefString)->GetString();
		}
		FxSize nameIndex = arc.AddName(&tempString);
		arc << nameIndex;
	}
	else
	{
		// If we are loading and the reference counted string is valid,
		// we should decrement the reference count of the string before
		// we load into it.
		if( name._ppRefString )
		{
			name._safeRemoveRef();
		}
		if( version < 1 )
		{
			// Prior to version 1 names were stored in archives as strings
			// everywhere.
			FxString tempString;
			arc << tempString;
			name._addNameToTable(tempString.GetData());
		}
		else
		{
			// Starting with version 1 and up strings are stored in archives
			// as indices into the archive's internal name table structure
			// in the archive directory.
			FxSize nameIndex = FxInvalidIndex;
			arc << nameIndex;
			name._ppRefString = reinterpret_cast<FxRefString**>(arc.GetName(nameIndex));
			if( name._ppRefString )
			{
				(*name._ppRefString)->AddReference();
			}
		}
	}
	return arc;
}

} // namespace Face

} // namespace OC3Ent
