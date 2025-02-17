//------------------------------------------------------------------------------
// A widget to view data in an FxActor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxActorWidget_H__
#define FxActorWidget_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxActor.h"
#include "FxExpandableWindow.h"
#include "FxWidget.h"
#include "FxSlider.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/listbox.h"
	#include "wx/listctrl.h"
	#include "wx/choice.h"
	#include "wx/imaglist.h"
#endif

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// The bone view.
//------------------------------------------------------------------------------
class FxActorBoneView : public FxExpandableWindow, public FxWidget
{
	typedef FxExpandableWindow Super;
	WX_DECLARE_CLASS(FxActorBoneView);
	DECLARE_EVENT_TABLE()
public:
	// Constructor.
	// Expandable windows must always be parented to an 
	// FxExpandableWindowContainer.  Never pass NULL for the parent.
	FxActorBoneView( wxWindow* parent, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxActorBoneView();

	// Creates the children.
	virtual void CreateChildren( void );
	// Destroys the children.
	virtual void DestroyChildren( void );

	// FxWidget message handlers.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnActorRenamed( FxWidget* sender );

protected:
	// Handles column clicks in the bone list control
	void OnColumnClick( wxListEvent& event );

	// The actor.
	FxActor* _actor;
	// The actor name static text
	wxStaticText* _actorName;
	// The bone list box.
	wxListCtrl* _boneListBox;
};

//------------------------------------------------------------------------------
// The face graph view.
//------------------------------------------------------------------------------
class FxActorFaceGraphView : public FxExpandableWindow, public FxWidget
{
	typedef FxExpandableWindow Super;
	WX_DECLARE_CLASS(FxActorFaceGraphView);
	DECLARE_EVENT_TABLE()
public:
	// Constructor.
	// Expandable windows must always be parented to an 
	// FxExpandableWindowContainer.  Never pass NULL for the parent.
	FxActorFaceGraphView( wxWindow* parent, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxActorFaceGraphView();

	// Creates the children.
	virtual void CreateChildren( void );
	// Destroys the children.
	virtual void DestroyChildren( void );

	// FxWidget message handlers.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnFaceGraphNodeGroupSelChanged( FxWidget* sender, const FxName& selGroup ); 
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnFaceGraphNodeSelChanged(FxWidget* sender, const FxArray<FxName>& selFaceGraphNodeNames, FxBool zoomToSelection);

	// Called from the slider's right click handler to the quick preview slider.
	void ResetQuickPreviewSlider( void );

protected:
	// The actor.
	FxActor* _actor;
	// The face graph node filter drop down.
	wxChoice* _faceGraphNodeGroupDropdown;
	// The face graph node list box.
	wxListCtrl* _faceGraphNodeListBox;
	// The quick preview slider.
	FxSlider* _quickPreviewSlider;
	// The face graph node we're controlling.
	FxFaceGraphNode* _linkedNode;
	// The selected face graph node group.
	FxName _selFaceGraphNodeGroup;
	// The selected face graph nodes.
	FxArray<FxName> _selFaceGraphNodes;
	// Tells the control not to re-issue a selection command.
	FxBool _noDispatchSelection;

	// Handle a selection event in the node group drop down.
	void OnFaceGraphNodeGroupChoiceSelChanged( wxCommandEvent& event );
	// Handle a selection event in the face graph node list box.
	void OnFaceGraphNodeListBoxSelChanged( wxListEvent& event );
	// Handle the slider movement.
	void OnPreviewSliderMoved( wxScrollEvent& event );

	// Sets the face graph node filter selection and the face graph node
	// selection.
	void _setNodeGroupAndNodeSelection( void );
	// Sets the list box selection
	void _setNodeListSelection( void );

	// Handles column clicks in the face graph node list control.
	void OnColumnClick( wxListEvent& event );
};

//------------------------------------------------------------------------------
// The animations view.
//------------------------------------------------------------------------------
class FxActorAnimationsView : public FxExpandableWindow, public FxWidget
{
	typedef FxExpandableWindow Super;
	WX_DECLARE_CLASS(FxActorAnimationsView);
	DECLARE_EVENT_TABLE()
public:
	// Constructor.
	// Expandable windows must always be parented to an 
	// FxExpandableWindowContainer.  Never pass NULL for the parent.
	FxActorAnimationsView( wxWindow* parent, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxActorAnimationsView();

	// Creates the children.
	virtual void CreateChildren( void );
	// Destroys the children.
	virtual void DestroyChildren( void );

	// FxWidget message handlers.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnAnimChanged( FxWidget* sender, const FxName& animGroupName, 
		FxAnim* anim );
	virtual void OnAnimCurveSelChanged( FxWidget* sender, const FxArray<FxName>& selAnimCurveNames );

protected:
	// The actor.
	FxActor* _actor;
	// The anim
	FxAnim* _anim;
	// The animation group drop down.
	wxChoice* _animGroupDropdown;
	// The animation drop down.
	wxChoice* _animDropdown;
	// The animation curve list box.
	wxListCtrl* _animCurveListBox;

	// The name of the currently selected animation group.
	FxName _selAnimGroup;
	// The name of the currently selected animation.
	FxName _selAnim;
	// The names of the currently selected animation curves.
	FxArray<FxName> _selAnimCurves;

	// The image list.
	wxImageList* _imageList;
	// The locked and unlocked bitmaps.
	wxBitmap _lockedBitmap;
	wxBitmap _unlockedBitmap;

	// To flag when the selection was internally generated
	FxBool _noDispatchSelection;

	// Handle a selection event in the animation group drop down.
	void OnAnimGroupSelChanged( wxCommandEvent& event );
	// Handle a selection event in the animation drop down.
	void OnAnimSelChanged( wxCommandEvent& event );
	// Handle a selection event in the animation curve list box.
	void OnAnimCurveListBoxSelChanged( wxListEvent& event );
	// Handle a right-click event in the anim curve list box.
	void OnAnimCurveListBoxItemRightClicked( wxListEvent& event );
	// Handle a column click in the anim curve list box.
	void OnAnimCurveListBoxColumnClick( wxListEvent& event );

	// Handle a menu command to display the curve manager.
	void OnMenuCurveManager( wxCommandEvent& event );

	// Handle a menu command to edit curve properties.
	void OnMenuCurveProperties( wxCommandEvent& event );

	// Sets the animation group, animation, and animation curve selections.
	void _setAnimViewSelection( void );
	// Gets the selected curve names from the list control.
	FxArray<FxName> _getSelectedCurveNamesFromListCtrl( void );
	
	// Fill out the anim drop down.
	void _fillAnimDropdown( void );
	// Fill out the curve list box.
	void _fillCurveListBox( void );

	// Creates the image list.
	void _createImageList( void );
	// Returns a pointer to the currently selected animation.
	FxAnim* _getCurrentAnim( void );
};

//------------------------------------------------------------------------------
// An actor widget.
//------------------------------------------------------------------------------
class FxActorWidget : public FxExpandableWindowContainer, public FxWidget
{
	typedef FxExpandableWindowContainer Super;
	WX_DECLARE_DYNAMIC_CLASS(FxActorWidget)
	DECLARE_EVENT_TABLE()

public:
	// Constructor.
	FxActorWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxActorWidget();

	// FxWidget message handlers.
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnSerializeOptions( FxWidget* sender, wxFileConfig* config, FxBool isLoading );

	void OnHelp( wxHelpEvent& event );

protected:
	// The actor.
	FxActor* _actor;
};

} // namespace Face

} // namespace OC3Ent

#endif

