//------------------------------------------------------------------------------
// External animation set manager.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnimSetManager.h"
#include "FxArchive.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreFileFast.h"
#include "FxArchiveStoreMemory.h"
#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxActor.h"
#include "FxStudioSession.h"
#include "FxConsole.h"

#ifdef IS_FACEFX_STUDIO
#include "FxSessionProxy.h"
#endif

#include "FxVM.h"

#ifdef __UNREAL__
	#include "UnrealEd.h"
	#include "EngineSequenceClasses.h"
	#include "EngineInterpolationClasses.h"
	#include "InterpEditor.h"
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// FxAnimSetMiniSession.
//------------------------------------------------------------------------------

#define kCurrentFxAnimSetMiniSessionVersion 0

FX_IMPLEMENT_CLASS(FxAnimSetMiniSession, kCurrentFxAnimSetMiniSessionVersion, FxObject)

FxAnimSetMiniSession::FxAnimSetMiniSession()
{
}

FxAnimSetMiniSession::FxAnimSetMiniSession( const FxAnimSetMiniSession& other )
{
	Clear();
	FxSize numAnimUserDataObjects = other._animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
	{
		if( other._animUserDataList[i] )
		{
			FxAnimUserData* pNewAnimUserData = new FxAnimUserData(*other._animUserDataList[i]);
			_animUserDataList.PushBack(pNewAnimUserData);
		}
	}
}

FxAnimSetMiniSession& FxAnimSetMiniSession::operator=( const FxAnimSetMiniSession& other )
{
	if( this == &other ) return *this;
	Clear();
	FxSize numAnimUserDataObjects = other._animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
	{
		if( other._animUserDataList[i] )
		{
			FxAnimUserData* pNewAnimUserData = new FxAnimUserData(*other._animUserDataList[i]);
			_animUserDataList.PushBack(pNewAnimUserData);
		}
	}
	return *this;
}

FxAnimSetMiniSession::~FxAnimSetMiniSession()
{
	Clear();
}

void FxAnimSetMiniSession::Clear( void )
{
	FxSize numAnimUserDataObjects = _animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
	{
		if( _animUserDataList[i] )
		{
			delete _animUserDataList[i];
			_animUserDataList[i] = NULL;
		}
	}
	_animUserDataList.Clear();
}

FxAnimUserData* FxAnimSetMiniSession::FindAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	FxSize numAnimUserDataObjects = _animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
	{
		if( _animUserDataList[i] )
		{
			FxString userDataGroupName, userDataAnimName;
			_animUserDataList[i]->GetNames(userDataGroupName, userDataAnimName);
			if( FxName(userDataGroupName.GetData()) == animGroupName && FxName(userDataAnimName.GetData()) == animName )
			{
				return _animUserDataList[i];
			}
		}
	}
	return NULL;
}

void FxAnimSetMiniSession::AddAnimUserData( FxAnimUserData* pAnimUserData )
{
	FxAssert(pAnimUserData);
	if( pAnimUserData )
	{
		FxString userDataGroupName, userDataAnimName;
		pAnimUserData->GetNames(userDataGroupName, userDataAnimName);
		if( !FindAnimUserData(userDataGroupName.GetData(), userDataAnimName.GetData()) )
		{
			_animUserDataList.PushBack(pAnimUserData);
		}
	}
}

void FxAnimSetMiniSession::RemoveAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	FxSize numAnimUserDataObjects = _animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
	{
		if( _animUserDataList[i] )
		{
			FxString userDataGroupName, userDataAnimName;
			_animUserDataList[i]->GetNames(userDataGroupName, userDataAnimName);
			if( FxName(userDataGroupName.GetData()) == animGroupName && FxName(userDataAnimName.GetData()) == animName )
			{
				delete _animUserDataList[i];
				_animUserDataList[i] = NULL;
				_animUserDataList.Remove(i);
				return;
			}
		}
	}
}

void FxAnimSetMiniSession::FindAndRemoveOrphanedAnimUserData( void )
{
#ifdef IS_FACEFX_STUDIO
	// First get a pointer to the current actor.
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxSize numAnimUserDataObjects = _animUserDataList.Length();
		for( FxSize i = 0; i < numAnimUserDataObjects; ++i )
		{
			if( _animUserDataList[i] )
			{
				FxString userDataGroupName, userDataAnimName;
				_animUserDataList[i]->GetNames(userDataGroupName, userDataAnimName);
				// Find any animations in the actor that could match this combination.
				if( !pActor->GetAnimPtr(FxName(userDataGroupName.GetData()), FxName(userDataAnimName.GetData())) )
				{
					// No match was found.  Warn the user and destroy the user data.
					delete _animUserDataList[i];
					_animUserDataList[i] = NULL;
					_animUserDataList.Remove(i);
					FxWarn("Found and removed orphaned animation user data object: <b>\"%s.%s\"</b>", userDataGroupName.GetData(), userDataAnimName.GetData());
				}
			}
		}
	}
