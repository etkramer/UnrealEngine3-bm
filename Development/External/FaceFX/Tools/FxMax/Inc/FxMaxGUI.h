//------------------------------------------------------------------------------
// The User Interface code for the FaceFx Max plug-in.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMaxGUI_H__
#define FxMaxGUI_H__

#include "FxMaxMain.h"
#include "FxMaxData.h"

// The class description of the FaceFX Max plug-in.
class FxMaxClassDesc : public ClassDesc2 
{
public:
	// Controls if the plug-in shows up in lists from the user to choose from.
	int IsPublic( void );
	// Max calls this method when it needs a pointer to a new instance of the 
	// plug-in class.
	void* Create( BOOL loading = FALSE );
	// Returns the name of the class.
	const TCHAR* ClassName( void );
	// Returns a system defined constant describing the class this plug-in 
	// class was derived from.
	SClass_ID SuperClassID( void );
	// Returns the unique ID for the object.
	Class_ID ClassID( void );
	// Returns a string describing the category the plug-in fits into.
	const TCHAR* Category( void );

	// Returns a string which provides a fixed, machine parsable internal name 
	// for the plug-in.  This name is used by MAXScript.
	const TCHAR* InternalName( void );
	// Returns the DLL instance handle of the plug-in.
	HINSTANCE HInstance( void );	
};

// The selection dialog callback for selecting the reference bones.
class FxReferenceBonesSelectionDialogCallback : public HitByNameDlgCallback
{
public:
	// Returns the title string displayed in the dialog.
	virtual TCHAR* dialogTitle( void );
	// Returns the text displayed in the 'Select' or 'Pick' button.
	virtual TCHAR* buttonText( void );
	// Returns TRUE if the user may only make a single selection in the list at 
	// one time, FALSE otherwise.
	virtual BOOL singleSelect( void );
	// This gives the callback the opportunity to filter out items from the 
	// list.  This is called before the dialog is presented.  It returns TRUE 
	// if the filter() method (below) should be called, FALSE otherwise.
	virtual BOOL useFilter( void );
	// This method will be called if useFilter() above returned TRUE.  This 
	// gives the callback the chance to filter out items from the list before 
	// they are presented to the user in the dialog.  This is called once for 
	// each node that would otherwise be presented.  Return nonzero to accept 
	// the item and zero to skip it.
	virtual int filter( INode* pNode );
	// Normally, when the user selects OK from the dialog, the system selects 
	// all the chosen nodes in the scene.  At times a developer may want to do 
	// something other than select the chosen nodes.  If this method returns 
	// TRUE then the nodes in the list will not be selected, but the proc() 
	// method is called instead (see below).  If this method returns FALSE, 
	// then the nodes are selected in the scene and proc() is not called.
	virtual BOOL useProc( void );
	// This allows the plug-in to process the nodes chosen from the dialog in 
	// any manner.  For example if the developer wanted to delete the nodes 
	// chosen using this dialog, they would implement useProc() to return TRUE 
	// and this method to delete all the nodes in the table passed.
	virtual void proc( INodeTab& nodeTab );
	// Normally, when the dialog is entered, the nodes in the scene that are 
	// selected are highlighted in the list.  If this method returns TRUE, the 
	// developer may control which items are highlighted by implementing 
	// doHilite() (see below).  If this method returns FALSE the selected nodes 
	// will have their names highlighted in the list.
	virtual BOOL doCustomHilite( void );
	// This method is called for each item in the list if doCustomHilite() 
	// returns TRUE.  This method returns TRUE or FALSE to control if each item 
	// is highlighted.
	virtual BOOL doHilite( INode* pNode );
	// This defaults to returning FALSE, which means that hidden and frozen 
	// objects will not be included in the select by name dialog list.  If this 
	// method is overridden to return TRUE, then hidden and frozen nodes will be 
	// sent through the user-supplied filter.
	virtual BOOL showHiddenAndFrozen( void );
};

// The main GUI panel for the FaceFX Max plug-in.
class FxMaxMainPanel : public UtilityObj 
{
public:
	// Constructor.
	FxMaxMainPanel();
	// Destructor.
	virtual ~FxMaxMainPanel();

	// This method is called when the utility plug-in may be used in the 
	// Utility branch of the command panel.  Roll-up pages are added to the 
	// panel here.
	void BeginEditParams( Interface* ip, IUtil* iu );
	// This method is called when the utility plug-in is done being used in the 
	// Utility branch of the command panel (for example if the user changes to 
	// the Create branch).  Roll-up pages are destroyed here.
	void EndEditParams( Interface* ip, IUtil* iu );

	// This method is called to delete the utility object allocated by 
	// ClassDesc::Create().  This method does nothing because FaceFX uses a 
	// single static instance of the plug-in class.
	void DeleteThis( void );

	// Updates all of the controls.
	void UpdateAll( void );
	// Updates the actor label.
	void UpdateActorLabel( void );
	// Updates the reference bones list box.
	void UpdateRefBonesListBox( void );
	// Updates the bone poses list box.
	void UpdateBonePosesListBox( void );
	// Updates the animation group combo box.
	void UpdateAnimationGroupComboBox( void );
	// Updates the animations list box.
	void UpdateAnimationsListBox( void );
	// Updates the Normalize Scale CheckBox.
	void UpdateNormalizeScaleCheckBoxBox( void );

