/*=============================================================================
	UnClient.h: Interface definition for platform specific client code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _UN_CLIENT_
#define _UN_CLIENT_

/**
 * A render target.
 */
class FRenderTarget
{
public:

	// Destructor
	virtual ~FRenderTarget(){}

	/**
	* Accessor for the surface RHI when setting this render target
	* @return render target surface RHI resource
	*/
	virtual const FSurfaceRHIRef& GetRenderTargetSurface() const;

	// Properties.
	virtual UINT GetSizeX() const = 0;
	virtual UINT GetSizeY() const = 0;

	/** 
	* @return display gamma expected for rendering to this render target 
	*/
	virtual FLOAT GetDisplayGamma() const;

	/**
	* Handles freezing/unfreezing of rendering
	*/
	virtual void ProcessToggleFreezeCommand() {};
	
	/**
	 * Returns if there is a command to toggle freezerendering
	 */
	virtual UBOOL HasToggleFreezeCommand() { return FALSE; };

	/**
	* Reads the viewport's displayed pixels into the given color buffer.
	* @param OutputBuffer - RGBA8 values will be stored in this buffer
	* @param CubeFace - optional cube face for when reading from a cube render target
	* @return True if the read succeeded.
	*/
	UBOOL ReadPixels(TArray<FColor>& OutputBuffer,ECubeFace CubeFace=CubeFace_PosX);

protected:
	FSurfaceRHIRef RenderTargetSurfaceRHI;
};

//
//	EInputEvent
//
enum EInputEvent
{
    IE_Pressed              =0,
    IE_Released             =1,
    IE_Repeat               =2,
    IE_DoubleClick          =3,
    IE_Axis                 =4,
    IE_MAX                  =5,
};

/**
 * An interface to the platform-specific implementation of a UI frame for a viewport.
 */
class FViewportFrame
{
public:

	virtual FViewport* GetViewport() = 0;
	virtual void Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX = -1, INT InPosY = -1) = 0;
};

/**
* The maximum size that the hit proxy kernel is allowed to be set to
*/
#define MAX_HITPROXYSIZE 200

/**
 * Encapsulates the I/O of a viewport.
 * The viewport display is implemented using the platform independent RHI.
 * The input must be implemented by a platform-specific subclass.
 * Use UClient::CreateViewport to create the appropriate platform-specific viewport subclass.
 */
class FViewport : public FRenderTarget, private FRenderResource
{
public:

	// Constructor.
	FViewport(FViewportClient* InViewportClient);
	// Destructor
	virtual ~FViewport(){}

	// FViewport interface.
	virtual void*	GetWindow() = 0;

	// New MouseCapture/MouseLock API
	virtual UBOOL	HasMouseCapture() const				{ return TRUE; }
	virtual UBOOL	HasFocus() const					{ return TRUE; }
	virtual void	CaptureMouse( UBOOL bCapture )		{ }
	virtual void	LockMouseToWindow( UBOOL bLock )	{ }
	virtual void	ShowCursor( UBOOL bVisible )		{ }

	virtual UBOOL	CaptureJoystickInput(UBOOL Capture) = 0;
	virtual UBOOL	KeyState(FName Key) const = 0;
	virtual INT		GetMouseX() = 0;
	virtual INT		GetMouseY() = 0;
	virtual void	GetMousePos( FIntPoint& MousePosition ) = 0;
	virtual void	SetMouse(INT x, INT y) = 0;
	virtual UBOOL	IsFullscreen()						{ return bIsFullscreen; }
	virtual void	ProcessInput( FLOAT DeltaTime ) = 0;

	/**
	 *	Starts a new rendering frame. Called from the rendering thread.
	 */
	virtual void	BeginRenderFrame();

	/**
	 *	Ends a rendering frame. Called from the rendering thread.
	 *	@param bPresent		Whether the frame should be presented to the screen
	 *	@param bLockToVsync	Whether the GPU should block until VSYNC before presenting
	 */
	virtual void	EndRenderFrame( UBOOL bPresent, UBOOL bLockToVsync );

	/**
	 * @return whether or not this Controller has Tilt Turned on
	 **/
	virtual UBOOL IsControllerTiltActive( INT ControllerID ) const { return FALSE; }

