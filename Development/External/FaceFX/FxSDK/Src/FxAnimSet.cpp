//------------------------------------------------------------------------------
// A FaceFx animation set that can be dynamically mounted and unmounted from
// any FaceFx Actor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxAnimSet.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreFileFast.h"
#include "FxArchiveStoreMemory.h"
#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxVersionInfo.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAnimSetVersion 0

FX_IMPLEMENT_CLASS(FxAnimSet, kCurrentFxAnimSetVersion, FxNamedObject)

FxAnimSet::FxAnimSet()
{
}

FxAnimSet::FxAnimSet( const FxAnimSet& other )
	: Super(other)
	, _owningActorName(other._owningActorName)
	, _animGroup(other._animGroup)
{
}

FxAnimSet& FxAnimSet::operator=( const FxAnimSet& other )
{
	if( this == &other ) return *this;
	Super::operator=(other);
	_owningActorName = other._owningActorName;
	_animGroup = other._animGroup;
	return *this;
}

FxAnimSet::~FxAnimSet()
{
}

void FxAnimSet::SetOwningActorName( const FxName& owningActorName )
{
	_owningActorName = owningActorName;
}

const FxName& FxAnimSet::GetOwningActorName( void ) const
{
	return _owningActorName;
}

void FxAnimSet::SetAnimGroup( const FxAnimGroup& animGroup )
{
	_animGroup = animGroup;
}

const FxAnimGroup& FxAnimSet::GetAnimGroup( void ) const
{
	return _animGroup;
}

FxAnim* FxAnimSet::GetAnimPtr( FxSize index )
{
	return _animGroup.GetAnimPtr(index);
}

#ifdef __UNREAL__
void FxAnimSet::NullAllAnimUserDataPtrs( void )
{
	FxSize numAnims = _animGroup.GetNumAnims();
	for( FxSize i = 0; i < numAnims; ++i )
	{
		FxAnim* pAnim = _animGroup.GetAnimPtr(i);
		if( pAnim )
		{
			pAnim->SetUserData(NULL);
		}
	}
}
#endif // __UNREAL__

void FxAnimSet::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxAnimSet");
	
	arc << _owningActorName << _animGroup;
}

FxBool FX_CALL FxLoadAnimSetFromFile( FxAnimSet& animSet, const FxChar* filename, const FxBool bUseFastMethod,
							          void(FX_CALL *callbackFunction)(FxReal), FxReal updateFrequency )
{
	FxArchiveStore* pStore = bUseFastMethod ? FxArchiveStoreFileFast::Create(filename)
											: FxArchiveStoreFile::Create(filename);
	FxArchive animSetArchive(pStore, FxArchive::AM_LinearLoad);
	animSetArchive.Open();
	if( animSetArchive.IsValid() )
	{
		animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
		animSetArchive << animSet;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FX_CALL FxSaveAnimSetToFile( FxAnimSet& animSet, const FxChar* filename,
						            FxArchive::FxArchiveByteOrder byteOrder,
						            void(FX_CALL *callbackFunction)(FxReal), FxReal updateFrequency )
{
#ifndef NO_SAVE_VERSION
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << animSet;
	FxArchive animSetArchive(FxArchiveStoreFile::Create(filename), FxArchive::AM_Save, byteOrder);
	animSetArchive.Open();
	if( animSetArchive.IsValid() )
	{
		animSetArchive.SetInternalDataState(directoryCreater.GetInternalData());
		animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
		animSetArchive << animSet;
		return FxTrue;
	}
#else
	animSet; filename; byteOrder; callbackFunction; updateFrequency;
#endif
	return FxFalse;
}

FxBool FX_CALL FxLoadAnimSetFromMemory( FxAnimSet& animSet, const FxByte* pMemory, const FxSize numBytes,
							            void(FX_CALL *callbackFunction)(FxReal), FxReal updateFrequency )
{
	if( pMemory )
	{
		FxArchive animSetArchive(FxArchiveStoreMemoryNoCopy::Create(pMemory, numBytes), FxArchive::AM_LinearLoad);
		animSetArchive.Open();
		if( animSetArchive.IsValid() )
		{
			animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
			animSetArchive << animSet;
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FX_CALL FxSaveAnimSetToMemory( FxAnimSet& animSet, FxByte*& pMemory, FxSize& numBytes,
							          FxArchive::FxArchiveByteOrder byteOrder,
							          void(FX_CALL *callbackFunction)(FxReal), FxReal updateFrequency )
{
#ifndef NO_SAVE_VERSION
	FxAssert(pMemory == NULL);
	if( !pMemory )
	{
		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		directoryCreater << animSet;
		FxArchiveStoreMemory* pStore = FxArchiveStoreMemory::Create(NULL, 0);
		FxArchive animSetArchive(pStore, FxArchive::AM_Save, byteOrder);
		animSetArchive.Open();
		if( animSetArchive.IsValid() )
		{
			animSetArchive.SetInternalDataState(directoryCreater.GetInternalData());
			animSetArchive.RegisterProgressCallback(callbackFunction, updateFrequency);
			animSetArchive << animSet;
			numBytes = pStore->GetSize();
			pMemory = static_cast<FxByte*>(FxAlloc(numBytes, "FxSaveAnimSetToMemory"));
			FxMemcpy(pMemory, pStore->GetMemory(), numBytes);
			return FxTrue;
		}
	}
#else
	animSet; pMemory; numBytes; byteOrder; callbackFunction; updateFrequency;
#endif
	return FxFalse;
}

} // namespace Face

} // namespace OC3Ent
