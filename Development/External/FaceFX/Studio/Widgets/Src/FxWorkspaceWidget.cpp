//------------------------------------------------------------------------------
// The widget for interacting with slider workspaces.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxWorkspaceWidget.h"
#include "FxStudioApp.h"
#include "FxCreateFaceGraphNodeDialog.h"
#include "FxSessionProxy.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/dcbuffer.h" // buffered paint dc
	#include "wx/minifram.h" // miniframe
#endif

namespace OC3Ent
{

namespace Face
{

WX_IMPLEMENT_DYNAMIC_CLASS(FxWorkspaceWidget, wxWindow)

BEGIN_EVENT_TABLE(FxWorkspaceWidget, wxWindow)
	EVT_SIZE(FxWorkspaceWidget::OnSize)
	EVT_PAINT(FxWorkspaceWidget::OnPaint)
	EVT_MOUSE_EVENTS(FxWorkspaceWidget::OnMouse)
	EVT_KEY_DOWN(FxWorkspaceWidget::OnKeyDown)
	EVT_SYS_COLOUR_CHANGED(FxWorkspaceWidget::OnSystemColoursChanged)

	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxWorkspaceWidget::OnHelp)

	EVT_CHOICE(ControlID_WorkspaceWidgetToolbarWorkspaceChoice, FxWorkspaceWidget::OnToolbarWorkspaceChoiceChange)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarEditWorkspaceButton, FxWorkspaceWidget::OnToolbarEditWorkspaceButton)
	EVT_CHOICE(ControlID_WorkspaceWidgetToolbarModeChoice, FxWorkspaceWidget::OnToolbarModeChoiceChange)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarDisplayNamesToggle, FxWorkspaceWidget::OnToolbarDisplayNamesToggle)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarAnimModeAutokeyToggle, FxWorkspaceWidget::OnToolbarWorkspaceOptionChange)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarAutoselectCurvesToggle, FxWorkspaceWidget::OnToolbarWorkspaceOptionChange)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarAnimModeKeyAllButton, FxWorkspaceWidget::OnToolbarAnimModeKeyAllButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarAnimModePrevKeyButton, FxWorkspaceWidget::OnToolbarAnimModePrevKeyButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarAnimModeNextKeyButton, FxWorkspaceWidget::OnToolbarAnimModeNextKeyButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarSharedModeClearButton, FxWorkspaceWidget::OnToolbarSharedModeClearButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarSharedModeLoadCurrentFrameButton, FxWorkspaceWidget::OnToolbarSharedModeLoadCurrentFrameButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarCombinerModeCreateButton, FxWorkspaceWidget::OnToolbarCombinerModeCreateButton)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarMappingModeApplyButton, FxWorkspaceWidget::OnToolbarMappingModeApplyButton)
	EVT_CHOICE(ControlID_WorkspaceWidgetToolbarMappingModePhonemeChoice, FxWorkspaceWidget::OnToolbarMappingModePhonemeChoiceChange)
	EVT_BUTTON(ControlID_WorkspaceWidgetToolbarInstanceWorkspaceButton, FxWorkspaceWidget::OnToolbarInstanceWorkspaceButton)
END_EVENT_TABLE()

FxWorkspaceWidget::FxWorkspaceWidget(wxWindow* parent, FxWidgetMediator* mediator)
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL|wxWANTS_CHARS)
	, FxWidget(mediator)
	, _actor(NULL)
	, _anim(NULL)
	, _animController(NULL)
	, _phonemeMap(NULL)
	, _toolbarWorkspaceChoice(NULL)
	, _toolbarInstanceWorkspaceButton(NULL)
	, _valid(FxFalse)
	, _autoKey(FxFalse)
	, _autoSelectCurve(FxFalse)
	, _mode(WM_Combiner)
{
	FxWorkspaceManager::Instance()->AddWorkspaceContainer(this);
	DoRefreshSystemColours();
	DoResizeRects();
	CreateToolbarControls();
	SetControlStates();
}

FxWorkspaceWidget::~FxWorkspaceWidget()
{
	if( _valid )
	{
		// We don't want to instance the workspace manager again if the app is
		// being shut down.
		FxWorkspaceManager::Instance()->RemoveWorkspaceContainer(this);
	}
}

void FxWorkspaceWidget::SetActiveWorkspace( const FxName& activeWorkspace )
{
	FxInt32 index = _toolbarWorkspaceChoice->FindString(wxString::FromAscii(activeWorkspace.GetAsCstr()));
	if( index != -1 )
	{
		_toolbarWorkspaceChoice->SetSelection(index);
		DoLoadSelectedWorkspace();
	}
}

//------------------------------------------------------------------------------
// Widget messages.
//------------------------------------------------------------------------------
void FxWorkspaceWidget::OnActorChanged(FxWidget* FxUnused(sender), FxActor* actor)
{
	_actor = actor;
	_valid = _actor ? FxTrue : FxFalse;
	DoFillWorkspaceChoice();
	_currentWorkspace.SetActor(_actor);
	SetControlStates();
	Refresh(FALSE);
	Update();
}

void FxWorkspaceWidget::OnActorInternalDataChanged(FxWidget* FxUnused(sender), FxActorDataChangedFlag flags)
{
	if( flags & ADCF_FaceGraphChanged || flags & ADCF_CurvesChanged )
	{
		// Force a relinking of the sliders.
		_currentWorkspace.SetActor(_actor);
		_currentWorkspace.SetAnim(_anim);
		Refresh(FALSE);
		Update();
	}
}

void FxWorkspaceWidget::OnAnimChanged(FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim)
{
	_anim = anim;
	_currentWorkspace.SetAnim(_anim);
	Refresh(FALSE);
	Update();
}

void FxWorkspaceWidget::OnPhonemeMappingChanged(FxWidget* FxUnused(sender), FxPhonemeMap* phonemeMap)
{
	_phonemeMap = phonemeMap;
}

void FxWorkspaceWidget::OnTimeChanged(FxWidget* FxUnused(sender), FxReal newTime)
{
	_currentTime = newTime;
	_currentWorkspace.SetCurrentTime(_currentTime);
	Refresh(FALSE, &_workspaceRect);
	Update();
}

void FxWorkspaceWidget::OnRefresh(FxWidget* FxUnused(sender))
{
	if( _animController )
	{
		_currentWorkspace.UpdateAllSliders();
		Refresh(FALSE, &_workspaceRect);
		Update();
	}
}

void FxWorkspaceWidget::OnAppStartup(FxWidget* FxUnused(sender))
{
	_valid = FxTrue;
}

void FxWorkspaceWidget::OnAppShutdown(FxWidget* FxUnused(sender))
{
	_valid = FxFalse;
}

//------------------------------------------------------------------------------
// Event handlers.
//------------------------------------------------------------------------------
void FxWorkspaceWidget::OnSize(wxSizeEvent& FxUnused(event))
{
	DoResizeRects();
	if( _valid )
	{
		_currentWorkspace.SetWorkspaceRect(_workspaceRect);
	}
}

void FxWorkspaceWidget::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	DrawToolbar(&dc);
	if( _valid && _actor )
	{
		_currentWorkspace.DrawOn(&dc);
	}
	else
	{
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
		dc.DrawRectangle(_workspaceRect);
	}
}

