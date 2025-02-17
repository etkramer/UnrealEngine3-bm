/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __UNEDVIEWPORT_H__
#define __UNEDVIEWPORT_H__

// Forward declarations.
class FEditorCameraController;
class FCameraControllerUserImpulseData;
class FMouseDeltaTracker;
class FWidget;

struct FEditorLevelViewportClient : public FViewportClient, public FViewElementDrawer
{
public:
	////////////////////////////
	// Viewport parameters

	FViewport*				Viewport;
	/** Viewport location. */
	FVector					ViewLocation;
	/** Viewport orientation; valid only for perspective projections. */
	FRotator				ViewRotation;
	/** Viewport's horizontal field of view. */
	FLOAT					ViewFOV;
	/** The viewport type. */
	ELevelViewportType		ViewportType;
	FLOAT					OrthoZoom;

	/** The viewport's scene view state. */
	FSceneViewStateInterface* ViewState;

	/** A set of flags that determines visibility for various scene elements. */
	EShowFlags				ShowFlags;

	/** Previous value for show flags, used for toggling. */
	EShowFlags				LastShowFlags;

	/** Last special flag mode that was entered. */
	EShowFlags				LastSpecialMode;

	/** If TRUE, render unlit while viewport camera is moving. */
	UBOOL					bMoveUnlit;

	/** If TRUE, this viewport is being used to previs level streaming volumes in the editor. */
	UBOOL					bLevelStreamingVolumePrevis;

	/** If TRUE, this viewport is being used to previs post process volumes in the editor. */
	UBOOL					bPostProcessVolumePrevis;

	/** If TRUE, the viewport's location and rotation is loced and can only be moved by user input. */
	UBOOL					bViewportLocked;

	/** If FALSE, the far clipping plane of persepective views is at infitiy; if TRUE, the far plane is adjustable. */
	UBOOL					bVariableFarPlane;

	/** Stores the current ViewMode while moving unlit. */
	EShowFlags				CachedShowFlags;

	/** If TRUE, move any selected actors to the cameras location/rotation. */
	UBOOL					bLockSelectedToCamera;

	/** If FALSE, ambient occlusion is disabled. */
	UBOOL					bAllowAmbientOcclusion;

	/** If TRUE, we force the PostProcess settings to the OverrideProcessSettings stored below. */
	UBOOL					bOverridePostProcessSettings;

	/** PostProcess settings to use for this viewport in the editor if bOverridePostProcessSettings is TRUE. */
	FPostProcessSettings	OverrideProcessSettings;

	/** If TRUE, we additively blend the settings from AdditiveProcessSettings below. */
	UBOOL					bAdditivePostProcessSettings;

	/** PostProcess settings to additively blend to for this viewport in the editor if bAdditivePostProcessSettings is TRUE. */
	FPostProcessSettings	AdditivePostProcessSettings;

	/** The last post process settings used for the viewport. */
	FPostProcessSettings	PostProcessSettings;

	////////////////////////////
	// Maya camera movement

	UBOOL		bAllowMayaCam;
	UBOOL		bUsingMayaCam;
	FRotator	MayaRotation;
	FVector		MayaLookAt;
	FVector		MayaZoom;
	AActor*		MayaActor;

	////////////////////////////

	UEditorViewportInput*	Input;

	/** If TRUE, audio system will be updated. */
	UBOOL					bAudioRealTime;

	/** If TRUE, the listener position will be set */
	UBOOL					bSetListenerPosition;

	/** The number of pending viewport redraws. */
	UINT NumPendingRedraws;

	/** If TRUE, draw the axis indicators when the viewport is perspective. */
	UBOOL					bDrawAxes;

	/** If TRUE, draw vertices for selected BSP brushes and static meshes if the large vertices show flag is set. */
	UBOOL					bDrawVertices;

	/**
	 * Used for actor drag duplication.  Set to TRUE on Alt+LMB so that the selected
	 * actors will be duplicated as soon as the widget is displaced.
	 */
	UBOOL					bDuplicateActorsOnNextDrag;

	/**
	* bDuplicateActorsOnNextDrag will not be set again while bDuplicateActorsInProgress is TRUE.
	* The user needs to release ALT and all mouse buttons to clear bDuplicateActorsInProgress.
	*/
	UBOOL					bDuplicateActorsInProgress;

