//------------------------------------------------------------------------------
// A FaceFx Studio session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxStudioSession.h"
#include "FxArchiveStoreNull.h"
#include "FxArchiveStoreFile.h"
#include "FxArchiveStoreMemory.h"
#include "FxArchiveStoreMemoryNoCopy.h"
#include "FxArchiveStoreFileFast.h"
#include "FxCurveWidget.h"
#include "FxProgressDialog.h"
#include "FxAudio.h"
#include "FxFaceGraphNodeGroupManager.h"
#include "FxCameraManager.h"
#include "FxSessionProxy.h"
#include "FxUndoRedoManager.h"
#include "FxColourMapping.h"
#include "FxVM.h"
#include "FxStudioOptions.h"
#include "FxTimeManager.h"
#include "FxStudioAnimPlayer.h"
#include "FxAnimSetManager.h"
#include "FxAudioFile.h"
#include "FxSecurityWrapper.h"

#include "FxCGConfigManager.h"

#ifdef WIN32
	#include "FxFileSystemUtilitiesWin32.h"
#endif

#include "FxConsole.h"
#include "FxStudioApp.h"
#include "FxStudioMainWin.h"

#ifdef __UNREAL__
	#include "UnrealEd.h"
#else
	#include "wx/textfile.h"
	#include "wx/filename.h"
#endif

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxStudioSessionVersion 1

FX_IMPLEMENT_CLASS(FxStudioSession, kCurrentFxStudioSessionVersion, FxObject);

FxStudioSession::FxStudioSession()
	: _debugWidgetMessages(FxFalse)
	, _promptToSaveActor(FxTrue)
	, _noDeleteActor(FxFalse)
	, _noDeleteActorInstance(FxFalse)
	, _pActor(NULL)
	, _pActorInstance(NULL)
	, _pAnim(NULL)
#ifdef __UNREAL__
	, _pFaceFXAsset(NULL)
	, _pFaceFXAnimSet(NULL)
#endif
	, _currentTime(0.0f)
	, _pPhonemeMap(NULL)
	, g_loopallfromhere(NULL)
	, _viewLeft(0)
	, _viewTop(0)
	, _viewRight(640)
	, _viewBottom(480)
	, _zoomLevel(1.0f)
	, _isForcedDirty(FxFalse)
{
	_pAnimController = new FxAnimController();
}

FxStudioSession::~FxStudioSession()
{
}

void FxStudioSession::Exit( void )
{
	if( FxStudioApp::GetMainWindow() )
	{
		FxStudioApp::GetMainWindow()->Close();
	}
}

void FxStudioSession::RefreshUndoRedoMenu( void )
{
	if( FxStudioApp::GetMainWindow() )
	{
        static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
		static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
	}
}

FxBool FxStudioSession::PromptToSaveChangedActor( void )
{
#ifdef __UNREAL__
	if( _pActor && _pFaceFXAsset )
#else
	if( _pActor )
#endif
	{
		if( _promptToSaveActor )
		{
			// Check to see if the session or the actor need be saved.  This turns
			// out to be a trivial check to see if there are any entries in the
			// undo buffer.  This could produce false positives, meaning that in rare
			// cases, it may prompt the user to save even if the data that is 
			// serializable has not changed.  However, this prevents any overhead of
			// "dirty" flags at the SDK level, keeping them out of the in-game code.
			if( FxUndoRedoManager::CommandsToUndo() || _isForcedDirty )
			{
				wxString prompt(_("Do you want to save the changes to "));
				prompt += wxString(_pActor->GetNameAsCstr(), wxConvLibc);
				prompt += _("?");
				FxInt32 answer = FxMessageBox(prompt, _("FaceFX Studio"), wxYES_NO|wxCANCEL);
				if( wxYES == answer )
				{
					// Use the main window's Ctrl+S handler so that if there is a 
					// filename the file is saved there, or if not, the Save As dialog
					// will come up.  ProcessEvent will force the execution of the
					// event handler before returning.
					if( FxStudioApp::GetMainWindow() )
					{
						wxCommandEvent saveCommand(wxEVT_COMMAND_MENU_SELECTED, 
							MenuID_MainWindowFileSaveActor);
						FxStudioApp::GetMainWindow()->ProcessEvent(saveCommand);
					}
				}
				else if( wxNO == answer )
				{
					// Since this code is called from two places so that it works from
					// the GUI and from commands entered on the command line, we need
					// to flush the undo / redo buffer so the user is not prompted to 
					// save twice if the originator was the main menu.
					FxUndoRedoManager::Flush();
					_isForcedDirty = FxFalse;
#ifdef __UNREAL__
					// In Unreal the actor needs to be re-serialized from the
					// raw actor bytes to keep the in-memory actor in the same
					// state as the session, only if the raw actor bytes are
					// valid (it could be a newly created actor).
					if( _pFaceFXAsset->RawFaceFXActorBytes.Num() > 0 )
					{
						FxArchiveStoreMemoryNoCopy* actorMemory = FxArchiveStoreMemoryNoCopy::Create(
							static_cast<FxByte*>(_pFaceFXAsset->RawFaceFXActorBytes.GetData()), 
							                     _pFaceFXAsset->RawFaceFXActorBytes.Num());
						FxArchive actorArchive(actorMemory, FxArchive::AM_LinearLoad);
						actorArchive.Open();
						_deselectAll();
						FxArchiveProgressDisplay::Begin(_("Loading actor..."), FxStudioApp::GetMainWindow());
						actorArchive.RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
						actorArchive << *_pActor;
						FxArchiveProgressDisplay::End();
						// Tell Unreal that it needs to perform its own relinking steps.
						_pActor->SetShouldClientRelink(FxTrue);
						// Prune just in case.
						_pActor->GetMasterBoneList().Prune(_pActor->GetDecompiledFaceGraph());
						_pFaceFXAsset->FixupReferencedSoundCues();
					}
					// Ditto for animation sets.
					if( _pFaceFXAnimSet )
					{
						if( _pFaceFXAnimSet->RawFaceFXAnimSetBytes.Num() > 0 )
						{
							FxArchiveStoreMemoryNoCopy* animSetMemory = FxArchiveStoreMemoryNoCopy::Create(
								static_cast<FxByte*>(_pFaceFXAnimSet->RawFaceFXAnimSetBytes.GetData()),
													 _pFaceFXAnimSet->RawFaceFXAnimSetBytes.Num());
							FxArchive animSetArchive(animSetMemory, FxArchive::AM_LinearLoad);
							animSetArchive.Open();
							_deselectAll();
							FxAnimSet* pAnimSet = _pFaceFXAnimSet->GetFxAnimSet();
							animSetArchive << *pAnimSet;
							_pFaceFXAnimSet->FixupReferencedSoundCues();
						}
					}
#endif
				}
				else if( wxCANCEL == answer )
				{
					// Return FxFalse if the user pressed cancel to signal that no
					// action should be taken.
					return FxFalse;
				}
			}
		}
		else
		{
			if( FxUndoRedoManager::CommandsToUndo() || _isForcedDirty )
			{
				FxString errorString = "Changes to ";
				errorString = FxString::Concat(errorString, _pActor->GetNameAsString());
				errorString = FxString::Concat(errorString, " discarded.  Did you forget to save the actor?");
				FxVM::DisplayError(errorString.GetData());
			}
		}
	}
	else
	{
		// Flush the undo redo system just in case.
		FxUndoRedoManager::Flush();
		_isForcedDirty = FxFalse;
	}
	return FxTrue;
}

void FxStudioSession::SetPromptToSaveActor( FxBool promptToSaveActor )
{
	_promptToSaveActor = promptToSaveActor;
}

void FxStudioSession::Clear( void )
{
	if( _pAnimController )
	{
		_pAnimController->DestroyUserData();
		_pAnimController->SetAnimPointer(NULL);
		_pAnimController->SetFaceGraphPointer(NULL);
	}
	_pAnim = NULL;
	FxSize numAnimUserDatas = _animUserDataList.Length();
	for( FxSize i = 0; i < numAnimUserDatas; ++i )
	{
		if( _animUserDataList[i] )
		{
			delete _animUserDataList[i];
			_animUserDataList[i] = NULL;
		}
	}
	_animUserDataList.Clear();
	if( _pPhonemeMap )
	{
		delete _pPhonemeMap;
		_pPhonemeMap = NULL;
	}
	if( _pActor )
	{
		_pActor->GetDecompiledFaceGraph().Clear();
		FxSize numAnimGroups = _pActor->GetNumAnimGroups();
		for( FxSize i = 0; i < numAnimGroups; ++i )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(i);
			FxSize numAnims = animGroup.GetNumAnims();
			for( FxSize j = 0; j < numAnims; ++j )
			{
				FxAnim* pAnim = animGroup.GetAnimPtr(j);
				if( pAnim )
				{
					pAnim->SetUserData(NULL);
				}
			}
		}
	}
	_dummyRegisters.Clear();
}

FxBool FxStudioSession::CreateActor( void )
{
	// If the actor or session need to be saved, prompt the user first.
	if( !PromptToSaveChangedActor() )
	{
		return FxTrue;
	}

	// Deselect everything.
	_deselectAll();
	Clear();
	FxFaceGraphNodeGroupManager::FlushAllGroups();
	FxCameraManager::FlushAllCameras();
	FxAnimSetManager::FlushAllMountedAnimSets();
	FxWorkspaceManager::Instance()->Clear();
	FxColourMap::Clear();

	if( _pActorInstance )
	{
		_pActorInstance->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActorInstance )
		{
			delete _pActorInstance;
		}
		_pActorInstance = NULL;
	}

	if( _pActor )
	{
		_pActor->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActor )
		{
			delete _pActor;
		}
		_pActor = NULL;
	}
	_pActor = new FxActor();
	_pActorInstance = new FxActorInstance(_pActor);
	_pActor->SetIsOpenInStudio(FxTrue);
	_pActorInstance->SetIsOpenInStudio(FxTrue);
	// Force the session to be dirty.
	SetIsForcedDirty(FxTrue);
	_pPhonemeMap = new FxPhonemeMap();
	_actorFilePath = "";
	OnActorChanged(NULL, _pActor);
	OnPhonemeMappingChanged(NULL, _pPhonemeMap);
	// Propagate.
	FxWidgetMediator::OnCreateActor(NULL);

	if( FxStudioApp::GetMainWindow() )
	{
        static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
		static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
	}
	return FxTrue;
}