void FxWorkspaceWidget::OnMouse(wxMouseEvent& event)
{
	if( _valid )
	{
		if( event.LeftDown() && !HasCapture() )
		{
			SetFocus();
			CaptureMouse();
		}

		FxInt32 refresh = WRF_None;
		_currentWorkspace.ProcessMouseEvent(event, refresh);
		if( refresh != WRF_None && _autoSelectCurve )
		{
			FxWorkspaceElementSlider* selectedSlider = _currentWorkspace.GetSelectedSlider();
			if( selectedSlider )
			{
				wxString selCurves;
				const FxString& xName = selectedSlider->GetNodeNameXAxis();
				if( xName.Length() )
				{
					selCurves.Append(wxString(xName.GetData(), wxConvLibc));
				}
				const FxString& yName = selectedSlider->GetNodeNameYAxis();
				if( yName.Length() )
				{
					if( xName.Length() )
					{
						selCurves += wxT("|");
					}
					selCurves.Append(wxString(yName.GetData(), wxConvLibc));
				}
				FxArray<FxString> currSelCurves;
				FxSessionProxy::GetSelectedAnimCurves(currSelCurves);
				FxBool goodX = xName.Length() == 0;
				FxBool goodY = yName.Length() == 0;
				for( FxSize i = 0; i < currSelCurves.Length(); ++i )
				{
					goodX |= currSelCurves[i] == xName;
					goodY |= currSelCurves[i] == yName;
				}
				if( !(goodX && goodY) )
				{
					wxString command = wxString::Format(wxT("select -type \"curve\" -names \"%s\""), selCurves);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		if( refresh & WRF_Workspaces )
		{
			FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
		}
		if( refresh & WRF_App )
		{
			DispatchRefreshMessage();
		}

		if( event.LeftUp() && HasCapture() )
		{
			ReleaseMouse();
		}
	}
}

void FxWorkspaceWidget::OnKeyDown(wxKeyEvent& event)
{
	FxInt32 refresh = WRF_None;
	FxBool processed = _currentWorkspace.ProcessKeyDown(event, refresh);

	if( !processed )
	{
		if( _mode == WM_Animation )
		{
			FxInt32 keyCode = event.GetKeyCode();
			if( keyCode == '[' )
			{
				wxCommandEvent prevKeyEvent(wxEVT_COMMAND_BUTTON_CLICKED, ControlID_WorkspaceWidgetToolbarAnimModePrevKeyButton);
				wxPostEvent(this, prevKeyEvent);
				processed = FxTrue;
			}
			else if( keyCode == ']' )
			{
				wxCommandEvent nextKeyEvent(wxEVT_COMMAND_BUTTON_CLICKED, ControlID_WorkspaceWidgetToolbarAnimModeNextKeyButton);
				wxPostEvent(this, nextKeyEvent);
				processed = FxTrue;
			}
		}
	}

	if( !processed &&
		(event.GetKeyCode() != WXK_CONTROL &&
		 event.GetKeyCode() != WXK_SHIFT) )
	{
		wxBell();
	}

	if( refresh & WRF_Workspaces )
	{
		FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
	}
	if( refresh & WRF_App )
	{
		DispatchRefreshMessage();
		Refresh(FALSE);
	}
}

void FxWorkspaceWidget::OnSystemColoursChanged(wxSysColourChangedEvent& FxUnused(event))
{
	DoRefreshSystemColours();
	Refresh(FALSE);
}

void FxWorkspaceWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Workspace Tab"));
}

void FxWorkspaceWidget::OnToolbarWorkspaceChoiceChange(wxCommandEvent& FxUnused(event))
{
	DoLoadSelectedWorkspace();
}

void FxWorkspaceWidget::OnToolbarEditWorkspaceButton(wxCommandEvent& FxUnused(event))
{
	static_cast<FxWidgetSwitcher*>(GetParent())->ToggleChildren(_currentWorkspace.GetName());
}

void FxWorkspaceWidget::OnToolbarModeChoiceChange(wxCommandEvent& FxUnused(event))
{
	_mode = static_cast<FxWorkspaceMode>(_toolbarModeChoice->GetSelection());
	DoCreateModeControls(_mode);

	FxWorkspaceManager::Instance()->ChangeMode(_mode);
}

void FxWorkspaceWidget::OnToolbarDisplayNamesToggle(wxCommandEvent& FxUnused(event))
{
	FxWorkspaceManager::Instance()->SetDisplayNames(!FxWorkspaceManager::Instance()->GetDisplayNames());
	FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
}

void FxWorkspaceWidget::OnToolbarWorkspaceOptionChange(wxCommandEvent& FxUnused(event))
{
	_autoKey = _toolbarAnimModeAutokeyToggle->GetToggleValue();
	_autoSelectCurve = _toolbarAutoselectCurvesToggle->GetToggleValue();
	FxWorkspaceManager::Instance()->ChangeOption(_autoKey, _autoSelectCurve);
}

void FxWorkspaceWidget::OnToolbarAnimModeKeyAllButton(wxCommandEvent& FxUnused(event))
{
	if( _anim )
	{
		// Build a list of curves that need to be inserted.
		FxArray<FxString> curvesToInsert;
		for( FxSize i = 0; i < _currentWorkspace.GetNumSliders(); ++i )
		{
			FxWorkspaceElementSlider* currSlider = _currentWorkspace.GetSlider(i);
			FxString curveNameX = currSlider->GetNodeNameXAxis();
			FxString curveNameY = currSlider->GetNodeNameYAxis();
			// Check if the curve does not exist in the animation.
			if( !curveNameX.IsEmpty() && _anim->FindAnimCurve(curveNameX.GetData()) == FxInvalidIndex )
			{
				// Make sure we haven't already flagged the curve for insertion.
				FxBool unique = FxTrue;
				for( FxSize j = 0; j < curvesToInsert.Length(); ++j )
				{
					if( curvesToInsert[j] == curveNameX )
					{
						unique = FxFalse;
					}
				}
				if( unique )
				{
					curvesToInsert.PushBack(curveNameX);
				}
			}
			// Repeat the process for the slider's y axis.
			if( !curveNameY.IsEmpty() && _anim->FindAnimCurve(curveNameY.GetData()) == FxInvalidIndex )
			{
				FxBool unique = FxTrue;
				for( FxSize j = 0; j < curvesToInsert.Length(); ++j )
				{
					if( curvesToInsert[j] == curveNameY )
					{
						unique = FxFalse;
					}
				}
				if( unique )
				{
					curvesToInsert.PushBack(curveNameY);
				}
			}
		}

		// Build the list of curve insertion commands.
		if( curvesToInsert.Length() > 0 )
		{
			FxString selAnimGroup;
			FxSessionProxy::GetSelectedAnimGroup(selAnimGroup);
			FxVM::ExecuteCommand("batch");
			for( FxSize i = 0; i < curvesToInsert.Length(); ++i )
			{
				FxVM::ExecuteCommand(FxString(wxString::Format(wxT("curve -group \"%s\" -anim \"%s\" -add -name \"%s\" -owner \"user\""),
					wxString(selAnimGroup.GetData(), wxConvLibc), wxString(_anim->GetNameAsCstr(), wxConvLibc),
					wxString(curvesToInsert[i].GetData(), wxConvLibc) ).mb_str(wxConvLibc)));
			}
			FxVM::ExecuteCommand("execBatch -addedcurves");
		}

		// Ensure the sliders are linked to the new curves before we proceed.
		_currentWorkspace.SetAnimController(_animController);
		_currentWorkspace.SetActor(_actor);
		_currentWorkspace.SetAnim(_anim);

		FxString selectedAnimGroup;
		FxString selectedAnim;
		FxSessionProxy::GetSelectedAnimGroup(selectedAnimGroup);
		FxSessionProxy::GetSelectedAnim(selectedAnim);

		// Build the list of key insertion commands.
		FxArray<FxString> keyedCurves(_anim->GetNumAnimCurves());
		FxVM::ExecuteCommand("batch");
		for( FxSize i = 0; i < _currentWorkspace.GetNumSliders(); ++i )
		{
			FxWorkspaceElementSlider* currSlider = _currentWorkspace.GetSlider(i);
			if( !currSlider->GetNodeNameXAxis().IsEmpty() )
			{
				FxBool alreadyKeyed = FxFalse;
				for( FxSize i = 0; i < keyedCurves.Length(); ++i )
				{
					if( currSlider->GetNodeNameXAxis() == keyedCurves[i] )
					{
						alreadyKeyed = FxTrue;
					}
				}

				if( !alreadyKeyed )
				{
					// Check that the curve isn't owned by analysis.
					FxBool ownedByAnalysis = FxFalse;
					FxSessionProxy::GetCurveOwner(selectedAnimGroup, selectedAnim, currSlider->GetNodeNameXAxis(), ownedByAnalysis);

					if( ownedByAnalysis )
					{
						FxWarn("The curve %s was not keyed.  It is owned by analysis.", currSlider->GetNodeNameXAxis().GetData());
					}
					else
					{
						FxString xCommand = currSlider->BuildKeyXCommand();
						if( !xCommand.IsEmpty() )
						{
							FxVM::ExecuteCommand(xCommand);
							keyedCurves.PushBack(currSlider->GetNodeNameXAxis());
						}
					}
				}
			}
			
			if( !currSlider->GetNodeNameYAxis().IsEmpty() )
			{
				FxBool alreadyKeyed = FxFalse;
				for( FxSize i = 0; i < keyedCurves.Length(); ++i )
				{
					if( currSlider->GetNodeNameYAxis() == keyedCurves[i] )
					{
						alreadyKeyed = FxTrue;
					}
				}

				if( !alreadyKeyed )
				{
					// Check that the curve isn't owned by analysis.
					FxBool ownedByAnalysis = FxFalse;
					FxSessionProxy::GetCurveOwner(selectedAnimGroup, selectedAnim, currSlider->GetNodeNameYAxis(), ownedByAnalysis);

					if( ownedByAnalysis )
					{
						FxWarn("The curve %s was not keyed.  It is owned by analysis.", currSlider->GetNodeNameYAxis().GetData());
					}
					else
					{
						FxString yCommand = currSlider->BuildKeyYCommand();
						if( !yCommand.IsEmpty() )
						{
							FxVM::ExecuteCommand(yCommand);
							keyedCurves.PushBack(currSlider->GetNodeNameYAxis());
						}
					}
				}
			}
		}
		FxVM::ExecuteCommand("execBatch -editedcurves");

		FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
		DispatchRefreshMessage();
	}
}

void FxWorkspaceWidget::OnToolbarAnimModePrevKeyButton(wxCommandEvent& FxUnused(event))
{
	if( _anim )
	{
		FxReal prevKeyTime = static_cast<FxReal>(FX_INT32_MIN);

		for( FxSize i = 0; i < _anim->GetNumAnimCurves(); ++i )
		{
			const FxAnimCurve& curve = _anim->GetAnimCurve(i);
			// Check that the curve is in a slider in the workspace.
			FxBool useCurve = FxFalse;
			if( _currentWorkspace.GetSelectedSlider() )
			{
				// If there is a selected slider, only use it.
				if( curve.GetName().GetAsString() == _currentWorkspace.GetSelectedSlider()->GetNodeNameXAxis() ||
					curve.GetName().GetAsString() == _currentWorkspace.GetSelectedSlider()->GetNodeNameYAxis() )
				{
					useCurve = FxTrue;
				}
			}
			else
			{

				for( FxSize k = 0; k < _currentWorkspace.GetNumSliders(); ++k )
				{
					if( curve.GetName().GetAsString() == _currentWorkspace.GetSlider(k)->GetNodeNameXAxis() ||
						curve.GetName().GetAsString() == _currentWorkspace.GetSlider(k)->GetNodeNameYAxis() )
					{
						useCurve = FxTrue;
					}
				}
			}

			if( useCurve )
			{
				// Search the curve to find any keys that might be the previous key.
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					FxReal keyTime = curve.GetKey(j).GetTime();
					if( keyTime < _currentTime && keyTime > prevKeyTime )
					{
						prevKeyTime = keyTime;
					}
				}
			}
		}

		if( prevKeyTime != static_cast<FxReal>(FX_INT32_MIN) )
		{
			FxSessionProxy::SetSessionTime(prevKeyTime);
		}
	}
}

void FxWorkspaceWidget::OnToolbarAnimModeNextKeyButton(wxCommandEvent& FxUnused(event))
{
	if( _anim )
	{
		FxReal nextKeyTime = FX_REAL_MAX;

		for( FxSize i = 0; i < _anim->GetNumAnimCurves(); ++i )
		{
			const FxAnimCurve& curve = _anim->GetAnimCurve(i);
			// Check that the curve is in a slider in the workspace.
			FxBool useCurve = FxFalse;
			if( _currentWorkspace.GetSelectedSlider() )
			{
				// If there is a selected slider, only use it.
				if( curve.GetName().GetAsString() == _currentWorkspace.GetSelectedSlider()->GetNodeNameXAxis() ||
					curve.GetName().GetAsString() == _currentWorkspace.GetSelectedSlider()->GetNodeNameYAxis() )
				{
					useCurve = FxTrue;
				}
			}
			else
			{

				for( FxSize k = 0; k < _currentWorkspace.GetNumSliders(); ++k )
				{
					if( curve.GetName().GetAsString() == _currentWorkspace.GetSlider(k)->GetNodeNameXAxis() ||
						curve.GetName().GetAsString() == _currentWorkspace.GetSlider(k)->GetNodeNameYAxis() )
					{
						useCurve = FxTrue;
					}
				}
			}

			if( useCurve )
			{
				// Search the curve to find any keys that might be the previous key.
				for( FxSize j = 0; j < curve.GetNumKeys(); ++j )
				{
					FxReal keyTime = curve.GetKey(j).GetTime();
					if( keyTime > _currentTime && keyTime < nextKeyTime )
					{
						nextKeyTime = keyTime;
					}
				}
			}
		}

		if( nextKeyTime != FX_REAL_MAX )
		{
			FxSessionProxy::SetSessionTime(nextKeyTime);
		}
	}
}

void FxWorkspaceWidget::OnToolbarSharedModeClearButton(wxCommandEvent& FxUnused(event))
{
	FxSessionProxy::DisableAllRegisters();
	FxSessionProxy::UpdateActorInstance();
	
	// If mapping, remove entry from the phoneme map.
	if( _phonemeMap && _currentWorkspace.GetMode() == WM_Mapping )
	{
		// Remove all mapping entries associated with the selected phoneme.
		FxSize selPhone = _toolbarMappingModePhonemeChoice->GetSelection();
		wxString label = FxGetPhonemeInfo(selPhone).talkback;
		if( FxStudioOptions::GetPhoneticAlphabet() != PA_DEFAULT )
		{
			// IPA (UNICODE) cannot be represented in the ANSI command system.
			// Use SAMPA for either IPA or SAMPA in the command system.
			label = FxGetPhonemeInfo(selPhone).sampa;
		}

		wxString command = wxString::Format(wxT("map -remove -phoneme \"%s\""), label );
		FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );
	}

	// Update all sliders.
	FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
	DispatchRefreshMessage();
}