	/**
	 * sets whether or not the the player wants to utilize the Tilt functionality
	 **/
	virtual void SetControllerTiltDesiredIfAvailable( INT ControllerID, UBOOL bActive ) { }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltActive( INT ControllerID, UBOOL bActive ) { }

	/**
	 * sets whether or not to ONLY use the tilt input controls
	 **/
	virtual void SetOnlyUseControllerTiltInput( INT ControllerID, UBOOL bActive ) { }

	/**
 	 * sets whether or not to use the tilt forward and back input controls
	 **/
	virtual void SetUseTiltForwardAndBack( INT ControllerID, UBOOL bActive ) { }

	/**
	 * @return whether or not this Controller has a keyboard available to be used
	 **/
	virtual UBOOL IsKeyboardAvailable( INT ControllerID ) const { return TRUE; }

	/**
	 * @return whether or not this Controller has a mouse available to be used
	 **/
	virtual UBOOL IsMouseAvailable( INT ControllerID ) const { return TRUE; }


	/** 
	* @return aspect ratio that this viewport should be rendered at
	*/
	virtual FLOAT GetDesiredAspectRatio() const
	{
        return (FLOAT)GetSizeX()/(FLOAT)GetSizeY();        
	}

	/**
	 * Invalidates the viewport's displayed pixels.
	 */
	virtual void InvalidateDisplay() = 0;

	/**
	 * Updates the viewport's displayed pixels with the results of calling ViewportClient->Draw.
	 */
	void Draw();

	/**
	 * Invalidates cached hit proxies
	 */
	void InvalidateHitProxy();	

	/**
	 * Invalidates cached hit proxies and the display.
	 */
	void Invalidate();	

	/**
	 * Copies the hit proxies from an area of the screen into a buffer.
	 * (MinX,MinY)->(MaxX,MaxY) must be entirely within the viewport's client area.
	 * If the hit proxies are not cached, this will call ViewportClient->Draw with a hit-testing canvas.
	 */
	void GetHitProxyMap(UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<HHitProxy*>& OutMap);

	/**
	 * Returns the dominant hit proxy at a given point.  If X,Y are outside the client area of the viewport, returns NULL.
	 * Caution is required as calling Invalidate after this will free the returned HHitProxy.
	 */
	HHitProxy* GetHitProxy(INT X,INT Y);

	/**
	 * Retrieves the interface to the viewport's frame, if it has one.
	 * @return The viewport's frame interface.
	 */
	virtual FViewportFrame* GetViewportFrame() = 0;
	
	/**
	 * Calculates the view inside the viewport when the aspect ratio is locked.
	 * Used for creating cinematic bars.
	 * @param Aspect [in] ratio to lock to
	 * @param CurrentX [in][out] coordinates of aspect locked view
	 * @param CurrentY [in][out]
	 * @param CurrentSizeX [in][out] size of aspect locked view
	 * @param CurrentSizeY [in][out]
	 */
	void CalculateViewExtents( FLOAT AspectRatio, INT& CurrentX, INT& CurrentY, UINT& CurrentSizeX, UINT& CurrentSizeY );

	/**
	 *	Sets a viewport client if one wasn't provided at construction time.
	 *	@param InViewportClient	- The viewport client to set.
	 **/
	virtual void SetViewportClient( FViewportClient* InViewportClient );

	// FRenderTarget interface.
	virtual UINT GetSizeX() const { return SizeX; }
	virtual UINT GetSizeY() const { return SizeY; }	

	// Accessors.
	FViewportClient* GetClient() const { return ViewportClient; }

	/**
	 * Globally enables/disables rendering
	 *
	 * @param bIsEnabled TRUE if drawing should occur
	 * @param PresentAndStopMovieDelay Number of frames to delay before enabling bPresent in RHIEndDrawingViewport, and before stopping the movie
	 */
	static void SetGameRenderingEnabled(UBOOL bIsEnabled, INT PresentAndStopMovieDelay=0);

	/**
	 * Returns whether rendering is globally enabled or disabled.
	 * @return	TRUE if rendering is globally enabled, otherwise FALSE.
	 **/
	UBOOL IsGameRenderingEnabled()	{ return bIsGameRenderingEnabled; }