#endif
}

FxSize FxAnimSetMiniSession::GetNumUserDataObjects( void ) const
{
	return _animUserDataList.Length();
}

FxAnimUserData* FxAnimSetMiniSession::GetUserDataObjectClone( FxSize index ) const
{
	if( _animUserDataList[index] )
	{
		FxAnimUserData* pClonedUserData = new FxAnimUserData(*_animUserDataList[index]);
		return pClonedUserData;
	}
	return NULL;
}

void FxAnimSetMiniSession::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxAnimSetMiniSession");
	
	arc << _animUserDataList;
}

FxBool FxLoadAnimSetMiniSessionFromFile( FxAnimSetMiniSession& miniSession, const FxChar* filename, const FxBool bUseFastMethod )
{
	FxArchiveStore* pStore = bUseFastMethod ? FxArchiveStoreFileFast::Create(filename)
											: FxArchiveStoreFile::Create(filename);
	FxArchive miniSessionArchive(pStore, FxArchive::AM_LinearLoad);
	miniSessionArchive.Open();
	if( miniSessionArchive.IsValid() )
	{
		miniSessionArchive << miniSession;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxSaveAnimSetMiniSessionToFile( FxAnimSetMiniSession& miniSession, const FxChar* filename )
{
	FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
	directoryCreater.Open();
	directoryCreater << miniSession;
	FxArchive miniSessionArchive(FxArchiveStoreFile::Create(filename), FxArchive::AM_Save);
	miniSessionArchive.Open();
	if( miniSessionArchive.IsValid() )
	{
		miniSessionArchive.SetInternalDataState(directoryCreater.GetInternalData());
		miniSessionArchive << miniSession;
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxLoadAnimSetMiniSessionFromMemory( FxAnimSetMiniSession& miniSession, const FxByte* pMemory, const FxSize numBytes )
{
	if( pMemory )
	{
		FxArchive miniSessionArchive(FxArchiveStoreMemoryNoCopy::Create(pMemory, numBytes), FxArchive::AM_LinearLoad);
		miniSessionArchive.Open();
		if( miniSessionArchive.IsValid() )
		{
			miniSessionArchive << miniSession;
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxSaveAnimSetMiniSessionToMemory( FxAnimSetMiniSession& miniSession, FxByte*& pMemory, FxSize& numBytes )
{
	FxAssert(pMemory == NULL);
	if( !pMemory )
	{
		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		directoryCreater << miniSession;
		FxArchiveStoreMemory* pStore = FxArchiveStoreMemory::Create(NULL, 0);
		FxArchive miniSessionArchive(pStore, FxArchive::AM_Save);
		miniSessionArchive.Open();
		if( miniSessionArchive.IsValid() )
		{
			miniSessionArchive.SetInternalDataState(directoryCreater.GetInternalData());
			miniSessionArchive << miniSession;
			numBytes = pStore->GetSize();
			pMemory = static_cast<FxByte*>(FxAlloc(numBytes, "FxSaveAnimSetToMemory"));
			FxMemcpy(pMemory, pStore->GetMemory(), numBytes);
			return FxTrue;
		}
	}
	return FxFalse;
}

#ifdef IS_FACEFX_STUDIO
//------------------------------------------------------------------------------
// FxAnimSetManagerEntry.
//------------------------------------------------------------------------------

FxAnimSetManagerEntry::FxAnimSetManagerEntry()
{
#ifdef __UNREAL__
	_pAnimSet = NULL;
	_bShouldRemount = FxFalse;
#endif // __UNREAL__
}

FxAnimSetManagerEntry::~FxAnimSetManagerEntry()
{
}

void FxAnimSetManagerEntry::Clear( void )
{
	_animSetMiniSession.Clear();
}

void FxAnimSetManagerEntry::SetAnimSetArchivePath( const FxString& animSetArchivePath )
{
	_animSetArchivePath = animSetArchivePath;
}

const FxString& FxAnimSetManagerEntry::GetAnimSetArchivePath( void ) const
{
	return _animSetArchivePath;
}

void FxAnimSetManagerEntry::SetAnimSetSessionArchivePath( const FxString& animSetSessionArchivePath )
{
	_animSetSessionArchivePath = animSetSessionArchivePath;
}

const FxString& FxAnimSetManagerEntry::GetAnimSetSessionArchivePath( void ) const
{
	return _animSetSessionArchivePath;
}

const FxName& FxAnimSetManagerEntry::GetContainedAnimGroupName( void ) const
{
	return _animSet.GetAnimGroup().GetName();
}

void FxAnimSetManagerEntry::SetAnimSet( const FxAnimSet& animSet )
{
	_animSet = animSet;
}

const FxAnimSet& FxAnimSetManagerEntry::GetAnimSet( void ) const
{
	return _animSet;
}

void FxAnimSetManagerEntry::SetAnimSetSession( const FxAnimSetMiniSession& animSetMiniSession )
{
	_animSetMiniSession= animSetMiniSession;
}

const FxAnimSetMiniSession& FxAnimSetManagerEntry::GetAnimSetSession( void ) const
{
	return _animSetMiniSession;
}

FxAnimUserData* FxAnimSetManagerEntry::FindAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	return _animSetMiniSession.FindAnimUserData(animGroupName, animName);
}

void FxAnimSetManagerEntry::AddAnimUserData( FxAnimUserData* pAnimUserData )
{
	_animSetMiniSession.AddAnimUserData(pAnimUserData);
}

void FxAnimSetManagerEntry::RemoveAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	_animSetMiniSession.RemoveAnimUserData(animGroupName, animName);
}

void FxAnimSetManagerEntry::FindAndRemoveOrphanedAnimUserData( void )
{
	_animSetMiniSession.FindAndRemoveOrphanedAnimUserData();
}

void FxAnimSetManagerEntry::Load( const FxString& animSetArchivePath )
{
	// Load the animation set from the archive.
	if( FxLoadAnimSetFromFile(_animSet, animSetArchivePath.GetData(), FxTrue) )
	{
		// Load the mini session from the archive.
		FxString miniSessionArchivePath = animSetArchivePath;
		miniSessionArchivePath[miniSessionArchivePath.Length()-3] = 'f';
		miniSessionArchivePath[miniSessionArchivePath.Length()-2] = 'x';
		miniSessionArchivePath[miniSessionArchivePath.Length()-1] = 'm';
		FxLoadAnimSetMiniSessionFromFile(_animSetMiniSession, miniSessionArchivePath.GetData(), FxTrue);
		_animSetArchivePath = animSetArchivePath;
		_animSetSessionArchivePath = miniSessionArchivePath;
	}
}

void FxAnimSetManagerEntry::Save( const FxString& animSetArchivePath )
{
	// Save the animation set to an archive.
	FxSaveAnimSetToFile(_animSet, animSetArchivePath.GetData());

	// Save the mini session to an archive.
	FxString miniSessionArchivePath = animSetArchivePath;
	miniSessionArchivePath[miniSessionArchivePath.Length()-3] = 'f';
	miniSessionArchivePath[miniSessionArchivePath.Length()-2] = 'x';
	miniSessionArchivePath[miniSessionArchivePath.Length()-1] = 'm';
	FxSaveAnimSetMiniSessionToFile(_animSetMiniSession, miniSessionArchivePath.GetData());
}

#ifdef __UNREAL__
void FxAnimSetManagerEntry::SetUE3AnimSet( UFaceFXAnimSet* pAnimSet )
{
	_pAnimSet = pAnimSet;
}

UFaceFXAnimSet* FxAnimSetManagerEntry::GetUE3AnimSet( void )
{
	return _pAnimSet;
}

void FxAnimSetManagerEntry::SetShouldRemount( FxBool bShouldRemount )
{
	_bShouldRemount = bShouldRemount;
}

FxBool FxAnimSetManagerEntry::ShouldRemount( void )
{
	return _bShouldRemount;
}

void FxAnimSetManagerEntry::Load( void )
{
	if( _pAnimSet )
	{
		// Load the animation set.
		FxAnimSet* pInternalAnimSet = _pAnimSet->GetFxAnimSet();
		if( pInternalAnimSet )
		{
			// Need to NULL all of the user data pointers to be safe since in
			// Unreal these things hang around in memory between runs of Studio.
			pInternalAnimSet->NullAllAnimUserDataPtrs();
			// Need to reload the animation set from the raw bytes.
			FxLoadAnimSetFromMemory(*pInternalAnimSet, static_cast<FxByte*>(_pAnimSet->RawFaceFXAnimSetBytes.GetData()), _pAnimSet->RawFaceFXAnimSetBytes.Num());
			_pAnimSet->ReferencedSoundCues.Empty();
			_pAnimSet->FixupReferencedSoundCues();
			_animSet = *pInternalAnimSet;
		}
		// Load the mini session.
		FxLoadAnimSetMiniSessionFromMemory(_animSetMiniSession, static_cast<FxByte*>(_pAnimSet->RawFaceFXMiniSessionBytes.GetData()),
			_pAnimSet->RawFaceFXMiniSessionBytes.Num());
	}
}

void FxAnimSetManagerEntry::Save( void )
{
	if( _pAnimSet )
	{
		// Save the animation set.
		FxAnimSet* pInternalAnimSet = _pAnimSet->GetFxAnimSet();
		if( pInternalAnimSet )
		{
			*pInternalAnimSet = _animSet;
			// Need to NULL all of the user data pointers to be safe since in
			// Unreal these things hang around in memory between runs of Studio.
			pInternalAnimSet->NullAllAnimUserDataPtrs();
		}
		FxByte* animSetMemory = NULL;
		FxSize  animSetMemorySize = 0;
		FxSaveAnimSetToMemory(_animSet, animSetMemory, animSetMemorySize);
		_pAnimSet->RawFaceFXAnimSetBytes.Empty();
		_pAnimSet->RawFaceFXAnimSetBytes.Add(animSetMemorySize);
		appMemcpy(_pAnimSet->RawFaceFXAnimSetBytes.GetData(), animSetMemory, _pAnimSet->RawFaceFXAnimSetBytes.Num());
		FxFree(animSetMemory, animSetMemorySize);
		_pAnimSet->ReferencedSoundCues.Empty();
		_pAnimSet->FixupReferencedSoundCues();
		// Save the mini session.
		FxByte* miniSessionMemory = NULL;
		FxSize  miniSessionMemorySize = 0;
		FxSaveAnimSetMiniSessionToMemory(_animSetMiniSession, miniSessionMemory, miniSessionMemorySize);
		_pAnimSet->RawFaceFXMiniSessionBytes.Empty();
		_pAnimSet->RawFaceFXMiniSessionBytes.Add(miniSessionMemorySize);
		appMemcpy(_pAnimSet->RawFaceFXMiniSessionBytes.GetData(), miniSessionMemory, _pAnimSet->RawFaceFXMiniSessionBytes.Num());
		FxFree(miniSessionMemory, miniSessionMemorySize);
		_pAnimSet->MarkPackageDirty();
	}
}
#endif // __UNREAL__

//------------------------------------------------------------------------------
// FxAnimSetManager.
//------------------------------------------------------------------------------

FxArray<FxAnimSetManagerEntry> FxAnimSetManager::_externalAnimSetEntries;
FxString FxAnimSetManager::_cachedSelectedAnimGroupName;
FxString FxAnimSetManager::_cachedSelectedAnimName;

void FxAnimSetManager::FlushAllMountedAnimSets( void )
{
	FxSize numEntries = _externalAnimSetEntries.Length();
	for( FxSize i = 0; i < numEntries; ++i )
	{
		_externalAnimSetEntries[i].Clear();
	}
	_externalAnimSetEntries.Clear();
}

FxSize FxAnimSetManager::GetNumMountedAnimSets( void )
{
	return _externalAnimSetEntries.Length();
}

FxString FxAnimSetManager::GetMountedAnimSetName( FxSize index )
{
	return _externalAnimSetEntries[index].GetContainedAnimGroupName().GetAsString();
}

FxBool FxAnimSetManager::AreAnyFilesReadOnly( void )
{
#ifdef __UNREAL__
	return FxFalse;
#else
	FxBool areAnyFilesReadOnly = FxFalse;
	FxSize numEntries = _externalAnimSetEntries.Length();
	for( FxSize i = 0; i < numEntries; ++i )
	{
		const FxString& fxePath = _externalAnimSetEntries[i].GetAnimSetArchivePath();
		const FxString& fxmPath = _externalAnimSetEntries[i].GetAnimSetSessionArchivePath();
		if( FxFileIsReadOnly(fxePath) )
		{
			FxError("%s is read-only!", fxePath.GetData());
			areAnyFilesReadOnly = FxTrue;
		}
		if( FxFileIsReadOnly(fxmPath) )
		{
			FxError("%s is read-only!", fxmPath.GetData());
			areAnyFilesReadOnly = FxTrue;
		}
	}
	return areAnyFilesReadOnly;
#endif
}

FxBool FxAnimSetManager::MountAnimSet( const FxString& animSetPath )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor && FxFileExists(animSetPath) )
	{
		FxString selectedAnimGroup;
		FxString selectedAnim;
		FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
		FxSessionProxy::GetSelectedAnim(selectedAnim);

		// This is going to cause a reallocation of some internal arrays, so
		// deselect the currently selected animation group by selecting
		// the default group with no animation.
		FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());

		// Load the animation set and the animation set mini session.
		FxAnimSetManagerEntry newEntry;
		newEntry.Load(animSetPath);

		// FxActor::MountAnimSet() ensures that no duplicates are loaded and 
		// thus it also ensures that there will be no duplicates loaded in
		// the FxAnimSetManager.
		if( pActor->MountAnimSet(newEntry.GetAnimSet()) )
		{
			_externalAnimSetEntries.PushBack(newEntry);
			// Broadcast an internal data changed message indicating that the animation
			// groups have changed.
			FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);
			// Select the newly mounted animation group.
			FxSessionProxy::SetSelectedAnimGroup(newEntry.GetContainedAnimGroupName().GetAsString());
			return FxTrue;
		}
		else
		{
			// If for some reason mounting failed, reselect the previously
			// selected animation.
			FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
			FxSessionProxy::SetSelectedAnim(selectedAnim);
		}
	}
	return FxFalse;
}

FxBool FxAnimSetManager::UnmountAnimSet( const FxName& animSetName )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxSize mountedSetIndex = FindMountedAnimSet(animSetName);
		if( FxInvalidIndex != mountedSetIndex )
		{
			// Check to see if the animation set has been modified since the
			// last save.
			const FxAnimSet& animSet = _externalAnimSetEntries[mountedSetIndex].GetAnimSet();
			const FxAnimGroup& animGroup = animSet.GetAnimGroup();
			FxSize index = pActor->FindAnimGroup(animGroup.GetName());
			if( FxInvalidIndex != index )
			{
				if( pActor->GetAnimGroup(index) != animGroup )
				{
					// It has been modified so warn the user.
					if( wxNO == FxMessageBox(_("That animation set has been modified since the last save operation. If you continue you will lose your changes. To save your changes choose No and then save in FaceFX Studio."), _("Continue?"), wxYES_NO|wxICON_QUESTION) )
					{
						// Return FxTrue so that the command does not fail with an error state.
						FxMsg("User chose not to continue unmounting because the animation set is modified.");
						return FxTrue;
					}
				}
			}

			FxString selectedAnimGroup;
			FxString selectedAnim;
			FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
			FxSessionProxy::GetSelectedAnim(selectedAnim);

			// This is going to cause a reallocation of some internal arrays, so
			// deselect the currently selected animation group by selecting
			// the default group with no animation.
			FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());

			// Unmount from the actor.
			pActor->UnmountAnimSet(animSetName);

			// Remove from the manager.
			_externalAnimSetEntries.Remove(mountedSetIndex);

			// Broadcast an internal data changed message indicating that the animation
			// groups have changed.
			FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);

			// If the previously selected animation group is not the group that
			// was just unmounted, we can safely reselect it and the previously 
			// selected animation.  Otherwise we leave the default group selected.
			if( selectedAnimGroup != animSetName.GetAsString() )
			{
				FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
				FxSessionProxy::SetSelectedAnim(selectedAnim);
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxAnimSetManager::ImportAnimSet( const FxString& animSetPath )
{
	FxActor* pActor = NULL;
	void* pVoidSession = NULL;
	FxSessionProxy::GetActor(&pActor);
	FxSessionProxy::GetSession(&pVoidSession);
	if( pActor && pVoidSession && FxFileExists(animSetPath) )
	{
		FxStudioSession* pSession = reinterpret_cast<FxStudioSession*>(pVoidSession);
		FxString selectedAnimGroup;
		FxString selectedAnim;
		FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
		FxSessionProxy::GetSelectedAnim(selectedAnim);

		// This is going to cause a reallocation of some internal arrays, so
		// deselect the currently selected animation group by selecting
		// the default group with no animation.
		FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());

		// Load the animation set and the animation set mini session.
		FxAnimSetManagerEntry tempEntry;
		tempEntry.Load(animSetPath);

		// FxActor::ImportAnimSet() ensures that no duplicates are loaded and 
		// thus it also ensures that there will be no duplicates loaded in
		// the FxAnimSetManager.  This prevents importing an animation set that 
		// is already mounted.
		if( pActor->ImportAnimSet(tempEntry.GetAnimSet()) )
		{
			// Add clones of all of the user data objects in the mini session to 
			// the main session.
			const FxAnimSetMiniSession& miniSession = tempEntry.GetAnimSetSession();
			for( FxSize i = 0; i < miniSession.GetNumUserDataObjects(); ++i )
			{
				pSession->_addAnimUserData(miniSession.GetUserDataObjectClone(i));	
			}

			// Broadcast an internal data changed message indicating that the animation
			// groups have changed.
			FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);
			// Select the newly imported animation group.
			FxSessionProxy::SetSelectedAnimGroup(tempEntry.GetContainedAnimGroupName().GetAsString());
			// Mark the actor and session as dirty.
			FxSessionProxy::SetIsForcedDirty(FxTrue);
			return FxTrue;
		}
		else
		{
			// Reselect the previously selected animation.
			FxVM::DisplayError("Cannot import an animation set that is already mounted.  Unmount it first.");
			FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
			FxSessionProxy::SetSelectedAnim(selectedAnim);
		}
	}
	return FxFalse;
}