FxBool FxStudioSession::LoadActor( const FxString& filename )
{
	// If the actor or session need to be saved, prompt the user first.
	if( !PromptToSaveChangedActor() )
	{
		return FxTrue;
	}

	// Deselect everything.
	_deselectAll();

	Clear();
	FxFaceGraphNodeGroupManager::FlushAllGroups();
	FxCameraManager::FlushAllCameras();
	FxAnimSetManager::FlushAllMountedAnimSets();
	FxWorkspaceManager::Instance()->Clear();
	FxColourMap::Clear();

	if( _pActorInstance )
	{
		_pActorInstance->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActorInstance )
		{
			delete _pActorInstance;
		}
		_pActorInstance = NULL;
	}

	if( _pActor )
	{
		_pActor->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActor )
		{
			delete _pActor;
		}
		_pActor = NULL;
	}
	// The code below requires at least a three letter file extension.
	if( filename.Length() < 3 )
	{
		return FxFalse;
	}
	// Make sure we're using trying to load an "fxa" file.
	wxString filenameStr(filename.GetData(), wxConvLibc);
	if( filenameStr.AfterLast('.').MakeUpper() != wxT("FXA") )
	{
		return FxFalse;
	}
	// Ensure the file exists.
	if( !FxFileExists(filename) )
	{
		return FxFalse;
	}

	FxBool canContinue = FxTrue;
	FxIsLicensed(canContinue);

	// Flush any managed archives from the system before loading a new archive.
	FxArchive::UnmountAllArchives();

	// Automatically execute a pre load script for the actor if it exists.
	FxString nonConstFilename = filename;
	FxTriggerScriptEvent(STET_PreLoadActor, &nonConstFilename);

	// FaceFx Studio needs everything in the actor explicitly loaded for
	// editing, so do a linear load.
	FxArchive actorArchive(FxArchiveStoreFileFast::Create(filename.GetData()), FxArchive::AM_LinearLoad);
	actorArchive.Open();
	if( actorArchive.IsValid() && canContinue )
	{
		FxArchiveProgressDisplay::Begin(_("Loading actor..."), FxStudioApp::GetMainWindow());
		_pActor = new FxActor();
		actorArchive.RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
		actorArchive << *_pActor;
		_pActor->DecompileFaceGraph();
		_pActor->SetIsOpenInStudio(FxTrue);
		_pActorInstance = new FxActorInstance(_pActor);
		_pActorInstance->SetIsOpenInStudio(FxTrue);
		FxSessionProxy::SetNumDummyRegisters(_pActor->GetCompiledFaceGraph().nodes.Length());
		_actorFilePath = filename;
		FxArchiveProgressDisplay::End();
		// Display actor version information.
		FxMsg("Actor created with FaceFX SDK version %s", actorArchive.GetSDKVersionString());
		FxMsg("Actor belongs to %s for %s", actorArchive.GetLicenseeName(), actorArchive.GetLicenseeProjectName());
		if( FxName::NullName == _pActor->GetName() ) 
		{
			_pActor->SetName("NewActor");
			SetIsForcedDirty(FxTrue);
			FxWarn("Actor had an invalid name.  FaceFX Studio has automatically reset the name to NewActor.");
		}
		FxMsg("Loaded actor named %s", _pActor->GetNameAsCstr());
		// Prune just in case.
		_pActor->GetMasterBoneList().Prune(_pActor->GetDecompiledFaceGraph());
		// Send an actor changed message.
		OnActorChanged(NULL, _pActor);
		// Load the session.
		FxString sessionFilename = filename;
		sessionFilename[sessionFilename.Length()-3] = 'f';
		sessionFilename[sessionFilename.Length()-2] = 'x';
		sessionFilename[sessionFilename.Length()-1] = 's';
		FxBool goodSession = FxFalse;
		if( FxFileExists(sessionFilename) )
		{
			FxArchive* sessionArchive = new FxArchive(FxArchiveStoreFile::Create(sessionFilename.GetData()), FxArchive::AM_LazyLoad);
			sessionArchive->Open();
			if( sessionArchive->IsValid() )
			{
				FxArchive::MountArchive(sessionArchive);
				FxArchiveProgressDisplay::Begin(_("Loading session..."), FxStudioApp::GetMainWindow());
				sessionArchive->RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
				Serialize(*sessionArchive);
				FxArchiveProgressDisplay::End();
				goodSession = FxTrue;
			}
			else
			{
				delete sessionArchive;
				sessionArchive = NULL;
			}
		}
		
		if( !goodSession )
		{
			// If there was no session, make sure there is a valid phoneme map.
			if( !_pPhonemeMap )
			{
				_pPhonemeMap = new FxPhonemeMap();
			}

			// If there was no session, make sure the face graph is properly
			// laid out.
			FxVM::ExecuteCommand("graph -layout");

			// Issue a warning if the loaded actor contains information that it
			// could possibly need the session file for.  If the actor has
			// valid animations, the audio from those animations would be in the
			// session, along with the proper phoneme mapping, and other session
			// stored data.
			if( _pActor )
			{
				FxBool validAnims = FxFalse;
				for( FxSize i = 0; i < _pActor->GetNumAnimGroups(); ++i )
				{
					FxAnimGroup& animGroup = _pActor->GetAnimGroup(i);
					if( animGroup.GetNumAnims() > 0 )
					{
						validAnims = FxTrue;
						break;
					}
				}
				if( validAnims )
				{
					FxMessageBox(_("You have loaded an actor that might need data from a session file (.fxs) that could not be loaded."), 
						         _("Could not load session"));
				}
			}
		}

		if( 0 == _pPhonemeMap->GetNumTargets() )
		{
			_pPhonemeMap->MakeDefaultMapping();
			FxWarn("The actor did not have a valid phoneme mapping. The default mapping was created for you.");
		}

		FxArray<FxName> previousAnimCurveSelection = _selAnimCurveNames;
		FxArray<FxName> previousFaceGraphNodeSelection = _selFaceGraphNodeNames;
		FxReal previousCurrentTime = _currentTime;
		
		OnPhonemeMappingChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _pPhonemeMap);
		// Load the anim.
		FxSize animGroupIndex = _pActor->FindAnimGroup(_selAnimGroupName);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(_selAnimName);
			if( animIndex != FxInvalidIndex )
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selAnimGroupName, animGroup.GetAnimPtr(animIndex));
			}
			else
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selAnimGroupName, NULL);
			}
		}
		else
		{
			OnAnimChanged(reinterpret_cast<FxWidget*>(0x10adac7a), FxAnimGroup::Default, NULL);
		}
		OnAnimCurveSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousAnimCurveSelection);
		if( _selNodeGroup == FxName::NullName )
		{
			OnFaceGraphNodeGroupSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData());
		}
		else
		{
			OnFaceGraphNodeGroupSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selNodeGroup);
		}
		OnFaceGraphNodeSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousFaceGraphNodeSelection, FxFalse);
		OnLinkSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _inputNode, _outputNode);
		OnTimeChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousCurrentTime);
		OnActorInternalDataChanged(reinterpret_cast<FxWidget*>(0x10adac7a), ADCF_NodeGroupsChanged);
		FxTimeManager::RequestTimeChange(_minTime, _maxTime);
	    if( FxStudioApp::GetMainWindow() )
	    {
			static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
	    }
	}
	else
	{
		if( FxStudioApp::GetMainWindow() )
	    {
			static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
        }
		return FxFalse;
	}
	if( FxStudioApp::GetMainWindow() )
	{
		static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
	}
	// Propagate.
	FxWidgetMediator::OnLoadActor(NULL, filename);

	// Automatically execute a post load script for the actor if it exists.
	FxTriggerScriptEvent(STET_PostLoadActor, &_actorFilePath);
	
	_checkContent();
	return FxTrue;
}

FxBool FxStudioSession::CloseActor( void )
{
	// If the actor or session need to be saved, prompt the user first.
	if( !PromptToSaveChangedActor() )
	{
		return FxTrue;
	}

	// Deselect everything.
	_deselectAll();
	Clear();
	FxFaceGraphNodeGroupManager::FlushAllGroups();
	FxCameraManager::FlushAllCameras();
	FxAnimSetManager::FlushAllMountedAnimSets();
	FxWorkspaceManager::Instance()->Clear();
	FxColourMap::Clear();

	if( _pActorInstance )
	{
		_pActorInstance->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActorInstance )
		{
			delete _pActorInstance;
		}
		_pActorInstance = NULL;
	}

	if( _pActor )
	{
		_pActor->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActor )
		{
			delete _pActor;
		}
		_pActor = NULL;
	}
	_actorFilePath = "";
	// Propagate.
	FxWidgetMediator::OnCloseActor(NULL);

	if( FxStudioApp::GetMainWindow() )
	{
	    static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
		static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
	}
	return FxTrue;
}

FxBool FxStudioSession::SaveActor( const FxString& filename )
{
#ifdef NO_SAVE_VERSION
	FxError("Could not save %s: FaceFX Studio is a no-save build.", filename.GetData());
	// Reset the prompt to save actor flag to prevent an 'infinite loop' of 
	// Studio: "Do you want to save?" 
	// User:   "Yes" 
	// Studio: "This is a no save version, you can't.  Do you want to save?"
	if( _promptToSaveActor )
	{
		_promptToSaveActor = FxFalse;
	}
	return FxFalse;
#else

#ifdef __UNREAL__
	if( _pActor && _pFaceFXAsset )
#else
	FxBool canContinue = FxTrue;
	FxIsLicensed(canContinue);
	if( _pActor && canContinue )
#endif
	{
		// Check if there are open changes in the workspace edit widget.
		FxBool shouldSave = FxFalse;
		OnInteractEditWidget(this, FxTrue, shouldSave);
		if( shouldSave )
		{
			// Force the workspace edit widget to commit any open changes.
			OnInteractEditWidget(this, FxFalse, shouldSave);
		}

#ifndef __UNREAL__
		// The code below requires at least a three letter file extension.
		if( filename.Length() < 3 )
		{
			return FxFalse;
		}

		// Check all files involved for read-only status.  If any file in the
		// system needing to be saved is marked as read-only the entire save
		// operation will fail immediately.
		FxBool areAnyFilesReadOnly = FxFalse;
		if( FxFileIsReadOnly(filename) )
		{
			FxError("%s is read-only!", filename.GetData());
			areAnyFilesReadOnly = FxTrue;
		}

		FxString sessionFilename = filename;
		sessionFilename[sessionFilename.Length()-3] = 'f';
		sessionFilename[sessionFilename.Length()-2] = 'x';
		sessionFilename[sessionFilename.Length()-1] = 's';

		if( FxFileIsReadOnly(sessionFilename) )
		{
			FxError("%s is read-only!", sessionFilename.GetData());
			areAnyFilesReadOnly = FxTrue;
		}

		if( FxAnimSetManager::AreAnyFilesReadOnly() )
		{
			areAnyFilesReadOnly = FxTrue;
		}

		if( areAnyFilesReadOnly )
		{
			FxMessageBox(_("Could not save!  Some files are marked as read-only.  Check the console log for details, fix the problem file(s), and try again."), _("Error"));
			return FxFalse;
		}

		// Automatically execute a pre save script for the actor if it exists.
		FxString nonConstFilename = filename;
		FxTriggerScriptEvent(STET_PreSaveActor, &nonConstFilename);

		// Cache the selected time window before the anim set manager resets
		// the selection.
		FxTimeManager::GetTimes(_minTime, _maxTime);

		// Save the actor.
		FxArchiveProgressDisplay::Begin(_("Saving Actor..."), FxStudioApp::GetMainWindow());
		FxSaveActorToFile(*_pActor, filename.GetData(), FxArchive::ABO_LittleEndian, FxArchiveProgressDisplay::UpdateProgress);
		_actorFilePath = filename;
		// Force all lazy-loaded objects in the session to be loaded as we could
		// be overwriting the same archive the lazy loaders would be loading from.
		FxArchive::ForceLoadAllMountedArchives();
		// Unmount the session archive.
		FxArchive::UnmountAllArchives();
		// Make sure that the above two lines that actually cause a lot of work
		// to happen behind the scenes are inside of a progress display of some
		// sort.
		FxArchiveProgressDisplay::End();
#else
		// Save the actor into the raw actor bytes so that if the actor is
		// loaded into Studio again we have a reference point for reverting
		// it if the user decides not to save the changes.
		FxArchiveProgressDisplay::Begin(_("Saving Actor..."), FxStudioApp::GetMainWindow());
		FxByte* ActorMemory = NULL;
		FxSize  NumActorMemoryBytes = 0;
		FxSaveActorToMemory(*_pActor, ActorMemory, NumActorMemoryBytes, FxArchive::ABO_LittleEndian, FxArchiveProgressDisplay::UpdateProgress);
		_pFaceFXAsset->RawFaceFXActorBytes.Empty();
		_pFaceFXAsset->RawFaceFXActorBytes.Add(NumActorMemoryBytes);
		appMemcpy(_pFaceFXAsset->RawFaceFXActorBytes.GetData(), ActorMemory, NumActorMemoryBytes);
		FxFree(ActorMemory, NumActorMemoryBytes);
		_pFaceFXAsset->ReferencedSoundCues.Empty();
		_pFaceFXAsset->FixupReferencedSoundCues();
		FxArchiveProgressDisplay::End();
#endif
		// Deselect the currently selected animation but cache it first.
		FxName previouslySelectedAnimGroupName = _selAnimGroupName;
		FxName previouslySelectedAnimName      = _selAnimName;
		FxArray<FxName> previouslySelectedAnimCurveNames = _selAnimCurveNames;
		FxArray<FxName> previouslySelectedFaceGraphNodes = _selFaceGraphNodeNames;
		FxReal previousCurrentTime = _currentTime;
		OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
		_selAnimGroupName = previouslySelectedAnimGroupName;
		_selAnimName = previouslySelectedAnimName;
		_selAnimCurveNames = previouslySelectedAnimCurveNames;
		_selFaceGraphNodeNames = previouslySelectedFaceGraphNodes;
		_currentTime = previousCurrentTime;
#ifdef __UNREAL__
		FxArchive directoryCreater(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater.Open();
		FxArchiveStoreMemory* sessionMemory = FxArchiveStoreMemory::Create(NULL, 0);
		FxArchive sessionArchive(sessionMemory, FxArchive::AM_Save);
		sessionArchive.Open();
		FxArchiveProgressDisplay::Begin(_("Saving session..."), FxStudioApp::GetMainWindow());
		Serialize(directoryCreater);
		sessionArchive.SetInternalDataState(directoryCreater.GetInternalData());
		sessionArchive.RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
		Serialize(sessionArchive);
		_pFaceFXAsset->RawFaceFXSessionBytes.Empty();
		_pFaceFXAsset->RawFaceFXSessionBytes.Add(sessionMemory->GetSize());
		appMemcpy(_pFaceFXAsset->RawFaceFXSessionBytes.GetData(), sessionMemory->GetMemory(), _pFaceFXAsset->RawFaceFXSessionBytes.Num());
		FxArchiveProgressDisplay::End();
		_pFaceFXAsset->MarkPackageDirty();
#else
		// Save the session.  The saving archive is created on the heap so that
		// it's store will be sure to be closed by the time the file needs to be
		// loaded.
		FxArchive directoryCreater2(FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory);
		directoryCreater2.Open();
		FxArchive* sessionArchive = new FxArchive(FxArchiveStoreFile::Create(sessionFilename.GetData()), FxArchive::AM_Save);
		sessionArchive->Open();
		FxArchiveProgressDisplay::Begin(_("Saving Session..."), FxStudioApp::GetMainWindow());
		if( sessionArchive->IsValid() )
		{
			Serialize(directoryCreater2);
			sessionArchive->SetInternalDataState(directoryCreater2.GetInternalData());
			sessionArchive->RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
			Serialize(*sessionArchive);
		}
		delete sessionArchive;
		sessionArchive = NULL;
		FxArchiveProgressDisplay::End();
#endif
		// Loop through all the animations in all the groups and set the user data
		// pointer to NULL.
		FxSize numAnimGroups = _pActor->GetNumAnimGroups();
		for( FxSize i = 0; i < numAnimGroups; ++i )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(i);
			FxSize numAnims = animGroup.GetNumAnims();
			for( FxSize j = 0; j < numAnims; ++j )
			{
				FxAnim* pAnim = animGroup.GetAnimPtr(j);
				if( pAnim )
				{
					pAnim->SetUserData(NULL);
				}
			}
		}
		// Deselect everything in all widgets.
		_deselectAll();
		// Clear the session's allocated memory.
		Clear();
		// Reselect the current actor to fill out the widgets.
		_pActor->DecompileFaceGraph();
		_pActor->SetIsOpenInStudio(FxTrue);
		OnActorChanged(NULL, _pActor);
		FxSessionProxy::SetNumDummyRegisters(_pActor->GetCompiledFaceGraph().nodes.Length());

#ifdef __UNREAL__
		// Load the just saved session.
		if( _pFaceFXAsset->RawFaceFXSessionBytes.Num() > 0 )
		{
			FxArchiveStoreMemoryNoCopy* sessionMemory = FxArchiveStoreMemoryNoCopy::Create(
				static_cast<FxByte*>(_pFaceFXAsset->RawFaceFXSessionBytes.GetData()), 
				                     _pFaceFXAsset->RawFaceFXSessionBytes.Num());
			FxArchive sessionArchive(sessionMemory, FxArchive::AM_LinearLoad);
			sessionArchive.Open();
			FxArchiveProgressDisplay::Begin(_("Loading session..."), FxStudioApp::GetMainWindow());
			sessionArchive.RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
			Serialize(sessionArchive);
			FxArchiveProgressDisplay::End();
		}
#else
		// Make sure the session exists.
		if( FxFileExists(sessionFilename) )
		{
			// Load and mount the just saved session from disk.
			FxArchive* newSessionArchive = new FxArchive(FxArchiveStoreFile::Create(sessionFilename.GetData()), FxArchive::AM_LazyLoad);
			newSessionArchive->Open();
			if( newSessionArchive->IsValid() )
			{
				FxArchive::MountArchive(newSessionArchive);
				FxArchiveProgressDisplay::Begin(_("Loading session..."), FxStudioApp::GetMainWindow());
				newSessionArchive->RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
				Serialize(*newSessionArchive);
				FxArchiveProgressDisplay::End();
			}
			else
			{
				delete newSessionArchive;
				newSessionArchive = NULL;
			}
		}
#endif

		// Restore the state of the session.
		OnPhonemeMappingChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), _pPhonemeMap);
		// Load the anim.
		FxSize animGroupIndex = _pActor->FindAnimGroup(previouslySelectedAnimGroupName);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(previouslySelectedAnimName);
			if( animIndex != FxInvalidIndex )
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), previouslySelectedAnimGroupName, animGroup.GetAnimPtr(animIndex));
			}
			else
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), previouslySelectedAnimGroupName, NULL);
			}
		}
		OnAnimCurveSelChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), previouslySelectedAnimCurveNames);
		OnFaceGraphNodeGroupSelChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), _selNodeGroup);
		OnFaceGraphNodeSelChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), previouslySelectedFaceGraphNodes, FxFalse);
		OnLinkSelChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), _inputNode, _outputNode);
		OnTimeChanged(reinterpret_cast<FxWidget*>(0x5af3ac7a), previousCurrentTime);
		FxTimeManager::RequestTimeChange(_minTime, _maxTime);

		// Propagate.
		FxWidgetMediator::OnSaveActor(NULL, filename);

		_isForcedDirty = FxFalse;