	/**
	 * Handles freezing/unfreezing of rendering
	 */
	virtual void ProcessToggleFreezeCommand();

	/**
	 * Returns if there is a command to freeze
	 */
	virtual UBOOL HasToggleFreezeCommand();

	/**
	* Accessor for RHI resource
	*/
	const FViewportRHIRef& GetViewportRHI() const { return ViewportRHI; }

	/**
	 * Update the render target surface RHI to the current back buffer 
	 */
	void UpdateRenderTargetSurfaceRHIToCurrentBackBuffer();

protected:

	/** The viewport's client. */
	FViewportClient* ViewportClient;

	/**
	 * Updates the viewport RHI with the current viewport state.
	 * @param bDestroyed - True if the viewport has been destroyed.
	 */
	void UpdateViewportRHI(UBOOL bDestroyed,UINT NewSizeX,UINT NewSizeY,UBOOL bNewIsFullscreen);

	/**
	 * Take a tiled, high-resolution screenshot and save to disk.
	 *
	 * @ResolutionMultiplier Increase resolution in each dimension by this multiplier.
	 */
	void TiledScreenshot( INT ResolutionMultiplier );

private:

	/** A map from 2D coordinates to cached hit proxies. */
	class FHitProxyMap : public FHitProxyConsumer, public FRenderTarget
	{
	public:

		/** Initializes the hit proxy map with the given dimensions. */
		void Init(UINT NewSizeX,UINT NewSizeY);

		/** Releases the hit proxy resources. */
		void Release();

		/** Invalidates the cached hit proxy map. */
		void Invalidate();

		// FHitProxyConsumer interface.
		virtual void AddHitProxy(HHitProxy* HitProxy);

		// FRenderTarget interface.
		virtual UINT GetSizeX() const { return SizeX; }
		virtual UINT GetSizeY() const { return SizeY; }

	private:

		/** The width of the hit proxy map. */
		UINT SizeX;

		/** The height of the hit proxy map. */
		UINT SizeY;

		/** References to the hit proxies cached by the hit proxy map. */
		TArray<TRefCountPtr<HHitProxy> > HitProxies;
	};

	/** The viewport's hit proxy map. */
	FHitProxyMap HitProxyMap;

	/** The RHI viewport. */
	FViewportRHIRef ViewportRHI;

	/** The width of the viewport. */
	UINT SizeX;

	/** The height of the viewport. */
	UINT SizeY;

	/** The size of the region to check hit proxies */
	UINT HitProxySize;

	/** True if the viewport is fullscreen. */
	BITFIELD bIsFullscreen : 1;

	/** True if the viewport client requires hit proxy storage. */
	BITFIELD bRequiresHitProxyStorage : 1;

	/** True if the hit proxy buffer buffer has up to date hit proxies for this viewport. */
	BITFIELD bHitProxiesCached : 1;

	/** If a toggle freeze request has been made */
	BITFIELD bHasRequestedToggleFreeze : 1;

	/** TRUE if we should draw game viewports (has no effect on Editor viewports) */
	static UBOOL bIsGameRenderingEnabled;

	/** Delay in frames to disable present (but still render scene) and stopping of a movie. This is useful to keep playing a movie while driver caches things on the first frame, which can be slow. */
	static INT PresentAndStopMovieDelay;

	// FRenderResource interface.
	virtual void InitDynamicRHI();
	virtual void ReleaseDynamicRHI();
};

// Shortcuts for checking the state of both left&right variations of control keys.
extern UBOOL IsCtrlDown(FViewport* Viewport);
extern UBOOL IsShiftDown(FViewport* Viewport);
extern UBOOL IsAltDown(FViewport* Viewport);

/**
 * An abstract interface to a viewport's client.
 * The viewport's client processes input received by the viewport, and draws the viewport.
 */
class FViewportClient
{
public:

