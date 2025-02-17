//------------------------------------------------------------------------------
// This is a window containing controls that can be expanded and collapsed.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxExpandableWindow_H__
#define FxExpandableWindow_H__

#include "FxPlatform.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

#define kCollapsedHeight 20
#define kChildIndent 5

// Forward declare the container window class.
class FxExpandableWindowContainer;

//------------------------------------------------------------------------------
// An expandable window.
//------------------------------------------------------------------------------
class FxExpandableWindow : public wxWindow
{
	typedef wxWindow Super;
	WX_DECLARE_CLASS(FxExpandableWindow)
	DECLARE_EVENT_TABLE()
public:
	// Constructor.
	// Expandable windows must always be parented to an 
	// FxExpandableWindowContainer.  Never pass NULL for the parent.
	FxExpandableWindow( wxWindow* parent );
	// Destructor.
	virtual ~FxExpandableWindow();

	// Less-than operator used for sorting the windows.
	FxBool operator<( const FxExpandableWindow& other ) const;

	// Expand the window.
	virtual void Expand( FxBool refresh = FxTrue );
	// Collapse the window.
	virtual void Collapse( FxBool refresh = FxTrue );

	// Handlers for various click events that can be overridden.
	virtual void LeftClickHandler( void );
	virtual void ControlLeftClickHandler( void );
	virtual void ControlShiftLeftClickHandler( void );
	virtual void RightClickHandler( void );

	// Returns FxTrue if the window is currently expanded and FxFalse 
	// otherwise.
	FxBool IsExpanded( void ) const;

	// Hide the window.
	void Hide( void );
	// Unhide the window.
	void UnHide( void );

    // Returns FxTrue if the window is currently hidden and FxFalse
	// otherwise.
	FxBool IsHidden( void ) const;

	// Creates the children.
	virtual void CreateChildren( void );
	// Destroys the children.
	virtual void DestroyChildren( void );

	// Draw the window.
	void Draw( void );

	// Returns the height of the window including all contained child controls.
	FxInt32 GetHeight( void ) const;

protected:
	// Is the window expanded?
	FxBool _isExpanded;

	// Should the children be resized to fit the window?
	FxBool _resizeChildren;

	// The container this expandable window belongs to.
	FxExpandableWindowContainer* _container;

	// Children of the expandable window.
	FxArray<wxWindow*> _children;

	// Handle left mouse clicks in the window.
	void OnLeftClick( wxMouseEvent& event );
	// Handle right mouse clicks in the window.
	void OnRightClick( wxMouseEvent& event );
	// Handle size events.
	void OnSize( wxSizeEvent& event );
};

//------------------------------------------------------------------------------
// An expandable window container.
//------------------------------------------------------------------------------
class FxExpandableWindowContainer : public wxScrolledWindow
{
	typedef wxScrolledWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxExpandableWindowContainer)
	DECLARE_EVENT_TABLE()
public:
	// Constructor.
	FxExpandableWindowContainer( wxWindow* parent = NULL );
	// Destructor.
	virtual ~FxExpandableWindowContainer();

	// Adds the expandable window to the container.
	void AddExpandableWindow( FxExpandableWindow* expandableWindow );
	// Removes the expandable window from the container and destroys it.
	void RemoveExpandableWindow( FxExpandableWindow* expandableWindow );
	// Removes all of the expandable windows from the container, destroying
	// them in the process.
	void RemoveAllExpandableWindows( void );
	// Returns the number of contained expandable windows.
	FxSize GetNumExpandableWindows( void ) const;
	// Returns a pointer to the expandable window at index or NULL if the
	// index is invalid.
	FxExpandableWindow* GetExpandableWindow( FxSize index );

	// Positions all expandable windows.
	void PositionExapandableWindows( void );

	// Sorts all expandable windows.
	void Sort( FxBool ascendingOrder );

	// Expands all expandable windows.
	void ExpandAll( void );
	// Collapses all expandable windows.
	void CollapseAll( void );

	// Returns FxTrue if all expandable windows have been expanded with a 
	// previous call to ExpandAll, otherwise returns FxFalse.
	FxBool AllAreExpanded( void ) const;

	friend class FxExpandableWindow;

protected:
	// The list of contained expandable windows.
	FxArray<FxExpandableWindow*> _expandableWindows;

	// The current height of all contained expandable windows.
	FxInt32 _height;

	// Have all expandable windows been expanded with a call to ExpandAll?
	FxBool _allAreExpanded;

	// The expandable window color.
	static wxColour _expandableWindowColor;
	// The panel background color.
	static wxColour _panelBackgroundColor;
	// The shadow color.
	static wxColour _shadowColor;
	// The highlight color.
	static wxColour _highlightColor;
	// The text color.
	static wxColour _textColor;

	// Draw the window.
	void OnDraw( wxDC& dc );
	// Handle size events.
	void OnSize( wxSizeEvent& event );
	// Handle system color changes on MS Windows.
	void OnSystemColorsChanged( wxSysColourChangedEvent& event );
	// Update the scroll bar.
	void UpdateScrollBar( void );
};

} // namespace Face

} // namespace OC3Ent

#endif