void FxWorkspaceWidget::OnToolbarSharedModeLoadCurrentFrameButton(wxCommandEvent& FxUnused(event))
{
	// Transfer curve values to user values in face graph.
	if( _currentWorkspace.GetMode() != WM_Animation )
	{
		if( _actor && _animController )
		{
			// Refresh the current animation, and clear any user values in the
			// face graph.
			_animController->Evaluate(_currentTime);

			// This is slightly tricky.  What we want is for the current frame
			// of the current animation to be used to fill out the slider panel.
			// We can't just look at the current values of all the nodes in the
			// face graph, because some values may be virtual and we'd end up
			// driving some nodes more than they should be.  So what we do is
			// go through all the curves in the current animation and look at
			// the current (final) value of the node the curve controls.  We
			// then take that value and set it as the node's user value (being
			// careful to use the replace operation).  If the curve is virtual,
			// we actually evaluate the curve (to get the offset curve value)
			// and use that (if it is non-zero) rather than the virtual curve 
			// value.  Then we clear all values from the face graph except the 
			// user values, and issue various refresh messages to update the 
			// slider panel.

			FxCompiledFaceGraph* pCG = NULL;
			FxSessionProxy::GetCompiledFaceGraph(&pCG);
			FxAssert(pCG);
			FxSize numCurves = _animController->GetNumCurves();
			for( FxSize i = 0; i < numCurves; ++i )
			{
				FxAnimCurve* pAnimCurve = _animController->GetCurve(i);
				if( pAnimCurve )
				{
					const FxName& curveName = pAnimCurve->GetName();
					FxSize nodeIndex = pCG->FindNodeIndex(curveName);
                    if( FxInvalidIndex != nodeIndex )
					{
						if( _animController->IsCurveVirtual(i) )
						{
							FxReal offsetValue = pAnimCurve->EvaluateAt(_animController->GetTime());
							if( !FxRealEquality(offsetValue, 0.0f) )
							{
								FxSessionProxy::SetRegister(curveName, offsetValue, RO_Replace);
							}
						}
						else
						{
							FxReal currentValue = 0.0f;
							FxSessionProxy::GetRegister(curveName, currentValue);
							FxSessionProxy::SetRegister(curveName, currentValue, RO_Replace);
						}
					}
				}
			}
			FxSessionProxy::UpdateActorInstance();
		}
	}

	// Update all sliders.
	FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
	DispatchRefreshMessage();
}

void FxWorkspaceWidget::OnToolbarCombinerModeCreateButton(wxCommandEvent& FxUnused(event))
{
	if( _actor )
	{
		wxString nodeName;
		wxTextEntryDialog textDialog(this, _("Enter new node's name:"), _("Name New Node") );
		FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
		int retval = textDialog.ShowModal();
		FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
		nodeName = textDialog.GetValue();
		if( nodeName != wxEmptyString && retval == wxID_OK )
		{
			FxFaceGraph* pFaceGraph = NULL;
			FxSessionProxy::GetFaceGraph(pFaceGraph);

			// Make sure the requested name doesn't already exist in the graph.
			if( pFaceGraph && pFaceGraph->FindNode(FxString(nodeName.mb_str(wxConvLibc)).GetData()) != NULL )
			{
				wxString msg = wxString::Format(wxT("A node named %s already exists."), nodeName);
				FxMessageBox(msg, _("Invalid Node Name"), wxOK|wxCENTRE|wxICON_EXCLAMATION, this);
			}
			else
			{
				// Create the node.
				wxString command = wxString::Format(wxT("graph -addnode -nodetype \"FxCombinerNode\" -name \"%s\""), nodeName);
				FxVM::ExecuteCommand("batch");
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				FxCompiledFaceGraph* pCG = NULL;
				FxSessionProxy::GetCompiledFaceGraph(&pCG);
				FxAssert(pCG);
				FxSize numNodes = pCG->nodes.Length();
				for( FxSize i = 0; i < numNodes; ++i )
				{
					FxReal originalRegisterValue = 0.0f;
					FxSessionProxy::GetOriginalRegisterValue(pCG->nodes[i].name, originalRegisterValue);
					if( !FxRealEquality(originalRegisterValue, 0.0f) )
					{
						command = wxString::Format(wxT("graph -link -from \"%s\" -to \"%s\" -linkfn \"linear\" -linkfnparams \"m=%f|b=0.0f\""),
							nodeName, wxString::FromAscii(pCG->nodes[i].name.GetAsCstr()), originalRegisterValue);
						FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
					}
				}
				FxVM::ExecuteCommand("execBatch -editednodes");

				// Display the creation dialog so the user can rename the node
				// or adjust the link functions.
				FxName nodeNameAsFxName(nodeName.mb_str(wxConvLibc));
				FxFaceGraphNode* pNode = _actor->GetDecompiledFaceGraph().FindNode(nodeNameAsFxName);
				FxCreateFaceGraphNodeDialog* createNodeDialog = new FxCreateFaceGraphNodeDialog();
				createNodeDialog->Initialize(_actor, pNode, _mediator);
				createNodeDialog->Create(this);
				FxTearoffWindow::CacheAndSetAllTearoffWindowsToNotAlwaysOnTop();
				createNodeDialog->ShowModal();
				FxTearoffWindow::RestoreAllTearoffWindowsAlwaysOnTop();
			}
		}
		else if( retval == wxID_OK && nodeName == wxEmptyString )
		{
			FxMessageBox(_("Please enter a name for the new node."), _("Invalid Node Name"), wxOK|wxCENTRE|wxICON_EXCLAMATION, this);
		}
	}
}