#ifdef __UNREAL__
		// In Unreal, the undo / redo buffers need to be flushed here because
		// we bypass the saveActor command.
		FxUndoRedoManager::Flush();
#endif
		if( FxStudioApp::GetMainWindow() )
		{
			static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
		}

		// Automatically execute a post save script for the actor if it exists.
		FxTriggerScriptEvent(STET_PostSaveActor, &_actorFilePath);

		return FxTrue;
	}
	return FxFalse;
#endif // ifdef nosave/else
}

FxBool FxStudioSession::MountAnimSet( const FxString& animSetPath )
{
	return FxAnimSetManager::MountAnimSet(animSetPath);	
}

FxBool FxStudioSession::UnmountAnimSet( const FxName& animSetName )
{
	return FxAnimSetManager::UnmountAnimSet(animSetName);
}

FxBool FxStudioSession::ImportAnimSet( const FxString& animSetPath )
{
	return FxAnimSetManager::ImportAnimSet(animSetPath);
}

FxBool FxStudioSession::
ExportAnimSet( const FxName& animSetName, const FxString& animSetPath )
{
	return FxAnimSetManager::ExportAnimSet(animSetName, animSetPath);
}

FxBool FxStudioSession::
AnalyzeDirectory( const FxString& directory, const FxString& animGroupName, const FxString& language, 
				  const FxString& coarticulationConfig, const FxString& gestureConfig,
				  FxBool generateSpeechGestures, FxBool overwrite, FxBool recurse, FxBool notext )
{
	FxStudioApp::SetIsUnattended(FxTrue);
#ifdef WIN32
	FxString statusMsg = "Building file list for directory: ";
	statusMsg = FxString::Concat(statusMsg, directory);
	statusMsg = FxString::Concat(statusMsg, "...");
	FxMsg(statusMsg.GetData());
	FxArray<FxString> fileList;
	FxArray<FxString> supportedAudioFileExts = FxAudioFile::GetSupportedExtensions();
	FxSize numExts = supportedAudioFileExts.Length();
	// Weed out duplicate extensions.
	FxArray<FxString> uniqueAudioFilesExts(numExts);;
	for( FxSize i = 0; i < numExts; ++i )
	{
		if( uniqueAudioFilesExts.Find(supportedAudioFileExts[i]) == FxInvalidIndex )
		{
			uniqueAudioFilesExts.PushBack(supportedAudioFileExts[i]);
		}
	}
	numExts = uniqueAudioFilesExts.Length();
	for( FxSize i = 0; i < numExts; ++i )
	{
		FxBuildFileList(directory, uniqueAudioFilesExts[i], fileList, recurse);
	}

	FxMsg( "Working..." );
	FxSize numFiles = fileList.Length();
	wxString originalPath(directory.GetData(), wxConvLibc);
	// Make sure the path doesn't end in a path separator.
	originalPath.Trim();
	if( originalPath.Last() == FxPlatformPathSeparator )
	{
		originalPath.RemoveLast();
	}
	originalPath = originalPath.BeforeLast(FxPlatformPathSeparator);

	FxProgressDialog progressDialog(_("Batch processing..."), FxStudioApp::GetMainWindow(), FxTrue);
	progressDialog.Show();
	for( FxSize i = 0; i < numFiles; ++i )
	{
		wxString audioFilePath(fileList[i].GetData(), wxConvLibc);
		// Get the animation group from the folder name if it is the empty 
		// string.
		FxString groupName = animGroupName;
		if( 0 == groupName.Length() )
		{
			// Build path from subdirs.
			wxString groupNameFromPath = audioFilePath;
			groupNameFromPath.Replace(originalPath, _(""));
			groupNameFromPath = groupNameFromPath.BeforeLast(FxPlatformPathSeparator);
			groupNameFromPath = groupNameFromPath.AfterFirst(FxPlatformPathSeparator);
			groupName = FxString(groupNameFromPath.mb_str(wxConvLibc));
		}

		// Get the animation name from the audio file name.
		wxString audioFileName = audioFilePath.AfterLast(FxPlatformPathSeparator);
		audioFileName = audioFileName.BeforeLast('.');
		FxString animName(audioFileName.mb_str(wxConvLibc));
		FxWString optionalText = L"__USE_TXT_FILE__";
		if( notext )
		{
			optionalText = L"";
		}
		
		wxString progressMessage = wxString::Format(_("Processing animation \"%s\" in group \"%s\" [%d/%d]"), 
			wxString::FromAscii(animName.GetData()), wxString::FromAscii(groupName.GetData()),
			i+1, numFiles);
		progressDialog.SetProgressText(progressMessage);
#ifdef __UNREAL__
		wxYield();
#else
		wxGetApp().Yield();
#endif // __UNREAL__

		if( !AnalyzeFile(fileList[i], optionalText, groupName, animName, 
			             language, coarticulationConfig, gestureConfig, 
						 generateSpeechGestures, overwrite) )
		{
			FxVM::DisplayError("Could not analyze file!");
			FxStudioApp::SetIsUnattended(FxFalse);
			return FxFalse;
		}
#ifdef __UNREAL__
		wxYield();
#else
		wxGetApp().Yield();
#endif // __UNREAL__
		progressDialog.Update((i+1) / static_cast<FxReal>(numFiles));
#ifdef __UNREAL__
		wxYield();
#else
		wxGetApp().Yield();
#endif // __UNREAL__
		if( progressDialog.HasBeenCancelled() )
		{
			break;
		}
	}
	progressDialog.Update(1.0f);
	FxStudioApp::SetIsUnattended(FxFalse);
	FxMsg( "Completed batch analysis." );
	return FxTrue;
#else
	FxStudioApp::SetIsUnattended(FxFalse);
	return FxFalse;
#endif
}

