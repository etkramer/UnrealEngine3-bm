//------------------------------------------------------------------------------
// The render widget for OC3's internal engine FxRenderer.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRenderWidgetOC3_H__
#define FxRenderWidgetOC3_H__

#ifdef USE_FX_RENDERER

#include "FxRenderWidget.h"
#include "FxRenderWindowWxOGL.h"
#include "FxConsoleVariable.h"
#include "FxButton.h"

#ifdef WIN32
	#include "FxRendererOGLOffscreenTargetWin32.h"
#endif // WIN32

namespace OC3Ent
{

namespace Face
{

class FxRendererContainerWindow : public wxWindow
{
	typedef wxWindow Super;
	WX_DECLARE_DYNAMIC_CLASS(FxRendererContainerWindow)
	DECLARE_EVENT_TABLE()

public:
	// Constructor.
	FxRendererContainerWindow( wxWindow* parent = NULL );
	// Destructor.
	virtual ~FxRendererContainerWindow();

	// Sets the contained render window pointer.
	void SetRenderWindow( FxRenderWindowWxOGL* renderWindow );

protected:
	// The render window.
	FxRenderWindowWxOGL* _renderWindow;

	// Handle size events.
	void OnSize( wxSizeEvent& event );

	void OnHelp( wxHelpEvent& event );
};

class FxRenderWidgetOC3OffscreenViewport : public FxRenderWidgetOffscreenViewport
{
	typedef FxRenderWidgetOffscreenViewport Super;
	// Disable copy construction and assignment.
    FX_NO_COPY_OR_ASSIGNMENT(FxRenderWidgetOC3OffscreenViewport);
public:
	// Constructor.
	FxRenderWidgetOC3OffscreenViewport();
	// Destructor.
	virtual ~FxRenderWidgetOC3OffscreenViewport();

	// Returns the render target size.
	virtual void GetRenderTargetSize( FxInt32& width, FxInt32& height );
	// Sets the render target size.
	virtual void SetRenderTargetSize( FxInt32 width, FxInt32 height );

	// Renders the view using the specified camera.
	virtual void Render( const FxName& cameraToUse );

	// Returns the image rendered.
	virtual const FxByte* GetImage( void ) const;

protected:
#ifdef WIN32
	// The offscreen render target.
	FxRendererOGLOffscreenTargetWin32* _pOffscreenRenderTarget;
#else
	// Currently on non-Win32 platforms this is dormant and only here to allow
	// the code to compile (it is never allocated or constructed).
	FxRenderTarget* _pOffscreenRenderTarget;
#endif
};

// The OC3 render window notification hook used to propagate rendering state
// changes to all open viewports.
class FxRenderWidgetOC3NotifyHook : public FxRenderWindowNotifyHook
{
	virtual void NotifyRenderStateChanged( void );
};

class FxRenderWidgetOC3 : public FxRenderWidget
{
	typedef FxRenderWidget Super;
	WX_DECLARE_DYNAMIC_CLASS(FxRenderWidgetOC3)
	DECLARE_EVENT_TABLE()

public:
	// Constructor.
	FxRenderWidgetOC3( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxRenderWidgetOC3();

	// Required FxWidget message handlers from the FxRenderWidget interface.
	virtual void OnAppStartup( FxWidget* sender );
	virtual void OnAppShutdown( FxWidget* sender );
	virtual void OnCreateActor( FxWidget* sender );
	virtual void OnLoadActor( FxWidget* sender, const FxString& actorPath );
	virtual void OnCloseActor( FxWidget* sender );
	virtual void OnSaveActor( FxWidget* sender, const FxString& actorPath );
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime );
	virtual void OnRefresh( FxWidget* sender );
	virtual void OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps );

	// Callback for when the rw_showbindpose console variable is updated.
	static void OnShowBindPoseUpdated( FxConsoleVariable& cvar );
	// Callback for when the rw_scalefactor console variable is updated.
	static void OnScaleFactorUpdated( FxConsoleVariable& cvar );