void FxWorkspaceWidget::OnToolbarMappingModeApplyButton(wxCommandEvent& FxUnused(event))
{
	if( _actor && _phonemeMap )
	{
		FxVM::ExecuteCommand("batch");

		// First remove all mapping entries associated with the selected phoneme.
		// Remove all mapping entries associated with the selected phoneme.
		FxSize selPhone = _toolbarMappingModePhonemeChoice->GetSelection();
		wxString label = FxGetPhonemeInfo(selPhone).talkback;
		if( FxStudioOptions::GetPhoneticAlphabet() != PA_DEFAULT )
		{
			// IPA (UNICODE) cannot be represented in the ANSI command system.
			// Use SAMPA for either IPA or SAMPA in the command system.
			label = FxGetPhonemeInfo(selPhone).sampa;
		}
		wxString command = wxString::Format(wxT("map -remove -phoneme \"%s\""), label);
		FxVM::ExecuteCommand( FxString(command.mb_str(wxConvLibc)) );

		// Next create new mapping entries for the selected phoneme by looking
		// at nodes that have non-zero user values.
		FxCompiledFaceGraph* pCG = NULL;
		FxSessionProxy::GetCompiledFaceGraph(&pCG);
		FxAssert(pCG);
		FxSize numNodes = pCG->nodes.Length();
		for( FxSize i = 0; i < numNodes; ++i )
		{
			FxReal originalRegisterValue = 0.0f;
			FxSessionProxy::GetOriginalRegisterValue(pCG->nodes[i].name, originalRegisterValue);
			if( !FxRealEquality(originalRegisterValue, 0.0f) )
			{
				wxString command = wxString::Format(wxT("map -set -phoneme \"%s\" -track \"%s\" -value %f"),
					label, wxString::FromAscii(pCG->nodes[i].name.GetAsCstr()), originalRegisterValue);
				FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
			}
		}
		FxVM::ExecuteCommand("execBatch -changedmapping");
	}
}

void FxWorkspaceWidget::OnToolbarMappingModePhonemeChoiceChange(wxCommandEvent& FxUnused(event))
{
	// Get the selected phoneme.
	const FxPhonExtendedInfo& phonInfo = FxGetPhonemeInfo(_toolbarMappingModePhonemeChoice->GetSelection());
	_toolbarMappingModeSampleWord->SetLabel(wxString(_(" Sample word: ")) + phonInfo.sampleWords);
	_toolbarMappingModeSampleWord->SetBackgroundColour(_colourFace);

	// Set face graph node user values to current mapping target.
	DoLoadCurrentPhonemeMappingToSliders();

	// Force update of sliders.
	DispatchRefreshMessage();
	FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
}

void FxWorkspaceWidget::OnToolbarInstanceWorkspaceButton(wxCommandEvent& FxUnused(event))
{
	wxMiniFrame* miniFrame = new wxMiniFrame();
	miniFrame->Create(FxStudioApp::GetMainWindow(), wxID_ANY, 
		wxString::FromAscii(_currentWorkspace.GetNameAsCstr()), 
		wxDefaultPosition, wxSize(320,240), 
		wxCAPTION|wxCLIP_CHILDREN|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX);
	miniFrame->SetSizer(new wxBoxSizer(wxVERTICAL));
	FxWorkspaceInstanceWidget* wsInstance = new FxWorkspaceInstanceWidget(miniFrame, _mediator);
	miniFrame->GetSizer()->Add(wsInstance, 1, wxEXPAND, 0);
	wsInstance->SetAnimController(_animController);
	wsInstance->OnAppStartup(this);
	wsInstance->OnActorChanged(NULL, _actor);
	wsInstance->OnAnimChanged(NULL, FxName::NullName, _anim);
	wsInstance->OnTimeChanged(NULL, _currentTime);
	wsInstance->SetWorkspace(&_currentWorkspace);
	miniFrame->Show();
	miniFrame->Layout();
}

//------------------------------------------------------------------------------
// Public methods.
//------------------------------------------------------------------------------

void FxWorkspaceWidget::SetWorkspace(FxWorkspace* workspace)
{
	if( workspace )
	{
		_currentWorkspace = *workspace;
	}
	else
	{
		_currentWorkspace = FxWorkspace();
	}
	InitWorkspace();
	Refresh(FALSE);
	Update();
}

void FxWorkspaceWidget::SetAnimController(FxAnimController* animController)
{
	_animController = animController;
	_currentWorkspace.SetAnimController(_animController);
}

//------------------------------------------------------------------------------
// Workspace container messages.
//------------------------------------------------------------------------------
void FxWorkspaceWidget::OnWorkspaceAdded(const FxName& FxUnused(newWorkspaceName))
{
	DoFillWorkspaceChoice();
}

void FxWorkspaceWidget::OnWorkspacePreRemove(const FxName& workspaceToRemove)
{
	if( workspaceToRemove == _currentWorkspace.GetName() )
	{
		_currentWorkspace = FxWorkspace();
	}
}

void FxWorkspaceWidget::OnWorkspacePostRemove(const FxName& FxUnused(removedWorkspace))
{
	DoFillWorkspaceChoice();
	InitWorkspace();
	Refresh(FALSE);
}

void FxWorkspaceWidget::OnWorkspaceRenamed(const FxName& oldName, const FxName newName)
{
	if( oldName == _currentWorkspace.GetName() )
	{
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(newName);
		InitWorkspace();
		Refresh(FALSE);
	}
	DoFillWorkspaceChoice();
}

void FxWorkspaceWidget::OnWorkspaceModified(const FxName& modifiedWorkspace)
{
	if( modifiedWorkspace == _currentWorkspace.GetName() )
	{
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(modifiedWorkspace);
		InitWorkspace();
		Refresh(FALSE);
	}
}

void FxWorkspaceWidget::OnWorkspaceNeedsUpdate()
{
	_currentWorkspace.UpdateAllSliders();
	Refresh(FALSE);
	Update();
}

void FxWorkspaceWidget::OnWorkspaceModeSwitch(FxWorkspaceMode newMode)
{
	_mode = newMode;
	_currentWorkspace.SetMode(_mode);
	Refresh(FALSE);
}

void FxWorkspaceWidget::OnWorkspaceOptionChanged(FxBool autoKey, FxBool autoSelectCurve)
{
	_autoKey = autoKey;
	_autoSelectCurve = autoSelectCurve;
	_currentWorkspace.SetOption(_autoKey, _autoSelectCurve);
}

void FxWorkspaceWidget::OnWorkspaceClose()
{
	//@todo stop drawing, etc.
}

//------------------------------------------------------------------------------
// Member functions.
//------------------------------------------------------------------------------

void FxWorkspaceWidget::DoResizeRects()
{
	wxRect clientRect(GetClientRect());
	_toolbarRect = wxRect(clientRect.GetLeft(), clientRect.GetTop(),
		clientRect.GetWidth(), FxToolbarHeight);
	_workspaceRect = wxRect(clientRect.GetLeft(), clientRect.GetTop() + FxToolbarHeight,
		clientRect.GetWidth(), clientRect.GetHeight() - FxToolbarHeight);

	MoveControls();
	Refresh(FALSE);
}

void FxWorkspaceWidget::DoRefreshSystemColours()
{
	_colourFace = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	_colourShadow = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
	_colourEngrave = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
}

void FxWorkspaceWidget::DoFillWorkspaceChoice()
{
	if( _toolbarWorkspaceChoice )
	{
		FxInt32 currentSel = _toolbarWorkspaceChoice->GetSelection();

		_toolbarWorkspaceChoice->Freeze();
		_toolbarWorkspaceChoice->Clear();
		if( _actor )
		{
			for( FxSize i = 0; i < FxWorkspaceManager::Instance()->GetNumWorkspaces(); ++i )
			{
				_toolbarWorkspaceChoice->Append(
					wxString::FromAscii(FxWorkspaceManager::Instance()->GetWorkspace(i).GetNameAsCstr()));
			}
		}
		_toolbarWorkspaceChoice->Thaw();

		// Set the current selection.
		currentSel = currentSel > static_cast<FxInt32>(_toolbarWorkspaceChoice->GetCount()) - 1 ? 0 : currentSel;
		_toolbarWorkspaceChoice->SetSelection(currentSel == -1 ? 0 : currentSel);

		DoLoadSelectedWorkspace();
	}
}

void FxWorkspaceWidget::DoLoadSelectedWorkspace()
{
	if( _actor && _toolbarWorkspaceChoice )
	{
		wxString workspace = _toolbarWorkspaceChoice->GetString(_toolbarWorkspaceChoice->GetSelection());
		FxName workspaceName(workspace.mb_str(wxConvLibc));
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(workspaceName);
	}
	else
	{
		_currentWorkspace = FxWorkspace();
	}


	InitWorkspace();
	Refresh(FALSE);
	SetControlStates();
}