	/**
	 * TRUE when within a FMouseDeltaTracker::StartTracking/EndTracking block.
	 */
	UBOOL					bIsTracking;

	/**
	 * TRUE if the user is dragging by a widget handle.
	 */
	UBOOL					bDraggingByHandle;

	////////////////////////////
	// Aspect ratio
	UBOOL					bConstrainAspectRatio;
	FLOAT					AspectRatio;

	/** near plane adjustable for each editor view */
	FLOAT					NearPlane;

	UBOOL					bEnableFading;
	FLOAT					FadeAmount;
	FColor					FadeColor;

	UBOOL					bEnableColorScaling;
	FVector					ColorScale;

	FWidget*				Widget;
	FMouseDeltaTracker*		MouseDeltaTracker;

	/** TRUE if the widget's axis is being controlled by an active mouse drag. */
	UBOOL					bWidgetAxisControlledByDrag;

	/** Variable holds the scalar value for the camera speed in the viewport */
	FLOAT					CameraSpeed;

private:
	UBOOL bIsRealtime;
	UBOOL bStoredRealtime;

	/** TRUE if squint mode is active */
	UBOOL bUseSquintMode;

	/** TRUE if this is a floating viewport window */
	UBOOL bIsFloatingViewport;

	/** TRUE if this window is allowed to be possessed by Matinee for previewing sequences in real-time */
	UBOOL bAllowMatineePreview;

	/** Camera controller object that's used for piloting the camera around */
	FEditorCameraController* CameraController;

	/** Current cached impulse state */
	FCameraControllerUserImpulseData* CameraUserImpulseData;

	/** Extra camera speed scale for flight camera */
	FLOAT FlightCameraSpeedScale;

	/** Real time that camera speed scale was changed list */
	DOUBLE RealTimeFlightCameraSpeedScaleChanged;


public:

	/**
	 * Constructor
	 *
	 * @param	ViewportInputClass	the input class to use for creating this viewport's input object
	 */
	FEditorLevelViewportClient( UClass* ViewportInputClass=NULL );

	/**
	 * Destructor
	 */
	virtual ~FEditorLevelViewportClient();