	// Sets the internal scene pointer.  Note this is never called in Studio.
	// It is only called from external tools containing a render widget.
	// When you call this it will simply swap the internal _pScene pointer with
	// the passed in pScene pointer and force an immediate render (if pScene is
	// non-NULL).  The old _pScene is not deleted or destroyed.  If the render 
	// widget shuts down _pScene will be destroyed internally, so make sure you 
	// know exactly how that affects what you're doing.  For example, if you are 
	// passing in a scene pointer that you are controlling / changing from 
	// elsewhere, you should set the internal scene pointer to NULL before
	// shutting down the render widget.
	void SetScenePtr( FxScene* pScene );
	// Returns the internal scene pointer.  Note this is never called in Studio,
	// same as SetScenePtr().
	FxScene* GetScenePtr( void );

protected:
	// The render window.
	FxRenderWindowWxOGL* _renderWindow;
	// The renderer's container window.
	FxRendererContainerWindow* _containerWindow;
	// Tool bar controls.
	FxButton* _toolbarResetViewButton;
	FxButton* _toolbarToggleWireframeRenderingButton;
	FxButton* _toolbarToggleLightingButton;
	FxButton* _toolbarToggleMeshRenderingButton;
	FxButton* _toolbarToggleNormalsRenderingButton;
	FxButton* _toolbarToggleInfluencesRenderingButton;
	FxButton* _toolbarToggleSkeletonRenderingButton;
	FxButton* _toolbarCreateCameraButton;
	FxButton* _toolbarDeleteCameraButton;
	wxChoice* _toolbarCameraChoice;
	// The rw_showbindpose console variable.
	FxConsoleVariable* rw_showbindpose;
	// The rw_scalefactor console variable.
	FxConsoleVariable* rw_scalefactor;
	// FxTrue if the render widget is ready to draw.
	FxBool _readyToDraw;
	// The notification hook.
	FxRenderWidgetOC3NotifyHook _notifyHook;
	// The path to the .fxr file.
	FxString _fxrPath;
	// The tool bar rect.
	wxRect _toolbarRect;
	// Cached system colors.
	wxColour _colourFace;
	wxColour _colourShadow;

	// Handle size events.
	void OnSize( wxSizeEvent& event );
	// Handles paint events.
	void OnPaint( wxPaintEvent& event );
	// Handles system color changes.
	void OnSystemColoursChanged( wxSysColourChangedEvent& event );
	// Handle tool bar button presses.
	void OnResetViewButton( wxCommandEvent& event );
	void OnToggleWireframeRenderingButton( wxCommandEvent& event );
	void OnToggleLightingButton( wxCommandEvent& event );
	void OnToggleMeshRenderingButton( wxCommandEvent& event );
	void OnToggleNormalsRenderingButton( wxCommandEvent& event );
	void OnToggleInfluencesRenderingButton( wxCommandEvent& event );
	void OnToggleSkeletonRenderingButton( wxCommandEvent& event );
	void OnCreateCameraButton( wxCommandEvent& event );
	void OnDeleteCameraButton( wxCommandEvent& event );
	void OnCameraChoiceChange( wxCommandEvent& event );
	// Updates the tool bar state.
	void UpdateToolbarState( wxUpdateUIEvent& event );
	// Draw the tool bar.
	void DrawToolbar( wxDC* pDC );
	// Grabs the system colors.
	void _refreshSystemColors( void );
	// Refreshes the children.
	void _refreshChildren( void );
	// Resizes the component windows.
	void _resizeComponents( void );
	// Links up the actor to bones in the current scene.
	void _linkupBones( FxBool& generatedWarnings );
	// Updates the actor.
	void _updateActor( void );
	// Updates any cameras in the scene that are "bound" to bones.
	void _updateBoundCameras( void );
	// Sets up default lighting for the scene.
	void _setupDefaultLighting( void );

	// Loads the icons for the tool bar.
	void _loadIcons( void );

	FxScene* _pScene;
};

} // namespace Face

} // namespace OC3Ent

#endif // USE_FX_RENDERER

#endif
