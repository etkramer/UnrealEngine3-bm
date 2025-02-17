/*=============================================================================
	XnaForceFeedbackManager.cpp: UXnaForceFeedbackManager code. This is the
	platform specific implementation of the UForceFeedbackManager interface.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if XBOX
#include "XeDrv.h"
#else
#include "WinDrv.h"
#endif

IMPLEMENT_CLASS(UXnaForceFeedbackManager);

/**
 * Applies the current waveform data to the gamepad/mouse/etc
 *
 * @param DeviceID The device that needs updating
 * @param DeltaTime The amount of elapsed time since the last update
 */
void UXnaForceFeedbackManager::ApplyForceFeedback(INT DeviceID,FLOAT DeltaTime)
{
	XINPUT_VIBRATION Feedback;
	// Update the rumble data
	GetRumbleState(DeltaTime,Feedback.wLeftMotorSpeed,
		Feedback.wRightMotorSpeed);
	// Update the motor with the data
	XInputSetState(DeviceID,&Feedback);
}

/**
 * Determines the amount of rumble to apply to the left and right motors of the
 * specified gamepad
 *
 * @param flDelta The amount of time that has elapsed
 * @param LeftAmount The amount to make the left motor spin
 * @param RightAmount The amount to make the right motor spin
 */
void UXnaForceFeedbackManager::GetRumbleState(FLOAT flDelta,WORD& LeftAmount,
	WORD& RightAmount)
{
	LeftAmount = RightAmount = 0;
	// Only update time/sample if there is a current waveform playing
	if (FFWaveform != NULL && bIsPaused == 0)
	{
		// Update the current tick for this
		UpdateWaveformData(flDelta);
		// Only calculate a new waveform amount if the waveform is still valid
		if (FFWaveform != NULL)
		{
			// Evaluate the left function
			LeftAmount = EvaluateFunction(
				FFWaveform->Samples(CurrentSample).LeftFunction,
				FFWaveform->Samples(CurrentSample).LeftAmplitude,flDelta,
				FFWaveform->Samples(CurrentSample).Duration);
			// And now the right
			RightAmount = EvaluateFunction(
				FFWaveform->Samples(CurrentSample).RightFunction,
				FFWaveform->Samples(CurrentSample).RightAmplitude,flDelta,
				FFWaveform->Samples(CurrentSample).Duration);
			// Scale the samples if the user requested it
			if (ScaleAllWaveformsBy != 1.f)
			{
				LeftAmount = (WORD)((FLOAT)LeftAmount * ScaleAllWaveformsBy);
				RightAmount = (WORD)((FLOAT)RightAmount * ScaleAllWaveformsBy);
			}
		}
	}
}

/**
 * Figures out which function to call to evaluate the current rumble amount. It
 * internally handles the constant function type.
 *
 * @param eFunc Which function to apply to the rumble
 * @param byAmplitude The max value to apply for this function
 * @param flDelta The amount of time that has elapsed
 * @param flDuration The max time for this waveform
 *
 * @return The amount to rumble the gamepad by
 */
WORD UXnaForceFeedbackManager::EvaluateFunction(BYTE eFunc,BYTE byAmplitude,
	FLOAT flDelta,FLOAT flDuration)
{
	// Defualt to no rumble in case of unknown type
	WORD wAmount = 0;
	// Determine which evaluator to use
	switch(eFunc)
	{
		case WF_Constant:
		{
			// Constant values represent percentage
			wAmount = byAmplitude * (65535 / 100);
			break;
		}
		case WF_LinearIncreasing:
		{
			// Calculate the value based upon elapsed time and the max in
			// byAmplitude. Goes from zero to byAmplitude.
			wAmount = (byAmplitude * (ElapsedTime / flDuration)) * (65535 / 100);
			break;
		}
		case WF_LinearDecreasing:
		{
			// Calculate the value based upon elapsed time and the max in
			// byAmplitude. Goes from byAmplitude to zero.
			wAmount = (byAmplitude * (1.f - (ElapsedTime / flDuration))) *
				(65535 / 100);
			break;
		}
		case WF_Sin0to90:
		{
			// Uses a sin(0..pi/2) function to go from zero to byAmplitude
			wAmount = (byAmplitude * (appSin(PI/2 * (flDelta / flDuration)))) *
				(65535 / 100);
			break;
		}
		case WF_Sin90to180:
		{
			// Uses a sin(pi/2..pi) function to go from byAmplitude to 0
			wAmount = (byAmplitude * (appSin(PI/2 + PI/2 *
				(flDelta / flDuration)))) * (65535 / 100);
			break;
		}
		case WF_Sin0to180:
		{
			// Uses a sin(0..pi) function to go from 0 to byAmplitude to 0
			wAmount = (byAmplitude * (appSin(PI * (flDelta / flDuration)))) *
				(65535 / 100);
			break;
		}
		case WF_Noise:
		{
			// Calculate a random value between 0 and byAmplitude
			wAmount = (byAmplitude * (65535 / 100)) * appFrand();
			break;
		}
	};
	return wAmount;
}

/** Clear any vibration going on this device right away. */
void UXnaForceFeedbackManager::ForceClearWaveformData(INT DeviceID)
{
	XINPUT_VIBRATION Feedback;
	Feedback.wLeftMotorSpeed = 0;
	Feedback.wRightMotorSpeed = 0;
	// Update the motor with the data
	XInputSetState(DeviceID,&Feedback);
}