FxBool FxStudioSession::
AnalyzeFile( const FxString& audioFile, const FxWString& optionalText, 
			 const FxString& animGroupName, const FxString& animName, 
			 const FxString& language, const FxString& coarticulationConfig,
			 const FxString& gestureConfig, FxBool generateSpeechGestures, 
			 FxBool overwrite )
{
	if( _pActor )
	{
		// Make sure audioFile exists.
		if( !FxFileExists(audioFile) )
		{
			FxVM::DisplayError("The specified audio file does not exist!");
			return FxFalse;
		}
		
		// Make sure the audioFile is in a valid format.
		FxAudioFile theAudioFile(audioFile);
		if( !theAudioFile.Load() )
		{
			FxVM::DisplayError("The specified audio file is not in the correct format!  Audio files must be 16-bit and have sample rates greater than or equal to 16-kHz.");
			return FxFalse;
		}

		// Adding a new animation or group can potentially cause a reallocation, so
		// give the widgets a NULL animation.  We're going to select the
		// new animation when it is finally created anyway.
		OnAnimChanged(NULL, FxAnimGroup::Default, NULL);

		wxString audioFilePath(audioFile.GetData(), wxConvLibc);
		// Make the relative path to the audio file from the actor.
		wxFileName relAudioFilePath(audioFilePath);
		wxFileName actorPath(wxString::FromAscii(GetActorFilePath().GetData()));
		relAudioFilePath.MakeRelativeTo(actorPath.GetPath());

		// Get the animation group from the folder name if it is the empty 
		// string.
		FxString groupName = animGroupName;
		if( 0 == groupName.Length() )
		{
			wxString groupNameFromPath = audioFilePath.BeforeLast(FxPlatformPathSeparator);
			groupNameFromPath = groupNameFromPath.AfterLast(FxPlatformPathSeparator);
			groupName = FxString(groupNameFromPath.mb_str(wxConvLibc));
		}
		// Get the animation name from the audio file name if it is the empty
		// string.
		FxString animationName = animName;
		if( 0 == animationName.Length() )
		{
			wxString audioFileName = audioFilePath.AfterLast(FxPlatformPathSeparator);
			audioFileName = audioFileName.BeforeLast('.');
			animationName = FxString(audioFileName.mb_str(wxConvLibc));
		}
		FxSize animGroupIndex = _pActor->FindAnimGroup(groupName.GetData());
		// Create the animation group here if it doesn't exist because it
		// could have come from a folder name.
		if( animGroupIndex == FxInvalidIndex )
		{
			_pActor->AddAnimGroup(groupName.GetData());
		}
		animGroupIndex = _pActor->FindAnimGroup(groupName.GetData());
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(animationName.GetData());
			if( animIndex != FxInvalidIndex )
			{
				if( overwrite )
				{
					// Remove the animation and the user data.
					OnDeleteAnim(NULL, groupName.GetData(), animationName.GetData());
				}
				else
				{
					FxVM::DisplayError("Animation already exists and overwrite is false!");
					return FxFalse;
				}
			}

			FxAnim newAnim;
			newAnim.SetName(animationName.GetData());
			// Add the new animation.
			_pActor->AddAnim(groupName.GetData(), newAnim);
			FxSize newAnimIndex = animGroup.FindAnim(animationName.GetData());
			if( newAnimIndex != FxInvalidIndex )
			{
				FxAnim* pNewAnim = animGroup.GetAnimPtr(newAnimIndex);
				if( pNewAnim )
				{
					FxAnimUserData* pNewAnimUserData = new FxAnimUserData();
					pNewAnimUserData->SetNames(groupName, animationName);
					pNewAnimUserData->SetRandomSeed(static_cast<FxInt32>(time(0)));
					wxString audioPath = relAudioFilePath.GetFullPath();
					pNewAnimUserData->SetAudioPath(FxString(audioPath.mb_str(wxConvLibc)));
					
					FxAudio* pAudio = new FxAudio(theAudioFile);
					pAudio->SetName(animationName.GetData());
					FxMsg( "Loaded audio file: %s", audioFile );
					FxMsg( "Bits per sample: %d", pAudio->GetBitsPerSample() );
					FxMsg( "Sample rate: %d", pAudio->GetSampleRate() );
					FxMsg( "Channel count: %d", pAudio->GetNumChannels() );
					FxMsg( "Sample count: %d", pAudio->GetNumSamples() );
					FxMsg( "Duration: %f seconds", pAudio->GetDuration() );
					pNewAnimUserData->SetAudio(pAudio);
					delete pAudio;

                    if( optionalText == L"__USE_TXT_FILE__" )
					{
						// Find a corresponding .txt file and use it.
						wxString audioPathString(audioFile.GetData(), wxConvLibc);
						wxString textPathString = audioPathString.BeforeLast('.');
						textPathString.Append(wxT(".txt"));
						wxString analysisText = FxLoadUnicodeTextFile(FxString(textPathString.mb_str(wxConvLibc)));
									pNewAnimUserData->SetAnalysisText(FxWString(analysisText.GetData()));	
					}
					else
					{
						// If optionalText is valid, it overrides any corresponding .txt files.
						pNewAnimUserData->SetAnalysisText(optionalText);
					}

					// Set up the language and configurations.
					pNewAnimUserData->SetLanguage(language);

					if( 0 == coarticulationConfig.Length() )
					{
						pNewAnimUserData->SetCoarticulationConfig(FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString());
					}
					else
					{
						pNewAnimUserData->SetCoarticulationConfig(coarticulationConfig);
					}
					if( 0 == gestureConfig.Length() )
					{
						pNewAnimUserData->SetGestureConfig(FxCGConfigManager::GetDefaultGestureConfigName().GetAsString());
					}
					else
					{
						pNewAnimUserData->SetGestureConfig(gestureConfig);
					}

					// Set if the animation should contain speech gestures.
					pNewAnimUserData->SetShouldContainSpeechGestures(generateSpeechGestures);

					FxMsg( "Language: %s", pNewAnimUserData->GetLanguage().GetData() );
					FxMsg( "Coarticulation Config: %s", pNewAnimUserData->GetCoarticulationConfig().GetData() );
					FxMsg( "Gesture Config: %s", pNewAnimUserData->GetGestureConfig().GetData() );
					FxMsg( "Generate Speech Gestures: %s", 
						pNewAnimUserData->ShouldContainSpeechGestures() ? 
						"true" : "false" );
					
					// It is not safe to unload this user data because it is not a lazy-loaded
					// object from the original session.
					pNewAnimUserData->SetIsSafeToUnload(FxFalse);
					// Set the user data.
					pNewAnim->SetUserData(pNewAnimUserData);

					// Broadcast an OnCreateAnim message to pass the animation off for
					// analysis and store the user data in the session.
					OnCreateAnim(NULL, groupName.GetData(), pNewAnim);
					// Flush the undo redo manager because undoing this operation
					// would cause Bad Things.
					FxUndoRedoManager::Flush();
					// Force the session to be dirty.
					SetIsForcedDirty(FxTrue);
					return FxTrue;					
				}
			}
		}
	}
	return FxFalse;
}

FxBool FxStudioSession:: 
AnalyzeRawAudio( FxAudio* pAudio, const FxWString& optionalText,
			     const FxString& animGroupName, const FxString& animName, 
				 const FxString& language, const FxString& coarticulationConfig, 
				 const FxString& gestureConfig, FxBool generateSpeechGestures, 
				 FxBool overwrite )
{
	if( _pActor && pAudio )
	{
		// Adding a new animation or group can potentially cause a reallocation, so
		// give the widgets a NULL animation.  We're going to select the
		// new animation when it is finally created anyway.
		OnAnimChanged(NULL, FxAnimGroup::Default, NULL);

		FxSize animGroupIndex = _pActor->FindAnimGroup(animGroupName.GetData());
		// Create the animation group here if it doesn't exist because it
		// could have come from a package name.
		if( animGroupIndex == FxInvalidIndex )
		{
			_pActor->AddAnimGroup(animGroupName.GetData());
		}
		animGroupIndex = _pActor->FindAnimGroup(animGroupName.GetData());
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(animName.GetData());
			if( animIndex != FxInvalidIndex )
			{
				if( overwrite )
				{
					// Remove the animation and the user data.
					OnDeleteAnim(NULL, animGroupName.GetData(), animName.GetData());
				}
				else
				{
					FxVM::DisplayError("Animation already exists and overwrite is false!");
					delete pAudio;
					pAudio = NULL;
					return FxFalse;
				}
			}

			FxAnim newAnim;
			newAnim.SetName(animName.GetData());
			// Add the new animation.
			_pActor->AddAnim(animGroupName.GetData(), newAnim);
			FxSize newAnimIndex = animGroup.FindAnim(animName.GetData());
			if( newAnimIndex != FxInvalidIndex )
			{
				FxAnim* pNewAnim = animGroup.GetAnimPtr(newAnimIndex);
				if( pNewAnim )
				{
					FxAnimUserData* pNewAnimUserData = new FxAnimUserData();
					pNewAnimUserData->SetNames(animGroupName, animName);
					pNewAnimUserData->SetRandomSeed(static_cast<FxInt32>(time(0)));

					pAudio->SetName(animName.GetData());
					FxMsg( "Bits per sample: %d", pAudio->GetBitsPerSample() );
					FxMsg( "Sample rate: %d", pAudio->GetSampleRate() );
					FxMsg( "Channel count: %d", pAudio->GetNumChannels() );
					FxMsg( "Sample count: %d", pAudio->GetNumSamples() );
					FxMsg( "Duration: %f seconds", pAudio->GetDuration() );
					pNewAnimUserData->SetAudio(pAudio);
					delete pAudio;
					pAudio = NULL;

					// If optionalText is valid, it overrides any corresponding .txt files.
					pNewAnimUserData->SetAnalysisText(optionalText);

					// Set up the language and configurations.
					pNewAnimUserData->SetLanguage(language);

					if( 0 == coarticulationConfig.Length() )
					{
						pNewAnimUserData->SetCoarticulationConfig(FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString());
					}
					else
					{
						pNewAnimUserData->SetCoarticulationConfig(coarticulationConfig);
					}
					if( 0 == gestureConfig.Length() )
					{
						pNewAnimUserData->SetGestureConfig(FxCGConfigManager::GetDefaultGestureConfigName().GetAsString());
					}
					else
					{
						pNewAnimUserData->SetGestureConfig(gestureConfig);
					}

					// Set if the animation should contain speech gestures.
					pNewAnimUserData->SetShouldContainSpeechGestures(generateSpeechGestures);

					FxMsg( "Language: %s", pNewAnimUserData->GetLanguage().GetData() );
					FxMsg( "Coarticulation Config: %s", pNewAnimUserData->GetCoarticulationConfig().GetData() );
					FxMsg( "Gesture Config: %s", pNewAnimUserData->GetGestureConfig().GetData() );
					FxMsg( "Generate Speech Gestures: %s", 
						pNewAnimUserData->ShouldContainSpeechGestures() ? 
						"true" : "false" );

					// It is not safe to unload this user data because it is not a lazy-loaded
					// ob ject from the original session.
					pNewAnimUserData->SetIsSafeToUnload(FxFalse);
					// Set the user data.
					pNewAnim->SetUserData(pNewAnimUserData);

					// Broadcast an OnCreateAnim message to pass the animation off for
					// analysis and store the user data in the session.
					OnCreateAnim(NULL, animGroupName.GetData(), pNewAnim);
					// Flush the undo redo manager because undoing this operation
					// would cause Bad Things.
					FxUndoRedoManager::Flush();
					// Force the session to be dirty.
					SetIsForcedDirty(FxTrue);
					return FxTrue;					
				}
			}
		}
		delete pAudio;
		pAudio = NULL;
	}
	return FxFalse;
}

FxActor* FxStudioSession::GetActor( void )
{
	return _pActor;
}

FxActorInstance* FxStudioSession::GetActorInstance( void )
{
	return _pActorInstance;
}

#ifdef __UNREAL__
UFaceFXAsset* FxStudioSession::GetFaceFXAsset( void )
{
	return _pFaceFXAsset;
}
#endif

const FxString& FxStudioSession::GetActorFilePath( void ) const
{
	return _actorFilePath;
}

FxAnimController* FxStudioSession::GetAnimController( void )
{
	return _pAnimController;
}

FxPhonemeMap* FxStudioSession::GetPhonemeMap( void )
{
	return _pPhonemeMap;
}

FxPhoneticAlphabet FxStudioSession::GetPhoneticAlphabet( void )
{
	return FxStudioOptions::GetPhoneticAlphabet();
}

FxBool FxStudioSession::IsForcedDirty( void ) const
{
	return _isForcedDirty;
}

void FxStudioSession::SetIsForcedDirty( FxBool isForcedDirty )
{
	_isForcedDirty = isForcedDirty;
	if( FxStudioApp::GetMainWindow() )
	{
		static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
	}
}

// Helper function to serialize node position information, used below.
FxArchive& operator<<( FxArchive& arc, FxNodePositionInfo& obj )
{
	arc << obj.nodeName << obj.x << obj.y;
	return arc;
}

void FxStudioSession::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	FxUInt16 version = arc.SerializeClassVersion("FxStudioSession");
	
	arc << _animUserDataList << _pPhonemeMap;
	FxFaceGraphNodeGroupManager::Serialize(arc);

	if( version >= 1 )
	{
		FxCameraManager::Serialize(arc);
		FxWorkspaceManager::Instance()->Serialize(arc);
	}

	// The version of the session does not matter here because the 
	// FxAnimSetManager does not serialize into the session archive.  It manages
	// its own archives internally and is only used when saving.
	if( arc.IsSaving() )
	{
		FxAnimSetManager::Serialize();
	}

	// Serialize the selections.
	arc << _currentTime << _selAnimGroupName << _selAnimName
		<< _selAnimCurveNames << _selFaceGraphNodeNames << _selNodeGroup
		<< _inputNode << _outputNode;
	// Serialize the colour mappings.
	FxColourMap::Serialize(arc);
	FxArray<FxNodePositionInfo> nodeInfos;
	FxInt32 viewLeft=0, viewTop=0, viewRight=640, viewBottom=480;
	FxReal zoomLevel=1.0f;
	if( arc.IsSaving() )
	{
		// Get the face graph info.
		OnSerializeFaceGraphSettings(this, FxTrue, nodeInfos, viewLeft, viewTop, 
			viewRight, viewBottom, zoomLevel);
	}
	// Serialize the face graph info.
	arc << nodeInfos << viewLeft << viewTop << viewRight << viewBottom << zoomLevel;
	if( arc.IsLoading() )
	{
		// Set the face graph info.
		OnSerializeFaceGraphSettings(this, FxFalse, nodeInfos, viewLeft, viewTop, 
			viewRight, viewBottom, zoomLevel);
	}
	// Serialize the current times
	arc << _minTime << _maxTime;

	// If a session is loaded with no phoneme mapping, create a blank one
	// by default.
	if( arc.IsLoading() )
	{
		if( !_pPhonemeMap )
		{
			_pPhonemeMap = new FxPhonemeMap();
		}
	}
}

