//------------------------------------------------------------------------------
// The User Interface code for the FaceFx Max plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMaxGUI.h"
#include "FxToolLog.h"

//------------------------------------------------------------------------------
// Forward declarations.
//------------------------------------------------------------------------------

BOOL FxGetStringFromUser( HWND hOwner, const OC3Ent::Face::FxString& prompt, 
					      OC3Ent::Face::FxString& userString );

//------------------------------------------------------------------------------
// Statics.
//------------------------------------------------------------------------------

// Note:  Do not attempt to initialize any of these static variables because
//        Max will refuse to load the plug-in DLL claiming "invalid memory
//        access" or some other such nonsense.
static FxMaxMainPanel fxMaxMainPanel;
static FxMaxClassDesc fxMaxClassDesc;
static OC3Ent::Face::FxString fxGetStringFromUser_Prompt;
static OC3Ent::Face::FxString fxGetStringFromUser_UserString;

//------------------------------------------------------------------------------
// Dialog procedure callbacks.
//------------------------------------------------------------------------------

// The dialog procedure for the generic "get string from user" dialog.
static BOOL CALLBACK
FxGetStringFromUserDialogProc( HWND hWnd,
							   UINT msg,
							   WPARAM wParam,
							   LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Set the window title.
			::SetWindowText(hWnd, _T(fxGetStringFromUser_Prompt.GetData()));
			// Set the initial edit box text.
			HWND hEditBox = ::GetDlgItem(hWnd, ID_USER_STRING_EDIT_BOX);
			::SetWindowText(hEditBox, _T(fxGetStringFromUser_UserString.GetData()));
			return TRUE;
		}
		break;
	case WM_CLOSE:
		{
			::EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) )
				{
				case ID_OK_BUTTON:
					{
						// Set the user string to the edit box text.
						HWND hEditBox = ::GetDlgItem(hWnd, ID_USER_STRING_EDIT_BOX);
						TCHAR userString[256] = {0};
						::GetWindowText(hEditBox, userString, 256);
						fxGetStringFromUser_UserString = userString;
						::EndDialog(hWnd, IDOK);
					}
					break;
				case ID_CANCEL_BUTTON:
					{
						::EndDialog(hWnd, IDCANCEL);
						return TRUE;
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

// The dialog procedure for the about dialog.
static BOOL CALLBACK
FxAboutDialogProc( HWND hWnd,
				   UINT msg,
				   WPARAM wParam,
				   LPARAM lParam )
{
	switch( msg )
	{
	case WM_INITDIALOG:
		{
			// Set the version information string.
			HWND hVersionInfo = ::GetDlgItem(hWnd, ID_VERSION_INFO_STRING);
			OC3Ent::Face::FxString versionInfoString = "(plug-in version ";
			versionInfoString = OC3Ent::Face::FxString::Concat(versionInfoString, kCurrentFxMaxVersion);
			versionInfoString = OC3Ent::Face::FxString::Concat(versionInfoString, ")");
			::SetWindowText(hVersionInfo, _T(versionInfoString.GetData()));
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) )
				{
				case ID_OK_BUTTON:
					{
						::EndDialog(hWnd, IDOK);
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

// The dialog procedure for the actor information roll out.
static BOOL CALLBACK 
FxActorInfoRolloutDialogProc( HWND hWnd, 
							  UINT msg, 
							  WPARAM wParam, 
							  LPARAM lParam )
{
	switch( msg )
	{
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) ) 
				{
				case ID_CREATE_ACTOR_BUTTON:
					{
						// Get the actor name from the user and create the actor.
						OC3Ent::Face::FxString actorName = "NewActor";
						if( FxGetStringFromUser(hWnd, "Enter Actor Name:", actorName) )
						{
							if( !OC3Ent::Face::FxMaxData::maxInterface.CreateActor(actorName) )
							{
								if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
								{
									::MessageBox(hWnd, _T("Failed to create actor!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
								}
							}
							else
							{
								fxMaxMainPanel.UpdateAll();
							}
						}
					}
					break;
				case ID_LOAD_ACTOR_BUTTON:
					{
						// Prompt the user to locate the actor file.
						OPENFILENAME ofn;
						char szFile[_MAX_PATH] = {0};
						ZeroMemory(&ofn, sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = szFile;
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = _T("FaceFX Actor Files\0*.fxa\0");
						ofn.nFilterIndex = 0;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if( ::GetOpenFileName(&ofn) ) 
						{
							// Load the actor.
							if( !OC3Ent::Face::FxMaxData::maxInterface.LoadActor(szFile) )
							{
								if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
								{
									::MessageBox(hWnd, _T("Failed to load actor!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
								}
							}
							fxMaxMainPanel.UpdateAll();
						}
					}
					break;
				case ID_SAVE_ACTOR_BUTTON:
					{
						// Prompt the user to enter a file name to save the
						// actor to.
						OPENFILENAME ofn;
						char szFile[_MAX_PATH] = {0};
						ZeroMemory(&ofn, sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = szFile;
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = _T("FaceFX Actor Files\0*.fxa\0");
						ofn.nFilterIndex = 0;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						ofn.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT;
						ofn.lpstrDefExt = _T("fxa");

						if( ::GetSaveFileName(&ofn) )
						{
							// Save the actor.
							if( !OC3Ent::Face::FxMaxData::maxInterface.SaveActor(szFile) )
							{
								if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
								{
									::MessageBox(hWnd, _T("Failed to save actor!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
								}
							}
						}
					}
					break;
				case ID_ABOUT_BUTTON:
					{
						// Display the about box.
						DialogBox(fxMaxClassDesc.HInstance(), 
								  MAKEINTRESOURCE(ID_ABOUT_DIALOG),
								  hWnd,
								  FxAboutDialogProc);
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if( fxMaxMainPanel.GetMaxInterfacePointer() )
			{
				fxMaxMainPanel.GetMaxInterfacePointer()->RollupMouseMessage(hWnd, msg, wParam, lParam);
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// The dialog procedure for the reference pose roll out.
static BOOL CALLBACK 
FxReferencePoseRolloutDialogProc( HWND hWnd, 
								  UINT msg, 
								  WPARAM wParam, 
								  LPARAM lParam )
{
	switch( msg )
	{
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) ) 
				{
				case ID_IMPORT_REFERENCE_POSE_BUTTON:
					{
						if( fxMaxMainPanel.GetMaxInterfacePointer() )
						{
                            TimeValue currentTime = fxMaxMainPanel.GetMaxInterfacePointer()->GetTime();
							if( !OC3Ent::Face::FxMaxData::maxInterface.ImportRefPose(currentTime / GetTicksPerFrame()) )
							{
								if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
								{
									::MessageBox(hWnd, _T("Failed to import reference pose!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
								}
							}
						}
					}
					break;
				case ID_EXPORT_REFERENCE_POSE_BUTTON:
					{
						if( fxMaxMainPanel.GetMaxInterfacePointer() )
						{
							FxReferenceBonesSelectionDialogCallback refBonesSelector;
							if( fxMaxMainPanel.GetMaxInterfacePointer()->DoHitByNameDialog(&refBonesSelector) )
							{
								OC3Ent::Face::FxArray<OC3Ent::Face::FxString> refBones;
								for( OC3Ent::Face::FxInt32 i = 0; 
									 i <  fxMaxMainPanel.GetMaxInterfacePointer()->GetSelNodeCount();
									 ++i )
								{
									INode* pNode = fxMaxMainPanel.GetMaxInterfacePointer()->GetSelNode(i);
									if( pNode )
									{
										refBones.PushBack(pNode->GetName());
									}
								}
								OC3Ent::Face::FxBool normalizeScale;
								fxMaxMainPanel.GetNormalizeScale(normalizeScale);
								OC3Ent::Face::FxMaxData::maxInterface.SetNormalizeScale(normalizeScale);
								TimeValue currentTime = fxMaxMainPanel.GetMaxInterfacePointer()->GetTime();
								if( !OC3Ent::Face::FxMaxData::maxInterface.ExportRefPose(currentTime / GetTicksPerFrame(), refBones) )
								{
									if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
									{
										::MessageBox(hWnd, _T("Failed to export reference pose!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
									}
								}
								else
								{
									fxMaxMainPanel.UpdateRefBonesListBox();
								}
							}
						}
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if( fxMaxMainPanel.GetMaxInterfacePointer() )
			{
				fxMaxMainPanel.GetMaxInterfacePointer()->RollupMouseMessage(hWnd, msg, wParam, lParam);
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// The dialog procedure for the bone poses roll out.
static BOOL CALLBACK 
FxBonePosesRolloutDialogProc( HWND hWnd, 
							  UINT msg, 
							  WPARAM wParam, 
							  LPARAM lParam )
{
	switch( msg )
	{
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) ) 
				{
				case ID_IMPORT_BONE_POSE_BUTTON:
					{
						OC3Ent::Face::FxString selectedBonePose;
						fxMaxMainPanel.GetSelectedBonePose(selectedBonePose);
						if( selectedBonePose.Length() > 0 )
						{
							if( fxMaxMainPanel.GetMaxInterfacePointer() )
							{
								TimeValue currentTime = fxMaxMainPanel.GetMaxInterfacePointer()->GetTime();
								if( !OC3Ent::Face::FxMaxData::maxInterface.ImportBonePose(selectedBonePose, currentTime / GetTicksPerFrame()) )
								{
									if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
									{
										::MessageBox(hWnd, _T("Failed to import bone pose!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
									}
								}
							}
						}
					}
					break;
				case ID_EXPORT_BONE_POSE_BUTTON:
					{
						// Get the bone pose name from the user and export it.
						OC3Ent::Face::FxString bonePoseName;
						fxMaxMainPanel.GetSelectedBonePose(bonePoseName);
						if( 0 == bonePoseName.Length() )
						{
							bonePoseName = "NewBonePose";
						}
						if( FxGetStringFromUser(hWnd, "Enter Bone Pose Name:", bonePoseName) )
						{
							if( fxMaxMainPanel.GetMaxInterfacePointer() )
							{
								OC3Ent::Face::FxBool normalizeScale;
								fxMaxMainPanel.GetNormalizeScale(normalizeScale);
								OC3Ent::Face::FxMaxData::maxInterface.SetNormalizeScale(normalizeScale);
								TimeValue currentTime = fxMaxMainPanel.GetMaxInterfacePointer()->GetTime();
								if( !OC3Ent::Face::FxMaxData::maxInterface.ExportBonePose(currentTime / GetTicksPerFrame(), bonePoseName) )
								{
									if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
									{
										::MessageBox(hWnd, _T("Failed to export bone pose!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
									}
								}
								else
								{
									fxMaxMainPanel.UpdateBonePosesListBox();
								}
							}
						}
					}
					break;
				case ID_BATCH_IMPORT_BONE_POSES_BUTTON:
					{
						// Prompt the user to locate the file.
						OPENFILENAME ofn;
						char szFile[_MAX_PATH] = {0};
						ZeroMemory(&ofn, sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = szFile;
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = _T("Text Files\0*.txt\0");
						ofn.nFilterIndex = 0;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if( ::GetOpenFileName(&ofn) ) 
						{
							OC3Ent::Face::FxInt32 startFrame = 0;
							OC3Ent::Face::FxInt32 endFrame   = 0;
							OC3Ent::Face::FxMaxData::maxInterface.BatchImportBonePoses(szFile, startFrame, endFrame);
						}
					}
					break;
				case ID_BATCH_EXPORT_BONE_POSES_BUTTON:
					{
						// Prompt the user to locate the file.
						OPENFILENAME ofn;
						char szFile[_MAX_PATH] = {0};
						ZeroMemory(&ofn, sizeof(OPENFILENAME));
						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = hWnd;
						ofn.lpstrFile = szFile;
						ofn.nMaxFile = sizeof(szFile);
						ofn.lpstrFilter = _T("Text Files\0*.txt\0");
						ofn.nFilterIndex = 0;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

						if( ::GetOpenFileName(&ofn) ) 
						{
							OC3Ent::Face::FxBool normalizeScale;
							fxMaxMainPanel.GetNormalizeScale(normalizeScale);
							OC3Ent::Face::FxMaxData::maxInterface.SetNormalizeScale(normalizeScale);
							OC3Ent::Face::FxMaxData::maxInterface.BatchExportBonePoses(szFile);
							fxMaxMainPanel.UpdateBonePosesListBox();
						}
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if( fxMaxMainPanel.GetMaxInterfacePointer() )
			{
				fxMaxMainPanel.GetMaxInterfacePointer()->RollupMouseMessage(hWnd, msg, wParam, lParam);
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

// The dialog procedure for the animations roll out.
static BOOL CALLBACK 
FxAnimationsRolloutDialogProc( HWND hWnd, 
							   UINT msg, 
							   WPARAM wParam, 
							   LPARAM lParam )
{
	switch( msg )
	{
	case WM_COMMAND:
		switch( HIWORD(wParam) )
		{
		case BN_CLICKED:
			{
				switch( LOWORD(wParam) ) 
				{
				case ID_IMPORT_ANIMATION_BUTTON:
					{
						OC3Ent::Face::FxString selectedAnimGroup;
						fxMaxMainPanel.GetSelectedAnimGroup(selectedAnimGroup);
						if( selectedAnimGroup.Length() > 0 )
						{
							OC3Ent::Face::FxString selectedAnim;
							fxMaxMainPanel.GetSelectedAnim(selectedAnim);
							if( selectedAnim.Length() > 0 )
							{
								if( !OC3Ent::Face::FxMaxData::maxInterface.ImportAnim(selectedAnimGroup, 
									selectedAnim, static_cast<OC3Ent::Face::FxReal>(GetFrameRate())) )
								{
									if( OC3Ent::Face::FxMaxData::maxInterface.GetShouldDisplayWarningDialogs() )
									{
										::MessageBox(hWnd, _T("Failed to import animation!"), _T("FaceFX"), MB_OK | MB_ICONERROR);
									}
								}
							}
						}
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		case CBN_SELCHANGE:
			{
				switch( LOWORD(wParam) )
				{
				case ID_ANIMATION_GROUP_COMBO_BOX:
					{
						fxMaxMainPanel.UpdateAnimationsListBox();
					}
					break;
				default:
					return FALSE;
				}
			}
			break;
		default:
			return FALSE;
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		{
			if( fxMaxMainPanel.GetMaxInterfacePointer() )
			{
				fxMaxMainPanel.GetMaxInterfacePointer()->RollupMouseMessage(hWnd, msg, wParam, lParam);
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
// Global functions.
//------------------------------------------------------------------------------

// Updates the FaceFX Panel for Max script functions.
void UpdateMainFaceFxPanel( void )
{ 
	fxMaxMainPanel.UpdateAll();
}

// Returns the class description for the FaceFX Max plug-in.
ClassDesc2* GetFxMaxClassDesc( void )
{
	return &fxMaxClassDesc;
}

// Displays the generic "get string from user" dialog with prompt as the title 
// and userString as the initial value in the user string edit box.  Returns 
// TRUE if the user pressed "OK" or FALSE if the user pressed "Cancel".  If the 
// user pressed "OK" and TRUE is returned, userString is filled out with the 
// user input.  Otherwise it is filled out with the empty string.
BOOL 
FxGetStringFromUser( HWND hOwner, 
					 const OC3Ent::Face::FxString& prompt, 
					 OC3Ent::Face::FxString& userString )
{
	// Set the prompt for the dialog.
	fxGetStringFromUser_Prompt = prompt;
	// Set the initial user string for the edit box.
	fxGetStringFromUser_UserString = userString;
	// Clear the passed in user string to the empty string.
	userString = "";

	// Display the dialog box.
	if( IDOK == DialogBox(fxMaxClassDesc.HInstance(), 
						  MAKEINTRESOURCE(ID_GET_STRING_FROM_USER_DIALOG),
						  hOwner,
						  FxGetStringFromUserDialogProc) )
	{
		userString = fxGetStringFromUser_UserString;
		return TRUE;
	}
	return FALSE;
}

//------------------------------------------------------------------------------
// FxMaxClassDesc.
//------------------------------------------------------------------------------

// Controls if the plug-in shows up in lists from the user to choose from.
int FxMaxClassDesc::IsPublic( void ) 
{ 
	return TRUE; 
}

// Max calls this method when it needs a pointer to a new instance of the 
// plug-in class.
void* FxMaxClassDesc::Create( BOOL loading )
{ 
	return &fxMaxMainPanel; 
}

// Returns the name of the class.
const TCHAR* FxMaxClassDesc::ClassName( void ) 
{ 
	return GetString(IDS_CLASS_NAME); 
}

// Returns a system defined constant describing the class this plug-in 
// class was derived from.
SClass_ID FxMaxClassDesc::SuperClassID( void ) 
{ 
	return UTILITY_CLASS_ID; 
}

// Returns the unique ID for the object.
Class_ID FxMaxClassDesc::ClassID( void ) 
{
	return FACEFX_CLASS_ID; 
}

// Returns a string describing the category the plug-in fits into.
const TCHAR* FxMaxClassDesc::Category( void ) 
{ 
	return _T("");
}

// Returns a string which provides a fixed, machine parsable internal name 
// for the plug-in.  This name is used by MAXScript.
const TCHAR* FxMaxClassDesc::InternalName( void ) 
{ 
	return _T("FaceFX"); 
}

// Returns the DLL instance handle of the plug-in.
HINSTANCE FxMaxClassDesc::HInstance( void ) 
{ 
	return hInstance;
}

//------------------------------------------------------------------------------
// FxReferenceBonesSelectionDialogCallback.
//------------------------------------------------------------------------------

// Returns the title string displayed in the dialog.
TCHAR* FxReferenceBonesSelectionDialogCallback::dialogTitle( void )
{
	return _T("Select all of the bones that FaceFX should control.");
}

// Returns the text displayed in the 'Select' or 'Pick' button.
TCHAR* FxReferenceBonesSelectionDialogCallback::buttonText( void )
{
	return _T("Select");
}

// Returns TRUE if the user may only make a single selection in the list at 
// one time, FALSE otherwise.
BOOL FxReferenceBonesSelectionDialogCallback::singleSelect( void )
{
	return FALSE;
}

// This gives the callback the opportunity to filter out items from the 
// list.  This is called before the dialog is presented.  It returns TRUE 
// if the filter() method (below) should be called, FALSE otherwise.
BOOL FxReferenceBonesSelectionDialogCallback::useFilter( void )
{
	return FALSE;
}

// This method will be called if useFilter() above returned TRUE.  This 
// gives the callback the chance to filter out items from the list before 
// they are presented to the user in the dialog.  This is called once for 
// each node that would otherwise be presented.  Return nonzero to accept 
// the item and zero to skip it.
int FxReferenceBonesSelectionDialogCallback::filter( INode* pNode )
{
	return TRUE;
}

// Normally, when the user selects OK from the dialog, the system selects 
// all the chosen nodes in the scene.  At times a developer may want to do 
// something other than select the chosen nodes.  If this method returns 
// TRUE then the nodes in the list will not be selected, but the proc() 
// method is called instead (see below).  If this method returns FALSE, 
// then the nodes are selected in the scene and proc() is not called.
BOOL FxReferenceBonesSelectionDialogCallback::useProc( void )
{
	return FALSE;
}

// This allows the plug-in to process the nodes chosen from the dialog in 
// any manner.  For example if the developer wanted to delete the nodes 
// chosen using this dialog, they would implement useProc() to return TRUE 
// and this method to delete all the nodes in the table passed.
void FxReferenceBonesSelectionDialogCallback::proc( INodeTab& nodeTab )
{
}

// Normally, when the dialog is entered, the nodes in the scene that are 
// selected are highlighted in the list.  If this method returns TRUE, the 
// developer may control which items are highlighted by implementing 
// doHilite() (see below).  If this method returns FALSE the selected nodes 
// will have their names highlighted in the list.
BOOL FxReferenceBonesSelectionDialogCallback::doCustomHilite( void )
{
	return FALSE;
}

// This method is called for each item in the list if doCustomHilite() 
// returns TRUE.  This method returns TRUE or FALSE to control if each item 
// is highlighted.
BOOL FxReferenceBonesSelectionDialogCallback::doHilite( INode* pNode )
{
	return FALSE;
}

// This defaults to returning FALSE, which means that hidden and frozen 
// objects will not be included in the select by name dialog list.  If this 
// method is overridden to return TRUE, then hidden and frozen nodes will be 
// sent through the user-supplied filter.
BOOL FxReferenceBonesSelectionDialogCallback::showHiddenAndFrozen( void )
{
	return FALSE;
}

//------------------------------------------------------------------------------
// FxMaxMainPanel.
//------------------------------------------------------------------------------

// Constructor.
FxMaxMainPanel::FxMaxMainPanel()
	: _pMaxInterface(NULL)
	, _pUtilityInterface(NULL)
	, _hActorInfoRollout(NULL)
	, _hReferencePoseRollout(NULL)
	, _hBonePosesRollout(NULL)
	, _hAnimationsRollout(NULL)
	, _hActorLabel(NULL)
	, _hCreateActorButton(NULL)
	, _hLoadActorButton(NULL)
	, _hSaveActorButton(NULL)
	, _hAboutButton(NULL)
	, _hNormalizeScaleCheckBox(NULL)
	, _hRefBonesListBox(NULL)
	, _hImportRefPoseButton(NULL)
	, _hExportRefPoseButton(NULL)
	, _hBonePosesListBox(NULL)
	, _hImportBonePoseButton(NULL)
	, _hExportBonePoseButton(NULL)
	, _hBatchImportBonePosesButton(NULL)
	, _hBatchExportBonePosesButton(NULL)
	, _hAnimationGroupComboBox(NULL)
	, _hAnimationsListBox(NULL)
	, _hImportAnimationButton(NULL)
{
}

// Destructor.
FxMaxMainPanel::~FxMaxMainPanel()
{
}

// This method is called when the utility plug-in may be used in the 
// Utility branch of the command panel.  Roll-up pages are added to the 
// panel here.
void FxMaxMainPanel::BeginEditParams( Interface* ip, IUtil* iu )
{
	_pMaxInterface = ip;
	_pUtilityInterface = iu;

	OC3Ent::Face::FxMaxData::maxInterface.SetMaxInterfacePointer(_pMaxInterface);

	if( _pMaxInterface && _pUtilityInterface )
	{
		_hActorInfoRollout = _pMaxInterface->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(ID_FACEFX_ACTOR_INFO_PANEL), 
			FxActorInfoRolloutDialogProc, 
			"FaceFX", 
			0);

		_hActorLabel             = ::GetDlgItem(_hActorInfoRollout, ID_ACTOR_LABEL);
		_hCreateActorButton      = ::GetDlgItem(_hActorInfoRollout, ID_CREATE_ACTOR_BUTTON);
		_hLoadActorButton        = ::GetDlgItem(_hActorInfoRollout, ID_LOAD_ACTOR_BUTTON);
		_hSaveActorButton        = ::GetDlgItem(_hActorInfoRollout, ID_SAVE_ACTOR_BUTTON);
		_hAboutButton			 = ::GetDlgItem(_hActorInfoRollout, ID_ABOUT_BUTTON);
		_hNormalizeScaleCheckBox = ::GetDlgItem(_hActorInfoRollout, ID_NORMALIZE_SCALE_CHECK_BOX);

		if(OC3Ent::Face::FxMaxData::maxInterface.IsNoSave()) 
		{
			::EnableWindow(	_hSaveActorButton, false);
		}

		_hReferencePoseRollout = _pMaxInterface->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(ID_FACEFX_REFERENCE_POSE_PANEL), 
			FxReferencePoseRolloutDialogProc, 
			"Reference Pose", 
			0);

		_hRefBonesListBox     = ::GetDlgItem(_hReferencePoseRollout, ID_REFERENCE_BONES_LIST_BOX);
		_hImportRefPoseButton = ::GetDlgItem(_hReferencePoseRollout, ID_IMPORT_REFERENCE_POSE_BUTTON);
		_hExportRefPoseButton = ::GetDlgItem(_hReferencePoseRollout, ID_EXPORT_REFERENCE_POSE_BUTTON);

		_hBonePosesRollout = _pMaxInterface->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(ID_FACEFX_BONE_POSES_PANEL), 
			FxBonePosesRolloutDialogProc, 
			"Bone Poses", 
			0);

		_hBonePosesListBox           = ::GetDlgItem(_hBonePosesRollout, ID_BONE_POSES_LIST_BOX);
		_hImportBonePoseButton       = ::GetDlgItem(_hBonePosesRollout, ID_IMPORT_BONE_POSE_BUTTON); 
		_hExportBonePoseButton       = ::GetDlgItem(_hBonePosesRollout, ID_EXPORT_BONE_POSE_BUTTON);
		_hBatchImportBonePosesButton = ::GetDlgItem(_hBonePosesRollout, ID_BATCH_IMPORT_BONE_POSES_BUTTON);
		_hBatchExportBonePosesButton = ::GetDlgItem(_hBonePosesRollout, ID_BATCH_EXPORT_BONE_POSES_BUTTON);

#ifndef MOD_DEVELOPER_VERSION
		_hAnimationsRollout = _pMaxInterface->AddRollupPage(
			hInstance, 
			MAKEINTRESOURCE(ID_FACEFX_ANIMATIONS_PANEL), 
			FxAnimationsRolloutDialogProc, 
			"Animations", 
			0);

		_hAnimationGroupComboBox = ::GetDlgItem(_hAnimationsRollout, ID_ANIMATION_GROUP_COMBO_BOX);
		_hAnimationsListBox      = ::GetDlgItem(_hAnimationsRollout, ID_ANIMATIONS_LIST_BOX);
		_hImportAnimationButton  = ::GetDlgItem(_hAnimationsRollout, ID_IMPORT_ANIMATION_BUTTON);
#endif
		
		// Update all of the controls.  This fills the controls out again if the
		// user changes tabs in Max and comes back to the FaceFX panel.
		UpdateAll();

		// Restore the selected reference bone.
		SetSelectedRefBone(_selectedRefBone);
		// Restore the selected bone pose.
		SetSelectedBonePose(_selectedBonePose);
		// Restore the selected animation group.
		SetSelectedAnimGroup(_selectedAnimGroup);
		// The selected animation group could have been changed here, so update 
		// the animations list box before attempting to select the animation.
		UpdateAnimationsListBox();
		// Restore the selected animation.
		SetSelectedAnim(_selectedAnim);
	}
}

// This method is called when the utility plug-in is done being used in the 
// Utility branch of the command panel (for example if the user changes to 
// the Create branch).  Roll-up pages are destroyed here.
void FxMaxMainPanel::EndEditParams( Interface* ip, IUtil* iu )
{
	_pMaxInterface = NULL;
	_pUtilityInterface = NULL;

	OC3Ent::Face::FxMaxData::maxInterface.SetMaxInterfacePointer(NULL);

	if( ip && iu )
	{
		// Cache the selected reference bone.
		GetSelectedRefBone(_selectedRefBone);
		// Cache the selected bone pose.
		GetSelectedBonePose(_selectedBonePose);
		// Cache the selected animation group.
		GetSelectedAnimGroup(_selectedAnimGroup);
		// Cache the selected animation.
		GetSelectedAnim(_selectedAnim);

		ip->DeleteRollupPage(_hActorInfoRollout);
		_hActorInfoRollout       = NULL;
		_hActorLabel             = NULL;
		_hCreateActorButton      = NULL;
		_hLoadActorButton        = NULL;
		_hSaveActorButton        = NULL;
		_hAboutButton			 = NULL;
		_hNormalizeScaleCheckBox = NULL;

		ip->DeleteRollupPage(_hReferencePoseRollout);
		_hReferencePoseRollout = NULL;
		_hRefBonesListBox      = NULL;
		_hImportRefPoseButton  = NULL;
		_hExportRefPoseButton  = NULL;

		ip->DeleteRollupPage(_hBonePosesRollout);
		_hBonePosesRollout           = NULL;
		_hBonePosesListBox           = NULL;
		_hImportBonePoseButton       = NULL;
		_hExportBonePoseButton       = NULL;
		_hBatchImportBonePosesButton = NULL;
		_hBatchExportBonePosesButton = NULL;

#ifndef MOD_DEVELOPER_VERSION
		ip->DeleteRollupPage(_hAnimationsRollout);
		_hAnimationsRollout      = NULL;
		_hAnimationGroupComboBox = NULL;
		_hAnimationsListBox      = NULL;
		_hImportAnimationButton  = NULL;
#endif
	}
}

// This method is called to delete the utility object allocated by 
// ClassDesc::Create().  This method does nothing because FaceFX uses a 
// single static instance of the plug-in class.
void FxMaxMainPanel::DeleteThis( void )
{
}

// Updates all of the controls.
void FxMaxMainPanel::UpdateAll( void )
{
	UpdateActorLabel();
	UpdateRefBonesListBox();
	UpdateBonePosesListBox();
	UpdateAnimationGroupComboBox();
	UpdateAnimationsListBox();
	UpdateNormalizeScaleCheckBoxBox();
}

// Updates the actor label.
void FxMaxMainPanel::UpdateActorLabel( void )
{
	if( _hActorLabel )
	{
		OC3Ent::Face::FxString actorLabel = "Actor: ";
		actorLabel = OC3Ent::Face::FxString::Concat(actorLabel, 
			OC3Ent::Face::FxMaxData::maxInterface.GetActorName());
		::SetWindowText(_hActorLabel, _T(actorLabel.GetData()));
	}
}

// Updates the reference bones list box.
void FxMaxMainPanel::UpdateRefBonesListBox( void )
{
	if( _hRefBonesListBox )
	{
		_clearListBox(_hRefBonesListBox);

		OC3Ent::Face::FxArray<OC3Ent::Face::FxString> refBones;
		for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumRefBones(); ++i )
		{
			refBones.PushBack(OC3Ent::Face::FxMaxData::maxInterface.GetRefBoneName(i));
		}
		_fillListBox(_hRefBonesListBox, refBones);
	}
}

// Updates the bone poses list box.
void FxMaxMainPanel::UpdateBonePosesListBox( void )
{
	if( _hBonePosesListBox )
	{
		_clearListBox(_hBonePosesListBox);

		_fillListBox(_hBonePosesListBox, 
			         OC3Ent::Face::FxMaxData::maxInterface.GetNodesOfType("FxBonePoseNode"));
	}
}

// Updates the animation group combo box.
void FxMaxMainPanel::UpdateAnimationGroupComboBox( void )
{
	if( _hAnimationGroupComboBox )
	{
		// Clear the combo box.
		while( 0 != ::SendMessage(_hAnimationGroupComboBox, 
								  CB_GETCOUNT,
								  0,
								  0) )
		{
			::SendMessage(_hAnimationGroupComboBox,
					 	  CB_DELETESTRING,
						  0,
						  0);
		}

		// Fill the combo box.
		for( OC3Ent::Face::FxSize i = 0; i < OC3Ent::Face::FxMaxData::maxInterface.GetNumAnimGroups(); ++ i )
		{
			::SendMessage(_hAnimationGroupComboBox,
						  CB_ADDSTRING,
						  0,
						  reinterpret_cast<LPARAM>(_T(OC3Ent::Face::FxMaxData::maxInterface.GetAnimGroupName(i).GetData())));
		}

		// Select the default animation group.
		::SendMessage(_hAnimationGroupComboBox,
					  CB_SELECTSTRING,
					  -1,
					  reinterpret_cast<LPARAM>(_T("Default")));
	}
}

// Updates the animations list box.
void FxMaxMainPanel::UpdateAnimationsListBox( void )
{
	if( _hAnimationsListBox )
	{
		_clearListBox(_hAnimationsListBox);

		// Get the selected animation group.
		OC3Ent::Face::FxString selectedAnimationGroup;
		GetSelectedAnimGroup(selectedAnimationGroup);

		// Fill out the animations list box with the animations contained
		// in the selected animation group.
		OC3Ent::Face::FxSize numAnims = 
			OC3Ent::Face::FxMaxData::maxInterface.GetNumAnims(selectedAnimationGroup);
		OC3Ent::Face::FxArray<OC3Ent::Face::FxString> animations;
		for( OC3Ent::Face::FxSize i = 0; i < numAnims; ++i )
		{
			animations.PushBack(OC3Ent::Face::FxMaxData::maxInterface.GetAnimName(selectedAnimationGroup, i));
		}
		_fillListBox(_hAnimationsListBox, animations);
	}
}
void FxMaxMainPanel::UpdateNormalizeScaleCheckBoxBox( void )
{
	SetNormalizeScale(OC3Ent::Face::FxMaxData::maxInterface.GetNormalizeScale());
}

// Gets normalizeScale. FxTrue if the "Normalize Scale" check box is 
// checked and FxFalse otherwise.
void FxMaxMainPanel::GetNormalizeScale( OC3Ent::Face::FxBool& normalizeScale )
{
	normalizeScale = FxFalse;

	if( _hNormalizeScaleCheckBox )
	{
		if( BST_CHECKED == ::SendMessage(_hNormalizeScaleCheckBox,
										 BM_GETCHECK,
										 0,
										 0) )
		{
			normalizeScale = FxTrue;
		}
	}
}

// Fills out selectedRefBone with the currently selected reference bone or 
// the empty string.
void FxMaxMainPanel::GetSelectedRefBone( OC3Ent::Face::FxString& selectedRefBone )
{
	selectedRefBone = "";

	if( _hRefBonesListBox )
	{
		// Get the selected reference bone.
		LRESULT selectedRefBoneIndex = ::SendMessage(_hRefBonesListBox,
													 LB_GETCURSEL,
													 0,
													 0);
		if( LB_ERR != selectedRefBoneIndex )
		{
			// Get the text for the selected reference bone.
			TCHAR selectedReferenceBone[256] = {0};
			::SendMessage(_hRefBonesListBox,
						  LB_GETTEXT,
						  selectedRefBoneIndex,
						  reinterpret_cast<LPARAM>(selectedReferenceBone));
			selectedRefBone = selectedReferenceBone;
		}
	}
}

// Fills out selectedBonePose with the currently selected bone pose or 
// the empty string.
void FxMaxMainPanel::GetSelectedBonePose( OC3Ent::Face::FxString& selectedBonePose )
{
	selectedBonePose = "";

	if( _hBonePosesListBox )
	{
		// Get the selected bone pose.
		LRESULT selectedBonePoseIndex = ::SendMessage(_hBonePosesListBox,
													  LB_GETCURSEL,
													  0,
													  0);
		if( LB_ERR != selectedBonePoseIndex )
		{
			// Get the text for the selected bone pose.
			TCHAR selBonePose[256] = {0};
			::SendMessage(_hBonePosesListBox,
						  LB_GETTEXT,
						  selectedBonePoseIndex,
						  reinterpret_cast<LPARAM>(selBonePose));
			selectedBonePose = selBonePose;
		}
	}
}

// Fills out selectedAnimGroup with the currently selected animation group 
// or the empty string.
void FxMaxMainPanel::GetSelectedAnimGroup( OC3Ent::Face::FxString& selectedAnimGroup )
{
	selectedAnimGroup = "";

	if( _hAnimationGroupComboBox )
	{
		// Get the selected animation group.
		LRESULT selectedAnimationGroupIndex = 
			::SendMessage(_hAnimationGroupComboBox,
						  CB_GETCURSEL,
						  0,
						  0);
		if( CB_ERR != selectedAnimationGroupIndex )
		{
			// Get the text for the selected animation group.
			TCHAR selectedAnimationGroup[256] = {0};
			::SendMessage(_hAnimationGroupComboBox,
						  CB_GETLBTEXT,
					      selectedAnimationGroupIndex,
						  reinterpret_cast<LPARAM>(selectedAnimationGroup));
			selectedAnimGroup = selectedAnimationGroup;
		}
	}
}

// Fills out selectedAnim with the currently selected animation or the empty
// string.
void FxMaxMainPanel::GetSelectedAnim( OC3Ent::Face::FxString& selectedAnim )
{
	selectedAnim = "";

	if( _hAnimationsListBox )
	{
		// Get the selected animation.
		LRESULT selectedAnimationIndex = ::SendMessage(_hAnimationsListBox,
													   LB_GETCURSEL,
													   0,
													   0);
		if( LB_ERR != selectedAnimationIndex )
		{
			// Get the text for the selected animation.
			TCHAR selectedAnimation[256] = {0};
			::SendMessage(_hAnimationsListBox,
						  LB_GETTEXT,
						  selectedAnimationIndex,
						  reinterpret_cast<LPARAM>(selectedAnimation));
			selectedAnim = selectedAnimation;
		}
	}
}

// Sets the state of the "Normalize Scale" check box.  Passing FxTrue causes
// the check box to be checked; FxFalse causes it to be unchecked.
void FxMaxMainPanel::SetNormalizeScale( OC3Ent::Face::FxBool normalizeScale )
{
	if( _hNormalizeScaleCheckBox )
	{
		if( normalizeScale )
		{
			::SendMessage(_hNormalizeScaleCheckBox,
						  BM_SETCHECK,
						  BST_CHECKED,
						  0);
		}
		else
		{
			::SendMessage(_hNormalizeScaleCheckBox,
						  BM_SETCHECK,
						  BST_UNCHECKED,
						  0);
		}
	}
}

// Sets the currently selected reference bone to selectedRefBone.
void FxMaxMainPanel::SetSelectedRefBone( const OC3Ent::Face::FxString& selectedRefBone )
{
	if( _hRefBonesListBox )
	{
		if( _selectedRefBone.Length() > 0 )
		{
			::SendMessage(_hRefBonesListBox,
						  LB_SELECTSTRING,
						  -1,
						  reinterpret_cast<LPARAM>(_T(selectedRefBone.GetData())));
		}
	}
}

// Sets the currently selected bone pose to selectedBonePose.
void FxMaxMainPanel::SetSelectedBonePose( const OC3Ent::Face::FxString& selectedBonePose )
{
	if( _hBonePosesListBox )
	{
		if( _selectedBonePose.Length() > 0 )
		{
			::SendMessage(_hBonePosesListBox,
						  LB_SELECTSTRING,
						  -1,
						  reinterpret_cast<LPARAM>(_T(selectedBonePose.GetData())));
		}
	}
}

// Sets the currently selected animation group to selectedAnimGroup.
void FxMaxMainPanel::SetSelectedAnimGroup( const OC3Ent::Face::FxString& selectedAnimGroup )
{
	if( _hAnimationGroupComboBox )
	{
		if( _selectedAnimGroup.Length() > 0 )
		{
			::SendMessage(_hAnimationGroupComboBox,
						  CB_SELECTSTRING,
						  -1,
						  reinterpret_cast<LPARAM>(_T(selectedAnimGroup.GetData())));
		}
	}
}

// Sets the currently selected animation to selectedAnim.
void FxMaxMainPanel::SetSelectedAnim( const OC3Ent::Face::FxString& selectedAnim )
{
	if( _hAnimationsListBox )
	{
		if( _selectedAnim.Length() > 0 )
		{
			::SendMessage(_hAnimationsListBox,
						  LB_SELECTSTRING,
						  -1,
						  reinterpret_cast<LPARAM>(_T(selectedAnim.GetData())));
		}
	}
}

// Returns a pointer to the current interface stored in the panel.
Interface* FxMaxMainPanel::GetMaxInterfacePointer( void )
{
	return _pMaxInterface;
}

// Returns a pointer to the current utility interface stored in the panel.
IUtil* FxMaxMainPanel::GetUtilityInterfacePointer( void )
{
	return _pUtilityInterface;
}

// Returns a handle to the actor label.
HWND FxMaxMainPanel::GetActorLabelHandle( void )
{
	return _hActorLabel;
}

// Returns a handle to the create actor button.
HWND FxMaxMainPanel::GetCreateActorButtonHandle( void )
{
	return _hCreateActorButton;
}

// Returns a handle to the load actor button.
HWND FxMaxMainPanel::GetLoadActorButtonHandle( void )
{
	return _hLoadActorButton;
}

// Returns a handle to the save actor button.
HWND FxMaxMainPanel::GetSaveActorButtonHandle( void )
{
	return _hSaveActorButton;
}

// Returns a handle to the about button.
HWND FxMaxMainPanel::GetAboutButtonHandle( void )
{
	return _hAboutButton;
}

// Returns a handle to the normalize scale check box.
HWND FxMaxMainPanel::GetNormalizeScaleCheckBoxHandle( void )
{
	return _hNormalizeScaleCheckBox;
}

// Returns a handle to the reference bones list box.
HWND FxMaxMainPanel::GetRefBonesListBoxHandle( void )
{
	return _hRefBonesListBox;
}

// Returns a handle to the import reference pose button.
HWND FxMaxMainPanel::GetImportRefPoseButtonHandle( void )
{
	return _hImportRefPoseButton;
}

// Returns a handle to the export reference pose button.
HWND FxMaxMainPanel::GetExportRefPoseButtonHandle( void )
{
	return _hExportRefPoseButton;
}

// Returns a handle to the bone poses list box.
HWND FxMaxMainPanel::GetBonePosesListBoxHandle( void )
{
	return _hBonePosesListBox;
}

// Returns a handle to the import bone pose button.
HWND FxMaxMainPanel::GetImportBonePoseButtonHandle( void )
{
	return _hImportBonePoseButton;
}

// Returns a handle to the export bone pose button.
HWND FxMaxMainPanel::GetExportBonePoseButtonHandle( void )
{
	return _hExportBonePoseButton;
}

// Returns a handle to the batch import bone poses button.
HWND FxMaxMainPanel::GetBatchImportBonePosesButtonHandle( void )
{
	return _hBatchImportBonePosesButton;
}

// Returns a handle to the batch export bone poses button.
HWND FxMaxMainPanel::GetBatchExportBonePosesButtonHandle( void )
{
	return _hBatchExportBonePosesButton;
}

// Returns a handle to the animation group combo box.
HWND FxMaxMainPanel::GetAnimationGroupComboBoxHandle( void )
{
	return _hAnimationGroupComboBox;
}

// Returns a handle to the animations list box.
HWND FxMaxMainPanel::GetAnimationsListBoxHandle( void )
{
	return _hAnimationsListBox;
}

// Returns a handle to the import animation button.
HWND FxMaxMainPanel::GetImportAnimationButtonHandle( void )
{
	return _hImportAnimationButton;
}

// Clears the specified list box.
void FxMaxMainPanel::_clearListBox( HWND hListBoxToClear )
{
	if( hListBoxToClear )
	{
		// Clear the list box.
		while( 0 != ::SendMessage(hListBoxToClear, 
								  LB_GETCOUNT,
								  0,
								  0) )
		{
			::SendMessage(hListBoxToClear,
						  LB_DELETESTRING,
						  0,
						  0);
		}
	}
}

// Fills the specified list box.
void FxMaxMainPanel::
_fillListBox( HWND hListBoxToFill, 
		      const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& listBoxStrings )
{
	if( hListBoxToFill )
	{
		for( OC3Ent::Face::FxSize i = 0; i < listBoxStrings.Length(); ++i )
		{
			::SendMessage(hListBoxToFill,
				          LB_ADDSTRING,
				          0,
				          reinterpret_cast<LPARAM>(_T(listBoxStrings[i].GetData())));
		}
	}
}
