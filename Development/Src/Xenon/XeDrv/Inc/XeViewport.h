/*=============================================================================
	XeDrv.h: Unreal Xenon viewport driver.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define XE_ANALOG_BUTTON_PRESS (1.f / 255.f)
//
//	FXenonViewport
//
class FXenonViewport : public FViewportFrame, public FViewport
{
public:
	enum 
	{
		// max number of controllers allowed
		MAX_NUM_CONTROLLERS=4, 
		// max buttons per controller
		MAX_NUM_BUTTONS=16 
	};

	/**
	 * Tracks information about the states of gamepad buttons
	 */
	struct FGamepadButtonInfo
	{
		/** the amount that each button was depressed in the last frame */
		INT AmountDepressed[MAX_NUM_BUTTONS];

		/** Next time a IE_Repeat event should be generated for each button */
		DOUBLE NextRepeatTime[MAX_NUM_BUTTONS];
	};

	/**
	* Constructor
	*/
	FXenonViewport(
		class UXenonClient* InClient,
		FViewportClient* InViewportClient,
		const TCHAR* InName,
		UINT InSizeX,
		UINT InSizeY,
		UBOOL InFullscreen
		);

	/**
	* Destructor
	*/
	virtual ~FXenonViewport();

	// FViewport interface.
	
	virtual void* GetWindow() { return NULL; }

	virtual void LockMouseToWindow(UBOOL bLock) {}
	virtual UBOOL KeyState(FName Key) const;

	virtual UBOOL CaptureJoystickInput(UBOOL Capture) { return TRUE; }	

	virtual INT GetMouseX() { return 0; }
	virtual INT GetMouseY() { return 0; }
	virtual void GetMousePos( FIntPoint& MousePosition ) {}
	virtual void SetMouse(INT x, INT y) {}

	virtual void InvalidateDisplay() {}
	void Invalidate() {}
	virtual void GetHitProxyMap(UINT MinX,UINT MinY,UINT MaxX,UINT MaxY,TArray<HHitProxy*>& OutMap) {}
	HHitProxy* GetHitProxy(INT X,INT Y) { return NULL; }

	virtual FViewportFrame* GetViewportFrame();

	void ProcessInput( FLOAT DeltaTime );

	/** 
	* @return aspect ratio that this viewport should be rendered at
	*/
	virtual FLOAT GetDesiredAspectRatio()
	{
		// on Xenon we always enforce a fixed aspect ratio based on the XVIDEO_MODE.fIsWidescreen mode
		return FixedAspectRatio;       
	}

	// FRenderTarget interface.
	
	virtual UINT GetSizeX() const { return SizeX; }
	virtual UINT GetSizeY() const { return SizeY; }	

	// FViewportFrame interface.
	virtual FViewport* GetViewport() { return this; }
	virtual UBOOL IsFullscreen() { return TRUE; }
	virtual void SetName(const TCHAR* NewName) {}
	virtual void Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX = -1, INT InPosY = -1);
	
	/**
	 * Determines if there is a waveform playing and updates the gamepad state
	 * if there is
	 *
	 * @param DeltaTime Used to process updates over time
	 */
	void UpdateGamepadWaveform(FLOAT DeltaTime);
	
	/**
	* Passes key states to viewport client, avoiding redundant calls if state hasn't changed.
	*
	* @param	KeyIndex			Key/ button index used to compare to previous state
	* @param	ControllerIndex		Index of controller generating the event
	* @param	KeyName				Name of key
	* @param	AmountDepressed		Amount button is depressed, 0 == released, 255 == full pressed
	* @param	CurrentTime			the current value of appSeconds(); used for setting repeat delays
	*/
	void PassKeyStateToViewportClient( INT KeyIndex, INT ControllerIndex, FName KeyName, INT AmountDepressed, const DOUBLE CurrentTime );

private:
	UXenonClient*			Client;

	UINT					SizeX,
							SizeY;

	UBOOL					KeyStates[256];
	UBOOL					bCtrlPressed, bAltPressed, bShiftPressed;

	/** Constant defining value associated with a pressed key */
	static const INT DIGITAL_PRESSED	= 255;
	/** Constant defining value associated with a released key */
	static const INT DIGITAL_RELEASED	= 0;

	/** Array containing button state information for each gamepad */
	FGamepadButtonInfo		ButtonInfo[MAX_NUM_CONTROLLERS];

	/** aspect ratio as determined by the XVIDEO_MODE.fIsWidescreen mode */
	FLOAT FixedAspectRatio;
};