void FxWorkspaceWidget::DoCreateModeControls(FxWorkspaceMode mode)
{
	// Hide the current mode-based controls
	for(FxSize i = 0; i < _modeControls.Length(); ++i )
	{
		_modeControls[i]->Hide();
	}
	_modeControls.Clear();

	FxInt32 xPos = _toolbarDisplayNamesToggle->GetRect().GetRight() + 8;
	FxInt32 yPos = _toolbarRect.GetTop() + 2;
	if( mode == WM_Animation )
	{
		_toolbarAnimModeAutokeyToggle->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarAnimModeAutokeyToggle->Show();
		_modeControls.PushBack(_toolbarAnimModeAutokeyToggle);
		xPos += 23;
		_toolbarAnimModeKeyAllButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarAnimModeKeyAllButton->Show();
		_modeControls.PushBack(_toolbarAnimModeKeyAllButton);
		xPos += 23;
		_toolbarAnimModePrevKeyButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarAnimModePrevKeyButton->Show();
		_modeControls.PushBack(_toolbarAnimModePrevKeyButton);
		xPos += 23;
		_toolbarAnimModeNextKeyButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarAnimModeNextKeyButton->Show();
		_modeControls.PushBack(_toolbarAnimModeNextKeyButton);
		xPos += 23;
	}

	if( mode != WM_Animation )
	{
		// Shared combiner/mapping controls.
		_toolbarSharedModeClearButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarSharedModeClearButton->Show();
		_modeControls.PushBack(_toolbarSharedModeClearButton);
		xPos += 23;
		_toolbarSharedModeLoadCurrentFrameButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarSharedModeLoadCurrentFrameButton->Show();
		_modeControls.PushBack(_toolbarSharedModeLoadCurrentFrameButton);
		xPos += 23;
	}

	if( mode == WM_Combiner )
	{
		_toolbarCombinerModeCreateButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarCombinerModeCreateButton->Show();
		_modeControls.PushBack(_toolbarCombinerModeCreateButton);
		xPos += 23;
	}
	else if( mode == WM_Mapping )
	{
		_toolbarMappingModeApplyButton->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(20,20)));
		_toolbarMappingModeApplyButton->Show();
		_modeControls.PushBack(_toolbarMappingModeApplyButton);
		xPos += 23;
		_toolbarMappingModePhonemeChoice->SetSize(wxRect(wxPoint(xPos, yPos), wxSize(70,20)));
		_toolbarMappingModePhonemeChoice->Show();
		_modeControls.PushBack(_toolbarMappingModePhonemeChoice);
		xPos += 73;
		_toolbarMappingModeSampleWord->SetSize(wxRect(wxPoint(xPos,yPos + 3), wxSize(150,20)));
		_toolbarMappingModeSampleWord->Show();
		_modeControls.PushBack(_toolbarMappingModeSampleWord);
		xPos += 48;

		DoFillPhonemeChoice();
		_toolbarMappingModePhonemeChoice->SetFont(wxFont(8.0f, wxDEFAULT, wxNORMAL, wxNORMAL, FALSE, FxStudioOptions::GetUnicodeFontName()));
	}
}

void FxWorkspaceWidget::DoFillPhonemeChoice()
{
	if( _toolbarMappingModePhonemeChoice )
	{
		_toolbarMappingModePhonemeChoice->Freeze();
		FxInt32 selection = _toolbarMappingModePhonemeChoice->GetSelection();
		_alphabet = FxStudioOptions::GetPhoneticAlphabet();
		_toolbarMappingModePhonemeChoice->Clear();
		for( FxSize i = PHON_FIRST; i <= PHON_LAST; ++i )
		{
			const FxPhonExtendedInfo& phonInfo = FxGetPhonemeInfo(i);
			switch( _alphabet )
			{
			default:
			case PA_DEFAULT:
				_toolbarMappingModePhonemeChoice->Append(phonInfo.talkback);
				break;
			case PA_IPA:
				_toolbarMappingModePhonemeChoice->Append(phonInfo.ipa);
				break;
			case PA_SAMPA:
				_toolbarMappingModePhonemeChoice->Append(phonInfo.sampa);
				break;
			}
		}
		selection = (selection==-1) ? 0 : selection;
		_toolbarMappingModePhonemeChoice->SetSelection(selection);
		_toolbarMappingModePhonemeChoice->Thaw();

		// Force an update of the sample word.
		wxCommandEvent event;
		OnToolbarMappingModePhonemeChoiceChange(event);
	}
}

void FxWorkspaceWidget::DoLoadCurrentPhonemeMappingToSliders()
{
	if( _actor && _phonemeMap && _toolbarMappingModePhonemeChoice )
	{
		FxSessionProxy::DisableAllRegisters();
		FxSize phonemeIndex = _toolbarMappingModePhonemeChoice->GetSelection();
		FxSize numTargets = _phonemeMap->GetNumTargets();
		for( FxSize j = 0; j < numTargets; ++j )
		{
			const FxName& targetName = _phonemeMap->GetTargetName(j);
			FxSessionProxy::SetRegister(targetName, _phonemeMap->GetMappingAmount(FxGetPhonemeInfo(phonemeIndex).phoneme, j), RO_Replace);
		}
	}
}

void FxWorkspaceWidget::DrawToolbar(wxDC* pDC)
{
	// Draw the toolbar background.
	pDC->SetBrush(wxBrush(_colourFace));
	pDC->SetPen(wxPen(_colourFace));
	pDC->DrawRectangle(_toolbarRect);

	// Shade the edges.
	pDC->SetPen(wxPen(_colourShadow));
	pDC->DrawLine(_toolbarRect.GetLeft(), _toolbarRect.GetBottom(),
		_toolbarRect.GetRight(), _toolbarRect.GetBottom());

	// Draw some dividing lines.
	pDC->SetPen(wxPen(_colourEngrave));
	FxInt32 line1x = _toolbarEditWorkspaceButton->GetRect().GetRight() + 3;
	FxInt32 line2x = _toolbarDisplayNamesToggle->GetRect().GetRight() + 4;
	pDC->DrawLine(line1x, _toolbarRect.GetTop() + 1, line1x, _toolbarRect.GetBottom() - 1);
	pDC->DrawLine(line2x, _toolbarRect.GetTop() + 1, line2x, _toolbarRect.GetBottom() - 1);

	// Clean up.
	pDC->SetBrush(wxNullBrush);
	pDC->SetPen(wxNullPen);
}

void FxWorkspaceWidget::CreateToolbarControls()
{
	FxInt32 xPos = _toolbarRect.GetLeft() + 5;
	FxInt32 yPos = _toolbarRect.GetTop() + 2;
	_toolbarWorkspaceChoice = new wxChoice(this, ControlID_WorkspaceWidgetToolbarWorkspaceChoice, wxPoint(xPos, yPos), wxSize(100,20));
	_toolbarWorkspaceChoice->SetToolTip(_("Select a workspace."));
	xPos += 103;
	_toolbarEditWorkspaceButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarEditWorkspaceButton, _("E"), wxPoint(xPos, yPos), wxSize(20,20));
	_toolbarEditWorkspaceButton->MakeToolbar();
	_toolbarEditWorkspaceButton->SetToolTip(_("Enter workspace edit mode."));
	xPos += 27;
	wxString choices[] = {_("Create Combiner"), _("Animate"), _("Visual Mapping")};
	_toolbarModeChoice = new wxChoice(this, ControlID_WorkspaceWidgetToolbarModeChoice, wxPoint(xPos, yPos), wxSize(120,20), 3, choices);
	_toolbarModeChoice->SetToolTip(_("Select a workspace mode."));
	xPos += 125;
	_toolbarModeChoice->SetSelection(static_cast<FxInt32>(_mode));

	_toolbarAutoselectCurvesToggle = new FxButton(this, ControlID_WorkspaceWidgetToolbarAutoselectCurvesToggle, _("SC"), wxPoint(xPos, yPos), wxSize(20,20));
	_toolbarAutoselectCurvesToggle->MakeToolbar();
	_toolbarAutoselectCurvesToggle->MakeToggle();
	_toolbarAutoselectCurvesToggle->SetValue(FxFalse);
	_toolbarAutoselectCurvesToggle->SetToolTip(_("Automatically select curves for sliders"));
	xPos += 27;
	
	_toolbarDisplayNamesToggle = new FxButton(this, ControlID_WorkspaceWidgetToolbarDisplayNamesToggle, _("DN"), wxPoint(xPos, yPos), wxSize(20,20));
	_toolbarDisplayNamesToggle->MakeToolbar();
	_toolbarDisplayNamesToggle->MakeToggle();
	_toolbarDisplayNamesToggle->SetValue(FxFalse);
	_toolbarDisplayNamesToggle->SetToolTip(_("Always display slider names"));
	xPos += 27;

	// Create the mode-dependant controls
	_toolbarAnimModeAutokeyToggle = new FxButton(this, ControlID_WorkspaceWidgetToolbarAnimModeAutokeyToggle, _("AK"), wxDefaultPosition, wxSize(20,20));
	_toolbarAnimModeAutokeyToggle->MakeToolbar();
	_toolbarAnimModeAutokeyToggle->MakeToggle();
	_toolbarAnimModeAutokeyToggle->SetValue(_autoKey);
	_toolbarAnimModeAutokeyToggle->SetToolTip(_("Auto-key"));
	_toolbarAnimModeAutokeyToggle->Hide();

	_toolbarAnimModeKeyAllButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarAnimModeKeyAllButton, _("KA"), wxDefaultPosition, wxSize(20,20));
	_toolbarAnimModeKeyAllButton->MakeToolbar();
	_toolbarAnimModeKeyAllButton->SetToolTip(_("Key all sliders"));
	_toolbarAnimModeKeyAllButton->Hide();

	_toolbarAnimModePrevKeyButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarAnimModePrevKeyButton, _("PK"), wxDefaultPosition, wxSize(20,20));
	_toolbarAnimModePrevKeyButton->MakeToolbar();
	_toolbarAnimModePrevKeyButton->SetToolTip(_("Previous key"));
	_toolbarAnimModePrevKeyButton->Hide();

	_toolbarAnimModeNextKeyButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarAnimModeNextKeyButton, _("NK"), wxDefaultPosition, wxSize(20,20));
	_toolbarAnimModeNextKeyButton->MakeToolbar();
	_toolbarAnimModeNextKeyButton->SetToolTip(_("Next key"));
	_toolbarAnimModeNextKeyButton->Hide();

	_toolbarMappingModePhonemeChoice = new wxChoice(this, ControlID_WorkspaceWidgetToolbarMappingModePhonemeChoice, wxDefaultPosition, wxSize(70,20));
	_toolbarMappingModePhonemeChoice->SetToolTip(_("Select phoneme to map"));
	_toolbarMappingModePhonemeChoice->Hide();

	_toolbarMappingModeApplyButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarMappingModeApplyButton, _("Ap"), wxDefaultPosition, wxSize(20,20));
	_toolbarMappingModeApplyButton->MakeToolbar();
	_toolbarMappingModeApplyButton->SetToolTip(_("Apply to mapping."));
	_toolbarMappingModeApplyButton->Hide();

	_toolbarCombinerModeCreateButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarCombinerModeCreateButton, _("Cr"), wxDefaultPosition, wxSize(20,20));
	_toolbarCombinerModeCreateButton->MakeToolbar();
	_toolbarCombinerModeCreateButton->SetToolTip(_("Create combiner node from sliders."));
	_toolbarCombinerModeCreateButton->Hide();

	_toolbarSharedModeLoadCurrentFrameButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarSharedModeLoadCurrentFrameButton, _("LC"), wxDefaultPosition, wxSize(20,20));
	_toolbarSharedModeLoadCurrentFrameButton->MakeToolbar();
	_toolbarSharedModeLoadCurrentFrameButton->SetToolTip(_("Load the current frame into the sliders."));
	_toolbarSharedModeLoadCurrentFrameButton->Hide();

	_toolbarSharedModeClearButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarSharedModeClearButton, _("Cl"), wxDefaultPosition, wxSize(20,20));
	_toolbarSharedModeClearButton->MakeToolbar();
	_toolbarSharedModeClearButton->SetToolTip(_("Reset all sliders to their default values."));
	_toolbarSharedModeClearButton->Hide();

	_toolbarMappingModeSampleWord = new wxStaticText(this, wxID_ANY, _("Sample word"), wxDefaultPosition, wxSize(100, 20));
	_toolbarMappingModeSampleWord->SetToolTip(_("Sample word"));
	_toolbarMappingModeSampleWord->SetBackgroundColour(_colourFace);
	_toolbarMappingModeSampleWord->Hide();

	_toolbarInstanceWorkspaceButton = new FxButton(this, ControlID_WorkspaceWidgetToolbarInstanceWorkspaceButton, _(">>"), wxPoint(_toolbarRect.GetRight() - 25, _toolbarRect.GetTop() + 2), wxSize(20,20));
	_toolbarInstanceWorkspaceButton->MakeToolbar();
	_toolbarInstanceWorkspaceButton->SetToolTip(_("Peel off current workspace"));

	// Force the mode controls to be created.
	wxCommandEvent event;
	OnToolbarModeChoiceChange(event);

	LoadIcons();
	MoveControls();
	DoFillPhonemeChoice();

}