void FxStudioSession::OnAppStartup( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: AppStartup.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnAppStartup(sender);
}

void FxStudioSession::OnAppShutdown( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: AppShutdown.", sender);
	}

	// Deselect everything.
	_deselectAll();
	Clear();

#ifdef __UNREAL__
	FxAnimSetManager::UnmountAllUE3AnimSets();
#endif // __UNREAL__

	FxAnimSetManager::FlushAllMountedAnimSets();

	// Propagate.
	FxWidgetMediator::OnAppShutdown(sender);

	if( _pActorInstance )
	{
		_pActorInstance->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActorInstance )
		{
			delete _pActorInstance;
		}
		_pActorInstance = NULL;
	}

	// Finally, make sure the user data has been cleared from the face graph - 
	// necessary because the session may have attached it without the face graph 
	// widget being present.
	if( _pActor )
	{
		FxFaceGraph& faceGraph = _pActor->GetDecompiledFaceGraph();
		FxFaceGraph::Iterator curr = faceGraph.Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator end  = faceGraph.End(FxFaceGraphNode::StaticClass());
		for( ; curr != end; ++curr )
		{
			if( curr.GetNode() && GetNodeUserData(curr.GetNode()) )
			{
				delete GetNodeUserData(curr.GetNode());
				curr.GetNode()->SetUserData(NULL);
			}
		}
		_pActor->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActor )
		{
			delete _pActor;
		}
		_pActor = NULL;
	}
	if( _pAnimController )
	{
		delete _pAnimController;
		_pAnimController = NULL;
	}
}

void FxStudioSession::OnActorChanged( FxWidget* sender, FxActor* actor )
{
	if( _debugWidgetMessages )
	{
		FxString actorNameString = "";
		if( actor )
		{
			actorNameString = actor->GetNameAsString();
		}
		FxMsg("Message received from %#x: ActorChanged.  Actor=%s (%#x)", sender, actorNameString.GetData(), actor);
	}

	// Set the Face Graph pointer in the animation controller.
	if( actor && _pAnimController )
	{
		_pAnimController->SetFaceGraphPointer(&actor->GetDecompiledFaceGraph());
	}

	// Propagate.
	FxWidgetMediator::OnActorChanged(sender, actor);
}

void FxStudioSession::OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags )
{
	if( _debugWidgetMessages )
	{
		FxString actorNameString = "";
		if( _pActor )
		{
			actorNameString = _pActor->GetNameAsString();
		}
		FxString flagString = "";
		if( flags == ADCF_All )
		{
			flagString = "All";
		}
		else
		{
			if( flags & ADCF_FaceGraphChanged ) flagString += "FaceGraph ";
			if( flags & ADCF_CurvesChanged ) flagString += "Curves ";
			if( flags & ADCF_AnimationsChanged ) flagString += "Animations ";
			if( flags & ADCF_AnimationGroupsChanged ) flagString += "AnimGroups ";
			if( flags & ADCF_CamerasChanged ) flagString += "Cameras ";
			if( flags & ADCF_WorkspacesChanged ) flagString += "Workspaces ";
			if( flags & ADCF_NodeGroupsChanged ) flagString += "NodeGroups ";
		}
		FxMsg("Message received from %#x: ActorInternalDataChanged.  Actor=%s Flags=%s", sender, actorNameString.GetData(), flagString.GetData());
	}

	// Propagate.
	FxWidgetMediator::OnActorInternalDataChanged(sender, flags);
}

void FxStudioSession::OnActorRenamed( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: ActorRenamed.", sender);
	}

	FxWidgetMediator::OnActorRenamed(sender);
}

void FxStudioSession::
OnQueryLanguageInfo( FxWidget* sender, FxArray<FxLanguageInfo>& languages )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: QueryLanguageInfo.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnQueryLanguageInfo(sender, languages);
}

void FxStudioSession::
OnCreateAnim( FxWidget* sender, const FxName& animGroupName, FxAnim* anim )
{
	if( _debugWidgetMessages )
	{
		FxName animName = anim ? anim->GetName() : FxName();
		FxMsg("Message received from %#x: CreateAnim.  AnimGroup=%s;  Anim=%s (%#x)", sender, animGroupName.GetAsCstr(), animName.GetAsString().GetData(), anim);
	}

	// Propagate.
	FxWidgetMediator::OnCreateAnim(sender, animGroupName, anim);

	FxAssert( anim != NULL );
	if( anim )
	{
		// Ensure whatever created the animation created and attached
		// valid user data.
		FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(anim->GetUserData());
		FxAssert( pAnimUserData != NULL );
		if( pAnimUserData )
		{
			// Add the user data to the user data list.
			_addAnimUserData(pAnimUserData);

			if( pAnimUserData->GetPhonemeWordListPointer() )
			{
                // If the phoneme word list contains zero phonemes the analysis must have failed so delete the
				// animation.
				if( 0 == pAnimUserData->GetPhonemeWordListPointer()->GetNumPhonemes() )
				{
					FxVM::DisplayError("Analysis failed - removing animation.");
					OnDeleteAnim(NULL, animGroupName, anim->GetName());
				}
				else
				{
					// Propagate an animation changed message.
					OnAnimChanged(NULL, animGroupName, anim);
				}
			}
			else
			{
				FxVM::DisplayError("Analysis failed - removing animation.");
				OnDeleteAnim(NULL, animGroupName, anim->GetName());
			}
		}
	}
}

void FxStudioSession::
OnAudioMinMaxChanged( FxWidget* sender, FxReal audioMin, FxReal audioMax )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: AudioMinMaxChanged.  AudioMin=%f;  AudioMax=%f",
			sender, audioMin, audioMax);
	}

	// Propagate.
	FxWidgetMediator::OnAudioMinMaxChanged(sender, audioMin, audioMax);
}

void FxStudioSession::
OnReanalyzeRange( FxWidget* sender, FxAnim* anim, FxReal rangeStart, 
				  FxReal rangeEnd, const FxWString& analysisText )
{
	if( _debugWidgetMessages )
	{
		FxName animName = anim ? anim->GetName() : FxName();
		FxMsg("Message received from %#x: ReanalyzeRange.  Anim=%s (%#x);  RangeStart=%f;  RangeEnd=%f;  AnalysisText=%s",
			sender, animName.GetAsCstr(), anim, rangeStart, rangeEnd, analysisText.GetCstr());
	}

	// Propagate.
	FxWidgetMediator::OnReanalyzeRange(sender, anim, rangeStart, rangeEnd, analysisText);
}

void FxStudioSession::
OnDeleteAnim( FxWidget* sender, const FxName& animGroupName, const FxName& animName )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: DeleteAnim.  AnimGroup=%s;  Anim=%s", sender, animGroupName.GetAsString().GetData(), animName.GetAsString().GetData());
	}

	// If the animation to be removed is in the same group as the currently 
	// selected animation, proper steps should be taken because the array
	// could be reallocated.
	FxName cachedSelAnimGroupName = _selAnimGroupName;
	FxName cachedSelAnimName = _selAnimName;
	FxArray<FxName> cachedAnimCurveSelection = _selAnimCurveNames;
	if( animGroupName == cachedSelAnimGroupName )
	{
		// Propagate an animation changed message to clear out the current
		// animation controller and prepare the widgets.
		OnAnimChanged(sender, cachedSelAnimGroupName, NULL);
	}
	
	// Remove the user data associated with the animation.
	_removeAnimUserData(animGroupName, animName);

	if( _pActor )
	{
		// Remove all user data associated with all curves in the animation.
		FxAnim* pAnimToDelete = _pActor->GetAnimPtr(animGroupName, animName);
		FxAssert(pAnimToDelete != NULL);
		if( pAnimToDelete )
		{
			FxSize numCurves = pAnimToDelete->GetNumAnimCurves();
			for( FxSize i = 0; i < numCurves; ++i )
			{
				FxAnimCurve& animCurve = pAnimToDelete->GetAnimCurveM(i);
				if( animCurve.GetUserData() )
				{
					delete animCurve.GetUserData();
					animCurve.SetUserData(NULL);
				}
			}
		}
		// Remove the animation.
		_pActor->RemoveAnim(animGroupName, animName);
	}
	
	// Propagate.
	FxWidgetMediator::OnDeleteAnim(sender, animGroupName, animName);

	// If the animation removed was in the same group as the currently selected
	// animation, but was not the currently selected animation, reselect the
	// previously selected animation.
	if( _pActor && animGroupName == cachedSelAnimGroupName && animName != cachedSelAnimName )
	{
		// Propagate an animation changed message to reselect the previously
		// selected animation.
		FxSize animGroupIndex = _pActor->FindAnimGroup(cachedSelAnimGroupName);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(cachedSelAnimName);
			if( animIndex != FxInvalidIndex )
			{
				OnAnimChanged(sender, cachedSelAnimGroupName, animGroup.GetAnimPtr(animIndex));
			}
			else
			{
				OnAnimChanged(sender, cachedSelAnimGroupName, NULL);
			}
		}
		// Propagate an animation curve selection changed message to reselect the
		// previously selected animation curves.
		OnAnimCurveSelChanged(sender, cachedAnimCurveSelection);
	}
}

void FxStudioSession::OnDeleteAnimGroup( FxWidget* sender, const FxName& animGroupName )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: DeleteAnimGroup.  AnimGroup=%s", sender, animGroupName.GetAsString().GetData());
	}

	FxBool deletedGroup = FxFalse;
	FxName cachedSelAnimGroupName = _selAnimGroupName;
	FxName cachedSelAnimName = _selAnimName;
	FxArray<FxName> cachedAnimCurveSelection = _selAnimCurveNames;
	if( _pActor )
	{
		FxSize animGroupIndex = _pActor->FindAnimGroup(animGroupName);
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& group = _pActor->GetAnimGroup(animGroupIndex);
			// Only empty groups can be removed.
			if( 0 == group.GetNumAnims() )
			{
				// Select the Default group because the array could be
				// reallocated.
				OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
				_pActor->RemoveAnimGroup(animGroupName);
				deletedGroup = FxTrue;
			}
		}
	}

	// Propagate.
	FxWidgetMediator::OnDeleteAnimGroup(sender, animGroupName);

	if( _pActor && deletedGroup && animGroupName != cachedSelAnimGroupName )
	{
		// Propagate an animation changed message to reselect the previously
		// selected animation.
		FxSize animGroupIndex = _pActor->FindAnimGroup(cachedSelAnimGroupName);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(cachedSelAnimName);
			if( animIndex != FxInvalidIndex )
			{
				OnAnimChanged(sender, cachedSelAnimGroupName, animGroup.GetAnimPtr(animIndex));
			}
			else
			{
				OnAnimChanged(sender, cachedSelAnimGroupName, NULL);
			}
		}
		// Propagate an animation curve selection changed message to reselect the
		// previously selected animation curves.
		OnAnimCurveSelChanged(sender, cachedAnimCurveSelection);
	}
}