FxBool FxAnimSetManager::ExportAnimSet( const FxName& animSetName, const FxString& animSetPath )
{
#ifdef NO_SAVE_VERSION
	animSetName;
	animSetPath; // get rid of the warnings.
	FxVM::DisplayError("Could not export anim set. FaceFX Studio is a no-save build.");
#else
	FxActor* pActor = NULL;
	void* pVoidSession = NULL;
	FxSessionProxy::GetActor(&pActor);
	FxSessionProxy::GetSession(&pVoidSession);
	if( pActor && pVoidSession )
	{
		FxStudioSession* pSession = reinterpret_cast<FxStudioSession*>(pVoidSession);
        // This only works if the animation group is NOT present in the animation set 
		// manager and it is NOT the default animation group.
		FxString selectedAnimGroup;
		FxString selectedAnim;
		FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
		FxSessionProxy::GetSelectedAnim(selectedAnim);
		if( FxInvalidIndex == FxAnimSetManager::FindMountedAnimSet(animSetName) &&
			selectedAnimGroup != FxAnimGroup::Default.GetAsString() )
		{
			// This is going to cause a reallocation of some internal arrays, so
			// deselect the currently selected animation group by selecting
			// the default group with no animation.
			FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());
			
			// Export the animation set.
			FxAnimSet exportedAnimSet;
			if( pActor->ExportAnimSet(animSetName, exportedAnimSet) )
			{
				// Save the animation set to an archive.
				FxSaveAnimSetToFile(exportedAnimSet, animSetPath.GetData());

				// Create the mini session.
				FxAnimSetMiniSession miniSession;
				for( FxSize i = 0; i < exportedAnimSet.GetAnimGroup().GetNumAnims(); ++i )
				{
					FxName animGroupName, animName;
					animGroupName = exportedAnimSet.GetAnimGroup().GetName();
					animName = exportedAnimSet.GetAnimGroup().GetAnim(i).GetName();
					FxAnimUserData* pSessionAnimUserData = pSession->_findAnimUserData(animGroupName, animName);
					if( pSessionAnimUserData )
					{
						FxAnimUserData* pMiniSessionAnimUserData = new FxAnimUserData(*pSessionAnimUserData);
						miniSession.AddAnimUserData(pMiniSessionAnimUserData);
					}
					pSession->_removeAnimUserData(animGroupName, animName);
				}

				// Save the mini session to an archive.
				FxString miniSessionPath = animSetPath;
				miniSessionPath[miniSessionPath.Length()-3] = 'f';
				miniSessionPath[miniSessionPath.Length()-2] = 'x';
				miniSessionPath[miniSessionPath.Length()-1] = 'm';
				FxSaveAnimSetMiniSessionToFile(miniSession, miniSessionPath.GetData());
				
				// Broadcast an internal data changed message indicating that the animation
				// groups have changed.
				FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);

				// If the previously selected animation group is not the group that
				// was just exported, we can safely reselect it and the previously 
				// selected animation.  Otherwise we leave the default group selected.
				if( selectedAnimGroup != animSetName.GetAsString() )
				{
					FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
					FxSessionProxy::SetSelectedAnim(selectedAnim);
				}

				// Mark the actor and session as dirty.
				FxSessionProxy::SetIsForcedDirty(FxTrue);
				return FxTrue;
			}
		}
	}