void FxWorkspaceWidget::SetControlStates()
{
	FxBool controlState = _actor != NULL;
	if( _toolbarWorkspaceChoice &&
		_toolbarInstanceWorkspaceButton )
	{
		_toolbarWorkspaceChoice->Enable(controlState);
		_toolbarEditWorkspaceButton->Enable(controlState);
		_toolbarModeChoice->Enable(controlState);
		_toolbarAutoselectCurvesToggle->Enable(controlState);
		_toolbarDisplayNamesToggle->Enable(controlState);
		_toolbarInstanceWorkspaceButton->Enable(controlState);
		for( FxSize i = 0; i < _modeControls.Length(); ++i )
		{
			_modeControls[i]->Enable(controlState);
		}
	}
}

void FxWorkspaceWidget::MoveControls()
{
	if( _toolbarInstanceWorkspaceButton )
	{
		_toolbarInstanceWorkspaceButton->SetSize(wxRect(
			wxPoint(_toolbarRect.GetRight() - 25, _toolbarRect.GetTop() + 2), wxSize(20,20)));
	}
}

void FxWorkspaceWidget::InitWorkspace()
{
	_currentWorkspace.SetActor(_actor);
	_currentWorkspace.SetAnim(_anim);
	_currentWorkspace.SetAnimController(_animController);
	_currentWorkspace.SetCurrentTime(_currentTime);
	_currentWorkspace.SetWorkspaceRect(_workspaceRect);
	_currentWorkspace.SetMode(_mode);
	_currentWorkspace.SetOption(_autoKey, _autoSelectCurve);
	_currentWorkspace.UpdateAllSliders();
}

void FxWorkspaceWidget::DispatchRefreshMessage()
{
	if( WM_Animation != _currentWorkspace.GetMode() )
	{
		FxSessionProxy::UpdateActorInstance();
	}

	// Tell the widgets to update.
	if( _mediator )
	{
		_mediator->OnRefresh(this);
	}
}

