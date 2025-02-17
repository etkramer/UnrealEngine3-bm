/*=============================================================================
	XeViewport.cpp: FXenonViewport code.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "XeDrv.h"
#include "EngineUserInterfaceClasses.h"
#include "AsyncInputThread.h"


FXenonViewport::FXenonViewport(UXenonClient* InClient,FViewportClient* InViewportClient,const TCHAR* InName,UINT InSizeX,UINT InSizeY,UBOOL InFullscreen)
:	FViewport(InViewportClient)
,	Client(InClient)
{
	// set the fixed aspect ratio for the viewport 
	// (fIsWideScreen is always TRUE if fIsHidef=TRUE)
	XVIDEO_MODE VideoMode;
	appMemzero(&VideoMode,sizeof(XVIDEO_MODE));
	XGetVideoMode(&VideoMode);
	// set the fixed aspect ratio for the viewport 
	FixedAspectRatio = VideoMode.fIsWideScreen ? 16.f / 9.f :
		((FLOAT)GScreenWidth / (FLOAT)GScreenHeight);

	// add to list of client viewports
	if( Client )
	{
		Client->Viewports.AddItem(this);
	}

	// Creates the viewport window.
	Resize(InSizeX,InSizeY,InFullscreen);

	// Set initial key state.
	for(UINT KeyIndex = 0;KeyIndex < 256;KeyIndex++)
	{
		KeyStates[KeyIndex] = 0;
	}
	bCtrlPressed = bAltPressed = bShiftPressed = FALSE;

	// Initialize async input poller
	FAsyncInputPoller::StartThread();

	// Set initial button state
	appMemzero(ButtonInfo, sizeof(ButtonInfo));
}

FXenonViewport::~FXenonViewport()
{
	ViewportClient = NULL;

	// Release the viewport RHI
	UpdateViewportRHI(TRUE,SizeX,SizeY,TRUE);

	// Clean up async input poller
	FAsyncInputPoller::DestroyThread();
}

FViewportFrame* FXenonViewport::GetViewportFrame()
{
	return this;
}

void FXenonViewport::Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX,INT InPosY)
{
	check(NewSizeX && NewSizeY);

	SizeX		= NewSizeX;
	SizeY		= NewSizeY;
	
	// Create the viewport
	UpdateViewportRHI(FALSE,SizeX,SizeY,TRUE);
}

/**
 * Returns the state of the passed in key.
 *
 * @param	Key name to query state of
 * @return	TRUE if pressed, FALSE otherwise
 */
UBOOL FXenonViewport::KeyState(FName Key) const
{
	UBOOL bResult = FALSE;
	if( Client )
	{
		BYTE* KeyIndex = Client->KeyMapNameToVirtual.Find(Key);
		
		// Key found.
		if( KeyIndex )
		{
			bResult = KeyStates[*KeyIndex] ? TRUE : FALSE; 
		}
		else
		{
			bResult
				=	((Key == KEY_LeftControl || Key == KEY_RightControl) && bCtrlPressed)
				||	((Key == KEY_LeftShift || Key == KEY_RightShift) && bShiftPressed)
				||	((Key == KEY_LeftAlt || Key == KEY_RightAlt) && bAltPressed);
		}
	}
	return bResult;
}

/**
 * Passes key states to viewport client, avoiding redundant calls if state hasn't changed.
 *
 * @param	KeyIndex			Key/ button index used to compare to previous state
 * @param	ControllerIndex		Index of controller generating the event
 * @param	KeyName				Name of key
 * @param	AmountDepressed		Amount button is depressed, 0 == released, 255 == full pressed
 * @param	CurrentTime			the current value of appSeconds(); used for setting repeat delays
 */
void FXenonViewport::PassKeyStateToViewportClient( INT KeyIndex, INT ControllerIndex, FName KeyName, INT AmountDepressed, const DOUBLE CurrentTime )
{

	// determine whether this button is currently considered "pressed"
	const UBOOL bIsPressed = AmountDepressed > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

	// Compare to the state of this button from the previous frame.
	const INT OldAmountDepressed = ButtonInfo[ControllerIndex].AmountDepressed[KeyIndex];
	const UBOOL bButtonStateChanged = 
		(OldAmountDepressed <= XINPUT_GAMEPAD_TRIGGER_THRESHOLD && bIsPressed) ||
		(OldAmountDepressed > XINPUT_GAMEPAD_TRIGGER_THRESHOLD && !bIsPressed);

	if ( bButtonStateChanged )
	{
		// Treat every key as analog.
		EInputEvent InputEvent = AmountDepressed > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? IE_Pressed : IE_Released;

		// Pass to input system.
		ViewportClient->InputKey( this, ControllerIndex, KeyName, InputEvent, AmountDepressed * XE_ANALOG_BUTTON_PRESS, TRUE );

		if( Client && InputEvent == IE_Pressed )
		{
			// if this button was just pressed, set the NextRepeatTime to the InitialButtonRepeatDelay
			ButtonInfo[ControllerIndex].NextRepeatTime[KeyIndex] = CurrentTime + Client->InitialButtonRepeatDelay;
		}
	}
	else if( bIsPressed && ButtonInfo[ControllerIndex].NextRepeatTime[KeyIndex] <= CurrentTime )
	{
		// it is time to generate an IE_Repeat event for this gamepad button
		ViewportClient->InputKey( this, ControllerIndex, KeyName, IE_Repeat, AmountDepressed * XE_ANALOG_BUTTON_PRESS, TRUE );

		if( Client )
		{
			// set the button's NextRepeatTime to the ButtonRepeatDelay
			ButtonInfo[ControllerIndex].NextRepeatTime[KeyIndex] = CurrentTime + Client->ButtonRepeatDelay;
		}
	}

	// store the current amount depressed for this key
	ButtonInfo[ControllerIndex].AmountDepressed[KeyIndex] = AmountDepressed;
}


