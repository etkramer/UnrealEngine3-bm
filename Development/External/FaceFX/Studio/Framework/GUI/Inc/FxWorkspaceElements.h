//------------------------------------------------------------------------------
// Definitions for elements that can be contained in a workspace.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxWorkspaceElements_H__
#define FxWorkspaceElements_H__

#include "FxVec2.h"
#include "FxRenderWidget.h"

#ifdef __UNREAL__
	#include "XWindow.h"
#else
	#include "wx/image.h"
#endif

namespace OC3Ent
{

namespace Face
{

// Forward-declare the face graph node.
class FxFaceGraphNode;

// Refresh flags
enum FxWorkspaceRefreshFlags
{
	WRF_None = 0,
	WRF_Workspaces = 1,
	WRF_App = 2,
	WRF_All = 3
};

//------------------------------------------------------------------------------
// FxWorkspaceElement
//
// A control or graphical object that is placeable on a workspace.
//------------------------------------------------------------------------------
class FxWorkspaceElement
{
public:
	enum HitResult
	{
		HR_Failed = 0,		// Did not hit the element.
		HR_Passed = 1,		// Hit the element.
		HR_ScaleN,			// Hit the element's north scaling point.
		HR_ScaleS,			// Hit the element's south scaling point.
		HR_ScaleE,			// Hit the element's east scaling point.
		HR_ScaleW,			// Hit the element's west scaling point.
		HR_ScaleNE,			// Hit the element's north-east scaling point.
		HR_ScaleSE,			// Hit the element's south-east scaling point.
		HR_ScaleNW,			// Hit the element's north-west scaling point.
		HR_ScaleSW,			// Hit the element's south-west scaling point.
		HR_Rotate,			// Hit the element's rotation point.
		HR_Translate		// Hit the element's translation point.
	};

	// Default constructor
	FxWorkspaceElement();

	// Sets the workspace rectangle.
	virtual void SetWorkspaceRect(const wxRect& workspaceRect);
	virtual const wxRect& GetWorkspaceRect() const;

	// Returns the position of the element.
	virtual const FxVec2& GetPosition() const;
	// Returns the scale of the element.
	virtual const FxVec2& GetScale() const;
	// Returns the rotation of the element.
	virtual FxReal GetRotation() const;

	// Sets the position of the element.
	virtual void SetPosition(FxReal x, FxReal y, FxInt32 snap = -1);
	virtual void SetPosition(const FxVec2& pos, FxInt32 snap = -1);
	// Sets the scale of the element.
	virtual void SetScale(FxReal x, FxReal y, FxInt32 xsnap = -1, FxInt32 ysnap = -1);
	virtual void SetScale(const FxVec2& scale, FxInt32 xsnap = -1, FxInt32 ysnap = -1);
	// Sets the rotation of the element, in degrees from zero.
	virtual void SetRotation(FxReal rot, FxInt32 snap = -1);

	// Draw the element on a device context.
	virtual void DrawOn(wxDC* pDC, FxBool editMode = FxFalse, FxBool selected = FxFalse);
	// Process a mouse message.
	virtual FxBool ProcessMouse(wxMouseEvent& event, FxInt32& needsRefresh, FxBool editMode = FxFalse);

	// Hit test a workspace point on the element.
	virtual HitResult HitTest(const wxPoint& pt, FxBool editMode = FxFalse);

	// Called when the element's transform changes.
	virtual void OnTransformChanged();

	// Returns the center of the element, in workspace coordinates.
	wxPoint GetCenter();

protected:
	// Drawing
	virtual void DrawControlHandles(wxDC* pDC, FxBool selected);

	// Returns true if the mouse is over a control point.
	virtual FxBool IsNearControl(const wxPoint& controlPoint, const wxPoint mousePoint);

	// Point transformation
	virtual wxPoint ElementToWorkspace(const FxVec2& elemCoord);
	virtual FxVec2  WorkspaceToElement(const wxPoint& workspaceCoord);
	virtual FxVec2  WorkspaceToElement(const wxPoint& workspaceCoord, const FxVec2& centre);

	// Element properties.
	wxRect _workspaceRect; // The workspace bounds.
	FxBool _canRotate;	   // True if an element can be rotated.

	FxVec2 _pos;	// The position of the element.
	FxVec2 _scale;	// The scale of the element.
	FxReal _rot;	// The rotation of the element.