	////////////////////////////
	// FViewElementDrawer interface
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI);

	////////////////////////////
	// FViewportClient interface

	virtual void RedrawRequested(FViewport* Viewport);
	virtual void Draw(FViewport* Viewport,FCanvas* Canvas);
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad=FALSE);

	virtual void MouseMove(FViewport* Viewport,INT x, INT y);

	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y);

	////////////////////////////
	// FEditorLevelViewportClient interface

	/**
	 * Configures the specified FSceneView object with the view and projection matrices for this viewport.
	 * @param	View		The view to be configured.  Must be valid.
	 * @return	A pointer to the view within the view family which represents the viewport's primary view.
	 */
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily);

	/** Returns true if this viewport is orthogonal. */
	UBOOL IsOrtho() const			{ return (ViewportType == LVT_OrthoXY || ViewportType == LVT_OrthoXZ || ViewportType == LVT_OrthoYZ ); }

	/** Returns true if this viewport is perspective. */
	UBOOL IsPerspective() const		{ return (ViewportType == LVT_Perspective); }


	/**
	 * Determines if InComponent is inside of InSelBBox.  This check differs depending on the type of component.
	 * If InComponent is NULL, FALSE is returned.
	 *
	 * @param	InActor				Used only when testing sprite components.
	 * @param	InComponent			The component to query.  If NULL
	 * @param	InSelBox			The selection box.
	 * @param	bConsiderOnlyBSP	If TRUE, consider only BSP.
	 */
	UBOOL ComponentIsTouchingSelectionBox( AActor* InActor, UPrimitiveComponent* InComponent, const FBox& InSelBBox, UBOOL bConsiderOnlyBSP );

	/**
	 * Invalidates this viewport and optionally child views.
	 *
	 * @param	bInvalidateChildViews		[opt] If TRUE (the default), invalidate views that see this viewport as their parent.
	 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
	 */
	void Invalidate(UBOOL bInvalidateChildViews=TRUE, UBOOL bInvalidateHitProxies=TRUE);


	////////////////////////////
	// Realtime update

	/**
	 * Toggles whether or not the viewport updates in realtime and returns the updated state.
	 *
	 * @return		The current state of the realtime flag.
	 */
	UBOOL ToggleRealtime()
	{ 
		bIsRealtime = !bIsRealtime; 
		return bIsRealtime;
	}

	/** Sets whether or not the viewport updates in realtime. */
	void SetRealtime( UBOOL bInRealtime, UBOOL bStoreCurrentValue = FALSE )
	{ 
		if( bStoreCurrentValue )
		{
			bStoredRealtime = bIsRealtime;
		}
		bIsRealtime	= bInRealtime;
	}

	/** @return		True if viewport is in realtime mode, false otherwise. */
	UBOOL IsRealtime() const				
	{ 
		return bIsRealtime; 
	}

	/**
	 * Restores realtime setting to stored value. This will only enable realtime and 
	 * never disable it.
	 */
	void RestoreRealtime()
	{
		bIsRealtime |= bStoredRealtime;
	}

	virtual FSceneInterface* GetScene();
	virtual void ProcessClick(FSceneView* View,HHitProxy* HitProxy,FName Key,EInputEvent Event,UINT HitX,UINT HitY);
	virtual void Tick(FLOAT DeltaSeconds);
	void UpdateMouseDelta();

	/** Sets whether or not the viewport uses squint mode. */
	void SetSquintMode( UBOOL bNewSquintMode )
	{ 
		bUseSquintMode	= bNewSquintMode;
	}

	/** @return True if viewport is in squint mode, false otherwise. */
	UBOOL IsUsingSquintMode() const				
	{ 
		return bUseSquintMode; 
	}

	/**
	 * Calculates absolute transform for mouse position status text using absolute mouse delta.
	 *
	 * @param bUseSnappedDelta Whether or not to use absolute snapped delta for transform calculations.
	 */
	void UpdateMousePositionTextUsingDelta( UBOOL bUseSnappedDelta );

	/**
	 * Returns the horizontal axis for this viewport.
	 */
	EAxis GetHorizAxis() const;

	/**
	 * Returns the vertical axis for this viewport.
	 */
	EAxis GetVertAxis() const;

	/** Returns true if this viewport is floating */
	UBOOL IsFloatingViewport() const { return bIsFloatingViewport; }

	/** Sets whether or not we're a floating viewport */
	void SetFloatingViewport( const UBOOL bInFloatingViewport )
	{
		bIsFloatingViewport = bInFloatingViewport;
	}

	/** Returns true if the viewport is allowed to be possessed by Matinee for previewing sequences */
	UBOOL AllowMatineePreview() const { return bAllowMatineePreview; }

	/** Sets whether or not this viewport is allowed to be possessed by Matinee */
	void SetAllowMatineePreview( const UBOOL bInAllowMatineePreview )
	{
		bAllowMatineePreview = bInAllowMatineePreview;
	}

	void InputAxisMayaCam(FViewport* Viewport, const FVector& DragDelta, FVector& Drag, FRotator& Rot);

	/**
	 * Implements screenshot capture for editor viewports.  Should be called by derived class' InputKey.
	 */
	void InputTakeScreenshot(FViewport* Viewport, FName Key, EInputEvent Event);

	/**
	 * Determines which axis InKey and InDelta most refer to and returns a corresponding FVector.  This
	 * vector represents the mouse movement translated into the viewports/widgets axis space.
	 *
	 * @param	InNudge		If 1, this delta is coming from a keyboard nudge and not the mouse
	 */
	virtual FVector TranslateDelta( FName InKey, FLOAT InDelta, UBOOL InNudge );

	/**
	 * Converts a generic movement delta into drag/rotation deltas based on the viewport and keys held down.
	 */
	void ConvertMovementToDragRot( const FVector& InDelta, FVector& InDragDelta, FRotator& InRotDelta );

	void ConvertMovementToDragRotMayaCam(const FVector& InDelta, FVector& InDragDelta, FRotator& InRotDelta);
	void SwitchStandardToMaya();
	void SwitchMayaToStandard();
	void SwitchMayaCam();

	/**
	 * Sets the camera view location such that the MayaLookAt point is at the specified location.
	 */
	void SetViewLocationForOrbiting(const FVector& Loc);

	/**
	 * Moves the viewport camera according to the specified drag and rotation.
	 *
	 * @param bUnlitMovement	If TRUE, go into unlit movement if the level viewport settings permit.
	 */
	void MoveViewportCamera( const FVector& InDrag, const FRotator& InRot, UBOOL bUnlitMovement=TRUE );

	void ApplyDeltaToActors( const FVector& InDrag, const FRotator& InRot, const FVector& InScale );
	void ApplyDeltaToActor( AActor* InActor, const FVector& InDeltaDrag, const FRotator& InDeltaRot, const FVector& InDeltaScale );
	virtual FLinearColor GetBackgroundColor();

	/**
	 * Draws a screen-space box around each Actor in the Kismet sequence of the specified level.
	 */
	void DrawKismetRefs(ULevel* Level, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas);

	/** Draw an Actors text Tag next to it in the viewport. */
	void DrawActorTags(FViewport* Viewport, const FSceneView* View, FCanvas* Canvas);

	void DrawAxes(FViewport* Viewport,FCanvas* Canvas, const FRotator* InRotation = NULL);

	/**
	 *	Draw the texture streaming bounds.
	 */
	void DrawTextureStreamingBounds(const FSceneView* View,FPrimitiveDrawInterface* PDI);

	/** Serialization. */
	friend FArchive& operator<<(FArchive& Ar,FEditorLevelViewportClient& Viewport)
	{
		return Ar << Viewport.Input;
	}

	
	/**
	 * Copies layout and camera settings from the specified viewport
	 *
	 * @param InViewport The viewport to copy settings from
	 */
	void CopyLayoutFromViewport( const FEditorLevelViewportClient& InViewport );