void FxWorkspaceWidget::LoadIcons()
{

	wxString editWorkspacePath = FxStudioApp::GetIconPath(wxT("editworkspace.ico"));
	wxString editWorkspaceDisabledPath = FxStudioApp::GetIconPath(wxT("editworkspace_disabled.ico"));
	if( FxFileExists(editWorkspacePath) && FxFileExists(editWorkspaceDisabledPath) )
	{
		wxIcon editWorkspaceIcon = wxIcon(wxIconLocation(editWorkspacePath));
		editWorkspaceIcon.SetHeight(16);
		editWorkspaceIcon.SetWidth(16);
		wxIcon editWorkspaceDisabledIcon = wxIcon(wxIconLocation(editWorkspaceDisabledPath));
		editWorkspaceDisabledIcon.SetHeight(16);
		editWorkspaceDisabledIcon.SetWidth(16);
		_toolbarEditWorkspaceButton->SetEnabledBitmap(editWorkspaceIcon);
		_toolbarEditWorkspaceButton->SetDisabledBitmap(editWorkspaceDisabledIcon);
	}

	wxString autoselectCurvePath = FxStudioApp::GetIconPath(wxT("transmit.ico"));
	wxString autoselectCurveDisabledPath = FxStudioApp::GetIconPath(wxT("transmit_disabled.ico"));
	if( FxFileExists(autoselectCurvePath) && FxFileExists(autoselectCurveDisabledPath) )
	{
		wxIcon autoselectCurveIcon = wxIcon(wxIconLocation(autoselectCurvePath));
		autoselectCurveIcon.SetHeight(16);
		autoselectCurveIcon.SetWidth(16);
		wxIcon autoselectCurveDisabledIcon = wxIcon(wxIconLocation(autoselectCurveDisabledPath));
		autoselectCurveDisabledIcon.SetHeight(16);
		autoselectCurveDisabledIcon.SetWidth(16);
		_toolbarAutoselectCurvesToggle->SetEnabledBitmap(autoselectCurveIcon);
		_toolbarAutoselectCurvesToggle->SetDisabledBitmap(autoselectCurveDisabledIcon);
	}

	wxString displayNamesPath = FxStudioApp::GetIconPath(wxT("displaynames.ico"));
	wxString displayNamesDisabledPath = FxStudioApp::GetIconPath(wxT("displaynames_disabled.ico"));
	if( FxFileExists(displayNamesPath) && FxFileExists(displayNamesDisabledPath) )
	{
		wxIcon displayNamesIcon = wxIcon(wxIconLocation(displayNamesPath));
		displayNamesIcon.SetHeight(16);
		displayNamesIcon.SetWidth(16);
		wxIcon displayNamesDisabledIcon = wxIcon(wxIconLocation(displayNamesDisabledPath));
		displayNamesDisabledIcon.SetHeight(16);
		displayNamesDisabledIcon.SetWidth(16);
		_toolbarDisplayNamesToggle->SetEnabledBitmap(displayNamesIcon);
		_toolbarDisplayNamesToggle->SetDisabledBitmap(displayNamesDisabledIcon);
	}

	wxString peeloffPath = FxStudioApp::GetIconPath(wxT("peeloff.ico"));
	wxString peeloffDisabledPath = FxStudioApp::GetIconPath(wxT("peeloff_disabled.ico"));
	if( FxFileExists(peeloffPath) && FxFileExists(editWorkspaceDisabledPath) )
	{
		wxIcon peeloffIcon = wxIcon(wxIconLocation(peeloffPath));
		peeloffIcon.SetHeight(16);
		peeloffIcon.SetWidth(16);
		wxIcon peeloffDisabledIcon = wxIcon(wxIconLocation(peeloffDisabledPath));
		peeloffDisabledIcon.SetHeight(16);
		peeloffDisabledIcon.SetWidth(16);
		_toolbarInstanceWorkspaceButton->SetEnabledBitmap(peeloffIcon);
		_toolbarInstanceWorkspaceButton->SetDisabledBitmap(peeloffDisabledIcon);
	}

	wxString keyPath = FxStudioApp::GetIconPath(wxT("key.ico"));
	wxString keyDisabledPath = FxStudioApp::GetIconPath(wxT("key_disabled.ico"));
	if( FxFileExists(keyPath) && FxFileExists(keyDisabledPath) )
	{
		wxIcon keyIcon = wxIcon(wxIconLocation(keyPath));
		keyIcon.SetHeight(16);
		keyIcon.SetWidth(16);
		wxIcon keyDisabledIcon = wxIcon(wxIconLocation(keyDisabledPath));
		keyDisabledIcon.SetHeight(16);
		keyDisabledIcon.SetWidth(16);
		_toolbarAnimModeAutokeyToggle->SetEnabledBitmap(keyIcon);
		_toolbarAnimModeAutokeyToggle->SetDisabledBitmap(keyDisabledIcon);
	}

	wxString keyallPath = FxStudioApp::GetIconPath(wxT("keyall.ico"));
	wxString keyallDisabledPath = FxStudioApp::GetIconPath(wxT("keyall_disabled.ico"));
	if( FxFileExists(keyallPath) && FxFileExists(keyallDisabledPath) )
	{
		wxIcon keyallIcon = wxIcon(wxIconLocation(keyallPath));
		keyallIcon.SetHeight(16);
		keyallIcon.SetWidth(16);
		wxIcon keyallDisabledIcon = wxIcon(wxIconLocation(keyallDisabledPath));
		keyallDisabledIcon.SetHeight(16);
		keyallDisabledIcon.SetWidth(16);
		_toolbarAnimModeKeyAllButton->SetEnabledBitmap(keyallIcon);
		_toolbarAnimModeKeyAllButton->SetDisabledBitmap(keyallDisabledIcon);
	}

	wxString previouskeyPath = FxStudioApp::GetIconPath(wxT("previouskey.ico"));
	wxString previouskeyDisabledPath = FxStudioApp::GetIconPath(wxT("previouskey_disabled.ico"));
	if( FxFileExists(previouskeyPath) && FxFileExists(previouskeyDisabledPath) )
	{
		wxIcon previouskeyIcon = wxIcon(wxIconLocation(previouskeyPath));
		previouskeyIcon.SetHeight(16);
		previouskeyIcon.SetWidth(16);
		wxIcon previouskeyDisabledIcon = wxIcon(wxIconLocation(previouskeyDisabledPath));
		previouskeyDisabledIcon.SetHeight(16);
		previouskeyDisabledIcon.SetWidth(16);
		_toolbarAnimModePrevKeyButton->SetEnabledBitmap(previouskeyIcon);
		_toolbarAnimModePrevKeyButton->SetDisabledBitmap(previouskeyDisabledIcon);
	}

	wxString nextkeyPath = FxStudioApp::GetIconPath(wxT("nextkey.ico"));
	wxString nextkeyDisabledPath = FxStudioApp::GetIconPath(wxT("nextkey_disabled.ico"));
	if( FxFileExists(nextkeyPath) && FxFileExists(nextkeyDisabledPath) )
	{
		wxIcon nextkeyIcon = wxIcon(wxIconLocation(nextkeyPath));
		nextkeyIcon.SetHeight(16);
		nextkeyIcon.SetWidth(16);
		wxIcon nextkeyDisabledIcon = wxIcon(wxIconLocation(nextkeyDisabledPath));
		nextkeyDisabledIcon.SetHeight(16);
		nextkeyDisabledIcon.SetWidth(16);
		_toolbarAnimModeNextKeyButton->SetEnabledBitmap(nextkeyIcon);
		_toolbarAnimModeNextKeyButton->SetDisabledBitmap(nextkeyDisabledIcon);
	}

	wxString clearPath = FxStudioApp::GetIconPath(wxT("clear.ico"));
	wxString clearDisabledPath = FxStudioApp::GetIconPath(wxT("clear_disabled.ico"));
	if( FxFileExists(clearPath) && FxFileExists(clearDisabledPath) )
	{
		wxIcon clearIcon = wxIcon(wxIconLocation(clearPath));
		clearIcon.SetHeight(16);
		clearIcon.SetWidth(16);
		wxIcon clearDisabledIcon = wxIcon(wxIconLocation(clearDisabledPath));
		clearDisabledIcon.SetHeight(16);
		clearDisabledIcon.SetWidth(16);
		_toolbarSharedModeClearButton->SetEnabledBitmap(clearIcon);
		_toolbarSharedModeClearButton->SetDisabledBitmap(clearDisabledIcon);
	}

	wxString loadCurrentPath = FxStudioApp::GetIconPath(wxT("loadcurrent.ico"));
	wxString loadCurrentDisabledPath = FxStudioApp::GetIconPath(wxT("loadcurrent_disabled.ico"));
	if( FxFileExists(loadCurrentPath) && FxFileExists(loadCurrentDisabledPath) )
	{
		wxIcon loadCurrentIcon = wxIcon(wxIconLocation(loadCurrentPath));
		loadCurrentIcon.SetHeight(16);
		loadCurrentIcon.SetWidth(16);
		wxIcon loadCurrentDisabledIcon = wxIcon(wxIconLocation(loadCurrentDisabledPath));
		loadCurrentDisabledIcon.SetHeight(16);
		loadCurrentDisabledIcon.SetWidth(16);
		_toolbarSharedModeLoadCurrentFrameButton->SetEnabledBitmap(loadCurrentIcon);
		_toolbarSharedModeLoadCurrentFrameButton->SetDisabledBitmap(loadCurrentDisabledIcon);
	}

	wxString applyPath = FxStudioApp::GetIconPath(wxT("apply.ico"));
	wxString applyDisabledPath = FxStudioApp::GetIconPath(wxT("apply_disabled.ico"));
	if( FxFileExists(applyPath) && FxFileExists(applyDisabledPath) )
	{
		wxIcon applyIcon = wxIcon(wxIconLocation(applyPath));
		applyIcon.SetHeight(16);
		applyIcon.SetWidth(16);
		wxIcon applyDisabledIcon = wxIcon(wxIconLocation(applyDisabledPath));
		applyDisabledIcon.SetHeight(16);
		applyDisabledIcon.SetWidth(16);
		_toolbarCombinerModeCreateButton->SetEnabledBitmap(applyIcon);
		_toolbarCombinerModeCreateButton->SetDisabledBitmap(applyDisabledIcon);
		_toolbarMappingModeApplyButton->SetEnabledBitmap(applyIcon);
		_toolbarMappingModeApplyButton->SetDisabledBitmap(applyDisabledIcon);
	}
}

//------------------------------------------------------------------------------
// FxWorkspaceInstanceWidget
//------------------------------------------------------------------------------

WX_IMPLEMENT_DYNAMIC_CLASS(FxWorkspaceInstanceWidget, wxWindow)

BEGIN_EVENT_TABLE(FxWorkspaceInstanceWidget, wxWindow)
	EVT_SIZE(FxWorkspaceInstanceWidget::OnSize)
	EVT_PAINT(FxWorkspaceInstanceWidget::OnPaint)
	EVT_MOUSE_EVENTS(FxWorkspaceInstanceWidget::OnMouse)
	EVT_KEY_DOWN(FxWorkspaceInstanceWidget::OnKeyDown)
	
	EVT_HELP_RANGE(wxID_ANY, wxID_HIGHEST, FxWorkspaceInstanceWidget::OnHelp)

	EVT_MENU(MenuID_WorkspaceInstanceWidgetUndo, FxWorkspaceInstanceWidget::OnUndo)
	EVT_MENU(MenuID_WorkspaceInstanceWidgetRedo, FxWorkspaceInstanceWidget::OnRedo)
	EVT_MENU(MenuID_WorkspaceInstanceWidgetPrevFrame, FxWorkspaceInstanceWidget::OnPrevFrame)
	EVT_MENU(MenuID_WorkspaceInstanceWidgetNextFrame, FxWorkspaceInstanceWidget::OnNextFrame)
END_EVENT_TABLE()

FxWorkspaceInstanceWidget::FxWorkspaceInstanceWidget(wxWindow* parent, FxWidgetMediator* mediator)
	: Super(parent, -1, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE|wxCLIP_CHILDREN|wxTAB_TRAVERSAL)
	, FxWidget(mediator)
	, _actor(NULL)
	, _anim(NULL)
	, _animController(NULL)
	, _valid(FxFalse)
{
	FxWorkspaceManager::Instance()->AddWorkspaceContainer(this);

	// Set up the accelerator table.
	static const int numEntries = 5;
	wxAcceleratorEntry entries[numEntries];
	entries[0].Set(wxACCEL_CTRL, (FxInt32)'Z', MenuID_WorkspaceInstanceWidgetUndo);
	entries[1].Set(wxACCEL_CTRL, (FxInt32)'Y', MenuID_WorkspaceInstanceWidgetRedo);
	entries[2].Set(wxACCEL_CTRL|wxACCEL_SHIFT, (FxInt32)'Z', MenuID_WorkspaceInstanceWidgetRedo);
	entries[3].Set(wxACCEL_NORMAL, WXK_F5, MenuID_WorkspaceInstanceWidgetPrevFrame);
	entries[4].Set(wxACCEL_NORMAL, WXK_F6, MenuID_WorkspaceInstanceWidgetNextFrame);

	// Note to self: when adding an accelerator entry, update numEntries. It works better.
	wxAcceleratorTable accelerator(numEntries, entries);
	SetAcceleratorTable(accelerator);
}

FxWorkspaceInstanceWidget::~FxWorkspaceInstanceWidget()
{
	if( _valid )
	{
		// We don't want to instance the workspace manager again if the app is
		// being shut down.
		FxWorkspaceManager::Instance()->RemoveWorkspaceContainer(this);
	}
}

//------------------------------------------------------------------------------
// Widget messages.
//------------------------------------------------------------------------------
void FxWorkspaceInstanceWidget::OnActorChanged(FxWidget* FxUnused(sender), FxActor* actor)
{
	_actor = actor;
	_valid = _actor ? FxTrue : FxFalse;
	_currentWorkspace.SetActor(_actor);
	Refresh(FALSE);
	Update();
}