/**
 * This is defined in the XeLaunch.cpp as indiv games may need to override it for automation
 * purposes
 **/
extern DWORD UnrealXInputGetState( DWORD ControllerIndex, PXINPUT_STATE CurrentInput );

void FXenonViewport::ProcessInput( FLOAT DeltaTime )
{
	const DOUBLE CurrentTime = appSeconds();
	FAsyncInputPacket Packet;
	UBOOL bAreResultsAvailable = FAsyncInputPoller::GetInterpolatedState(Packet);
	if (bAreResultsAvailable)
	{
		for( INT ControllerIndex = 0;ControllerIndex < MAX_NUM_CONTROLLERS; ControllerIndex++ )
		{
			//@todo xenon: handle insertion, removal, merging input from several controllers and all the input mojo from the XDK sample
			//@todo xenon: Add splitscreen support, arbitrary controller choices. None of this code remotely passes cert
			if (Packet.ControllerPresent[ControllerIndex])
			{
				XINPUT_STATE CurrentInput = Packet.State[ControllerIndex];
				INT ButtonIndex = 0;

				{
				// Analog buttons.
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_LeftTrigger		, CurrentInput.Gamepad.bLeftTrigger, CurrentTime );			
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_RightTrigger	, CurrentInput.Gamepad.bRightTrigger, CurrentTime );

				// Digital buttons.
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_LeftThumbstick	, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB		? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_RightThumbstick	, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB	? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_Back			, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_BACK			? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_Start			, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_START			? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_DPad_Up			, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP		? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_DPad_Down		, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN		? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_DPad_Left		, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT		? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_DPad_Right		, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT		? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_A				, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_A				? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_B				, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_B				? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_X				, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_X				? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_Y				, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_Y				? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_LeftShoulder	, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER	? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );
				PassKeyStateToViewportClient( ButtonIndex++, ControllerIndex, KEY_XboxTypeS_RightShoulder	, CurrentInput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER	? DIGITAL_PRESSED : DIGITAL_RELEASED, CurrentTime );

				// Sanity check ensuring that amount of buttons matches up with array size.
				check(ButtonIndex == MAX_NUM_BUTTONS);

				// Axis.
				ViewportClient->InputAxis( this, ControllerIndex, KEY_XboxTypeS_LeftX			,  CurrentInput.Gamepad.sThumbLX / 32768.f, DeltaTime, TRUE );
				ViewportClient->InputAxis( this, ControllerIndex, KEY_XboxTypeS_LeftY			,  CurrentInput.Gamepad.sThumbLY / 32768.f, DeltaTime, TRUE );
				ViewportClient->InputAxis( this, ControllerIndex, KEY_XboxTypeS_RightX			,  CurrentInput.Gamepad.sThumbRX / 32768.f, DeltaTime, TRUE );
				// this needs to be inverted as the XboxTypeS axis are flipped from the "norm"
				ViewportClient->InputAxis( this, ControllerIndex, KEY_XboxTypeS_RightY			, -CurrentInput.Gamepad.sThumbRY / 32768.f, DeltaTime, TRUE );

				// Now update any waveform data
				UForceFeedbackManager* Manager = ViewportClient->GetForceFeedbackManager(ControllerIndex);
				if (Manager != NULL)
				{
					Manager->ApplyForceFeedback(ControllerIndex,DeltaTime);
				}
				}
			}
		}
	}

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	if( Client && ViewportClient->RequiresKeyboardInput() )
	{
		// Rudimentary keyboard support.
		XINPUT_KEYSTROKE KeyStroke;
		while( XInputGetKeystroke( XUSER_INDEX_ANY, XINPUT_FLAG_KEYBOARD, &KeyStroke ) == ERROR_SUCCESS ) //@hack xenon: can't use "any device" flag as joypad input gets treated as keyboard input then
		{
			//@todo. Temporary hack to get around the fact that the XINPUT_FLAG_KEYBOARD is ignored!
			if ((KeyStroke.VirtualKey & 0xff00) == 0x5800)
			{
				continue;
			}

			bCtrlPressed = (KeyStroke.Flags&XINPUT_KEYSTROKE_CTRL) != 0;
			bShiftPressed = (KeyStroke.Flags&XINPUT_KEYSTROKE_SHIFT) != 0;
			bAltPressed = (KeyStroke.Flags&XINPUT_KEYSTROKE_ALT) != 0;

			FName* Key = Client->KeyMapVirtualToName.Find(KeyStroke.VirtualKey);
			if( Key )
			{
				if( KeyStroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN | XINPUT_KEYSTROKE_REPEAT) )
				{
					ViewportClient->InputKey( this,0,*Key,KeyStates[KeyStroke.VirtualKey] ? IE_Repeat : IE_Pressed, 1.f, TRUE );
					KeyStates[KeyStroke.VirtualKey] = 1;

					if( ViewportClient && KeyStroke.Unicode )
					{
						ViewportClient->InputChar( this,0,KeyStroke.Unicode );	
					}
				}
				else
				if( KeyStroke.Flags & XINPUT_KEYSTROKE_KEYUP )
				{
					ViewportClient->InputKey( this,0,*Key,IE_Released, 1.f, TRUE );
					KeyStates[KeyStroke.VirtualKey] = 0;
				}
			}
		}
	}
#endif
}