#endif
	return FxFalse;
}

#ifdef __UNREAL__
FxBool FxAnimSetManager::MountUE3AnimSet( UFaceFXAnimSet* pAnimSet )
{
	FxActor*      pActor       = NULL;
	UFaceFXAsset* pFaceFXAsset = NULL;

	FxSessionProxy::GetActor(&pActor);
	FxSessionProxy::GetFaceFXAsset(&pFaceFXAsset);

	if( pActor && pAnimSet && pFaceFXAsset )
	{
		FxString message = "Attempting to mount UE3 FaceFXAnimSet ";
		message += FxString(TCHAR_TO_ANSI(*pAnimSet->GetName()));
		message += "...";
		FxVM::DisplayInfo(message);
		FxString selectedAnimGroup;
		FxString selectedAnim;
		FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
		FxSessionProxy::GetSelectedAnim(selectedAnim);

		// This is going to cause a reallocation of some internal arrays, so
		// deselect the currently selected animation group by selecting
		// the default group with no animation.
		FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());

		// Load the animation set and the animation set mini session.
		FxAnimSetManagerEntry newEntry;
		newEntry.SetUE3AnimSet(pAnimSet);
		newEntry.Load();

		// FxActor::MountAnimSet() ensures that no duplicates are loaded and 
		// thus it also ensures that there will be no duplicates loaded in
		// the FxAnimSetManager.  This can no longer fail; even if the 
		// pActor->MountAnimSet() fails (which could mean the set is already
		// mounted via Matinee) it still gets an entry in _externalAnimSetEntries
		// so that saving works properly.  This behavior is unique to Unreal.
		FxBool bMountAttemptSucceeded = FxFalse;
		bMountAttemptSucceeded = pActor->MountAnimSet(newEntry.GetAnimSet());
		if( !bMountAttemptSucceeded )
		{
			newEntry.SetShouldRemount(FxTrue);
			FxVM::DisplayWarning("Mount attempt failed most likely because Matinee has already mounted it.");
		}
		_externalAnimSetEntries.PushBack(newEntry);
		// Broadcast an internal data changed message indicating that the animation
		// groups have changed.
		FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);
		// Select the newly mounted animation group.
		FxSessionProxy::SetSelectedAnimGroup(newEntry.GetContainedAnimGroupName().GetAsString());
		FxVM::DisplayInfo("Success.");
		// Add AnimSet to the list of MountedAnimSets only if it isn't already there, so UE3 knows which sets have been mounted.
		if( bMountAttemptSucceeded )
		{
			pFaceFXAsset->MountedFaceFXAnimSets.AddUniqueItem(pAnimSet);
		}
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxAnimSetManager::UnmountUE3AnimSet( UFaceFXAnimSet* pAnimSet )
{
	FxActor*      pActor       = NULL;
	UFaceFXAsset* pFaceFXAsset = NULL;

	FxSessionProxy::GetActor(&pActor);
	FxSessionProxy::GetFaceFXAsset(&pFaceFXAsset);

	if( pActor && pAnimSet && pFaceFXAsset )
	{
		FxAnimSet* pInternalAnimSet = pAnimSet->GetFxAnimSet();
		if( pInternalAnimSet )
		{
			const FxName& animSetName = pInternalAnimSet->GetAnimGroup().GetName();
			FxSize mountedSetIndex = FindMountedAnimSet(animSetName);
			if( FxInvalidIndex != mountedSetIndex )
			{
				// Check to see if the animation set has been modified since the
				// last save.
				const FxAnimSet& animSet = _externalAnimSetEntries[mountedSetIndex].GetAnimSet();
				const FxAnimGroup& animGroup = animSet.GetAnimGroup();
				FxSize index = pActor->FindAnimGroup(animGroup.GetName());
				if( FxInvalidIndex != index )
				{
					if( pActor->GetAnimGroup(index) != animGroup )
					{
						// It has been modified so warn the user.
						if( wxYES == FxMessageBox(_("That animation set has been modified since the last save operation. Do you want to save?"), _("Save?"), wxYES_NO|wxICON_QUESTION) )
						{
							Serialize();
						}
						else
						{
							_externalAnimSetEntries[mountedSetIndex].Load();
						}
					}
				}

				FxString selectedAnimGroup;
				FxString selectedAnim;
				FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
				FxSessionProxy::GetSelectedAnim(selectedAnim);

				// This is going to cause a reallocation of some internal arrays, so
				// deselect the currently selected animation group by selecting
				// the default group with no animation.
				FxSessionProxy::SetSelectedAnimGroup(FxAnimGroup::Default.GetAsString());

				// Unmount from the actor.
				FxBool bShouldRemount = _externalAnimSetEntries[mountedSetIndex].ShouldRemount();
				pActor->UnmountAnimSet(animSetName);

				// Remove from the manager.
				_externalAnimSetEntries.Remove(mountedSetIndex);

				// Broadcast an internal data changed message indicating that the animation
				// groups have changed.
				FxSessionProxy::ActorDataChanged(ADCF_AnimationGroupsChanged);

				// If the previously selected animation group is not the group that
				// was just unmounted, we can safely reselect it and the previously 
				// selected animation.  Otherwise we leave the default group selected.
				if( selectedAnimGroup != animSetName.GetAsString() )
				{
					FxSessionProxy::SetSelectedAnimGroup(selectedAnimGroup);
					FxSessionProxy::SetSelectedAnim(selectedAnim);
				}

				// Remove AnimSet from mounted list.
				pFaceFXAsset->MountedFaceFXAnimSets.RemoveItem(pAnimSet);
				
				if( bShouldRemount )
				{
					FxAnimSet* pNewInternalAnimSet = pAnimSet->GetFxAnimSet();
					if( pNewInternalAnimSet )
					{
						pActor->MountAnimSet(*pNewInternalAnimSet);
					}
					pFaceFXAsset->MountedFaceFXAnimSets.AddUniqueItem(pAnimSet);
				}

                return FxTrue;
			}
		}
	}
	return FxFalse;
}