private:

	/** Returns true if perspective flight camera input mode is currently active in this viewport */
	UBOOL IsFlightCameraInputModeActive() const;

	/** Returns true if we're currently allowed to update the 'Mouse position' status bar text at this time */
	UBOOL CanUpdateStatusBarText() const
	{
		// Make sure the 'camera speed changed' display is visible for awhile before we clear it
		return ( appSeconds() - RealTimeFlightCameraSpeedScaleChanged ) > 2.0f;
	}

	/** Moves a perspective camera */
	void MoveViewportPerspectiveCamera( const FVector& InDrag, const FRotator& InRot, UBOOL bUnlitMovement );

	/**
	 * Updates real-time camera movement.  Should be called every viewport tick!
	 *
	 * @param	DeltaTime	Time interval in seconds since last update
	 */
	void UpdateCameraMovement( FLOAT DeltaTime );


	/**
	 * Checks to see if the current input event modified any show flags.
	 * @param Key				Key that was pressed.
	 * @param bControlDown		Flag for whether or not the control key is held down.
	 * @param bAltDown			Flag for whether or not the alt key is held down.
	 * @param bShiftDown		Flag for whether or not the shift key is held down.
	 * @return					Flag for whether or not we handled the input.
	 */
	UBOOL CheckForShowFlagInput(FName Key, UBOOL bControlDown, UBOOL bAltDown, UBOOL bShiftDown);

};

///////////////////////////////////////////////////////////////////////////////

struct FViewportClick
{
public:
	FViewportClick(class FSceneView* View,FEditorLevelViewportClient* ViewportClient,FName InKey,EInputEvent InEvent,INT X,INT Y);

	/** @return The 2D screenspace cursor position of the mouse when it was clicked. */
	const FIntPoint& GetClickPos() const	{ return ClickPos; }
	const FVector&	GetOrigin()		const	{ return Origin; }
	const FVector&	GetDirection()	const	{ return Direction; }
	const FName&	GetKey()		const	{ return Key; }
	EInputEvent		GetEvent()		const	{ return Event; }

	UBOOL	IsControlDown()	const			{ return ControlDown; }
	UBOOL	IsShiftDown()	const			{ return ShiftDown; }
	UBOOL	IsAltDown()		const			{ return AltDown; }

private:
	FVector		Origin,
				Direction;
	FName		Key;
	EInputEvent	Event;
	UBOOL		ControlDown,
				ShiftDown,
				AltDown;
	FIntPoint   ClickPos;
};

#endif // __UNEDVIEWPORT_H__