	// Cached derived elements.
	FxReal _sinRot;	// The sin of the rotation of the element.
	FxReal _cosRot; // The cos of the rotation of the element.
	FxReal _sinNegRot; // The sin of the negative rotation of the element.
	FxReal _cosNegRot; // The cos of the negative rotation of the element.

	wxPoint _tl;	// The top left corner of the bounding box. (only in theory)
	wxPoint _tr;	// The top right corner of the bounding box. (only in theory)
	wxPoint _br;	// The bottom right corner of the bounding box. (only in theory)
	wxPoint _bl;	// The bottom left corner of the bounding box. (only in theory)

	wxPoint _tc;	// The centre of the top of the bounding box.
	wxPoint _bc;	// The centre of the bottom of the bounding box.
	wxPoint _rc;	// The centre of the right of the bounding box.
	wxPoint _lc;	// The centre of the left of the bounding box.

	wxPoint _sc;	// The control spin point.
	wxPoint _mc;	// The control move point.

	enum MouseAction
	{
		MA_None = 0,
		MA_Translate = 1,
		MA_Rotate = 2,
		MA_ScaleN = 3,
		MA_ScaleS,
		MA_ScaleE,
		MA_ScaleW,
		MA_ScaleNE,
		MA_ScaleNW,
		MA_ScaleSW,
		MA_ScaleSE
	};
	MouseAction _mouseAction;

	FxVec2 _cachePos;
	FxVec2 _cacheScale;
};

//------------------------------------------------------------------------------
// FxWorkspaceElementSlider
//
// A multi-dimensional slider shown on a workspace.
//------------------------------------------------------------------------------

// Forward declare a few classes.
class FxFaceGraph;
class FxAnim;
class FxAnimCurve;
class FxAnimController;

enum FxWorkspaceMode
{
	WM_Combiner  = 0,
	WM_Animation = 1,
	WM_Mapping   = 2
};

class FxWorkspaceElementSlider : public FxWorkspaceElement
{
public:
	// Default constructor.
	FxWorkspaceElementSlider();

	// Returns the name of the node the x-axis controls.
	const FxString& GetNodeNameXAxis() const;
	// Returns the name of the node the y-axis controls.
	const FxString& GetNodeNameYAxis() const;

	// Sets the name of the node the x-axis controls.
	void SetNodeNameXAxis(const FxString& nodeNameX);
	// Sets the name of the node the y-axis controls.
	void SetNodeNameYAxis(const FxString& nodeNameY);

	// Returns the command to key the x value of the slider.
	FxString BuildKeyXCommand();
	// Returns the command to key the y value of the slider.
	FxString BuildKeyYCommand();

	// Links the slider to a nodes.
	void LinkToNodes(FxFaceGraph* pFaceGraph);
	// Links the slider to an anim.
	void LinkToAnim(FxAnim* pAnim);
	// Links the slider to an anim controller.
	void LinkToAnimController(FxAnimController* pAnimController);
	// Sets the current time.
	void SetCurrentTime(FxReal currentTime);

	// Draw the slider.
	virtual void DrawOn(wxDC* pDC, FxBool editMode = FxFalse, FxBool selected = FxFalse);
	// Process a mouse movement.
	virtual FxBool ProcessMouse(wxMouseEvent& event, FxInt32& needsRefresh, FxBool editMode = FxFalse);
	// Process a key event.
	FxBool ProcessKeyDown(wxKeyEvent& event, FxInt32& needsRefresh, FxBool editMode = FxFalse);

	// Update the slider from the nodes/anim curves
	void UpdateSlider();

	// Sets the slider mode.
	void SetMode(FxWorkspaceMode mode);
	// Sets the slider option.
	void SetOption(FxBool autoKey);
	// Sets the highlight flag.
	void SetHighlight(FxBool highlight);
	// Returns the highlight flag.
	FxBool GetHighlight();

	friend FxArchive& operator<<(FxArchive& arc, FxWorkspaceElementSlider& obj);

protected:
	// Set the node values
	void SetNodeValues(FxReal valX, FxReal valY);
	// Set a key value, when in animation mode.
	void OnSetKey();
	// Add the curves, if necessary.
	void EnsureCurvesExist();
	// Apply the buffer to the node
	void ApplyBufferChanges();