	virtual void Precache() {}
	virtual void RedrawRequested(FViewport* Viewport) { Viewport->Draw(); }
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas) {}

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @param	bGamepad - input came from gamepad (ie xbox controller)
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE) { InputKey(Viewport,Key,Event,AmountDepressed,bGamepad); return FALSE; }

	/**
	 * Check an axis movement received by the viewport.
	 * If the viewport client uses the movement, it should return true to consume it.
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Key - The name of the axis which moved.
	 * @param	Delta - The axis movement delta.
	 * @param	DeltaTime - The time since the last axis update.
	 * @return	True to consume the axis movement, false to pass it on.
	 */
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime,UBOOL bGamepad=FALSE) { InputAxis(Viewport,Key,Delta,DeltaTime,bGamepad); return FALSE; }

	/**
	 * Check a character input received by the viewport.
	 * If the viewport client uses the character, it should return true to consume it.
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Character - The character.
	 * @return	True to consume the character, false to pass it on.
	 */
	virtual UBOOL InputChar(FViewport* Viewport,INT ControllerId,TCHAR Character) { InputChar(Viewport,Character); return FALSE; }

	/** @name Obsolete input interface.  Called by the new interface to ensure implementors of the old interface aren't broken. */
	//@{
	virtual void InputKey(FViewport* Viewport,FName Key, EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE) {}
	virtual void InputAxis(FViewport* Viewport,FName Key,FLOAT Delta,FLOAT DeltaTime,UBOOL bGamepad=FALSE) {}
	virtual void InputChar(FViewport* Viewport,TCHAR Character) {}
	//@}

	/** Returns the platform specific forcefeedback manager this viewport associates with the specified controller. */
	virtual class UForceFeedbackManager* GetForceFeedbackManager(INT ControllerId) { return NULL; }

	/**
	 * @return whether or not this Controller has Tilt Turned on
	 **/
	virtual UBOOL IsControllerTiltActive() const { return FALSE; }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltDesiredIfAvailable( UBOOL bActive ) { }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltActive( UBOOL bActive ) { }

	/**
	 * sets whether or not to ONLY use the tilt input controls
	 **/
	virtual void SetOnlyUseControllerTiltInput( UBOOL bActive ) { }


	virtual void MouseMove(FViewport* Viewport,INT X,INT Y) {}

	/**
	 * Retrieves the cursor that should be displayed by the OS
	 *
	 * @param	Viewport	the viewport that contains the cursor
	 * @param	X			the x position of the cursor
	 * @param	Y			the Y position of the cursor
	 * 
	 * @return	the cursor that the OS should display
	 */
	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y) { return MC_Arrow; }

	virtual void LostFocus(FViewport* Viewport) {}
	virtual void ReceivedFocus(FViewport* Viewport) {}
	virtual UBOOL IsFocused(FViewport* Viewport) { return TRUE; }

	virtual void CloseRequested(FViewport* Viewport) {}

	virtual UBOOL RequiresHitProxyStorage() { return TRUE; }

	/**
	 * Determines whether this viewport client should receive calls to InputAxis() if the game's window is not currently capturing the mouse.
	 * Used by the UI system to easily receive calls to InputAxis while the viewport's mouse capture is disabled.
	 */
	virtual UBOOL RequiresUncapturedAxisInput() const { return FALSE; }

	/**
	* Determine if the viewport client is going to need any keyboard input
	* @return TRUE if keyboard input is needed
	*/
	virtual UBOOL RequiresKeyboardInput() const { return TRUE; }
};

//
//	UClient - Interface to platform-specific code.
//

class UClient : public UObject, FExec
{
	DECLARE_ABSTRACT_CLASS(UClient,UObject,CLASS_Config|CLASS_Intrinsic,Engine);
public:

	// Configuration.

	FLOAT		MinDesiredFrameRate;

	/** The gamma value of the display device. */
	FLOAT		DisplayGamma;

	/** The time (in seconds) before the initial IE_Repeat event is generated for a button that is held down */
	FLOAT		InitialButtonRepeatDelay;

	/** the time (in seconds) before successive IE_Repeat input key events are generated for a button that is held down */
	FLOAT		ButtonRepeatDelay;

	// UObject interface.

	void StaticConstructor();
	/**
	 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
	 * is initialized against its archetype, but before any objects of this class are created.
	 */
	void InitializeIntrinsicPropertyValues();
	// UClient interface.

	virtual void Init(UEngine* InEngine) PURE_VIRTUAL(UClient::Init,);
	virtual void Tick(FLOAT DeltaTime) PURE_VIRTUAL(UClient::Tick,);