void FxStudioSession::
OnAnimChanged( FxWidget* sender, const FxName& animGroupName, FxAnim* anim )
{
	if( _debugWidgetMessages )
	{
		FxName animName = anim ? anim->GetName() : FxName();
		FxMsg("Message received from %#x: AnimChanged.  AnimGroup=%s;  Anim=%s (%#x)", sender, animGroupName.GetAsString().GetData(), animName.GetAsString().GetData(), anim);
	}

	// Stop playing the animation.
	OnStopAnimation(sender);

	// Cache the previous animation group and name, so we can unload its user
	// data later if necessary.
	FxName prevAnimGroupName = _selAnimGroupName;
	FxName prevAnimName = _selAnimName;

	// Cache the state.
	_selAnimGroupName = animGroupName;
	_selAnimName      = anim ? anim->GetName() : FxName();

	// Trap cases of selecting an animation group that does not exist.
	if( _pActor )
	{
		// If the animation group does not exist, simply keep the previous 
		// selection.
		if( FxInvalidIndex == _pActor->FindAnimGroup(animGroupName) )
		{
			_selAnimGroupName = prevAnimGroupName;
			_selAnimName = prevAnimName;
		}
		else
		{
			_pAnim = anim;
		}
	}

	// Set the animation pointer in the animation controller.
	if( _pAnimController )
	{
		_pAnimController->DestroyUserData();
		_pAnimController->SetAnimPointer(_pAnim);
	}

	// Clear the curve selection when the animation changes.
	OnAnimCurveSelChanged(NULL, FxArray<FxName>());

	// Unload the previously selected animation's user data if the newly selected
	// animation is not the previously selected animation.
	if( _pActor )
	{
		FxSize animGroupIndex = _pActor->FindAnimGroup(prevAnimGroupName);
		if( FxInvalidIndex != animGroupIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(prevAnimName);
			if( FxInvalidIndex != animIndex )
			{
				FxAnim* pAnim = animGroup.GetAnimPtr(animIndex);
				if( pAnim && pAnim != _pAnim )
				{
					FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(pAnim->GetUserData());
					if( pAnimUserData )
					{
						// Unload will check to see if it is safe to unload the user data. Animation user data
						// objects that are no longer linked to the session (via an animation delete command
						// followed by an undo) should not be unloaded or their contained data will be lost
						// before the next save command.
						pAnimUserData->Unload();
					}
				}
			}
		}
	}

	if( _pAnim && _pActor )
	{
		// Always search for the user data!
		_pAnim->SetUserData(_findAnimUserData(_selAnimGroupName, _selAnimName));

		// If there was no animation user data, create it.
		if( _pAnim->GetUserData() == NULL )
		{
			FxAnimUserData* pNewAnimUserData = new FxAnimUserData();
			pNewAnimUserData->SetNames(_selAnimGroupName.GetAsString(), _selAnimName.GetAsString());
			pNewAnimUserData->SetRandomSeed(static_cast<FxInt32>(time(0)));
			pNewAnimUserData->SetShouldContainAnalysisResults(FxFalse);
			pNewAnimUserData->SetShouldContainSpeechGestures(FxFalse);

			_pAnim->SetUserData(pNewAnimUserData);
			_addAnimUserData(pNewAnimUserData);
		}
		
		// Set the phoneme list pointer in the session proxy.
		FxAnimUserData* pAnimUserData = reinterpret_cast<FxAnimUserData*>(_pAnim->GetUserData());
#ifdef __UNREAL__
		if( pAnimUserData )
		{
			FxString soundCuePath = _pAnim->GetSoundCuePath();
			FxString soundNodeWaveName = _pAnim->GetSoundNodeWave();
			USoundNodeWave* SoundNodeWave = NULL;
			if( soundCuePath.Length() > 0 && soundNodeWaveName.Length() > 0 )
			{
				FxMsg("Loading SoundNodeWave %s from SoundCue %s...", soundNodeWaveName.GetData(), soundCuePath.GetData());
				USoundCue* SoundCue = reinterpret_cast<USoundCue*>(_pAnim->GetSoundCuePointer());
				SoundNodeWave = FindSoundNodeWave(SoundCue, soundNodeWaveName);
				if( !SoundCue || !SoundNodeWave )
				{
					FxWarn("Couldn't load on first attempt - assuming it was renamed and attempting to hook to first SoundNodeWave in SoundCue...");
					// Make another attempt, assuming some crazy person as renamed the USoundNodeWave object.
					FxArray<wxString> SoundNodeWaves = FindAllSoundNodeWaves(SoundCue);
					if( SoundNodeWaves.Length() >= 1 )
					{
						FxString FoundSoundNodeWaveName(SoundNodeWaves[0].mb_str(wxConvLibc));
						SoundNodeWave = FindSoundNodeWave(SoundCue, FoundSoundNodeWaveName);
						if( SoundCue && SoundNodeWave )
						{
							_pAnim->SetSoundNodeWave(FoundSoundNodeWaveName);
							FxWarn("Set new SoundNodeWave to %s", FoundSoundNodeWaveName.GetData());
						}
					}
					if( !SoundCue || !SoundNodeWave )
					{
						FxMessageBox(_("Could not find linked SoundCue or SoundNodeWave!  Check the Console tab for more information."),
							         _("Error Detected"));
						FxError("Could not load SoundCue or SoundNodeWave!  Have you deleted, moved, or renamed the object or package?");
						FxError("Use the FaceFX Studio ue3 console command to fix invalid references to SoundCue and SoundNodeWave objects.");
						FxError("Example:  ue3 -link -group \"Default\" -anim \"Welcome\" -soundcuepath \"OC3_Slade.welcomeCue\" -soundnodewavename \"welcome\"");
					}
				}
			}
			if( SoundNodeWave )
			{
				FxAudio* pAudio = FxAudio::CreateFromSoundNodeWave(SoundNodeWave);
				if( pAudio )
				{
					pAudio->SetName(_selAnimName);
					pAnimUserData->SetAudio(pAudio);
					delete pAudio;
					pAudio = NULL;
				}
			}
		}
#endif
		FxPhonWordList* pPhonWordList = NULL;
		if( pAnimUserData )
		{
			pPhonWordList = pAnimUserData->GetPhonemeWordListPointer();
		}
		FxSessionProxy::SetPhonemeList(pPhonWordList);
		FxTimeManager::SetPhonWordList(pPhonWordList);
		// Propagate a phoneme list changed message.
		OnAnimPhonemeListChanged(NULL, _pAnim, pPhonWordList);

		FxString coarticulationConfig;
		FxString gestureConfig;
		if( pAnimUserData )
		{
			coarticulationConfig = pAnimUserData->GetCoarticulationConfig();
			gestureConfig = pAnimUserData->GetGestureConfig();
		
			if( 0 == coarticulationConfig.Length() )
			{
				coarticulationConfig = FxCGConfigManager::GetDefaultGenericCoarticulationConfigName().GetAsString();
			}
			if( 0 == gestureConfig.Length() )
			{
				gestureConfig = FxCGConfigManager::GetDefaultGestureConfigName().GetAsString();
			}

			pAnimUserData->SetCoarticulationConfig(coarticulationConfig);
			pAnimUserData->SetGestureConfig(gestureConfig);
		}

		// Propagate.
		FxWidgetMediator::OnAnimChanged(sender, _selAnimGroupName, _pAnim);
		// Reset the time to the beginning of the animation.
		OnTimeChanged(NULL, _pAnim->GetStartTime());
	}
	else
	{
		// Set the phoneme list pointer in the session proxy to NULL.
		FxSessionProxy::SetPhonemeList(NULL);
		FxTimeManager::SetPhonWordList(NULL);
		// Propagate a phoneme list changed message.
		OnAnimPhonemeListChanged(NULL, _pAnim, NULL);
		// Propagate.
		FxWidgetMediator::OnAnimChanged(sender, _selAnimGroupName, _pAnim);
		// Set the current time to FxInvalidValue so that the timeline widget
		// disables the appropriate time controls.
		OnTimeChanged(NULL, FxInvalidValue);
	}
}

void FxStudioSession::
OnAnimPhonemeListChanged( FxWidget* sender, FxAnim* anim, FxPhonWordList* phonemeList )
{
	if( _debugWidgetMessages )
	{
		FxName animName = anim ? anim->GetName() : FxName();
		FxMsg("Message received from %#x: AnimPhonemeListChanged.  Anim=%s (%#x) PhonemeList (%#x)", sender, animName.GetAsString().GetData(), anim, phonemeList);
	}

	// Propagate.
	FxWidgetMediator::OnAnimPhonemeListChanged(sender, anim, phonemeList);

	// Issue an animation curve selection changed message to all widgets because
	// the animation phoneme list changed message recreates the data in the
	// animation controller.
	// Clear all curve visibility flags in the animation controller.
	if( _pAnimController )
	{
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			_pAnimController->SetCurveVisible(i, FxFalse);
		}

		// Set all curve visibility flags in the animation controller.
		for( FxSize i = 0; i < _selAnimCurveNames.Length(); ++i )
		{
			_pAnimController->SetCurveVisible(_selAnimCurveNames[i], FxTrue);
		}
	}
	// Propagate.
	FxWidgetMediator::OnAnimCurveSelChanged(sender, _selAnimCurveNames);

	// Issue a refresh message to all widgets.
	FxWidgetMediator::OnRefresh(NULL);
}

void FxStudioSession::
OnPhonemeMappingChanged( FxWidget* sender, FxPhonemeMap* phonemeMap )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: PhonemeMappingChanged.  PhonemeMap (%#x)", sender, phonemeMap);
	}

	// Propagate.
	FxWidgetMediator::OnPhonemeMappingChanged(sender, phonemeMap);

	// Update the phoneme map in the actor.
	if( _pActor && phonemeMap )
	{
		_pActor->GetPhonemeMap() = *phonemeMap;
	}

	// Issue an animation curve selection changed message to all widgets because
	// the animation phoneme list changed message recreates the data in the
	// animation controller.
	// Clear all curve visibility flags in the animation controller.
	if( _pAnimController )
	{
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			_pAnimController->SetCurveVisible(i, FxFalse);
		}

		// Set all curve visibility flags in the animation controller.
		for( FxSize i = 0; i < _selAnimCurveNames.Length(); ++i )
		{
			_pAnimController->SetCurveVisible(_selAnimCurveNames[i], FxTrue);
		}
	}
	// Propagate.
	FxWidgetMediator::OnAnimCurveSelChanged(sender, _selAnimCurveNames);

	// Issue a refresh message to all widgets.
	FxWidgetMediator::OnRefresh(NULL);
}

void FxStudioSession::
OnAnimCurveSelChanged( FxWidget* sender, const FxArray<FxName>& selAnimCurveNames )
{
	if( _debugWidgetMessages )
	{
		FxString selCurves("");
		for( FxSize i = 0; i < selAnimCurveNames.Length(); ++i )
		{
			selCurves = FxString::Concat( selCurves, selAnimCurveNames[i].GetAsString() );
			if( i != selAnimCurveNames.Length() - 1 ) selCurves = FxString::Concat(selCurves, ", ");
		}
		FxMsg("Message received from %#x: AnimCurveSelChanged.  Selected curves=%s", sender, selCurves);
	}

	// Cache the state.
	_selAnimCurveNames.Clear();
	for( FxSize i = 0; i < selAnimCurveNames.Length(); ++i )
	{
		_selAnimCurveNames.PushBack(selAnimCurveNames[i]);
	}

	if( _pAnimController )
	{
		// Clear all curve visibility flags in the animation controller.
		for( FxSize i = 0; i < _pAnimController->GetNumCurves(); ++i )
		{
			_pAnimController->SetCurveVisible(i, FxFalse);
		}

		// Set all curve visibility flags in the animation controller.
		for( FxSize i = 0; i < _selAnimCurveNames.Length(); ++i )
		{
			_pAnimController->SetCurveVisible(_selAnimCurveNames[i], FxTrue);
		}
	}

	// Propagate.
	FxWidgetMediator::OnAnimCurveSelChanged(sender, _selAnimCurveNames);
}

void FxStudioSession::OnLayoutFaceGraph( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: LayoutFaceGraph.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnLayoutFaceGraph(sender);
}

void FxStudioSession::OnAddFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: AddFaceGraphNode.  node (%#x)", sender, node);
	}

	// Propagate.
	FxWidgetMediator::OnAddFaceGraphNode(sender, node);
}

void FxStudioSession::OnRemoveFaceGraphNode( FxWidget* sender, FxFaceGraphNode* node )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: RemoveFaceGraphNode.  node (%#x)", sender, node);
	}

	// Propagate.
	FxWidgetMediator::OnRemoveFaceGraphNode(sender, node);
}

void FxStudioSession::
OnFaceGraphNodeSelChanged( FxWidget* sender, const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection )
{
	if( _debugWidgetMessages )
	{
		FxString selFaceGraphNodes("");
		for( FxSize i = 0; i < selFaceGraphNodeNames.Length(); ++i )
		{
			selFaceGraphNodes = FxString::Concat( selFaceGraphNodes, selFaceGraphNodeNames[i].GetAsString() );
			if( i != selFaceGraphNodeNames.Length() - 1 ) selFaceGraphNodes = FxString::Concat(selFaceGraphNodes, ", ");
		}
		if( selFaceGraphNodeNames.Length() == 0 )
		{
			selFaceGraphNodes = "Null selection";
		}
		FxMsg("Message received from %#x: FaceGraphNodeSelChanged.  Selected nodes=%s", sender, selFaceGraphNodes.GetData());
	}

	// Cache the state.
	_selFaceGraphNodeNames.Clear();
	for( FxSize i = 0; i < selFaceGraphNodeNames.Length(); ++i )
	{
		_selFaceGraphNodeNames.PushBack(selFaceGraphNodeNames[i]);
	}

	// Propagate.
	FxWidgetMediator::OnFaceGraphNodeSelChanged(sender, _selFaceGraphNodeNames, zoomToSelection);
}