	FxString		  _nodeNameX;	// The name of the node the x-axis controls.
	FxString		  _nodeNameY;	// The name of the node the y-axis controls.
	FxFaceGraph*	  _faceGraph;	// The face graph we control.
	FxFaceGraphNode*  _nodeLinkX;	// The node the x-axis controls.
	FxFaceGraphNode*  _nodeLinkY;	// The node the y-axis controls.
	FxAnim*			  _anim;		// The anim the slider controls.
	FxAnimCurve*	  _animLinkX;	// The anim curve the x-axis controls.
	FxAnimCurve*	  _animLinkY;	// The anim curve the y-axis controls.
	FxInt32			  _curveIndexX; // The curve index of the x-axis.
	FxInt32			  _curveIndexY; // The curve index of the y-axis.
	FxAnimController* _animController; // The anim controller.
	FxReal			  _currentTime;	// The current time.
	FxReal			  _originalOffsetXValue;
	FxReal			  _originalOffsetYValue;
	FxReal			  _originalXValue;
	FxReal			  _originalYValue;
	FxBool			  _cachedXValue;
	FxBool			  _cachedYValue;
	FxWorkspaceMode	  _mode;
	FxBool			  _keyExistsX;
	FxBool			  _keyExistsY;

	FxReal			  _valX;		// The x-axis value of the slider [0,1]
	FxReal			  _valY;		// The y-axis value of the slider [0,1]

	FxBool			  _autoKey;		// Whether or not to auto-key the slider.
	FxBool			  _mousing;		// True when changing the slider value.
	FxBool			  _highlight;	// True if we should highlight the slider.

	FxBool			  _typeMode;	// True when the user is typing stuff.
	FxBool			  _typingX;		// True if the user is editing the x-axis, false otherwise.
	wxString		  _buffer;		// The character buffer.
	FxInt32			  _bufferPos;	// The position in the buffer.
};

//------------------------------------------------------------------------------
// FxWorkspaceElementBackground
//
// A bitmapped background shown over a workspace.
//------------------------------------------------------------------------------
class FxWorkspaceElementBackground : public FxWorkspaceElement
{
public:
	// Default constructor.
	FxWorkspaceElementBackground();
	// Construct from an image.
	FxWorkspaceElementBackground(const wxImage& image);

	// Copy ctor.
	FxWorkspaceElementBackground(const FxWorkspaceElementBackground& other);
	// Assignment.
	FxWorkspaceElementBackground& operator=(const FxWorkspaceElementBackground& other);

	// Destructor.
	~FxWorkspaceElementBackground();

	// Set the background image.
	void SetImage(const wxImage& image);

	// Draw the element on a device context.
	virtual void DrawOn(wxDC* pDC, FxBool editMode = FxFalse, FxBool selected = FxFalse);

	// Called when the element's transform changes to pre-compute a correctly
	// scaled bitmap.
	virtual void OnTransformChanged();

	friend FxArchive& operator<<(FxArchive& arc, FxWorkspaceElementBackground& obj);

protected:
	FxUChar* _imageData;	// The image data.
	FxSize   _imageDataSize;// The size of the image data.
	wxImage  _original;		// The original image.
	wxBitmap _bitmap;		// The bitmap to draw - properly scaled, rotated, etc.
};

//------------------------------------------------------------------------------
// FxWorkspaceElementViewport
//
// A real-time viewport.
//------------------------------------------------------------------------------
class FxWorkspaceElementViewport : public FxWorkspaceElement
{
public:
	FxWorkspaceElementViewport();
	FxWorkspaceElementViewport(const FxWorkspaceElementViewport& other);
	FxWorkspaceElementViewport& operator=(const FxWorkspaceElementViewport& other);
	~FxWorkspaceElementViewport();

	virtual void DrawOn(wxDC* pDC, FxBool editMode = FxFalse, FxBool selected = FxFalse);

	virtual void OnTransformChanged();

	// Returns the camera name associated with the viewport.
	const FxName& GetCameraName() const;
	// Sets the camera name associated with the viewport.
	void SetCameraName(const FxName& cameraName);

	friend FxArchive& operator<<(FxArchive& arc, FxWorkspaceElementViewport& obj);

protected:
	// The offscreen viewport.
	FxRenderWidgetOffscreenViewport* _pOffscreenViewport;
	// The name of the camera the offscreen viewport uses to render.
	FxName _cameraName;
};

} // namespace Face

} // namespace OC3Ent

#endif