	/**
	 * Exec handler used to parse console commands.
	 *
	 * @param	Cmd		Command to parse
	 * @param	Ar		Output device to use in case the handler prints anything
	 * @return	TRUE if command was handled, FALSE otherwise
	 */
	virtual UBOOL Exec(const TCHAR* Cmd,FOutputDevice& Ar=*GLog);

	/**
	 * PC only, debugging only function to prevent the engine from reacting to OS messages. Used by e.g. the script
	 * debugger to avoid script being called from within message loop (input).
	 *
	 * @param InValue	If FALSE disallows OS message processing, otherwise allows OS message processing (default)
	 */
	virtual void AllowMessageProcessing( UBOOL InValue )  PURE_VIRTUAL(UClient::AllowMessageProcessing,);

	virtual FViewportFrame* CreateViewportFrame(FViewportClient* ViewportClient,const TCHAR* InName,UINT SizeX,UINT SizeY,UBOOL Fullscreen = 0) PURE_VIRTUAL(UClient::CreateViewport,return NULL;);
	virtual FViewport* CreateWindowChildViewport(FViewportClient* ViewportClient,void* ParentWindow,UINT SizeX=0,UINT SizeY=0,INT InPosX = -1, INT InPosY = -1) PURE_VIRTUAL(UClient::CreateWindowChildViewport,return NULL;);
	virtual void CloseViewport(FViewport* Viewport)  PURE_VIRTUAL(UClient::CloseViewport,);

	virtual class UAudioDevice* GetAudioDevice() PURE_VIRTUAL(UClient::GetAudioDevice,return NULL;);

	/** Function to immediately stop any force feedback vibration that might be going on this frame. */
	virtual void ForceClearForceFeedback() PURE_VIRTUAL(UClient::ForceClearForceFeedback,);

	/**
	 * Retrieves the name of the key mapped to the specified character code.
	 *
	 * @param	KeyCode	the key code to get the name for; should be the key's ANSI value
	 */
	virtual FName GetVirtualKeyName( INT KeyCode ) const PURE_VIRTUAL(UClient::GetVirtualKey,return NAME_None;);
};


/**
 * Encapsulates a latency timer that measures the time from when mouse input
 * is read on the gamethread until that frame is fully displayed by the GPU.
 */
struct FInputLatencyTimer
{
	/**
	 * Constructor
	 * @param InUpdateFrequency	How often the timer should be updated (in seconds).
	 */
	FInputLatencyTimer( FLOAT InUpdateFrequency )
	{
		bInitialized = FALSE;
		RenderThreadTrigger = FALSE;
		GameThreadTrigger = FALSE;
		StartTime = 0;
		DeltaTime = 0;
		LastCaptureTime = 0.0f;
		UpdateFrequency = InUpdateFrequency;
	}

	/** Potentially starts the timer on the gamethread, based on the UpdateFrequency. */
	void	GameThreadTick();

	/** Weather GInputLatencyTimer is initialized or not. */
	UBOOL	bInitialized;

	/** Weather a measurement has been triggered on the gamethread. */
	UBOOL	GameThreadTrigger;

	/** Weather a measurement has been triggered on the renderthread. */
	UBOOL	RenderThreadTrigger;

	/** Start time (in appCycles). */
	DWORD	StartTime;

	/** Last delta time that was measured (in appCycles). */
	DWORD	DeltaTime;

	/** Last time we did a measurement (in seconds). */
	FLOAT	LastCaptureTime;

	/** How often we should do a measurement (in seconds). */
	FLOAT	UpdateFrequency;
};


/** Global input latency timer. Defined in UnClient.cpp */
extern FInputLatencyTimer GInputLatencyTimer;
/** Accumulates how many cycles the renderthread has been idle. It's defined in RenderingThread.cpp. */
extern DWORD GRenderThreadIdle;
/** Accumulates how many cycles the gamethread has been idle. It's defined in RenderingThread.cpp. */
extern DWORD GGameThreadIdle;
/** How many cycles the renderthread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern DWORD GRenderThreadTime;
/** How many cycles the gamethread used (excluding idle time). It's set once per frame in FViewport::Draw. */
extern DWORD GGameThreadTime;


#endif // #ifndef _UN_CLIENT_