void FxStudioSession::OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: FaceGraphNodeGroupSelChanged.  Group: %s", sender, selGroup.GetAsCstr());
	}

	// Cache the currently selected group.
	_selNodeGroup = selGroup;

	// Propagate.
	FxWidgetMediator::OnFaceGraphNodeGroupSelChanged(sender, _selNodeGroup);
}

void FxStudioSession::
OnLinkSelChanged( FxWidget* sender, const FxName& inputNode, const FxName& outputNode )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: LinkSelChanged.  Link: %s->%s", sender, outputNode.GetAsString().GetData(), inputNode.GetAsString().GetData());
	}

	// Cache the state.
	_inputNode  = inputNode;
	_outputNode = outputNode;

	// Propagate.
	FxWidgetMediator::OnLinkSelChanged(sender, _inputNode, _outputNode);
}

void FxStudioSession::OnRefresh( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: Refresh.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnRefresh(sender);
}

void FxStudioSession::OnTimeChanged( FxWidget* sender, FxReal newTime )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: TimeChanged.  Time=%f", sender, newTime);
	}

	// Cache the new time.
	_currentTime = newTime;

	if( _pActor )
	{
		// Reset the reference bone weights.
		FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
		masterBoneList.ResetRefBoneWeights();
		if( _pAnim )
		{
			// Set the reference bone weights for the current animation.
			FxSize numBoneWeights = _pAnim->GetNumBoneWeights();
			for( FxSize i = 0; i < numBoneWeights; ++i )
			{
				const FxAnimBoneWeight& boneWeight = _pAnim->GetBoneWeight(i);
				masterBoneList.SetRefBoneCurrentWeight(boneWeight.boneName, boneWeight.boneWeight);
			}
		}
	}

	// Update the animation controller with the new time.
	if( _pAnimController )
	{
		_pAnimController->SetTime(_currentTime);
	}

	// Propagate.
	FxWidgetMediator::OnTimeChanged(sender, _currentTime);
}

void FxStudioSession::OnPlayAnimation( FxWidget* sender, FxReal startTime,
									   FxReal endTime, FxBool loop )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: PlayAnimation.  startTime=%f endTime=%f loop=%d", sender, startTime, endTime, loop);
	}

	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		if( startTime != FxInvalidValue && endTime != FxInvalidValue )
		{
			animPlayer->PlaySection(startTime, endTime);
		}
		else
		{
			animPlayer->Play(loop);
		}
	}

	// Propagate.
	FxWidgetMediator::OnPlayAnimation(sender, startTime, endTime, loop);
}

void FxStudioSession::OnStopAnimation( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: StopAnimation.", sender);
	}

	FxStudioAnimPlayer* animPlayer = FxStudioAnimPlayer::Instance();
	if( animPlayer )
	{
		animPlayer->Stop();
	}

	// Propagate.
	FxWidgetMediator::OnStopAnimation(sender);
}

void FxStudioSession::OnAnimationStopped( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: AnimationStopped.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnAnimationStopped(sender);

	// Defer searching for and caching the g_loopallfromhere console variable.
	if( !g_loopallfromhere )
	{
		g_loopallfromhere = FxVM::FindConsoleVariable("g_loopallfromhere");
	}
	if( g_loopallfromhere && _pActor )
	{
		FxBool loopallfromhere = static_cast<FxBool>(*g_loopallfromhere);
		if( loopallfromhere )
		{
			//@todo Deferred implementing the "loop all from here" feature until
			//      further notice.
			FxMsg("Loop all from here active!");
		}
	}
}

// Called to get/set options in the face graph
void FxStudioSession::OnSerializeFaceGraphSettings( FxWidget* sender, FxBool isGetting, 
													FxArray<FxNodePositionInfo>& nodeInfos, FxInt32& viewLeft, FxInt32& viewTop,
													FxInt32& viewRight, FxInt32& viewBottom, FxReal& zoomLevel )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x %s: SerializeFaceGraphSettings.", sender, 
			isGetting ? "(getting)" : "(setting)");
	}

	if( _pActor )
	{
		FxFaceGraph* pGraph = &_pActor->GetDecompiledFaceGraph();
		if( isGetting && nodeInfos.Length() == 0 )
		{
			nodeInfos.Reserve(pGraph->GetNumNodes());
			// Go through each node in the face graph and get their positions.
			FxFaceGraph::Iterator curr = pGraph->Begin(FxFaceGraphNode::StaticClass());
			for( ; curr != pGraph->End(FxFaceGraphNode::StaticClass()); ++curr )
			{
				FxFaceGraphNode* node = curr.GetNode();
				if( node )
				{
					FxNodePositionInfo nodeInfo;
					nodeInfo.nodeName = node->GetNameAsString();
					// Ensure the node has user data.
					if( !GetNodeUserData(node) )
					{
						FxNodeUserData* nodeUserData = new FxNodeUserData();
						nodeUserData->bounds = FxRect(FxPoint(0,0), FxSize2D(0,0));
						node->SetUserData(nodeUserData);
					}
					nodeInfo.x = GetNodeUserData(node)->bounds.GetLeft();
					nodeInfo.y = GetNodeUserData(node)->bounds.GetTop();
					nodeInfos.PushBack(nodeInfo);
				}
			}
			viewLeft   = _viewLeft;
			viewTop    = _viewTop;
			viewRight  = _viewRight;
			viewBottom = _viewBottom;
			zoomLevel  = _zoomLevel;
		}
		else
		{
			for( FxSize i = 0; i < nodeInfos.Length(); ++i )
			{
				FxFaceGraphNode* node = pGraph->FindNode(nodeInfos[i].nodeName.GetData());
				if( node )
				{
					// Ensure the node has user data.
					if( !GetNodeUserData(node) )
					{
						FxNodeUserData* nodeUserData = new FxNodeUserData();
						nodeUserData->bounds = FxRect(FxPoint(0,0), FxSize2D(0,0));
						node->SetUserData(nodeUserData);
					}
					GetNodeUserData(node)->bounds.SetLeft(nodeInfos[i].x);
					GetNodeUserData(node)->bounds.SetTop(nodeInfos[i].y);
				}
			}

			_viewLeft   = viewLeft;
			_viewTop    = viewTop;
			_viewRight  = viewRight;
			_viewBottom = viewBottom;
			_zoomLevel  = zoomLevel;
		}
	}

	// Propagate.
	FxWidgetMediator::OnSerializeFaceGraphSettings(sender, isGetting, nodeInfos,
		viewLeft, viewTop, viewRight, viewBottom, zoomLevel);
}

void FxStudioSession::OnRecalculateGrid( FxWidget* sender )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: RecalculateGrid.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnRecalculateGrid(sender);
}

void FxStudioSession::OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: QueryRenderWidgetCaps.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnQueryRenderWidgetCaps(sender, renderWidgetCaps);
}

void FxStudioSession::OnInteractEditWidget( FxWidget* sender, FxBool isQuery, FxBool& shouldSave )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: InteractEditWidget.", sender);
	}

	// Propagate.
	FxWidgetMediator::OnInteractEditWidget(sender, isQuery, shouldSave);
}

void FxStudioSession::OnGenericCoarticulationConfigChanged( FxWidget* sender, FxCoarticulationConfig* config )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: OnGenericCoarticulationConfigChanged config=%#x", sender, config);
	}

	// Propagate.
	FxWidgetMediator::OnGenericCoarticulationConfigChanged(sender, config);
}

void FxStudioSession::OnGestureConfigChanged( FxWidget* sender, FxGestureConfig* config )
{
	if( _debugWidgetMessages )
	{
		FxMsg("Message received from %#x: OnGestureConfigChanged config=%#x", sender, config);
	}

	// Propagate.
	FxWidgetMediator::OnGestureConfigChanged(sender, config);
}

#ifdef __UNREAL__
FxBool FxStudioSession::SetActor( FxActor*& pActor, UFaceFXAsset* pFaceFXAsset, 
								  UFaceFXAnimSet* pFaceFXAnimSet )
{
	// If the actor or session need to be saved, prompt the user first.
	if( !PromptToSaveChangedActor() )
	{
		// Return FxFalse because the actor was not actually changed (ie, the
		// user pressed cancel when prompted to save the current actor).
		return FxFalse;
	}

	// Deselect everything.
	_deselectAll();

	Clear();
	FxFaceGraphNodeGroupManager::FlushAllGroups();
	// Unmount a previously selected animset.  If it was mounted via Matinee,
	// the FxAnimSetManager will remount it afterwards.
	FxAnimSetManager::UnmountAllUE3AnimSets();
	FxAnimSetManager::FlushAllMountedAnimSets();
	FxArchive::UnmountAllArchives();

	if( _pActor )
	{
		_pActor->SetIsOpenInStudio(FxFalse);
		if( !_noDeleteActor )
		{
			delete _pActor;
		}
		_pActor = NULL;
	}
	_noDeleteActor = FxTrue;
	_pActor = pActor;
	
	_pFaceFXAsset   = pFaceFXAsset;
	_pFaceFXAnimSet = pFaceFXAnimSet;

	if( _pActor )
	{
		// Automatically execute a pre load script for the actor if it exists.
		FxString nonConstActorNameString = _pActor->GetNameAsString();
		FxTriggerScriptEvent(STET_PreLoadActor, &nonConstActorNameString);

		FxSessionProxy::SetNumDummyRegisters(_pActor->GetCompiledFaceGraph().nodes.Length());
		_pActor->DecompileFaceGraph();
		_pActor->SetIsOpenInStudio(FxTrue);
		// Prune just in case.
		_pActor->GetMasterBoneList().Prune(_pActor->GetDecompiledFaceGraph());
		if( FxName::NullName == _pActor->GetName() ) 
		{
			_pActor->SetName("NewActor");
			SetIsForcedDirty(FxTrue);
			FxWarn("Actor had an invalid name.  FaceFX Studio has automatically reset the name to NewActor.");
		}
		// Display actor information.
		FxMsg("Set actor to %s", _pActor->GetNameAsCstr());
		OnActorChanged(NULL, _pActor);

		// Load the session.
		FxBool goodSession = FxFalse;
		if( _pFaceFXAsset && _pFaceFXAsset->RawFaceFXSessionBytes.Num() > 0 )
		{
			FxArchiveStoreMemoryNoCopy* sessionMemory = FxArchiveStoreMemoryNoCopy::Create(
				static_cast<FxByte*>(_pFaceFXAsset->RawFaceFXSessionBytes.GetData()), 
				                     _pFaceFXAsset->RawFaceFXSessionBytes.Num());
			FxArchive sessionArchive(sessionMemory, FxArchive::AM_LinearLoad);
			sessionArchive.Open();
			FxArchiveProgressDisplay::Begin(_("Loading session..."), FxStudioApp::GetMainWindow());
			sessionArchive.RegisterProgressCallback(FxArchiveProgressDisplay::UpdateProgress);
			Serialize(sessionArchive);
			FxArchiveProgressDisplay::End();
			goodSession = FxTrue;
		}

		if( !goodSession )
		{
			// If there was no session, make sure there is a valid phoneme map.
			if( !_pPhonemeMap )
			{
				_pPhonemeMap = new FxPhonemeMap();
			}

			// If there was no session, make sure the face graph is properly
			// laid out.
			FxVM::ExecuteCommand("graph -layout");

			// Issue a warning if the loaded actor contains information that it
			// could possibly need the session file for.  If the actor has
			// valid animations, the audio from those animations would be in the
			// session, along with the proper phoneme mapping, and other session
			// stored data.
			if( _pActor )
			{
				FxBool validAnims = FxFalse;
				for( FxSize i = 0; i < _pActor->GetNumAnimGroups(); ++i )
				{
					FxAnimGroup& animGroup = _pActor->GetAnimGroup(i);
					if( animGroup.GetNumAnims() > 0 )
					{
						validAnims = FxTrue;
						break;
					}
				}
				if( validAnims )
				{
					FxMessageBox(_("You have loaded an actor that might need data from a session that could not be loaded."), 
							     _("Could not load session"));
				}
			}
		}

		if( 0 == _pPhonemeMap->GetNumTargets() )
		{
			_pPhonemeMap->MakeDefaultMapping();
			FxWarn("The actor did not have a valid phoneme mapping. The default mapping was created for you.");
		}

		FxArray<FxName> previousAnimCurveSelection = _selAnimCurveNames;
		FxArray<FxName> previousFaceGraphNodeSelection = _selFaceGraphNodeNames;
		FxReal previousCurrentTime = _currentTime;

		OnPhonemeMappingChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _pPhonemeMap);
		// Load the anim.
		FxSize animGroupIndex = _pActor->FindAnimGroup(_selAnimGroupName);
		if( animGroupIndex != FxInvalidIndex )
		{
			FxAnimGroup& animGroup = _pActor->GetAnimGroup(animGroupIndex);
			FxSize animIndex = animGroup.FindAnim(_selAnimName);
			if( animIndex != FxInvalidIndex )
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selAnimGroupName, animGroup.GetAnimPtr(animIndex));
			}
			else
			{
				OnAnimChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selAnimGroupName, NULL);
			}
		}
		OnAnimCurveSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousAnimCurveSelection);
		if( _selNodeGroup == FxName::NullName )
		{
			OnFaceGraphNodeGroupSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), FxFaceGraphNodeGroupManager::GetAllNodesGroupName().GetData());
		}
		else
		{
			OnFaceGraphNodeGroupSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _selNodeGroup);
		}
		OnFaceGraphNodeSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousFaceGraphNodeSelection, FxFalse);
		OnLinkSelChanged(reinterpret_cast<FxWidget*>(0x10adac7a), _inputNode, _outputNode);
		OnTimeChanged(reinterpret_cast<FxWidget*>(0x10adac7a), previousCurrentTime);
		OnActorInternalDataChanged(reinterpret_cast<FxWidget*>(0x10adac7a), ADCF_NodeGroupsChanged);
		FxTimeManager::RequestTimeChange(_minTime, _maxTime);

		if( FxStudioApp::GetMainWindow() )
		{
			static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateWindowUI();
			static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->UpdateTitleBar();
		}
		FxWidgetMediator::OnLoadActor(NULL, FxString());

		if( _pFaceFXAnimSet )
		{
			if( !FxAnimSetManager::MountUE3AnimSet(_pFaceFXAnimSet) )
			{
				FxMessageBox(_("Could not mount FaceFXAnimSet because an animation group with the same name already exists in the FaceFXAsset!"), 
							 _("Duplicate animation group"));
				FxError("Could not mount FaceFXAnimSet because an animation group with the same name already exists in the FaceFXAsset!");
			}
		}

		// Search for and "officially" mount all animation sets that have been mounted external to FaceFX Studio
		// (from Matinee for example).  This is essential so that artists cannot lose work.
		FxSize NumAnimGroups = _pActor->GetNumAnimGroups();
		for( FxSize i = 0; i < NumAnimGroups; ++i )
		{
			const FxName& AnimGroupName = _pActor->GetAnimGroup(i).GetName();
			if( _pActor->IsAnimGroupMounted(AnimGroupName) )
			{
				for( TObjectIterator<UFaceFXAnimSet> It; It; ++It )
				{
					UFaceFXAnimSet* AnimSet = *It;
					if( AnimSet && AnimSet != _pFaceFXAnimSet && _pFaceFXAsset->MountedFaceFXAnimSets.ContainsItem(AnimSet) )
					{
						FxAnimSet* pInternalAnimSet = AnimSet->GetFxAnimSet();
						if( pInternalAnimSet )
						{
							if( pInternalAnimSet->GetAnimGroup().GetName() == AnimGroupName )
							{
								if( !FxAnimSetManager::MountUE3AnimSet(AnimSet) )
								{
									FxMessageBox(_("Could not mount FaceFXAnimSet because an animation group with the same name already exists in the FaceFXAsset!"), 
										         _("Duplicate animation group"));
									FxError("Could not mount FaceFXAnimSet because an animation group with the same name already exists in the FaceFXAsset!");
								}	
							}
						}
					}
				}
			}
		}

		// Automatically execute a post load script for the actor if it exists.
		FxTriggerScriptEvent(STET_PostLoadActor, &nonConstActorNameString);
	}
	_checkContent();
	return FxTrue;
}