void FxWorkspaceInstanceWidget::OnActorInternalDataChanged(FxWidget* FxUnused(sender), FxActorDataChangedFlag flags)
{
	if( flags & ADCF_FaceGraphChanged || flags & ADCF_CurvesChanged )
	{
		// Force a relinking of the sliders.
		_currentWorkspace.SetActor(_actor);
		_currentWorkspace.SetAnim(_anim);
		Refresh(FALSE);
		Update();
	}
}

void FxWorkspaceInstanceWidget::OnAnimChanged(FxWidget* FxUnused(sender), const FxName& FxUnused(animGroupName), FxAnim* anim)
{
	_anim = anim;
	_currentWorkspace.SetAnim(_anim);
	Refresh(FALSE);
	Update(); 
}

void FxWorkspaceInstanceWidget::OnTimeChanged(FxWidget* FxUnused(sender), FxReal newTime)
{
	_currentTime = newTime;
	_currentWorkspace.SetCurrentTime(_currentTime);
	Refresh(FALSE);
	Update();
}

void FxWorkspaceInstanceWidget::OnRefresh(FxWidget* FxUnused(sender))
{
	if( _animController )
	{
		_currentWorkspace.UpdateAllSliders();
		Refresh(FALSE);
		Update();
	}
}

void FxWorkspaceInstanceWidget::OnAppStartup(FxWidget* FxUnused(sender))
{
	_valid = _actor ? FxTrue : FxFalse;
}

void FxWorkspaceInstanceWidget::OnAppShutdown(FxWidget* FxUnused(sender))
{
	_valid = FxFalse;
}

//------------------------------------------------------------------------------
// Event handlers.
//------------------------------------------------------------------------------
void FxWorkspaceInstanceWidget::OnSize(wxSizeEvent& FxUnused(event))
{
	if( _valid )
	{
		_currentWorkspace.SetWorkspaceRect(GetClientRect());
	}
}

void FxWorkspaceInstanceWidget::OnPaint(wxPaintEvent& FxUnused(event))
{
	wxBufferedPaintDC dc(this);
	if( _valid )
	{
		_currentWorkspace.DrawOn(&dc);
	}
	else
	{
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
		dc.DrawRectangle(GetClientRect());
	}
}

void FxWorkspaceInstanceWidget::OnMouse(wxMouseEvent& event)
{
	if( _valid )
	{
		if( event.LeftDown() && !HasCapture() )
		{
			SetFocus();
			CaptureMouse();
		}

		FxInt32 refresh = WRF_None;
		_currentWorkspace.ProcessMouseEvent(event, refresh);

		if( refresh != WRF_None && _autoSelectCurve )
		{
			FxWorkspaceElementSlider* selectedSlider = _currentWorkspace.GetSelectedSlider();
			if( selectedSlider )
			{
				wxString selCurves;
				const FxString& xName = selectedSlider->GetNodeNameXAxis();
				if( xName.Length() )
				{
					selCurves.Append(wxString(xName.GetData(), wxConvLibc));
				}
				const FxString& yName = selectedSlider->GetNodeNameYAxis();
				if( yName.Length() )
				{
					if( xName.Length() )
					{
						selCurves += wxT("|");
					}
					selCurves.Append(wxString(yName.GetData(), wxConvLibc));
				}
				FxArray<FxString> currSelCurves;
				FxSessionProxy::GetSelectedAnimCurves(currSelCurves);
				FxBool goodX = xName.Length() == 0;
				FxBool goodY = yName.Length() == 0;
				for( FxSize i = 0; i < currSelCurves.Length(); ++i )
				{
					goodX |= currSelCurves[i] == xName;
					goodY |= currSelCurves[i] == yName;
				}
				if( !(goodX && goodY) )
				{
					wxString command = wxString::Format(wxT("select -type \"curve\" -names \"%s\""), selCurves);
					FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
				}
			}
		}
		if( refresh & WRF_Workspaces )
		{
			FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
		}
		if( refresh & WRF_App )
		{
			DispatchRefreshMessage();
		}

		if( event.LeftUp() && HasCapture() )
		{
			ReleaseMouse();
		}
	}
}

void FxWorkspaceInstanceWidget::OnKeyDown(wxKeyEvent& event)
{
	FxInt32 refresh = WRF_None;
	_currentWorkspace.ProcessKeyDown(event, refresh);
	if( refresh & WRF_Workspaces )
	{
		FxWorkspaceManager::Instance()->FlagWorkspacesNeedUpdate();
	}
	if( refresh & WRF_App )
	{
		DispatchRefreshMessage();
		Refresh(FALSE);
	}
}

void FxWorkspaceInstanceWidget::OnHelp(wxHelpEvent& FxUnused(event))
{
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().LoadFile();
	static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow())->GetHelpController().DisplaySection(wxT("Workspace Tab"));
}

void FxWorkspaceInstanceWidget::OnUndo(wxCommandEvent& event)
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow());
	if( pMainWin )
	{
		pMainWin->OnUndoProxy(event);
	}
}

void FxWorkspaceInstanceWidget::OnRedo(wxCommandEvent& event)
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow());
	if( pMainWin )
	{
		pMainWin->OnRedoProxy(event);
	}
}

void FxWorkspaceInstanceWidget::OnPrevFrame(wxCommandEvent& event)
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow());
	if( pMainWin )
	{
		pMainWin->OnPrevFrame(event);
	}
}

void FxWorkspaceInstanceWidget::OnNextFrame(wxCommandEvent& event)
{
	FxStudioMainWin* pMainWin = static_cast<FxStudioMainWin*>(FxStudioApp::GetMainWindow());
	if( pMainWin )
	{
		pMainWin->OnNextFrame(event);
	}
}

void FxWorkspaceInstanceWidget::SetWorkspace(FxWorkspace* workspace)
{
	_mode = workspace->GetMode();
	_autoKey = workspace->GetAutoKey();
	_currentWorkspace = workspace ? *workspace : FxWorkspace();
	InitWorkspace();
	Refresh(FALSE);
	Update();
}

void FxWorkspaceInstanceWidget::SetAnimController(FxAnimController* animController)
{
	_animController = animController;
	_currentWorkspace.SetAnimController(_animController);
}

//------------------------------------------------------------------------------
// Workspace container messages.
//------------------------------------------------------------------------------
void FxWorkspaceInstanceWidget::OnWorkspaceAdded(const FxName& FxUnused(newWorkspaceName))
{
	// Nothing here.
}

void FxWorkspaceInstanceWidget::OnWorkspacePreRemove(const FxName& workspaceToRemove)
{
	if( workspaceToRemove == _currentWorkspace.GetName() )
	{
		// Reset the workspace to the default workspace.  Not technically
		// necessary, but it's there just in case destroy fails.
		_currentWorkspace = FxWorkspace();
		GetParent()->SetName(_("<Deleted workspace>"));
		GetParent()->Destroy();
	}
}

void FxWorkspaceInstanceWidget::OnWorkspacePostRemove(const FxName& FxUnused(removedWorkspace))
{
	// Shouldn't ever get here, but fill it out just in case.
	InitWorkspace();
	Refresh(FALSE);
}

void FxWorkspaceInstanceWidget::OnWorkspaceRenamed(const FxName& oldName, const FxName newName)
{
	if( oldName == _currentWorkspace.GetName() )
	{
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(newName);
		GetParent()->SetName(wxString::FromAscii(newName.GetAsCstr()));
		InitWorkspace();
		Refresh(FALSE);
	}
}

void FxWorkspaceInstanceWidget::OnWorkspaceModified(const FxName& modifiedWorkspace)
{
	if( modifiedWorkspace == _currentWorkspace.GetName() )
	{
		_currentWorkspace = FxWorkspaceManager::Instance()->Find(modifiedWorkspace);
		InitWorkspace();
		Refresh(FALSE);
	}
}

void FxWorkspaceInstanceWidget::OnWorkspaceNeedsUpdate()
{
	_currentWorkspace.UpdateAllSliders();
	Refresh(FALSE);
	Update();
}

void FxWorkspaceInstanceWidget::OnWorkspaceModeSwitch(FxWorkspaceMode newMode)
{
	_mode = newMode;
	_currentWorkspace.SetMode(_mode);
	Refresh(FALSE);
}

void FxWorkspaceInstanceWidget::OnWorkspaceOptionChanged(FxBool autoKey, FxBool autoSelectCurve)
{
	_autoKey = autoKey;
	_autoSelectCurve = autoSelectCurve;
	_currentWorkspace.SetOption(_autoKey, _autoSelectCurve);
}

void FxWorkspaceInstanceWidget::OnWorkspaceClose()
{
	GetParent()->SetName(_("<Closed workspace>"));
	GetParent()->Destroy();
}

void FxWorkspaceInstanceWidget::InitWorkspace()
{
	_currentWorkspace.SetActor(_actor);
	_currentWorkspace.SetAnim(_anim);
	_currentWorkspace.SetAnimController(_animController);
	_currentWorkspace.SetCurrentTime(_currentTime);
	_currentWorkspace.SetWorkspaceRect(GetClientRect());
	_currentWorkspace.SetMode(_mode);
	_currentWorkspace.SetOption(_autoKey, _autoSelectCurve);
	_currentWorkspace.UpdateAllSliders();
}

void FxWorkspaceInstanceWidget::DispatchRefreshMessage()
{
	// Tell the widgets to update.
	if( _mediator )
	{
		_mediator->OnRefresh(NULL);
	}
}

} // namespace Face

} // namespace OC3Ent