void FxAnimSetManager::UnmountAllUE3AnimSets( void )
{
	FxArray<UFaceFXAnimSet*> AnimSets;
	FxSize NumEntries = _externalAnimSetEntries.Length();
	AnimSets.Reserve(NumEntries);
	for( FxSize i = 0; i < NumEntries; ++i )
	{
		AnimSets.PushBack(_externalAnimSetEntries[i].GetUE3AnimSet());
	}
	for( FxSize i = 0; i < NumEntries; ++i )
	{
		UnmountUE3AnimSet(AnimSets[i]);
	}
	// Do some crazy Unreal Matinee trickery to force everything to be
	// remounted and redrawn correctly.
	if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
	{
		FEdModeInterpEdit* InterpEditMode = (FEdModeInterpEdit*)GEditorModeTools().GetCurrentMode();
		USeqAct_Interp* Interp = InterpEditMode->InterpEd->Interp;
		for( INT i = 0; i < Interp->GroupInst.Num(); ++i )
		{
			UInterpGroupInst* GrInst = Interp->GroupInst(i);
			UInterpGroup* Group = GrInst->Group;
			check(GrInst->TrackInst.Num() == Group->InterpTracks.Num());
			for( INT j = 0; j < Group->InterpTracks.Num(); ++j )
			{
				UInterpTrackFaceFX* FaceFXTrack = Cast<UInterpTrackFaceFX>(Group->InterpTracks(j));
				if( FaceFXTrack )
				{
					UInterpTrackInstFaceFX* FaceFXTrInst = CastChecked<UInterpTrackInstFaceFX>(GrInst->TrackInst(j));
					AActor* Actor = FaceFXTrInst->GetGroupActor();
					if( Actor )
					{
						UFaceFXAsset* Asset = Actor->PreviewGetActorFaceFXAsset();
						if( Asset )
						{
							for( INT k = 0; k < FaceFXTrack->FaceFXAnimSets.Num(); ++k )
							{
								UFaceFXAnimSet* Set = FaceFXTrack->FaceFXAnimSets(k);
								if( Set )
								{
									Asset->MountFaceFXAnimSet(Set);
								}
							}
						}
					}
				}
			}
		}
		InterpEditMode->InterpEd->RefreshInterpPosition();
	}
}
#endif // __UNREAL__