FxBool FxStudioSession::SetActorInstance( FxActorInstance* pActorInstance )
{
	_pActorInstance = NULL;
	if( pActorInstance )
	{
		_pActorInstance = pActorInstance;
		pActorInstance->SetIsOpenInStudio(FxTrue);
		_noDeleteActorInstance = FxTrue;
	}
	return FxTrue;
}
#endif  // __UNREAL__

FxAnimUserData* FxStudioSession::_findAnimUserData( const FxName& animGroupName, const FxName& animName )
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
	// Search any mounted animation sets as well.  If it cannot be found in 
	// the animation set manager, it does not exist and NULL will be returned.
	return FxAnimSetManager::FindAnimUserData(animGroupName, animName);
}

void FxStudioSession::_addAnimUserData( FxAnimUserData* pAnimUserData )
{
	FxAssert(pAnimUserData);
	if( pAnimUserData )
	{
		FxString animGroupName;
		FxString animName;
		pAnimUserData->GetNames(animGroupName, animName);
		if( FxInvalidIndex != FxAnimSetManager::FindMountedAnimSet(animGroupName.GetData()) )
		{
			// The animation user data should be routed to a mounted animation
			// set.
			FxAnimSetManager::AddAnimUserData(pAnimUserData);
		}
		else
		{
			// The animation user data should not be routed to a mounted
			// animation set.
			if( !_findAnimUserData(animGroupName.GetData(), animName.GetData()) )
			{
				_animUserDataList.PushBack(pAnimUserData);
			}
		}
	}
}

void FxStudioSession::_removeAnimUserData( const FxName& animGroupName, const FxName& animName )
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
	// Search any mounted animation sets as well.
	FxAnimSetManager::RemoveAnimUserData(animGroupName, animName);
}

void FxStudioSession::_findAndRemoveOrphanedAnimUserData( void )
{
	if( _pActor )
	{
		for( FxSize i = 0; i < _animUserDataList.Length(); ++i )
		{
			if( _animUserDataList[i] )
			{
				FxString userDataGroupName, userDataAnimName;
				_animUserDataList[i]->GetNames(userDataGroupName, userDataAnimName);
				// Find any animations in the actor that could match this combination.
				if( !_pActor->GetAnimPtr(FxName(userDataGroupName.GetData()), FxName(userDataAnimName.GetData())) )
				{
					// No match was found.  Warn the user and destroy the user data.
					delete _animUserDataList[i];
					_animUserDataList[i] = NULL;
					_animUserDataList.Remove(i);
					--i;
					FxWarn("Found and removed orphaned animation user data object: <b>\"%s.%s\"</b>", userDataGroupName.GetData(), userDataAnimName.GetData());
				}
			}
		}
		// Search any mounted animation sets as well.
		FxAnimSetManager::FindAndRemoveOrphanedAnimUserData();
	}
}

void FxStudioSession::_checkContent( void )
{
	if( _pActor )
	{
		// First, check for Face Graph nodes with suspicious min and max values
		// and depricated link functions.
		FxBool warningsGenerated = FxFalse;
		FxFaceGraph& faceGraph = _pActor->GetDecompiledFaceGraph();
		FxFaceGraph::Iterator faceGraphNodeIter    = faceGraph.Begin(FxFaceGraphNode::StaticClass());
		FxFaceGraph::Iterator faceGraphNodeEndIter = faceGraph.End(FxFaceGraphNode::StaticClass());
		for( ; faceGraphNodeIter != faceGraphNodeEndIter; ++faceGraphNodeIter )
		{
			FxFaceGraphNode* pNode = faceGraphNodeIter.GetNode();
			FxAssert(pNode);
			if( pNode->GetMin() >= pNode->GetMax() )
			{
				FxWarn("Found face graph node <b>\"%s\"</b> with suspicious min(<b>%f</b>) and max(<b>%f</b>) values (min >= max)", 
					pNode->GetNameAsCstr(), pNode->GetMin(), pNode->GetMax());
				warningsGenerated = FxTrue;
			}

			FxSize numInputs = pNode->GetNumInputLinks();
			for( FxSize i = 0; i < numInputs; ++i )
			{
				const FxFaceGraphNodeLink& link = pNode->GetInputLink(i);
				if( link.GetLinkFnType() == LFT_Null )
				{
					FxWarn("Found deprecated link function <b>\"%s\"</b> going from face graph node <b>\"%s\"</b> to <b>\"%s\"</b>",
						link.GetLinkFnName().GetAsCstr(),
						link.GetNode()->GetNameAsCstr(),
						pNode->GetNameAsCstr());
					warningsGenerated = FxTrue;
				}
			}
		}

		// Next, check for bone pose nodes with rotations different than the reference pose
		// but with min < 0.0 or max > 1.0.
		FxMasterBoneList& masterBoneList = _pActor->GetMasterBoneList();
		FxFaceGraph::Iterator bonePoseNodeIter    = faceGraph.Begin(FxBonePoseNode::StaticClass());
		FxFaceGraph::Iterator bonePoseNodeEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
		for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
		{
			FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(bonePoseNodeIter.GetNode());
			if( pBonePoseNode )
			{
				FxBool bShouldDisplayWarning = FxFalse;
				FxSize numBones = pBonePoseNode->GetNumBones();
				for( FxSize i = 0; i < numBones; ++i )
				{
					const FxBone& poseBone = pBonePoseNode->GetBone(i);

					// Find the pose bone in the master bone list reference bones.
					FxSize numRefBones = masterBoneList.GetNumRefBones();
					for( FxSize i = 0; i < numRefBones; ++i )
					{
						const FxBone& refBone = masterBoneList.GetRefBone(i);
						if( poseBone.GetName() == refBone.GetName() )
						{
							// Are the rotations different and the min and max
							// out of range?
							if( (poseBone.GetRot() != refBone.GetRot()) && (pBonePoseNode->GetMin() < 0.0f || pBonePoseNode->GetMax() > 1.0f) )
							{
								bShouldDisplayWarning = FxTrue;
							}
						}
					}
				}
				if( bShouldDisplayWarning )
				{
					FxWarn("Found face graph bone pose node <b>\"%s\"</b> containing bones with rotations but with min(<b>%f</b>) or max(<b>%f</b>) out of range (min &lt 0.0 or max &gt 1.0)", 
						pBonePoseNode->GetNameAsCstr(), pBonePoseNode->GetMin(), pBonePoseNode->GetMax());
					warningsGenerated = FxTrue;
				}
			}
		}

		if( warningsGenerated )
		{
			FxMessageBox(_("Warnings were generated during loading. Please check the Console for details."),
				         _("Check Console"));
		}

#ifdef __UNREAL__
		if( _pFaceFXAsset )
		{
			FxBool bThereWereErrors = FxFalse;
			if( _pFaceFXAsset->NumLoadErrors > 0 )
			{
				bThereWereErrors = FxTrue;
			}
			FxSize NumMountedSets = static_cast<FxSize>(_pFaceFXAsset->MountedFaceFXAnimSets.Num());
			for( FxSize i = 0; i < NumMountedSets; ++i )
			{
				if( _pFaceFXAsset->MountedFaceFXAnimSets(i) )
				{
					if( _pFaceFXAsset->MountedFaceFXAnimSets(i)->NumLoadErrors > 0 )
					{
						bThereWereErrors = FxTrue;
					}
				}
			}
			if( bThereWereErrors )
			{
				FxMessageBox(_("Errors were generating during loading.  Check the UnrealEd log."), 
						     _("Load Errors Detected"));
				FxError("Errors were generating during loading.  Check the UnrealEd log.");
			}
		}
#endif // __UNREAL__
		
		// Now check the session data.
		_findAndRemoveOrphanedAnimUserData();
	}
}

void FxStudioSession::_deselectAll( void )
{
	// Deselect everything in all widgets by issuing widget messages.
	OnStopAnimation(NULL);
	OnFaceGraphNodeSelChanged(NULL, FxArray<FxName>());
	OnLinkSelChanged(NULL, FxName::NullName, FxName::NullName);
	OnActorChanged(NULL, NULL);
	OnPhonemeMappingChanged(NULL, NULL);
	OnAnimCurveSelChanged(NULL, FxArray<FxName>());
	OnAnimPhonemeListChanged(NULL, NULL, NULL);
	OnAnimChanged(NULL, FxAnimGroup::Default, NULL);
	OnTimeChanged(NULL, FxInvalidValue);
}

} // namespace Face

} // namespace OC3Ent