	// Gets normalizeScale.  FxTrue if the "Normalize Scale" check box is 
	// checked and FxFalse otherwise.
	void GetNormalizeScale( OC3Ent::Face::FxBool& normalizeScale );
	// Fills out selectedRefBone with the currently selected reference bone or 
	// the empty string.
	void GetSelectedRefBone( OC3Ent::Face::FxString& selectedRefBone );
	// Fills out selectedBonePose with the currently selected bone pose or 
	// the empty string.
	void GetSelectedBonePose( OC3Ent::Face::FxString& selectedBonePose );
	// Fills out selectedAnimGroup with the currently selected animation group 
	// or the empty string.
	void GetSelectedAnimGroup( OC3Ent::Face::FxString& selectedAnimGroup );
	// Fills out selectedAnim with the currently selected animation or the empty
	// string.
	void GetSelectedAnim( OC3Ent::Face::FxString& selectedAnim );

	// Sets the state of the "Normalize Scale" check box.  Passing FxTrue causes
	// the check box to be checked; FxFalse causes it to be unchecked.
	void SetNormalizeScale( OC3Ent::Face::FxBool normalizeScale );
	// Sets the currently selected reference bone to selectedRefBone.
	void SetSelectedRefBone( const OC3Ent::Face::FxString& selectedRefBone );
	// Sets the currently selected bone pose to selectedBonePose.
	void SetSelectedBonePose( const OC3Ent::Face::FxString& selectedBonePose );
	// Sets the currently selected animation group to selectedAnimGroup.
	void SetSelectedAnimGroup( const OC3Ent::Face::FxString& selectedAnimGroup );
	// Sets the currently selected animation to selectedAnim.
	void SetSelectedAnim( const OC3Ent::Face::FxString& selectedAnim );

	// Returns a pointer to the current interface stored in the panel.
	Interface* GetMaxInterfacePointer( void );
	// Returns a pointer to the current utility interface stored in the panel.
	IUtil* GetUtilityInterfacePointer( void );

	// Returns a handle to the actor label.
	HWND GetActorLabelHandle( void );
	// Returns a handle to the create actor button.
	HWND GetCreateActorButtonHandle( void );
	// Returns a handle to the load actor button.
	HWND GetLoadActorButtonHandle( void );
	// Returns a handle to the save actor button.
	HWND GetSaveActorButtonHandle( void );
	// Returns a handle to the about button.
	HWND GetAboutButtonHandle( void );
	// Returns a handle to the normalize scale check box.
	HWND GetNormalizeScaleCheckBoxHandle( void );
	// Returns a handle to the reference bones list box.
	HWND GetRefBonesListBoxHandle( void );
	// Returns a handle to the import reference pose button.
	HWND GetImportRefPoseButtonHandle( void );
	// Returns a handle to the export reference pose button.
	HWND GetExportRefPoseButtonHandle( void );
	// Returns a handle to the bone poses list box.
	HWND GetBonePosesListBoxHandle( void );
	// Returns a handle to the import bone pose button.
	HWND GetImportBonePoseButtonHandle( void );
	// Returns a handle to the export bone pose button.
	HWND GetExportBonePoseButtonHandle( void );
	// Returns a handle to the batch import bone poses button.
	HWND GetBatchImportBonePosesButtonHandle( void );
	// Returns a handle to the batch export bone poses button.
	HWND GetBatchExportBonePosesButtonHandle( void );
	// Returns a handle to the animation group combo box.
	HWND GetAnimationGroupComboBoxHandle( void );
	// Returns a handle to the animations list box.
	HWND GetAnimationsListBoxHandle( void );
	// Returns a handle to the import animation button.
	HWND GetImportAnimationButtonHandle( void );

protected:
	// A pointer to the current Max interface.
	Interface* _pMaxInterface;
	// A pointer to the current utility interface.
	IUtil* _pUtilityInterface;

	OC3Ent::Face::FxString _selectedRefBone;
	OC3Ent::Face::FxString _selectedBonePose;
	OC3Ent::Face::FxString _selectedAnimGroup;
	OC3Ent::Face::FxString _selectedAnim;
	
	// A handle to the actor information roll out panel.
	HWND _hActorInfoRollout;
	// A handle to the reference pose roll out panel.
	HWND _hReferencePoseRollout;
	// A handle to the bone poses roll out panel.
	HWND _hBonePosesRollout;
	// A handle to the animations roll out panel.
	HWND _hAnimationsRollout;

	// The actor label.
	HWND _hActorLabel;
	// The create actor button.
	HWND _hCreateActorButton;
	// The load actor button.
	HWND _hLoadActorButton;
	// The save actor button.
	HWND _hSaveActorButton;
	// The about button.
	HWND _hAboutButton;
	// The normalize scale check box.
	HWND _hNormalizeScaleCheckBox;
	
	// The reference bones list box.
	HWND _hRefBonesListBox;
	// The import reference pose button.
	HWND _hImportRefPoseButton;
	// The export reference pose button.
	HWND _hExportRefPoseButton;

	// The bone poses list box.
	HWND _hBonePosesListBox;
	// The import bone pose button.
	HWND _hImportBonePoseButton;
	// The export bone pose button.
	HWND _hExportBonePoseButton;
	// The batch import bone poses button.
	HWND _hBatchImportBonePosesButton;
	// The batch export bone poses button.
	HWND _hBatchExportBonePosesButton;
	
	// The animation group combo box.
	HWND _hAnimationGroupComboBox;
	// The animations list box.
	HWND _hAnimationsListBox;
	// The import animation button.
	HWND _hImportAnimationButton;

	// Clears the specified list box.
	void _clearListBox( HWND hListBoxToClear );
	// Fills the specified list box.
	void _fillListBox( HWND hListBoxToFill, 
		               const OC3Ent::Face::FxArray<OC3Ent::Face::FxString>& listBoxStrings );
};


// Updates the FaceFX Panel for Max script functions.
void UpdateMainFaceFxPanel( void );

#endif