FxSize FxAnimSetManager::FindMountedAnimSet( const FxName& animGroupName )
{
	for( FxSize i = 0; i < _externalAnimSetEntries.Length(); ++i )
	{
		if( _externalAnimSetEntries[i].GetContainedAnimGroupName() == animGroupName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

FxAnimUserData* FxAnimSetManager::FindAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	FxSize animSetIndex = FindMountedAnimSet(animGroupName);
	FxAnimUserData* pAnimUserData = NULL;
	if( FxInvalidIndex != animSetIndex )
	{
		pAnimUserData = _externalAnimSetEntries[animSetIndex].FindAnimUserData(animGroupName, animName);
	}
	return pAnimUserData;
}

void FxAnimSetManager::AddAnimUserData( FxAnimUserData* pAnimUserData )
{
	FxAssert(pAnimUserData);
	if( pAnimUserData )
	{
		FxString animGroupName;
		FxString animName;
		pAnimUserData->GetNames(animGroupName, animName);
		FxSize animSetIndex = FindMountedAnimSet(animGroupName.GetData());
		if( FxInvalidIndex != animSetIndex )
		{
			_externalAnimSetEntries[animSetIndex].AddAnimUserData(pAnimUserData);
		}
	}
}

void FxAnimSetManager::RemoveAnimUserData( const FxName& animGroupName, const FxName& animName )
{
	FxSize animSetIndex = FindMountedAnimSet(animGroupName);
	if( FxInvalidIndex != animSetIndex )
	{
		_externalAnimSetEntries[animSetIndex].RemoveAnimUserData(animGroupName, animName);
	}
}

void FxAnimSetManager::FindAndRemoveOrphanedAnimUserData( void )
{
	FxSize numExternalAnimSetEntries = _externalAnimSetEntries.Length();
	for( FxSize i = 0; i < numExternalAnimSetEntries; ++i )
	{
		_externalAnimSetEntries[i].FindAndRemoveOrphanedAnimUserData();
	}
}

void FxAnimSetManager::Serialize( void )
{
	// Note that this is used for saving only!
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		FxSize numExternalAnimSetEntries = _externalAnimSetEntries.Length();
		for( FxSize i = 0; i < numExternalAnimSetEntries; ++i )
		{
			// Pull any updates from the actor into the 'true' external animation
			// set.
			FxSize index = pActor->FindAnimGroup(_externalAnimSetEntries[i].GetContainedAnimGroupName());
			if( FxInvalidIndex != index )
			{
				FxAnimSet updatedAnimSet = _externalAnimSetEntries[i].GetAnimSet();
				updatedAnimSet.SetAnimGroup(pActor->GetAnimGroup(index));
				_externalAnimSetEntries[i].SetAnimSet(updatedAnimSet);
			}
#ifdef __UNREAL__
			_externalAnimSetEntries[i].Save();
#else
			_externalAnimSetEntries[i].Save(_externalAnimSetEntries[i].GetAnimSetArchivePath());
#endif // __UNREAL__
		}
	}
}

#endif // IS_FACEFX_STUDIO

} // namespace Face

} // namespace OC3Ent
