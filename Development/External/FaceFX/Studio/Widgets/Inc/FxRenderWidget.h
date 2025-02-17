//------------------------------------------------------------------------------
// A render window widget.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRenderWidget_H__
#define FxRenderWidget_H__

#include "FxPlatform.h"
#include "FxWidget.h"

namespace OC3Ent
{

namespace Face
{

// Abstract render widget offscreen viewport class.  Derive from this class
// and apply the correct render widget capabilities in OnQueryRenderWidgetCaps()
// to enable real-time viewports in work spaces.
class FxRenderWidgetOffscreenViewport
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxRenderWidgetOffscreenViewport);
public:
	// Constructor.
	FxRenderWidgetOffscreenViewport() {};
	// Destructor.
	virtual ~FxRenderWidgetOffscreenViewport() {};

	// Returns the render target size.
	virtual void GetRenderTargetSize( FxInt32& width, FxInt32& height ) = 0;
	// Sets the render target size.
	virtual void SetRenderTargetSize( FxInt32 width, FxInt32 height ) = 0;
	
	// Renders the view using the specified camera.
	virtual void Render( const FxName& cameraToUse ) = 0;

	// Returns the image rendered.
	virtual const FxByte* GetImage( void ) const = 0;
};

// Abstract render widget class.
class FxRenderWidget : public wxWindow, public FxWidget
{
	typedef wxWindow Super;
	WX_DECLARE_ABSTRACT_CLASS(FxRenderWidget)

public:
	// Constructor.
	FxRenderWidget( wxWindow* parent = NULL, FxWidgetMediator* mediator = NULL );
	// Destructor.
	virtual ~FxRenderWidget();

	// FxWidget message handlers.
	virtual void OnAppStartup( FxWidget* sender ) = 0;
	virtual void OnAppShutdown( FxWidget* sender ) = 0;
	virtual void OnCreateActor( FxWidget* sender ) = 0;
	virtual void OnLoadActor( FxWidget* sender, const FxString& actorPath ) = 0;
	virtual void OnCloseActor( FxWidget* sender ) = 0;
	virtual void OnSaveActor( FxWidget* sender, const FxString& actorPath ) = 0;
	virtual void OnActorChanged( FxWidget* sender, FxActor* actor );
	virtual void OnActorInternalDataChanged( FxWidget* sender, FxActorDataChangedFlag flags );
	virtual void OnTimeChanged( FxWidget* sender, FxReal newTime ) = 0;
	virtual void OnRefresh( FxWidget* sender ) = 0;
	// All derived render widgets must correctly fill out the renderWidgetCaps structure according
	// to their implementations.
	virtual void OnQueryRenderWidgetCaps( FxWidget* sender, FxRenderWidgetCaps& renderWidgetCaps ) = 0;

protected:
    // The actor.  Set this in OnActorChanged().
	FxActor* _actor;
};

} // namespace Face

} // namespace OC3Ent

#endif
